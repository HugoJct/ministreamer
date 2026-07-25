#include "MinistreamerApp.h"

#include "MinistreamerWindow.h"
#include "gio/gio.h"

G_DEFINE_TYPE(MinistreamerApp, ministreamer_app, GTK_TYPE_APPLICATION)

static void preferences_activated(GSimpleAction *action, GVariant *parameter,
                                  gpointer app) {}

static void quit_activated(GSimpleAction *action, GVariant *parameter,
                           gpointer app) {
  g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] = {
    {"preferences", preferences_activated, NULL, NULL, NULL},
    {"quit", quit_activated, NULL, NULL, NULL},
};

static const char *quit_shortcut[2] = {"<Ctrl>Q", NULL};

static void ministreamer_app_startup(GApplication *app) {
  G_APPLICATION_CLASS(ministreamer_app_parent_class)->startup(app);

  g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries,
                                  G_N_ELEMENTS(app_entries), app);

  gtk_application_set_accels_for_action(GTK_APPLICATION(app), "app.quit",
                                        quit_shortcut);
}

static void ministreamer_app_activate(GApplication *app) {

  MinistreamerWindow *win = ministreamer_window_new(MINISTREAMER_APP(app));
  gtk_window_present(GTK_WINDOW(win));

  MinistreamerApp *self = MINISTREAMER_APP(app);

  g_signal_connect_swapped(win->start_screenshare_1_button, "clicked",
                           G_CALLBACK(self->on_activate.button1_clicked), win);
  g_signal_connect_swapped(win->start_screenshare_2_button, "clicked",
                           G_CALLBACK(self->on_activate.button2_clicked), win);
  g_signal_connect_swapped(win->quit_button, "clicked",
                           G_CALLBACK(self->on_activate.quit), win);

  g_autoptr(GdkPaintable) paintable = NULL;
  g_object_get(self->on_activate.displaysink, "paintable", &paintable, NULL);
  gtk_picture_set_paintable(GTK_PICTURE(win->video), paintable);
  g_object_unref(paintable);
}

static void ministreamer_app_class_init(MinistreamerAppClass *klass) {
  GApplicationClass *appclass = G_APPLICATION_CLASS(klass);
  appclass->startup = ministreamer_app_startup;
  appclass->activate = ministreamer_app_activate;
}

static void ministreamer_app_init(MinistreamerApp *self) {}

MinistreamerApp *ministreamer_app_new(ActivateParams params) {
  MinistreamerApp *app = g_object_new(MINISTREAMER_APP_TYPE, "application-id",
                                      "fr.jacotot.ministreamer", NULL);

  app->on_activate = params;

  return app;
}
