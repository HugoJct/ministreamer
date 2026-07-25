#include <gst/gst.h>
#include <gtk/gtk.h>
#include <libportal/portal.h>

#include "MinistreamerApp.h"
#include "gio/gio.h"

int main(int argc, char **argv) {

  gst_init(&argc, &argv);
  gtk_init();

  return g_application_run(G_APPLICATION(ministreamer_app_new()), argc,
                           argv);
}
