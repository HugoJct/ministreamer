#include "ScreenShare.h"
#include "MinistreamerGstPipeline.h"
#include "glib-object.h"

#include <stdio.h>

G_DEFINE_TYPE(ScreenShare, screen_share, G_TYPE_OBJECT)

enum { SIGNAL_READY, SIGNAL_LAST };

static guint screen_share_signals[SIGNAL_LAST];

static void session_started(GObject *obj, GAsyncResult *res, gpointer data) {
  ScreenShare *screen_share = (ScreenShare *)data;
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) ret = NULL;

  xdp_session_start_finish(screen_share->session, res, &error);

  if (screen_share->session == NULL)
    return;

  ret = xdp_session_get_streams(screen_share->session);

  guint32 node_id;

  for (gsize i = 0; i < g_variant_n_children(ret); i++) {
    g_autoptr(GVariant) elem = g_variant_get_child_value(ret, i);

    g_variant_get(elem, "(ua{sv})", &node_id, NULL);
  }

  char buf[5];
  int len = snprintf(buf, 5, "%u", node_id);
  screen_share->path = malloc(sizeof(char) * len);
  strncpy(screen_share->path, buf, len);

  xdp_session_open_pipewire_remote(screen_share->session);

  g_signal_emit(screen_share, screen_share_signals[SIGNAL_READY], 0);
}

static void session_created(GObject *obj, GAsyncResult *res, gpointer data) {
  ScreenShare *screen_share = (ScreenShare *)data;
  g_autoptr(GError) error = NULL;

  screen_share->session = xdp_portal_create_screencast_session_finish(
      screen_share->portal, res, &error);

  if (screen_share->session == NULL)
    return;

  xdp_session_start(screen_share->session, NULL, NULL, session_started,
                    screen_share);
}

static void screen_share_class_init(ScreenShareClass *klass) {
  screen_share_signals[SIGNAL_READY] =
      g_signal_new("ready", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0,
                   NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void screen_share_init(ScreenShare *self) {

  self->portal = xdp_portal_new();

  xdp_portal_create_screencast_session(
      self->portal, XDP_OUTPUT_MONITOR | XDP_OUTPUT_VIRTUAL | XDP_OUTPUT_WINDOW,
      XDP_SCREENCAST_FLAG_NONE, XDP_CURSOR_MODE_EMBEDDED,
      XDP_PERSIST_MODE_TRANSIENT, NULL, NULL, session_created, self);
}

ScreenShare *screen_share_new() {
  return g_object_new(SCREEN_TYPE_SHARE, NULL);
}

void screen_share_add_to_ministreamer_gst_pipeline(ScreenShare *sc_sh,
                                                   gpointer user_data) {
  static int screen_share_id = 0;

  MinistreamerGstPipeline *target = MINISTREAMER_GST_PIPELINE(user_data);

  char buf[20];
  snprintf(buf, 20, "pipewiresrc_%d", screen_share_id++);

  sc_sh->src = gst_element_factory_make("pipewiresrc", buf);
  g_object_set(sc_sh->src, "path", sc_sh->path, NULL);

  gst_bin_add_many(GST_BIN(target->pipeline), sc_sh->src, NULL);

  GstPad *sink0 = gst_element_request_pad_simple(target->compositor, "sink_%u");

  GstPad *sc1src = gst_element_get_static_pad(sc_sh->src, "src");
  gst_pad_link(sc1src, sink0);

  gst_object_unref(sink0);
  gst_object_unref(sc1src);

  g_array_append_val(target->srcs, sc_sh);

  gst_element_set_state(target->pipeline, GST_STATE_PLAYING);
}
