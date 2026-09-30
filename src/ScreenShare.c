#include "ScreenShare.h"
#include "MinistreamerGstPipeline.h"
#include "glib-object.h"
#include "glib.h"
#include "gmodule.h"
#include "gst/gstelement.h"
#include "gst/gstpad.h"
#include "gtk/gtk.h"

#include <limits.h>
#include <stdio.h>

G_DEFINE_TYPE(ScreenShare, screen_share, G_TYPE_OBJECT)

enum { SIGNAL_READY, SIGNAL_LAST };

enum {
  PROP_NAME = 1,
  PROP_XPOS,
  PROP_YPOS,
  PROP_ZORDER,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_LAST
};

static guint screen_share_signals[SIGNAL_LAST];

typedef struct {
  ScreenShare *sc_sh;
  MinistreamerGstPipeline *pipeline;
} RemoveData;

static void screen_share_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *prspec) {
  ScreenShare *self = SCREEN_SHARE(object);
  switch (property_id) {
  case PROP_NAME:
    g_value_set_string(value, self->name);
    break;
  case PROP_XPOS:
    g_value_set_int(value, self->xpos);
    break;
  case PROP_YPOS:
    g_value_set_int(value, self->ypos);
    break;
  case PROP_ZORDER:
    g_value_set_uint(value, self->zorder);
    break;
  case PROP_WIDTH:
    g_value_set_int(value, self->width);
    break;
  case PROP_HEIGHT:
    g_value_set_int(value, self->height);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, prspec);
    break;
  }
}

