#ifndef GSTREAMER_HANDLING_H
#define GSTREAMER_HANDLING_H

#include <gst/gst.h>

#include "ScreenShare.h"

extern GstElement *compositor;
extern GstElement *displaysink;
extern GstElement *pipeline;

int gstreamer_handling_init_sink();
void gstreamer_handling_add_screen_share(ScreenShare *sc_sh);

#endif
