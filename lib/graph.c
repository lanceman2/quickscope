#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"



#if 0
static void pressed_cb(GtkGestureClick *gesture,
        int n_press, double x, double y, struct QsGraph *g) {

    WARN("  n_press=%d  x,y=%lg,%lg", n_press, x, y);

    ShowTabPopupMenu(g, (int)(x + 0.5), (int)(y + 0.5));
}
#endif


static void Destroy_cb(GtkWidget *widget, struct QsGraph *g) {

    DASSERT(widget);
    DASSERT(g);

    DSPEW("Freeing graph=%p", g);

    DZMEM(g, sizeof(*g));
    free(g);
}


// Add a new graph and tab to the windows notebook
//
void AddNewGraph(struct QsWindow *w, const char *title) {

    DASSERT(w);

    struct QsGraph *g = calloc(1, sizeof(*g));
    ASSERT(g, "calloc(1,%zu) failed", sizeof(*g));

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1/*spacing*/);
    g_signal_connect(vbox, "destroy", G_CALLBACK(Destroy_cb), g);

    GtkWidget *da = gtk_drawing_area_new();
    gtk_box_pack_end(GTK_BOX(vbox), da,
            FALSE/*expand*/, TRUE/*fill*/, 2/*padding*/);
    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_end(GTK_BOX(vbox), entry, 
            FALSE/*expand*/, FALSE/*fill*/, 2/*padding*/);
    GtkWidget *statusbar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(vbox), statusbar,
            FALSE/*expand*/, FALSE/*fill*/, 2/*padding*/);

    if(!title || !title[0])
        title = "new tab";

    GtkWidget *tab = gtk_label_new(title);
    //g_object_set_data(G_OBJECT(tab), "qsGraph", g);
    //g_object_set_data(G_OBJECT(vbox), "qsGraph", g);

    gtk_notebook_append_page(w->gtkNotebook, vbox, tab);

    gtk_widget_show_all(vbox);

    g->tab = tab;
}


