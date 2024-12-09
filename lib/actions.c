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
//GActionGroup *actions = 0;

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
//GtkAccelGroup *accel_group = 0;


static void quit_cb(void) {
    qsApp_destroy();
}

// All the show/hide widget things will have a check_menu_item that we
// wish to keep consistent with whither the widget is showing or hiding.
// The thing that causes the widget to show or hide can vary.  GTK did
// not provide the flexibility we needed, so ya, this shit happened.
//
static inline void FlipShowHide(struct QsWindow *win,
            GtkWidget *w, GtkCheckMenuItem *item,
            bool *showing) {
    DASSERT(win);
    DASSERT(w);
    DASSERT(item);

    // Stop re-entrance.
    if(win->ignore_showHide) return;
    win->ignore_showHide = true;

    // Flip the state:
    if((*showing)) *showing = false;
    else *showing = true;


    if((*showing)) {
        // We want check_menu_item and widget showing.
        if(!gtk_check_menu_item_get_active(item))
            // This could cause a re-enter to this function.
            gtk_check_menu_item_set_active(item, TRUE);
        if(!gtk_widget_is_visible(w))
            gtk_widget_show(w);
    } else {
        // We want check_menu_item and widget hidden.
        if(gtk_check_menu_item_get_active(item))
            // This could cause a re-enter to this function.
            gtk_check_menu_item_set_active(item, FALSE);
        if(gtk_widget_is_visible(w))
            gtk_widget_hide(w);
    }

    win->ignore_showHide = false;
}

static void
showMenubar_cb(void) {

    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, win->menubar, win->showMenubar_item,
            &win->menubar_showing);
}

static void
showButtonbar_cb(void) {

    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, win->buttonbar, win->showButtonbar_item,
            &win->buttonbar_showing);
}

static void newTab_cb(void) {
    AddNewGraph(GetCurrentWindow(), 0);
}


// activate_*() gets called from the GActionGroup stuff.
static void
activate_quit(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    quit_cb();
}
static void
activate_showMenubar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showMenubar_cb();
}
static void
activate_showButtonbar(GSimpleAction *action, GVariant *parameter,
        void *user_data) {
    showButtonbar_cb();
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
  { "quit",          activate_quit },
  { "showMenubar",   activate_showMenubar},
  { "showButtonbar", activate_showButtonbar },
  { "newTab",        activate_newTab }
};


static inline GtkWidget *AddAccelerator(struct QsWindow *win,
    GtkBuilder *builder, const char *id, guint key) {

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
          key, 0, GTK_ACCEL_VISIBLE);
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
    gtk_window_add_accel_group(GTK_WINDOW(win->gtkWindow), win->accel_group);
    AddAccelerator(win, builder, "quit_item", GDK_KEY_q);
    win->showMenubar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showMenubar_item", GDK_KEY_m));
    win->showButtonbar_item = GTK_CHECK_MENU_ITEM(AddAccelerator(win, builder,
          "showButtonbar_item", GDK_KEY_b));

    AddAccelerator(win, builder, "newTab_button", GDK_KEY_t);
    AddAccelerator(win, builder, "newTab_item", GDK_KEY_t);

    win->menubar_showing = true;
    win->buttonbar_showing = true;
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
