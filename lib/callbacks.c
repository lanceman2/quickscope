#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"



void quit_cb(void) {
    qsApp_destroy();
}

void closeWindow_cb(void) {
    qsWindow_destroy(GetCurrentWindow());
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

void showHideMenubar_cb(void) {

    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, win->menubar, win->showHideMenubar_item,
            &win->menubar_showing);
}

void showHideButtonbar_cb(void) {

    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, win->buttonbar, win->showHideButtonbar_item,
            &win->buttonbar_showing);
}

void newTab_cb(void) {
    AddNewGraph(GetCurrentWindow(), 0);
}

void newWindow_cb(void) {
    qsWindow_create();
}