static void screen_share_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *prspec) {

  ScreenShare *self = SCREEN_SHARE(object);

  switch (property_id) {
  case PROP_XPOS:
    self->xpos = g_value_get_int(value);
    g_object_set(G_OBJECT(self->compositor_pad), "xpos", self->xpos, NULL);
    break;
  case PROP_YPOS:
    self->ypos = g_value_get_int(value);
    g_object_set(G_OBJECT(self->compositor_pad), "ypos", self->ypos, NULL);
    break;
  case PROP_ZORDER:
    self->zorder = g_value_get_uint(value);
    g_object_set(G_OBJECT(self->compositor_pad), "zorder", self->zorder, NULL);
    break;
  case PROP_WIDTH:
    self->width = g_value_get_int(value);
    break;
  case PROP_HEIGHT:
    self->height = g_value_get_int(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, prspec);
    break;
  }
}

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

  screen_share->path = g_strdup_printf("%u", node_id);

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

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->get_property = screen_share_get_property;
  object_class->set_property = screen_share_set_property;

  g_object_class_install_property(
      object_class, PROP_NAME,
      g_param_spec_string("name", NULL, NULL, NULL,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_XPOS,
      g_param_spec_int("xpos", NULL, NULL, INT_MIN, INT_MAX, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_YPOS,
      g_param_spec_int("ypos", NULL, NULL, INT_MIN, INT_MAX, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_ZORDER,
      g_param_spec_uint("zorder", NULL, NULL, 0, UINT_MAX, 0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_WIDTH,
      g_param_spec_int("width", NULL, NULL, 0, INT_MAX, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
      object_class, PROP_HEIGHT,
      g_param_spec_int("height", NULL, NULL, 0, INT_MAX, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void screen_share_init(ScreenShare *self) {

  self->portal = xdp_portal_new();

  xdp_portal_create_screencast_session(
      self->portal, XDP_OUTPUT_MONITOR | XDP_OUTPUT_VIRTUAL | XDP_OUTPUT_WINDOW,
      XDP_SCREENCAST_FLAG_NONE, XDP_CURSOR_MODE_EMBEDDED,
      XDP_PERSIST_MODE_TRANSIENT, NULL, NULL, session_created, self);

  self->xpos = 0;
  self->ypos = 0;
  self->zorder = 0;
  self->width = 0;
  self->height = 0;
}

ScreenShare *screen_share_new() {
  return g_object_new(SCREEN_TYPE_SHARE, NULL);
}

void screen_share_add_to_ministreamer_gst_pipeline(ScreenShare *sc_sh,
                                                   gpointer user_data) {
  static int screen_share_id = 0;

  MinistreamerGstPipeline *target = MINISTREAMER_GST_PIPELINE(user_data);

  snprintf(sc_sh->name, 20, "pipewiresrc_%d", screen_share_id);

  char buf_queue[20];
  snprintf(buf_queue, 20, "queue_%d", screen_share_id);

  screen_share_id++;

  sc_sh->src = gst_element_factory_make("pipewiresrc", sc_sh->name);
  g_object_set(sc_sh->src, "path", sc_sh->path, NULL);

  sc_sh->queue = gst_element_factory_make("queue", buf_queue);
  g_object_set(sc_sh->queue, "leaky", 2, NULL);

  gst_bin_add_many(GST_BIN(target->pipeline), sc_sh->src, sc_sh->queue, NULL);

  GstPad *sink0 = gst_element_request_pad_simple(target->compositor, "sink_%u");
  GstPad *newsrcpad = gst_element_get_static_pad(sc_sh->queue, "src");

  gst_element_link(sc_sh->src, sc_sh->queue);

  gst_pad_link(newsrcpad, sink0);
  sc_sh->compositor_pad = sink0;

  gst_object_unref(newsrcpad);

  gst_element_sync_state_with_parent(sc_sh->src);
  gst_element_sync_state_with_parent(sc_sh->queue);
  // gst_element_set_state(sc_sh->src, GST_STATE_PAUSED);
  // gst_element_set_state(sc_sh->queue, GST_STATE_PAUSED);
  // gst_bin_sync_children_states(GST_BIN(target->pipeline));

  gst_element_set_state(target->pipeline, GST_STATE_PLAYING);
  g_list_store_append(target->srcs, sc_sh);
}

static gboolean deferred_remove_source(gpointer user_data) {
  RemoveData *data = (RemoveData *)user_data;
  ScreenShare *sc_sh = data->sc_sh;
  MinistreamerGstPipeline *pipeline = data->pipeline;
  GstPad *compositorpad = data->sc_sh->compositor_pad;

  gst_element_set_state(sc_sh->src, GST_STATE_NULL);
  gst_element_set_state(sc_sh->queue, GST_STATE_NULL);

  gst_bin_remove_many(GST_BIN(pipeline->pipeline), sc_sh->src, sc_sh->queue,
                      NULL);

  gst_element_release_request_pad(pipeline->compositor, compositorpad);

  gst_object_unref(compositorpad);
  g_object_unref(sc_sh);
  g_free(data);

  return G_SOURCE_REMOVE;
}

static GstPadProbeReturn screen_share_block_source(GstPad *pad,
                                                   GstPadProbeInfo *info,
                                                   gpointer user_data) {
  RemoveData *data = (RemoveData *)user_data;

  GstPad *queuesrcpad = gst_element_get_static_pad(data->sc_sh->queue, "src");

  GstElement *compositor =
      GST_ELEMENT(gst_pad_get_parent(data->sc_sh->compositor_pad));

  gst_element_unlink_many(data->sc_sh->src, data->sc_sh->queue, compositor,
                          NULL);

  gst_pad_send_event(data->sc_sh->compositor_pad, gst_event_new_eos());

  gst_object_unref(compositor);
  gst_object_unref(queuesrcpad);

  g_idle_add(deferred_remove_source, data);

  return GST_PAD_PROBE_REMOVE;
}

void screen_share_remove_from_ministreamer_pipeline(
    ScreenShare *sc_sh, guint index, MinistreamerGstPipeline *pipeline) {

  GstPad *src = gst_element_get_static_pad(sc_sh->src, "src");

  RemoveData *data = g_new0(RemoveData, 1);
  data->sc_sh = g_object_ref(sc_sh);
  data->pipeline = pipeline;

  gst_pad_add_probe(src,
                    GST_PAD_PROBE_TYPE_PUSH | GST_PAD_PROBE_TYPE_BLOCK |
                        GST_PAD_PROBE_TYPE_BUFFER,
                    screen_share_block_source, data, NULL);

  g_list_store_remove(pipeline->srcs, index);

  gst_object_unref(src);
}

void screen_share_xpos_changed(GtkSpinButton *spin, GtkListItem *list_item) {
  ScreenShare *share = SCREEN_SHARE(gtk_list_item_get_item(list_item));

  if (share == NULL) {
    return;
  }

  gint new_val = (gint)gtk_spin_button_get_value_as_int(spin);

  if (share->xpos != new_val) {
    g_object_set(share, "xpos", new_val, NULL);
  }
}

void screen_share_ypos_changed(GtkSpinButton *spin, GtkListItem *list_item) {
  ScreenShare *share = SCREEN_SHARE(gtk_list_item_get_item(list_item));

  if (share == NULL) {
    return;
  }

  gint new_val = (gint)gtk_spin_button_get_value_as_int(spin);

  if (share->xpos != new_val) {
    g_object_set(share, "ypos", new_val, NULL);
  }
}

void screen_share_zorder_changed(GtkSpinButton *spin, GtkListItem *list_item) {
  ScreenShare *share = SCREEN_SHARE(gtk_list_item_get_item(list_item));

  if (share == NULL) {
    return;
  }

  guint new_val = (guint)gtk_spin_button_get_value_as_int(spin);

  if (share->xpos != new_val) {
    g_object_set(share, "zorder", new_val, NULL);
  }
}
