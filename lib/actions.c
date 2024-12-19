#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"

#include "accel_keys.h"


// This is a lot of code in order to change the GTK3 (and GTK4) action
// user interface pattern which we did not want.


// We tried to use actions as they are intended to be used, but they are
// much to constraining for what we need in quickscope.  Actions is even
// worse in GTK4 conpared to GTK3.
//
// This GActionGroup is just another name space mapping on top of the
// signals name space mapping to callback functions which is too
// constraining for quickstream.  Layers of shit on shit, leading to
// broken shit that I can't use as was intended.

// This accel_group object will never call the attached "activate"
// function handlers, because we want these "accelerator keys" to do their
// thing when the widget is hidden too, so we catch and block the key
// events in the main window and call the would be function handlers
// there.  This would be very hard to do in GTK4, and this is one
// reason we use GTK3 and not GTK4.  In GTK4 there is no catching of
// key-press events of the main window widget, but GTK3 has that.
//
// We just use it so menu items label the accelerator keys on them.


// activate_*() gets called from the GActionGroup stuff.

static void
activate_showHideMenubar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showHideMenubar_cb();
}
static void
activate_showHideButtonbar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showHideButtonbar_cb();
}
static void
activate_showHideTabbar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showHideTabbar_cb();
}
static void
activate_showHideControlbar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showHideControlbar_cb();
}
static void
activate_showHideStatusbar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showHideStatusbar_cb();
}
static void
activate_newTab(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    newTab_cb();
}
static void
activate_newWindow(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    newWindow_cb();
}
static void
activate_closeWindow(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    closeWindow_cb();
}
static void
activate_closeTab(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    closeTab_cb();
}
static void
activate_quit(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    quit_cb();
}


// The stupid GActionEntry interface does not let us pass a fucking
// pointer, so this is functions calling the other functions, or
// we could have made another stupid-ass mapping data structure.
//
static GActionEntry entries[] = {
  { "showHideMenubar",    activate_showHideMenubar},
  { "showHideButtonbar",  activate_showHideButtonbar },
  { "showHideTabbar",     activate_showHideTabbar },
  { "showHideControlbar", activate_showHideControlbar },
  { "showHideStatusbar",  activate_showHideStatusbar },
  { "newTab",             activate_newTab },
  { "newWindow",          activate_newWindow },
  { "closeTab",           activate_closeTab },
  { "closeWindow",        activate_closeWindow },
  { "quit",               activate_quit }
};


static inline GtkWidget *AddAccelerator(struct QsWindow *win,
    GtkBuilder *builder, const char *id, guint key,
    uint32_t mod_mask) {

  GtkWidget *w = (GtkWidget*) gtk_builder_get_object(builder, id);
  DASSERT(w);

  ///////////////////////////////////////////////////////////////////////
  /// THIS IS BROKEN: You cannot have an accelerator for a hidden widget
  /// which totally sucks for quickscope.  quickscpe needs key press
  /// controls regardless of whether widgets are showing.
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
  gtk_widget_add_accelerator(w, "activate", win->accel_group,
          key, mod_mask, GTK_ACCEL_VISIBLE);
  return w;
}


void AddActions(struct QsWindow *win, GtkBuilder *builder) {

    if(!win) return;

    DSPEW();
    win->actions = (GActionGroup*) g_simple_action_group_new();
    DASSERT(win->actions);

    win->accel_group = gtk_accel_group_new();
    DASSERT(win->accel_group);

    DASSERT(win->gtkWindow);
    gtk_builder_connect_signals(builder, NULL);
    g_action_map_add_action_entries(G_ACTION_MAP(win->actions),
                entries, G_N_ELEMENTS(entries), 0);

    win->menubar = GTK_WIDGET(gtk_builder_get_object(builder, "menubar"));
    DASSERT(win->menubar);
    win->buttonbar = GTK_WIDGET(gtk_builder_get_object(builder, "buttonbar"));
    DASSERT(win->buttonbar);

    gtk_widget_insert_action_group(GTK_WIDGET(win->gtkWindow), "app",
            win->actions);
    g_object_ref(G_OBJECT(win->accel_group));
    gtk_window_add_accel_group(GTK_WINDOW(win->gtkWindow), win->accel_group);
    win->showHideMenubar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showHideMenubar_item", ShowHideMenubar_key,0 ));
    win->showHideButtonbar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showHideButtonbar_item", ShowHideButtonbar_key, 0));
    win->showHideTabbar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showHideTabbar_item", ShowHideTabbar_key, 0));
    win->showHideControlbar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showHideControlbar_item", ShowHideControlbar_key, 0));
    win->showHideStatusbar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showHideStatusbar_item", ShowHideStatusbar_key, 0));

    AddAccelerator(win, builder, "newWindow_item", NewWindow_key, 0);
    AddAccelerator(win, builder, "newTab_item", NewTab_key, 0);
    AddAccelerator(win, builder, "closeTab_item", CloseTab_key, GDK_SHIFT_MASK);
    AddAccelerator(win, builder, "closeWindow_item", CloseWindow_key,
            GDK_SHIFT_MASK);
    AddAccelerator(win, builder, "quit_item", Quit_key, 0);

    win->menubar_showing = true;
    win->buttonbar_showing = true;
    win->tabbar_showing = true;
}


void FreeActions(struct QsWindow *win) {

  if(win->actions) {

      // TODO: ?? does the main window widget own these?
      // This does not seem to crash, so ya, we'll do this:
      g_object_unref(win->actions);
      g_object_unref(win->accel_group);
      win->accel_group = 0;
      win->actions = 0;
  }
}
