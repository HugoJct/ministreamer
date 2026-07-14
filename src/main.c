#include "ScreenShare.h"
#include <gst/gst.h>
#include <libportal/portal.h>
#include <stdio.h>

ScreenShare *sc1 = NULL;
ScreenShare *sc2 = NULL;
GstElement *compositor = NULL;
GstElement *xvimagesink = NULL;
GstElement *pipeline = NULL;

static void created(ScreenShare *sc_sh) {
  static int id = 0;

  char buf[20];
  snprintf(buf, 20, "pipewiresrc_%d", id++);

  sc_sh->src = gst_element_factory_make("pipewiresrc", buf);
  g_object_set(sc_sh->src, "path", sc_sh->path, NULL);

  if (sc1->src != NULL && sc2->src != NULL) {
    gst_bin_add_many(GST_BIN(pipeline), sc1->src, sc2->src, NULL);

    GstPad *sink0 = gst_element_request_pad_simple(compositor, "sink_%u");

    GstPad *sink1 = gst_element_request_pad_simple(compositor, "sink_%u");
    g_object_set(sink1, "xpos", 2560, "ypos", 0, NULL);

    GstPad *sc1src = gst_element_get_static_pad(sc1->src, "src");
    gst_pad_link(sc1src, sink0);

    GstPad *sc2src = gst_element_get_static_pad(sc2->src, "src");
    gst_pad_link(sc2src, sink1);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
  }
}

int main(int argc, char **argv) {

  gst_init(&argc, &argv);

  pipeline = gst_pipeline_new("mypipeline");
  compositor = gst_element_factory_make("compositor", "compositor");
  xvimagesink = gst_element_factory_make("xvimagesink", "xvimagesink");

  gst_bin_add_many(GST_BIN(pipeline), compositor, xvimagesink, NULL);
  gst_element_link(compositor, xvimagesink);

  sc1 = screen_share_new(created);
  sc2 = screen_share_new(created);

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  return 0;
}
