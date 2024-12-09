#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


// We tried to use actions as they are intended to be used, but they are
// much to constraining for what we need in quickscope.  Actions is even
// worse in GTK4 conpared to GTK3.
//
// This GActionGroup is just another name space mapping on top of the
// signals name space mapping to callback functions which is too
// constraining for quickstream.  Layers of shit on shit, leading to
// broken shit that I can't use as was intended.
//
GActionGroup *actions = 0;

// This accel_group object will never call the attached "activate"
// function handlers, because we want these "accelerator keys" to do their
// thing when the widget is hidden too, so we catch and block the key
// events in the main window and call the would be function handlers
// there.  This would be very hard to do in GTK4, and this is the main
// reason we use GTK3 and not GTK4.  In GTK4 there is no catching of
// key-press events of the main window widget, but GTK3 has that.
//
// We just use it so menu items label the accelerator keys on them.
//
GtkAccelGroup *accel_group = 0;


static void quit_cb(void) {
    qsApp_destroy();
}

#if 0
static void
showMenubar_cb(void) {

    ERROR("window=%p", GetCurrentWindow());
}
#endif

static void newTab_cb(void) {
    AddNewGraph(GetCurrentWindow(), 0);
}


// This gets called from the broken GActionGroup bull-shit.
static void
activate_quit(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    quit_cb();
}
static void
activate_showMenubar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    ERROR("window=%p", GetCurrentWindow());
}
static void
activate_newTab(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    newTab_cb();
}

// The stupid GActionEntry interface does not let us pass a fucking
// pointer, so this is functions calling the "real" functions:
//
static GActionEntry entries[] = {
  { "quit", activate_quit },
  { "showMenubar", activate_showMenubar },
  { "newTab", activate_newTab }
};


static inline void AddAccelerator(GtkBuilder *builder,
    const char *id, guint key) {

  GtkWidget *item = (GtkWidget*) gtk_builder_get_object(builder, id);
  DASSERT(item);


  ///////////////////////////////////////////////////////////////////////
  /// THIS IS BROKEN: You cannot have an accelerator for a hidden widget
  /// which totally sucks for quickscope.  quickscpe needs key press
  /// controls regardless of whether widgets are showing.  Given we have
  /// to fix the 
  ///
  ///  I know what to do now:
  ///
  ///  use gtk_widget_add_accelerator() for menu items so that the menu
  ///  item is labeled, but catch the event before the menu item gets it
  ///  in the main window widget in a main window key-press event and have
  ///  that catcher call the "catcher function", and block the event from
  ///  reaching the menu item if it is showing.  Have the "catcher
  ///  function" fix the check state of any check menu items for show and
  ///  hide memu items.
  ///
  gtk_widget_add_accelerator(item, "activate", accel_group,
          key, 0, GTK_ACCEL_VISIBLE);
}


void AddActions(struct QsWindow *win, GtkBuilder *builder) {

    DSPEW();
    if(!actions) {
        actions = (GActionGroup*) g_simple_action_group_new();
        DASSERT(actions);
        
        accel_group = gtk_accel_group_new();
        DASSERT(accel_group);
    }
    if(!win) return;

    DASSERT(win->gtkWindow);
    gtk_builder_connect_signals(builder, NULL);
    g_action_map_add_action_entries(G_ACTION_MAP(actions),
                entries, G_N_ELEMENTS(entries), 0);

    win->menubar = GTK_WIDGET(gtk_builder_get_object(builder, "menubar"));
    DASSERT(win->menubar);

    gtk_widget_insert_action_group(GTK_WIDGET(win->gtkWindow), "app", actions);
    gtk_window_add_accel_group(GTK_WINDOW(win->gtkWindow), accel_group);
    AddAccelerator(builder, "quit_item", GDK_KEY_q);
    AddAccelerator(builder, "showMenubar_item", GDK_KEY_m);
    AddAccelerator(builder, "newTab_button", GDK_KEY_t);
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

