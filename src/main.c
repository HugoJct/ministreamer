#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gtk/gtk.h>
#include <libportal/portal.h>
#include <stdio.h>

#include "ScreenShare.h"

ScreenShare *sc1 = NULL;
ScreenShare *sc2 = NULL;
GstElement *compositor = NULL;
GstElement *displaysink = NULL;
GstElement *pipeline = NULL;

static void created(ScreenShare *sc_sh) {
  static int id = 0;

  char buf[20];
  snprintf(buf, 20, "pipewiresrc_%d", id++);

  sc_sh->src = gst_element_factory_make("pipewiresrc", buf);
  g_object_set(sc_sh->src, "path", sc_sh->path, NULL);

  if (sc1->src != NULL && sc2->src != NULL) {
    gst_bin_add_many(GST_BIN(pipeline), sc1->src, sc2->src, NULL);

    GstPad *sink0 = gst_element_request_pad_simple(compositor, "sink_%u");

    GstPad *sink1 = gst_element_request_pad_simple(compositor, "sink_%u");
    g_object_set(sink1, "xpos", 100, "ypos", 100, NULL);

    GstPad *sc1src = gst_element_get_static_pad(sc1->src, "src");
    gst_pad_link(sc1src, sink0);

    GstPad *sc2src = gst_element_get_static_pad(sc2->src, "src");
    gst_pad_link(sc2src, sink1);

    gst_object_unref(sink0);
    gst_object_unref(sink1);
    gst_object_unref(sc1src);
    gst_object_unref(sc2src);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
  }
}

static void quit(GtkWindow *window) {
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gtk_window_close(window);
}

static void activate(GtkApplication *app, gpointer user_data) {

  GtkBuilder *builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "src/ui/builder.ui", NULL);

  GObject *window = gtk_builder_get_object(builder, "window");
  gtk_window_set_application(GTK_WINDOW(window), app);

  GObject *button = gtk_builder_get_object(builder, "quit");
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(quit), window);

  GObject *pic = gtk_builder_get_object(builder, "video");

  g_autoptr(GdkPaintable) paintable = NULL;
  g_object_get(displaysink, "paintable", &paintable, NULL);
  gtk_picture_set_paintable(GTK_PICTURE(pic), paintable);
  g_object_unref(paintable);

  gtk_widget_set_visible(GTK_WIDGET(window), TRUE);

  g_object_unref(builder);
}

int main(int argc, char **argv) {

  gst_init(&argc, &argv);
  gtk_init();

  pipeline = gst_pipeline_new("mypipeline");
  compositor = gst_element_factory_make("compositor", "compositor");
  displaysink = gst_element_factory_make("gtk4paintablesink", "displaysink");

  gst_bin_add_many(GST_BIN(pipeline), compositor, displaysink, NULL);
  gst_element_link(compositor, displaysink);

  sc1 = screen_share_new(created);
  sc2 = screen_share_new(created);

  GtkApplication *app;
  int status;
  (void)status;

  app = gtk_application_new("fr.jacotot.ministreamer",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return 0;
}
