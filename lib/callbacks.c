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

void closeTab_cb(void) {
    struct QsWindow *win = GetCurrentWindow();
    DASSERT(win->gtkNotebook);
    gint n = gtk_notebook_get_n_pages(win->gtkNotebook);
    DASSERT(n > 0);

    if(n == 1) {
        qsWindow_destroy(win);
        return;
    }

    gint page = gtk_notebook_get_current_page(win->gtkNotebook);
    DASSERT(page >= 0);
    gtk_notebook_remove_page(win->gtkNotebook, page);
}

// All the show/hide widget things will have a check_menu_item that we
// wish to keep consistent with whither the widget is showing or hiding.
// The thing that causes the widget to show or hide can vary.  GTK did
// not provide the flexibility we needed, so ya, this shit happened.
//
static inline void FlipShowHide(struct QsWindow *win,
            GtkWidget *w, GtkCheckMenuItem *item,
            bool *showing,
            void (*show)(GtkWidget *w), void (*hide)(GtkWidget *w)) {
    DASSERT(win);
    DASSERT(w);
    DASSERT(item);

    // Stop re-entrance.
    if(win->ignore_showHide) return;
    win->ignore_showHide = true;

    // Flip the state:
    if((*showing)) *showing = false;
    else *showing = true;

    if(!show) show = gtk_widget_show;
    if(!hide) hide = gtk_widget_hide;

    if((*showing)) {
        // We want check_menu_item and widget showing.
        if(!gtk_check_menu_item_get_active(item))
            // This could cause a re-enter to this function.
            gtk_check_menu_item_set_active(item, TRUE);
        // We have found that gtk_widget_is_visible() does not
        // work for the notebook tab bar, so we just show.
        // TODO: use gtk_notebook_get_show_tabs()
        show(w);
    } else {
        // We want check_menu_item and widget hidden.
        if(gtk_check_menu_item_get_active(item))
            // This could cause a re-enter to this function.
            gtk_check_menu_item_set_active(item, FALSE);
        // We have found that gtk_widget_is_visible() does not
        // work for the notebook tab bar, so we just hide.
        // TODO: use gtk_notebook_get_show_tabs()
        hide(w);
    }

    win->ignore_showHide = false;
}

void showHideMenubar_cb(void) {

    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, win->menubar, win->showHideMenubar_item,
            &win->menubar_showing, 0, 0);
}

void showHideButtonbar_cb(void) {

    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, win->buttonbar, win->showHideButtonbar_item,
            &win->buttonbar_showing, 0, 0);
}

static void ShowTabbar(GtkWidget *w) {
  DASSERT(w);
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK(w), TRUE);
}
static void HideTabbar(GtkWidget *w) {
  DASSERT(w);
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK(w), FALSE);
}
//
void showHideTabbar_cb(void) {
    struct QsWindow *win = GetCurrentWindow();
    FlipShowHide(win, GTK_WIDGET(win->gtkNotebook),
            win->showHideTabbar_item,
            &win->buttonbar_showing, ShowTabbar, HideTabbar);
}

void newTab_cb(void) {
    AddNewGraph(GetCurrentWindow(), 0);
}

void newWindow_cb(void) {
    qsWindow_create();
}

