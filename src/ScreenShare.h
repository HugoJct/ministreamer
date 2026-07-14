#ifndef SCREEN_SHARE_H
#define SCREEN_SHARE_H

#include <libportal/portal.h>

typedef struct _ScreenShare ScreenShare;
typedef void (*ScreenShareCreatedCallback)(ScreenShare *);

struct _ScreenShare {
  XdpPortal *portal;
  XdpSession *session;
  int fd;
  char *path;
  void (*ready)(ScreenShare *);
};

ScreenShare *screen_share_new(ScreenShareCreatedCallback);

#endif
