#include "MinistreamerApp.h"

#include "MinistreamerWindow.h"
#include "ScreenShare.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "gtk/gtksingleselection.h"

G_DEFINE_TYPE(MinistreamerApp, ministreamer_app, GTK_TYPE_APPLICATION)

static void preferences_activated(GSimpleAction *action, GVariant *parameter,
                                  gpointer app) {}

static void ministreamer_app_quit_clicked(GSimpleAction *action,
                                          GVariant *parameter, gpointer app) {
  gst_element_set_state(MINISTREAMER_APP(app)->pipeline->pipeline,
                        GST_STATE_NULL);
  g_application_quit(G_APPLICATION(app));
}

static void ministreamer_app_start_new_screenshare(GtkButton *self,
                                                   gpointer user_data) {
  MinistreamerApp *app = MINISTREAMER_APP(user_data);
  ScreenShare *new = screen_share_new();
  g_signal_connect(new, "ready",
                   G_CALLBACK(screen_share_add_to_ministreamer_gst_pipeline),
                   app->pipeline);
}

static void ministreamer_app_remove_screenshare(GtkButton *self,
                                                gpointer user_data) {
  MinistreamerApp *app = MINISTREAMER_APP(user_data);

  ScreenShare *selected = gtk_single_selection_get_selected_item(
      GTK_SINGLE_SELECTION(app->selection));
  if (selected == NULL) {
    return;
  }

  guint index = gtk_single_selection_get_selected(GTK_SINGLE_SELECTION(app->selection));

  screen_share_remove_from_ministreamer_pipeline(selected, index, app->pipeline);

  g_print("%s\n", selected->name);
}

static GActionEntry app_entries[] = {
    {"preferences", preferences_activated, NULL, NULL, NULL},
    {"quit", ministreamer_app_quit_clicked, NULL, NULL, NULL},
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

  g_signal_connect(win->add_screenshare_button, "clicked",
                   G_CALLBACK(ministreamer_app_start_new_screenshare), self);

  g_signal_connect(win->remove_screenshare_button, "clicked",
                   G_CALLBACK(ministreamer_app_remove_screenshare), self);

  g_autoptr(GdkPaintable) paintable = NULL;
  g_object_get(self->pipeline->displaysink, "paintable", &paintable, NULL);
  gtk_picture_set_paintable(GTK_PICTURE(win->video), paintable);
  g_object_unref(paintable);

  self->selection = GTK_SELECTION_MODEL(
      gtk_single_selection_new(G_LIST_MODEL(self->pipeline->srcs)));
  gtk_column_view_set_model(GTK_COLUMN_VIEW(win->column_view),
                            GTK_SELECTION_MODEL(self->selection));

  // g_signal_connect(win->aaa, "value-changed", G_CALLBACK(value_changed),
  // NULL);
}

static void ministreamer_app_class_init(MinistreamerAppClass *klass) {
  GApplicationClass *appclass = G_APPLICATION_CLASS(klass);
  appclass->startup = ministreamer_app_startup;
  appclass->activate = ministreamer_app_activate;
}

static void ministreamer_app_init(MinistreamerApp *self) {}

MinistreamerApp *ministreamer_app_new() {
  MinistreamerApp *app = g_object_new(MINISTREAMER_APP_TYPE, "application-id",
                                      "fr.jacotot.ministreamer", NULL);

  app->pipeline = ministreamer_gst_pipeline_new();

  return app;
}
