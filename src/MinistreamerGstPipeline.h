#ifndef GSTREAMER_HANDLING_H
#define GSTREAMER_HANDLING_H

#include "gio/gio.h"
#include "glib.h"
#include <gst/gst.h>

struct _MinistreamerGstPipeline {
  GObject parent;

  GListStore *srcs;

  //main pipeline
  GstElement *compositor;
  GstElement *tee;
  GstElement *queue;
  GstElement *displaysink;
  GstElement *pipeline;

  //pluggable pipeline
  GstElement *recqueue;
  GstElement *convert;
  GstElement *x264enc;
  GstElement *mp4mux;
  GstElement *filesink;

  gboolean canrecord;
};

#define MINISTREAMER_TYPE_GSTPIPELINE (ministreamer_gst_pipeline_get_type())
G_DECLARE_FINAL_TYPE(MinistreamerGstPipeline, ministreamer_gst_pipeline,
                     MINISTREAMER, GST_PIPELINE, GObject)

MinistreamerGstPipeline *ministreamer_gst_pipeline_new();

void ministreamer_gst_pipeline_enable_record(MinistreamerGstPipeline *self);
void ministreamer_gst_pipeline_disable_record(MinistreamerGstPipeline *self);
gboolean ministreamer_gst_pipeline_is_recording(MinistreamerGstPipeline *self);

#endif
