#include "GStreamerHandling.h"

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
