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

  // g_print("session start\n");
  ret = xdp_session_get_streams(session);

  guint32 node_id;
  // g_print("%s\n", g_variant_print(ret, FALSE));

  for (gsize i = 0; i < g_variant_n_children(ret); i++) {
    g_autoptr(GVariant) elem = g_variant_get_child_value(ret, i);

    g_variant_get(elem, "(ua{sv})", &node_id, NULL);
    // g_print("%d\n", node_id);
  }

  char buf[500];
  sprintf(buf, "pipewiresrc path=%u ! videoconvert ! xvimagesink", node_id);

  xdp_session_open_pipewire_remote(session);

  GstElement *pipeline = gst_parse_launch(buf, NULL);
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
