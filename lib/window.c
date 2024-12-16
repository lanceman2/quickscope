#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"

#include "accel_keys.h"


uint32_t mod_keys = 0;

GdkCursor *hand_cursor = 0;


static void DestroyWindow_cb(GtkWindow *gtkWindow, struct QsWindow *w) {

    DASSERT(gtkWindow);
    DASSERT(w);
    DASSERT(w->gtkWindow == gtkWindow);

    DSPEW("freeing GTK main window=%p", w->gtkWindow);

    FreeActions(w);

    DZMEM(w, sizeof(*w));
    free(w);

    --windowCount;
}


static gboolean keyRelease_window_cb(GtkWidget *widget, GdkEvent *e,
        gpointer data) {

    switch(e->key.keyval) {
        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            // Mark the key as not set.
            mod_keys &= ~MOD_SHIFT;
            break;
    }

    return TRUE;
}

static gboolean keyPress_window_cb(GtkWidget *widget, GdkEvent *e,
        struct QsWindow *win) {
    // This is something that you can no longer do (at least easily) in
    // GTK4; that is override action group events.  GtkWindow has no
    // key-press-event signal/callback shit in GTK4.  They seemed to
    // replace it with gestures, making you write 10 times more code;
    // and near impossible to follow the code.
    DASSERT(widget);
    DASSERT(GTK_WINDOW(widget) == win->gtkWindow);
    DASSERT(e);

    // Ya! This fucking works.  I can grab events before the GAction
    // bullshit fucks it up.  GTK4 can't do it this way, if at all.

    if(mod_keys & MOD_SHIFT) {
        switch(e->key.keyval) {

            case CloseTab_key:
                closeTab_cb();
                return TRUE;
            case CloseWindow_key:
                closeWindow_cb();
                return TRUE;
        }
        return FALSE;
    }


    switch(e->key.keyval) {

        case GDK_KEY_Shift_L:
        case GDK_KEY_Shift_R:
            // Mark the key as set.
            mod_keys |= MOD_SHIFT;
            return TRUE;

        case ShowHideTabbar_key:
            showHideTabbar_cb();
            return TRUE;
        case ShowHideButtonbar_key:
            showHideButtonbar_cb();
            return TRUE;
        case ShowHideMenubar_key:
            showHideMenubar_cb();
            return TRUE;
        case ShowHideControlbar_key:
            showHideControlbar_cb();
            return TRUE;
        case ShowHideStatusbar_key:
            showHideStatusbar_cb();
            return TRUE;
          case NewWindow_key:
            newWindow_cb();
            return TRUE;
        case Quit_key:
            quit_cb();
            return TRUE;
        case NewTab_key:
            newTab_cb();
            return TRUE;
    }

    /* return FALSE means the event is not handled; so it's propagated to
     * child widgets. */
    /* return TRUE means the event is handled; so it's not propagated*/
    return FALSE;
}

static inline void 
FixCheckMenu(struct QsWindow *win, GtkCheckMenuItem *item, bool showing) {

    bool is_active = gtk_check_menu_item_get_active(item);
    // Stop callback re-entrance:
    win->ignore_showHide = true;

    if(showing && !is_active)
        gtk_check_menu_item_set_active(item, TRUE);
    else if(!showing && is_active)
        gtk_check_menu_item_set_active(item, FALSE);

    win->ignore_showHide = false;
}

static gboolean notebookSwitchPage_cb(GtkNotebook *notebook,
        GtkWidget* vbox, int page_num, struct QsWindow *win) {
    // Looks like this is called before 
    // gtk_notebook_get_current_page() will return this page_num.
    DASSERT(notebook);
    DASSERT(win);
    DASSERT(notebook == win->gtkNotebook);
    struct QsGraph *g = g_object_get_data(G_OBJECT(vbox), "qsGraph");
    DASSERT(g);
    DASSERT(g->vbox == vbox);

    // We can show and hide the control and status bars in the graph tab,
    // but there is per window menu for that, so we need to check and fix
    // the show/hide menu items in the main menu bar to be the same as the
    // show/hide state of the graph tab that we just switched to.
    //
    FixCheckMenu(win, win->showHideControlbar_item, g->controlbar_showing);
    FixCheckMenu(win, win->showHideStatusbar_item, g->statusbar_showing);

    return FALSE;
}


