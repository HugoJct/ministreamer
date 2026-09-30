#ifndef SCREEN_SHARE_H
#define SCREEN_SHARE_H

#include "MinistreamerGstPipeline.h"
#include "glib-object.h"
#include <gst/gst.h>
#include <libportal/portal.h>

struct _ScreenShare {
  GObject parent;

  XdpPortal *portal;
  XdpSession *session;
  char *path;
  char name[20];
  GstElement *src;
  GstElement *queue;
  GstPad *compositor_pad;

  guint xpos;
  guint ypos;
  guint zorder;
  guint width;
  guint height;
};

#define SCREEN_TYPE_SHARE (screen_share_get_type())
G_DECLARE_FINAL_TYPE(ScreenShare, screen_share, SCREEN, SHARE, GObject)

ScreenShare *screen_share_new();
void screen_share_add_to_ministreamer_gst_pipeline(ScreenShare *sc_sh,
                                                   gpointer user_data);
void screen_share_remove_from_ministreamer_pipeline(
    ScreenShare *sc_sh, guint index, MinistreamerGstPipeline *pipeline);

#endif
