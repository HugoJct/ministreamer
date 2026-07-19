#ifndef UI_H
#define UI_H

#include <gst/gstelement.h>
#include <gtk/gtk.h>

typedef struct {
  void (*quit)(GtkWindow *);
  GstElement *displaysink;
  void (*button1_clicked)(GtkButton *self, gpointer user_data);
  void (*button2_clicked)(GtkButton *self, gpointer user_data);
} ActivateParams;

void window_activate(GtkApplication *app, gpointer user_data);

#endif
