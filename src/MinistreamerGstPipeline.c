#include "MinistreamerGstPipeline.h"
#include "ScreenShare.h"
#include "glib-object.h"
#include "gst/gstbin.h"
#include "gst/gstdebugutils.h"
#include "gst/gstelement.h"
#include "gst/gstelementfactory.h"
#include "gst/gstutils.h"

#include <stdio.h>

G_DEFINE_TYPE(MinistreamerGstPipeline, ministreamer_gst_pipeline, G_TYPE_OBJECT)

typedef struct {
  MinistreamerGstPipeline *pipeline;
  GstPad *requestpad;
} DisableRecord;

static void
ministreamer_gst_pipeline_class_init(MinistreamerGstPipelineClass *klass) {}

static void ministreamer_gst_pipeline_init(MinistreamerGstPipeline *self) {

  self->srcs = g_list_store_new(SCREEN_TYPE_SHARE);

  self->pipeline = gst_pipeline_new("mypipeline");
  self->compositor = gst_element_factory_make("compositor", "compositor");
  self->tee = gst_element_factory_make("tee", "tee");
  self->queue = gst_element_factory_make("queue", "queue");
  self->displaysink =
      gst_element_factory_make("gtk4paintablesink", "displaysink");
  g_object_set(self->displaysink, "sync", FALSE, NULL);

  self->recqueue = gst_element_factory_make("queue", "record queue");
  g_object_set(self->recqueue, "leaky", 2, NULL);
  self->convert = gst_element_factory_make("videoconvert", "videoconvert");
  self->x264enc = gst_element_factory_make("x264enc", "h264 encoder");
  self->mp4mux = gst_element_factory_make("mp4mux", "mp4 multiplexer");
  self->filesink = gst_element_factory_make("filesink", "fileoutput");
  g_object_set(self->filesink, "location", "/tmp/record.mp4", NULL);

  if (self->pipeline == NULL || self->compositor == NULL || self->tee == NULL ||
      self->queue == NULL || self->displaysink == NULL) {
    g_printerr("could not create pipeline\n");
    // return -1;
  }

  gst_bin_add_many(GST_BIN(self->pipeline), self->compositor, self->tee,
                   self->queue, self->displaysink, NULL);
  gst_element_link_many(self->compositor, self->tee, self->queue,
                        self->displaysink, NULL);

  self->canrecord = TRUE;
  if (self->recqueue == NULL || self->convert == NULL ||
      self->x264enc == NULL || self->mp4mux == NULL || self->filesink == NULL) {
    self->canrecord = FALSE;
  }

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(self->pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
                            "initpipeline");
}

void ministreamer_gst_pipeline_enable_record(MinistreamerGstPipeline *self) {
  if (ministreamer_gst_pipeline_is_recording(self) ||
      self->canrecord == FALSE) {
    return;
  }

  gst_bin_add_many(GST_BIN(self->pipeline), self->convert, self->recqueue,
                   self->x264enc, self->mp4mux, self->filesink, NULL);
  gst_element_link_many(self->recqueue, self->convert, self->x264enc,
                        self->mp4mux, self->filesink, NULL);

  GstPad *newteepad = gst_element_request_pad_simple(self->tee, "src_%u");
  GstPad *queuepad = gst_element_get_static_pad(self->recqueue, "sink");

  gst_pad_link(newteepad, queuepad);
  gst_object_unref(queuepad);
  gst_object_unref(newteepad);

  gst_element_sync_state_with_parent(self->recqueue);
  gst_element_sync_state_with_parent(self->convert);
  gst_element_sync_state_with_parent(self->x264enc);
  gst_element_sync_state_with_parent(self->mp4mux);
  gst_element_sync_state_with_parent(self->filesink);

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(self->pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
                            "record_enabled");
}

static gboolean
ministreamer_gst_pipeline_disable_record_idle(gpointer user_data) {

  DisableRecord *record = (DisableRecord *)user_data;

  MinistreamerGstPipeline *pipeline = record->pipeline;

  gst_element_set_state(pipeline->recqueue, GST_STATE_NULL);
  gst_element_set_state(pipeline->convert, GST_STATE_NULL);
  gst_element_set_state(pipeline->x264enc, GST_STATE_NULL);
  gst_element_set_state(pipeline->mp4mux, GST_STATE_NULL);
  gst_element_set_state(pipeline->filesink, GST_STATE_NULL);

  gst_bin_remove_many(GST_BIN(pipeline->pipeline), pipeline->convert,
                      pipeline->recqueue, pipeline->x264enc, pipeline->mp4mux,
                      pipeline->filesink, NULL);

  gst_element_release_request_pad(pipeline->tee, record->requestpad);
  gst_object_unref(record->requestpad);

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline->pipeline),
                            GST_DEBUG_GRAPH_SHOW_ALL,
                            "record_disabled_cleaned");

  g_free(record);

  return G_SOURCE_REMOVE;
}

static GstPadProbeReturn ministreamer_gst_pipeline_disable_record_probe(
    GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
  MinistreamerGstPipeline *pipeline = MINISTREAMER_GST_PIPELINE(user_data);

  GstPad *queuepad = gst_element_get_static_pad(pipeline->recqueue, "sink");

  gst_pad_unlink(pad, queuepad);

  gst_pad_send_event(queuepad, gst_event_new_eos());

  gst_object_unref(queuepad);

  DisableRecord *disrec = g_new0(DisableRecord, 1);
  disrec->pipeline = pipeline;
  disrec->requestpad = pad;

  g_idle_add(ministreamer_gst_pipeline_disable_record_idle, disrec);

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline->pipeline),
                            GST_DEBUG_GRAPH_SHOW_ALL, "record_disabled");

  return GST_PAD_PROBE_REMOVE;
}

void ministreamer_gst_pipeline_disable_record(MinistreamerGstPipeline *self) {
  if (ministreamer_gst_pipeline_is_recording(self) == FALSE) {
    return;
  }

  GstPad *encpad = gst_element_get_static_pad(self->recqueue, "sink");
  GstPad *teepad = gst_pad_get_peer(encpad);
  gst_pad_add_probe(teepad,
                    GST_PAD_PROBE_TYPE_PUSH | GST_PAD_PROBE_TYPE_BLOCK |
                        GST_PAD_PROBE_TYPE_BUFFER,
                    ministreamer_gst_pipeline_disable_record_probe, self, NULL);

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(self->pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
                            "record_disabled_before");
}

gboolean ministreamer_gst_pipeline_is_recording(MinistreamerGstPipeline *self) {
  GstPad *queuepad = gst_element_get_static_pad(self->recqueue, "sink");
  gboolean ret = gst_pad_is_linked(queuepad);
  gst_object_unref(queuepad);
  return ret;
}

MinistreamerGstPipeline *ministreamer_gst_pipeline_new() {
  return g_object_new(MINISTREAMER_TYPE_GSTPIPELINE, NULL);
}
