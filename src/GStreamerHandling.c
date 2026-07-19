#include "GStreamerHandling.h"

#include <stdio.h>

GstElement *compositor;
GstElement *displaysink;
GstElement *pipeline;

int gstreamer_handling_init_sink() {

  pipeline = gst_pipeline_new("mypipeline");
  compositor = gst_element_factory_make("compositor", "compositor");
  displaysink = gst_element_factory_make("gtk4paintablesink", "displaysink");

  if (pipeline == NULL || compositor == NULL || displaysink == NULL) {
    return -1;
  }

  gst_bin_add_many(GST_BIN(pipeline), compositor, displaysink, NULL);
  gst_element_link(compositor, displaysink);

  return 0;
}

void gstreamer_handling_add_screen_share(ScreenShare *sc_sh) {
  static int screen_share_id = 0;

  char buf[20];
  snprintf(buf, 20, "pipewiresrc_%d", screen_share_id++);

  sc_sh->src = gst_element_factory_make("pipewiresrc", buf);
  g_object_set(sc_sh->src, "path", sc_sh->path, NULL);

  gst_bin_add_many(GST_BIN(pipeline), sc_sh->src, NULL);

  GstPad *sink0 = gst_element_request_pad_simple(compositor, "sink_%u");

  GstPad *sc1src = gst_element_get_static_pad(sc_sh->src, "src");
  gst_pad_link(sc1src, sink0);

  gst_object_unref(sink0);
  gst_object_unref(sc1src);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}
