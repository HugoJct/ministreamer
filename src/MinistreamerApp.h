#ifndef MINISTREAMER_APP_H
#define MINISTREAMER_APP_H

#include "MinistreamerGstPipeline.h"
#include <gst/gstelement.h>
#include <gtk/gtk.h>

struct _MinistreamerApp {
  GtkApplication parent;

  MinistreamerGstPipeline *pipeline;
};

#define MINISTREAMER_APP_TYPE (ministreamer_app_get_type())
G_DECLARE_FINAL_TYPE(MinistreamerApp, ministreamer_app, MINISTREAMER, APP,
                     GtkApplication)

MinistreamerApp *ministreamer_app_new();

#endif
