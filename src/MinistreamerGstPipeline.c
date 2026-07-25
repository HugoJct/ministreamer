#include "MinistreamerGstPipeline.h"
#include "ScreenShare.h"

#include <stdio.h>

G_DEFINE_TYPE(MinistreamerGstPipeline, ministreamer_gst_pipeline, G_TYPE_OBJECT)

static void
ministreamer_gst_pipeline_class_init(MinistreamerGstPipelineClass *klass) {}

static void ministreamer_gst_pipeline_init(MinistreamerGstPipeline *self) {

  self->srcs = g_list_store_new(SCREEN_TYPE_SHARE);

  self->pipeline = gst_pipeline_new("mypipeline");
  self->compositor = gst_element_factory_make("compositor", "compositor");
  self->displaysink =
      gst_element_factory_make("gtk4paintablesink", "displaysink");

  if (self->pipeline == NULL || self->compositor == NULL ||
      self->displaysink == NULL) {
    g_printerr("could not create pipeline\n");
    // return -1;
  }

  gst_bin_add_many(GST_BIN(self->pipeline), self->compositor, self->displaysink,
                   NULL);
  gst_element_link(self->compositor, self->displaysink);
}


MinistreamerGstPipeline *ministreamer_gst_pipeline_new() {
  return g_object_new(MINISTREAMER_TYPE_GSTPIPELINE, NULL);
}