static void CreateGTKWindow_cb(GtkApplication* gApp, struct QsWindow *w) {

    DASSERT(gApp == app);

    AddCSS();

    GtkBuilder *builder = Get_builder_from_file("window.ui");

    GtkWindow *gtkWindow = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    w->gtkWindow = gtkWindow;
    w->gtkNotebook = GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook"));
    DASSERT(w->gtkNotebook);

    gtk_widget_add_events(GTK_WIDGET(w->gtkWindow),
            GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

    // The XML GTK .ui file parsing of label markup is broken, so we set
    // these two labels here.
    GtkLabel *l = GTK_LABEL(gtk_builder_get_object(builder, "newWindow_label"));
    DASSERT(l);
    gtk_label_set_markup(l, "New <u>W</u>indow");
    l = GTK_LABEL(gtk_builder_get_object(builder, "newTab_label"));
    DASSERT(l);
    gtk_label_set_markup(l, "New <u>G</u>raph Tab");

    gtk_widget_show_all(GTK_WIDGET(gtkWindow));

    g_object_set_data(G_OBJECT(gtkWindow), "qsWindow", w);
    gtk_window_set_application(gtkWindow, app);
    gtk_window_set_title(gtkWindow, "Window");
    gtk_widget_show(GTK_WIDGET(gtkWindow));
    g_signal_connect(gtkWindow, "destroy", G_CALLBACK(DestroyWindow_cb), w);
    g_signal_connect(gtkWindow, "key-press-event",
            G_CALLBACK(keyPress_window_cb), w);
    g_signal_connect(gtkWindow, "key-release-event",
            G_CALLBACK(keyRelease_window_cb), w);

    AddActions(w, builder);

    g_object_unref(builder);

    // Add a new graph tab.
    AddNewGraph(w, 0);

    g_signal_connect(w->gtkNotebook,"switch-page",
            G_CALLBACK(notebookSwitchPage_cb), w);

    if(!hand_cursor) {
        hand_cursor = gdk_cursor_new_from_name(
                gtk_widget_get_display(GTK_WIDGET(gtkWindow)),
                "grabbing");
        ASSERT(hand_cursor);
    }
}


// We tried other ways, but this is the only one that works so far.
//
GtkBuilder *Get_builder_from_file(const char *filename) {

    DSPEW();
    GtkBuilder *builder = gtk_builder_new();
    DASSERT(builder);
    GError *error = 0;
    char *path = g_strjoin("/", qsResourceDir, filename, NULL);
    if(!gtk_builder_add_from_file(builder, path, &error)) {
        ASSERT(0, "gtk_builder_add_from_file(,\"%s\",) failed: %s",
                path, error->message);
        // Not that we get here.
        g_object_unref(error);
        g_free(path);
    }
    DASSERT(!error);
    DSPEW("Loaded GTK4 builder file \"%s\"", path);
    g_free(path);
    return builder;
}


struct QsWindow *qsWindow_create(void) {

    DSPEW();

    GetApp();

    struct QsWindow *w = calloc(1, sizeof(*w));
    ASSERT(w, "calloc(1,%zu) failed", sizeof(*w));

    if(looping)
        CreateGTKWindow_cb(GTK_APPLICATION(app), w);
    else
        g_signal_connect(app, "activate", G_CALLBACK(CreateGTKWindow_cb), w);

    ++windowCount;

    return w;
}


void qsWindow_destroy(struct QsWindow *w) {

    DASSERT(w);
    DASSERT(w->gtkWindow);

    gtk_widget_destroy(GTK_WIDGET(w->gtkWindow));
    // The QsWindow memory is freed in the GTK "destroy" callback.
}
