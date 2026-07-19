#ifndef SCREEN_SHARE_H
#define SCREEN_SHARE_H

#include <gst/gst.h>
#include <libportal/portal.h>

typedef struct _ScreenShare ScreenShare;
typedef void (*ScreenShareCreatedCallback)(ScreenShare *);

struct _ScreenShare {
  XdpPortal *portal;
  XdpSession *session;
  char *path;
  ScreenShareCreatedCallback ready;
  GstElement *src;
};

ScreenShare *screen_share_new(ScreenShareCreatedCallback);

#endif
