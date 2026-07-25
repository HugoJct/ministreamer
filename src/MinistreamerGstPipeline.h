#ifndef GSTREAMER_HANDLING_H
#define GSTREAMER_HANDLING_H

#include <gst/gst.h>

struct _MinistreamerGstPipeline {
  GObject parent;

  GArray *srcs;

  GstElement *compositor;
  GstElement *displaysink;
  GstElement *pipeline;
};

#define MINISTREAMER_TYPE_GSTPIPELINE (ministreamer_gst_pipeline_get_type())
G_DECLARE_FINAL_TYPE(MinistreamerGstPipeline, ministreamer_gst_pipeline,
                     MINISTREAMER, GST_PIPELINE, GObject)

MinistreamerGstPipeline *ministreamer_gst_pipeline_new();

#endif
