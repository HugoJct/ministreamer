#include "ui.h"

void window_activate(GtkApplication *app, gpointer user_data) {

  ActivateParams *params = (ActivateParams *)user_data;

  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "res/ui/builder.ui", NULL);

  GObject *window = gtk_builder_get_object(builder, "window");
  gtk_window_set_application(GTK_WINDOW(window), app);

  GObject *quit_button = gtk_builder_get_object(builder, "quit");
  g_signal_connect_swapped(quit_button, "clicked", G_CALLBACK(params->quit), window);

  GObject *button1 = gtk_builder_get_object(builder, "button1");
  g_signal_connect_swapped(button1, "clicked", G_CALLBACK(params->button1_clicked), window);

  GObject *button2 = gtk_builder_get_object(builder, "button2");
  g_signal_connect_swapped(button2, "clicked", G_CALLBACK(params->button2_clicked), window);

  GObject *pic = gtk_builder_get_object(builder, "video");

  g_autoptr(GdkPaintable) paintable = NULL;
  g_object_get(params->displaysink, "paintable", &paintable, NULL);
  gtk_picture_set_paintable(GTK_PICTURE(pic), paintable);
  g_object_unref(paintable);

  gtk_widget_set_visible(GTK_WIDGET(window), TRUE);

  g_object_unref(builder);
}
