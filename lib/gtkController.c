#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"


static void
not_implemented(GSimpleAction *action,
          GVariant *parameter, void *userData) {

  fprintf(stderr, "     Action “%s” not implemented\n",
      g_action_get_name(G_ACTION(action)));
}


static void
App_quit(GSimpleAction *action,
          GVariant *parameter, void *userData) {

  qsApp_destroy();
  // The libquickscope.so users program may keep running and may create a
  // new qsApp object.
}


static void
App_close(GSimpleAction *action, GVariant *parameter,
        void *userData) {

    gtk_window_destroy(GetCurrentGtkWindow());
}


static void
App_newTab(GSimpleAction *action, GVariant *parameter,
        void *userData) {

    AddNewGraph(GetCurrentWindow(), "New Tab");
}


static void
App_newWindow(GSimpleAction *action, GVariant *parameter,
        void *userData) {

    qsWindow_create();
}



static GActionEntry win_entries[] = {
  { "newWindow", (void *) App_newWindow, NULL, NULL, NULL },
  { "newTab",    (void *) App_newTab, NULL, NULL, NULL },
  { "save",    (void *) not_implemented, NULL, NULL, NULL },
  { "save-as", (void *) not_implemented, NULL, NULL, NULL },
  { "close",   (void *) App_close,       NULL, NULL, NULL },
  { "copy",    (void *) not_implemented, NULL, NULL, NULL },
  { "cut",     (void *) not_implemented, NULL, NULL, NULL },
  { "paste",   (void *) not_implemented, NULL, NULL, NULL },
  { "quit",    (void *) App_quit,        NULL, NULL, NULL },
  { "about",   (void *) not_implemented, NULL, NULL, NULL },
  { "help",    (void *) not_implemented, NULL, NULL, NULL }
  // no terminator ?
};


static inline void AddShortcut(GtkEventController *ctl,
        const char *name, uint16_t key) {

    // Note: testing shows that the last Addition with a given
    // name (like "app.newWindow") will be the one that shows
    // on the menu item.

    uint32_t mask = GDK_CONTROL_MASK;
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(ctl),
            gtk_shortcut_new(gtk_keyval_trigger_new(key, mask),
                    gtk_named_action_new(name)));
#if 1
    mask = 0;
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(ctl),
            gtk_shortcut_new(gtk_keyval_trigger_new(key, mask),
                    gtk_named_action_new(name)));
#endif
}


void AddActions(GtkWidget *w) {

    DASSERT(gActionGroup);
    DASSERT(w);

    gtk_widget_insert_action_group(w, "app", gActionGroup);

#if 1 // this part just adds the keyboard short cuts.
    GtkEventController *c = gtk_shortcut_controller_new();
    DASSERT(c);

    // This does not make sense to me:
    gtk_shortcut_controller_set_scope(GTK_SHORTCUT_CONTROLLER(c),
                      GTK_SHORTCUT_SCOPE_GLOBAL);

    AddShortcut(c, "app.newWindow", GDK_KEY_w);
    AddShortcut(c, "app.newTab",    GDK_KEY_t);
    AddShortcut(c, "app.save",      GDK_KEY_s);
    AddShortcut(c, "app.save-as",   GDK_KEY_a);
    AddShortcut(c, "app.close",     GDK_KEY_c);
    AddShortcut(c, "app.quit",      GDK_KEY_q);

    // This widget will own this gtkEventController, or so it appears.
    gtk_widget_add_controller(w, c);
#endif
}


// For the process global app we need a GActionGroup
//
void MakeGtkActionGroup(void) {

    DASSERT(!gActionGroup);

    gActionGroup = (GActionGroup*) g_simple_action_group_new();
    DASSERT(gActionGroup);
    g_action_map_add_action_entries(G_ACTION_MAP(gActionGroup),
            win_entries, G_N_ELEMENTS(win_entries), 0/* user_data */);
}
