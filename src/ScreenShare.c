#include "ScreenShare.h"

#include <stdio.h>

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
  screen_share->ready(screen_share);
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

ScreenShare *screen_share_new(void (*func)(ScreenShare *)) {

  ScreenShare *new_screen_share = calloc(1, sizeof(ScreenShare));
  if (new_screen_share == NULL)
    return NULL;
  new_screen_share->ready = func;

  new_screen_share->portal = xdp_portal_new();

  xdp_portal_create_screencast_session(
      new_screen_share->portal,
      XDP_OUTPUT_MONITOR | XDP_OUTPUT_VIRTUAL | XDP_OUTPUT_WINDOW,
      XDP_SCREENCAST_FLAG_NONE, XDP_CURSOR_MODE_EMBEDDED,
      XDP_PERSIST_MODE_TRANSIENT, NULL, NULL, session_created,
      new_screen_share);

  return new_screen_share;
}
