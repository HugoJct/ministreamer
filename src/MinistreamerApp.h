#ifndef MINISTREAMER_APP_H
#define MINISTREAMER_APP_H

#include <gtk/gtk.h>
#include <gst/gstelement.h>

typedef struct {
  void (*quit)(GtkWindow *);
  GstElement *displaysink;
  void (*button1_clicked)(GtkButton *self, gpointer user_data);
  void (*button2_clicked)(GtkButton *self, gpointer user_data);
} ActivateParams;

struct _MinistreamerApp {
  GtkApplication parent;

  ActivateParams on_activate;
};

#define MINISTREAMER_APP_TYPE (ministreamer_app_get_type())
G_DECLARE_FINAL_TYPE(MinistreamerApp, ministreamer_app, MINISTREAMER, APP,
                     GtkApplication)

MinistreamerApp *ministreamer_app_new(ActivateParams params);

#endif
