#include "MinistreamerWindow.h"
#include "gtk/gtk.h"

G_DEFINE_TYPE(MinistreamerWindow, ministreamer_window,
              GTK_TYPE_APPLICATION_WINDOW);

static void ministreamer_window_dispose(GObject *self) {
  gtk_widget_dispose_template(GTK_WIDGET(self), MINISTREAMER_WINDOW_TYPE);

  G_OBJECT_CLASS(ministreamer_window_parent_class)->dispose(self);
}

static void ministreamer_window_class_init(MinistreamerWindowClass *klass) {

  G_OBJECT_CLASS(klass)->dispose = ministreamer_window_dispose;

  gtk_widget_class_set_template_from_resource(
      GTK_WIDGET_CLASS(klass), "/org/gtk/ministreamer/window.ui");

  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       MinistreamerWindow, start_screenshare_1_button);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       MinistreamerWindow, start_screenshare_2_button);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       MinistreamerWindow, settings);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       MinistreamerWindow, quit_button);
  gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass),
                                       MinistreamerWindow, video);
}

static void ministreamer_window_init(MinistreamerWindow *self) {

  GtkBuilder *builder =
      gtk_builder_new_from_resource("/org/gtk/ministreamer/settings.ui");

  gtk_widget_init_template(GTK_WIDGET(self));

  GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
  gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(self->settings), menu);
  g_object_unref(builder);
}

MinistreamerWindow *ministreamer_window_new(MinistreamerApp *app) {
  return g_object_new(MINISTREAMER_WINDOW_TYPE, "application", app, NULL);
}

