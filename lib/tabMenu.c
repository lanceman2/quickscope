#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


// We keep the same tab popover (popup) menu to use for all tabs in the
// process.  We create it once for a given app run.
static GtkPopover *tab_popupMenu = 0;

static struct QsGraph *graph = 0;


#if 0
static void closed_cb(GtkWidget *widget, gpointer user_data) {


    DSPEW();

    //gtk_widget_unparent(GTK_WIDGET(tab_popupMenu));
}
#endif


void CreatePopoverMenu(void) {

    if(tab_popupMenu) return;


    //g_signal_connect(G_OBJECT(tab_popupMenu), "closed", G_CALLBACK(closed_cb), 0);
    DSPEW();
}


void ShowTabPopupMenu(struct QsGraph *g, int x, int y) {

    DASSERT(g);

    if(!tab_popupMenu)
        CreatePopoverMenu();

    //DASSERT(gtk_widget_get_parent(GTK_WIDGET(tab_popupMenu)) == 0);

    if(g != graph)
        graph = g;

#if 0
    gtk_widget_set_parent(GTK_WIDGET(tab_popupMenu), graph->tab);

    GdkRectangle rect = { .x=x, .y=y, .width=0, .height=0 };
    gtk_popover_set_pointing_to(GTK_POPOVER(tab_popupMenu), &rect);
    gtk_popover_popup(GTK_POPOVER(tab_popupMenu));

    // TODO: BUG: The first time the code runs past here we get
    // 4 spews: Gtk-CRITICAL, Gtk-WARNING, Gtk-CRITICAL, and a Gtk-WARNING
    // but seems to run fine after.
#endif

    DSPEW();
}


void CleanupTabPopupMenu(void) {

    if(tab_popupMenu) {
      g_object_unref(tab_popupMenu);
      tab_popupMenu = 0;
    }
    DSPEW();
}
