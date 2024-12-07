#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


GActionGroup *actions = 0;
GtkAccelGroup *accel_group = 0;


static void
quit_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {

    qsApp_destroy();
}


static void
showMenubar_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data) {

    ERROR();
    
}


static GActionEntry entries[] = {
  { "quit", quit_cb, NULL, NULL, NULL },
  { "showMenubar", showMenubar_cb, NULL, NULL, NULL }
};


static inline void AddAccelerator(GtkBuilder *builder,
    const char *id, guint key) {

  GtkWidget *item = (GtkWidget*) gtk_builder_get_object(builder, id);

  // This first call to gtk_widget_add_accelerator() will add
  // the key label to the menu item without the Ctrl+key since
  // the key mask is 0.
  gtk_widget_add_accelerator(item, "activate", accel_group,
          key, 0, GTK_ACCEL_LOCKED);
  // This call will not have the Ctrl+key showing in the
  // menu item, because the call above got it set before this.
  gtk_widget_add_accelerator(item, "activate", accel_group,
          key, GDK_CONTROL_MASK, GTK_ACCEL_LOCKED);
}


void AddActions(struct QsWindow *win, GtkBuilder *builder) {


    DSPEW();
    if(!actions) {
        actions = (GActionGroup*) g_simple_action_group_new();
        g_action_map_add_action_entries(G_ACTION_MAP(actions),
                entries, G_N_ELEMENTS(entries), 0);
        accel_group = gtk_accel_group_new();
    }
    if(!win) return;

    DASSERT(win->gtkWindow);

    win->menubar = GTK_WIDGET(gtk_builder_get_object(builder, "menubar"));
    DASSERT(win->menubar);
    gtk_widget_hide(win->menubar);

    gtk_widget_insert_action_group(GTK_WIDGET(win->gtkWindow), "app", actions);
    gtk_window_add_accel_group(GTK_WINDOW(win->gtkWindow), accel_group);
    AddAccelerator(builder, "quit_item", GDK_KEY_q);
    AddAccelerator(builder, "showMenubar_item", GDK_KEY_m);
}


void FreeActions(struct QsWindow *window) {

  if(!actions) {

      g_object_unref(actions);
      g_object_unref(accel_group);
      accel_group = 0;
      actions = 0;
  }
}


void FreeAccelGroup(void) {}

