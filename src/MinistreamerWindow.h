#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>

#include "MinistreamerApp.h"

struct _MinistreamerWindow {
  GtkApplicationWindow parent;

  GtkWidget *video;
  GtkWidget *column_view;
  GtkWidget *record_button;
  GtkWidget *remove_screenshare_button;
  GtkWidget *add_screenshare_button;
  GtkWidget *quit_button;
  GtkWidget *settings;
};

#define MINISTREAMER_WINDOW_TYPE (ministreamer_window_get_type())
G_DECLARE_FINAL_TYPE(MinistreamerWindow, ministreamer_window, MINISTREAMER,
                     WINDOW, GtkApplicationWindow);

MinistreamerWindow *ministreamer_window_new(MinistreamerApp *app);

#endif
