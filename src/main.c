#include <gst/gst.h>
#include <gtk/gtk.h>
#include <libportal/portal.h>

#include "GStreamerHandling.h"
#include "ScreenShare.h"
#include "ui.h"

ScreenShare *sc1 = NULL;
ScreenShare *sc2 = NULL;

static void quit(GtkWindow *window) {
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gtk_window_close(window);
}

static void button1_click(GtkButton *self, gpointer user_data) {
  if (sc1 != NULL)
    return;

  sc1 = screen_share_new(gstreamer_handling_add_screen_share);
}

static void button2_click(GtkButton *self, gpointer user_data) {
  if (sc2 != NULL)
    return;

  sc2 = screen_share_new(gstreamer_handling_add_screen_share);
}

int main(int argc, char **argv) {

  gst_init(&argc, &argv);
  gtk_init();

  int ret = gstreamer_handling_init_sink();
  if (ret < 0) {
    goto error;
  }

  GtkApplication *app;
  int status;
  (void)status;

  app = gtk_application_new("fr.jacotot.ministreamer",
                            G_APPLICATION_DEFAULT_FLAGS);

  ActivateParams params = {quit, displaysink, button1_click, button2_click};
  g_signal_connect(app, "activate", G_CALLBACK(window_activate), &params);

  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return 0;

error:
  return EXIT_FAILURE;
}
