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

    if(g->bgSurface) {
        cairo_surface_destroy(g->bgSurface);
        g->bgSurface = 0;
    }

    DZMEM(g, sizeof(*g));
    free(g);
}

static gboolean drawingArea_configure_cb(GtkWidget *w,
        GdkEventConfigure *e, struct QsGraph *g) {

    DASSERT(g);
    DASSERT(g->drawingArea);
    DASSERT(w == GTK_WIDGET(g->drawingArea));

    //DSPEW();

    GtkAllocation a;
    gtk_widget_get_allocation(w, &a);

    if(g->bgSurface)
        cairo_surface_destroy(g->bgSurface);

    g->bgSurface = gdk_window_create_similar_surface(
            gtk_widget_get_window(w),
            // I tried using CAIRO_CONTENT_COLOR_ALPHA and it's buggy as
            // fuck.  Looks like the GNOME compositor is buggy as fuck.
            //CAIRO_CONTENT_COLOR_ALPHA,
            CAIRO_CONTENT_COLOR,
            a.width, a.height);
    DASSERT(g->bgSurface);

    cairo_t *cr = cairo_create(g->bgSurface);
    cairo_set_source_rgba(cr, 0, 1, 1, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    g->drawCount = 0;

    return TRUE; // TRUE == done processing event
}


static gboolean drawingArea_draw_cb(GtkWidget *w,
        cairo_t *cr, struct QsGraph *g) {
    DASSERT(w);
    DASSERT(cr);
    DASSERT(g);
    DASSERT(g->bgSurface);
    DASSERT(w == GTK_WIDGET(g->drawingArea));

    DSPEW();

    if(g->drawCount)
        // Calling draw for go good reason:
        return;

    ++g->drawCount;

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(cr, g->bgSurface, 0, 0);
    cairo_paint(cr);
    return TRUE;
}

// Add a new graph and tab to the windows notebook
//
void AddNewGraph(struct QsWindow *w, const char *title) {

    DASSERT(w);

    struct QsGraph *g = calloc(1, sizeof(*g));
    ASSERT(g, "calloc(1,%zu) failed", sizeof(*g));

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0/*spacing*/);
    g_signal_connect(vbox, "destroy", G_CALLBACK(Destroy_cb), g);
    gtk_widget_set_name(vbox, "vbox");

    GtkWidget *drawingArea = gtk_drawing_area_new();
    DASSERT(drawingArea);
    gtk_box_pack_start(GTK_BOX(vbox), drawingArea,
            TRUE/*expand*/, TRUE/*fill*/, 0/*padding*/);
    GtkWidget *controlbar = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), controlbar, 
            FALSE/*expand*/, FALSE/*fill*/, 0/*padding*/);
    GtkWidget *statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vbox), statusbar,
            FALSE/*expand*/, FALSE/*fill*/, 0/*padding*/);

    if(!title || !title[0])
        title = "new tab";

    GtkWidget *tab = gtk_label_new(title);

    gtk_widget_set_events(drawingArea,
            gtk_widget_get_events(drawingArea)
                | GDK_LEAVE_NOTIFY_MASK
                | GDK_BUTTON_PRESS_MASK
                | GDK_POINTER_MOTION_MASK
                | GDK_POINTER_MOTION_HINT_MASK);

    g_signal_connect(drawingArea, "draw",
            G_CALLBACK(drawingArea_draw_cb), g);
    g_signal_connect(drawingArea,"configure-event",
            G_CALLBACK(drawingArea_configure_cb), g);


    g_object_set_data(G_OBJECT(vbox), "qsGraph", g);

    gtk_notebook_append_page(w->gtkNotebook, vbox, tab);

    g->tab = tab;
    g->vbox = vbox;
    g->drawingArea = drawingArea;
    g->controlbar = controlbar;
    g->statusbar = GTK_STATUSBAR(statusbar);

    g->controlbar_showing = true;
    g->statusbar_showing = true;

    gtk_widget_show_all(vbox);
WARN();
}

