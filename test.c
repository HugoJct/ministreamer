#include <gst/gst.h>
#include <libportal/portal.h>
#include <stdio.h>

XdpPortal *portal = NULL;
XdpSession *session = NULL;

static void session_started(GObject *obj, GAsyncResult *res, gpointer data) {
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) ret = NULL;

  xdp_session_start_finish(session, res, &error);

  if (session == NULL)
    return;

  char *pipelinestr =
      "compositor name=comp sink_1::width=420 sink_1::height=240 "
      "sink_1::xpos=100 sink_1::ypos=100 ! "
      "xvimagesink "
      "pipewiresrc ! tee name=sp ! comp.sink_0 "
      "videotestsrc ! video/x-raw,width=420,height=240 ! comp.sink_1";

  // xdp_session_open_pipewire_remote(session);

  GstElement *pipeline = gst_parse_launch(pipelinestr, NULL);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

static void session_created(GObject *obj, GAsyncResult *res, gpointer data) {
  g_autoptr(GError) error = NULL;

  session = xdp_portal_create_screencast_session_finish(portal, res, &error);

  if (session == NULL)
    return;

  // g_print("session created\n");
  xdp_session_start(session, NULL, NULL, session_started, NULL);
}

int main(int argc, char **argv) {

  gst_init(&argc, &argv);

  portal = xdp_portal_new();

  xdp_portal_create_screencast_session(
      portal, XDP_OUTPUT_MONITOR | XDP_OUTPUT_VIRTUAL | XDP_OUTPUT_WINDOW,
      XDP_SCREENCAST_FLAG_NONE, XDP_CURSOR_MODE_EMBEDDED,
      XDP_PERSIST_MODE_TRANSIENT, NULL, NULL, session_created, NULL);

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  return 0;
}
