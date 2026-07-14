#include "ScreenShare.h"
#include <gst/gst.h>
#include <libportal/portal.h>
#include <stdio.h>

static void created(ScreenShare *sc_sh) {
  GstElement *pipewiresrc =
      gst_element_factory_make("pipewiresrc", "pipewiresrc");
  g_object_set(pipewiresrc, "path", 00, NULL);

  GstElement *videoconvert =
      gst_element_factory_make("videoconvert", "videoconvert");
  GstElement *xvimagesink =
      gst_element_factory_make("xvimagesink", "xvimagesink");

  GstElement *pipeline = gst_pipeline_new("mypipeline");

  gst_bin_add_many(GST_BIN(pipeline), pipewiresrc, videoconvert, xvimagesink,
                   NULL);

  gst_element_link_many(pipewiresrc, videoconvert, xvimagesink, NULL);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

int main(int argc, char **argv) {

  gst_init(&argc, &argv);

  ScreenShare *sc_sh = screen_share_new(created);

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  return 0;
}
