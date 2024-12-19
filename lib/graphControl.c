#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


static gboolean
focusIn_cb(GtkWidget *entry, GdkEventFocus *e, struct QsGraph *g) {

    DASSERT(g);

    if(!g->window->graphControlHasFocus)
        // This seems to turn off the accelerators without fucking up
        // the menu items short cut key labels.  That is, we keep the
        // accelerators from taking keyboard events from this file.
        // We added another gobject reference to accel_group so this
        // does not destroy the accel_group (we just do not know).
        gtk_window_remove_accel_group(
                GTK_WINDOW(g->window->gtkWindow),
                g->window->accel_group);

    // Set the flag, so the window knows not to take our keyboard input.
    g->window->graphControlHasFocus = g;

    return FALSE;
}

static gboolean
focusOut_cb(GtkWidget *entry, GdkEventFocus *e, struct QsGraph *g) {

    DASSERT(g);

    // We just can't count on GTK not having a focus event race condition;
    // so we need to see if we marked the current focused graph, and not
    // assume that we do.  If another graph control has the focus we do
    // not want to unset this flag.
    if(g->window->graphControlHasFocus == g)
        g->window->graphControlHasFocus = 0;

    if(!g->window->graphControlHasFocus)
        // This seems to turn on the accelerators without fucking up the
        // menu items short cut key labels.  For all we know the short cut
        // key labels are ripped off and put back on without us seeing
        // them change.  The GTK accel_group stuff is far to intrusive
        // (high level), but we use them to label the main menus;
        // otherwise the menus will not have nice key short cut labels.
        gtk_window_add_accel_group(
                GTK_WINDOW(g->window->gtkWindow),
                g->window->accel_group);

    return FALSE;
}

static gboolean keyPress_cb(GtkWidget *entry,
        GdkEventKey *e, struct QsGraph *g) {

    DSPEW();

    return FALSE;
}

static gboolean keyRelease_cb(GtkWidget *entry,
        GdkEventKey *e, struct QsGraph *g) {

    DSPEW();

    return FALSE;
}

static void destroy_cb(GtkWidget *entry, struct QsGraph *g) {

}


// The returned GTK widget will be owned by the caller.
//
GtkWidget *CreateGraphControl(struct QsGraph *g) {

    GtkWidget *entry = gtk_entry_new();

    gtk_widget_set_events(entry,
            gtk_widget_get_events(entry)
                | GDK_KEY_PRESS_MASK
                | GDK_KEY_RELEASE_MASK
                | GDK_FOCUS_CHANGE_MASK);

    g_signal_connect(entry, "key-press-event",
            G_CALLBACK(keyPress_cb), g);
    g_signal_connect(entry, "key-release-event",
            G_CALLBACK(keyRelease_cb), g);
    g_signal_connect(entry, "focus-in-event",
            G_CALLBACK(focusIn_cb), g);
    g_signal_connect(entry, "focus-out-event",
            G_CALLBACK(focusOut_cb), g);
    g_signal_connect(entry, "destroy",
            G_CALLBACK(destroy_cb), g);

    return entry;
}
