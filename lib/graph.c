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



#if 0
static void pressed_cb(GtkGestureClick *gesture,
        int n_press, double x, double y, struct QsGraph *g) {

    WARN("  n_press=%d  x,y=%lg,%lg", n_press, x, y);

    ShowTabPopupMenu(g, (int)(x + 0.5), (int)(y + 0.5));
}
#endif

// Return true if there are still zooms to pop.
//
// We do not Pop (free) the last one; unless we are destroying the graph.
// We need to keep the last one so we can draw something.
//
static inline bool PopZoom(struct QsGraph *g) {

    DASSERT(g->zoom);
    DASSERT(g->top);

    if(!g->zoom) {
        DASSERT(!g->top);
        return false;
    }

    // g->zoom is always the last zoom.
    DASSERT(!g->zoom->next);

    if(g->zoom == g->top)
        return false;

    DASSERT(g->zoom->prev);

    struct QsZoom *z = g->zoom;
    g->zoom = z->prev;
    g->zoom->next = 0;
 
    DZMEM(z, sizeof(*z));
    free(z);

    return (g->zoom == g->top)?false:true;
}

static void Destroy_cb(GtkWidget *widget, struct QsGraph *g) {

    DASSERT(widget);
    DASSERT(g);

    DSPEW("Freeing graph=%p", g);

    if(g->bgSurface) {
        cairo_surface_destroy(g->bgSurface);
        g->bgSurface = 0;
    }

    // Free all the zooms.
    while(PopZoom(g));

    if(g->top) {
        // Free the last zoom too.
        DASSERT(g->top == g->zoom);
        DZMEM(g->top, sizeof(*g->top));
        free(g->top);
    }

    DZMEM(g, sizeof(*g));
    free(g);
}


static struct QsZoom *PushZoom(struct QsGraph *g,
        double xMin, double xMax, double yMin, double yMax) {

    struct QsZoom *z = malloc(sizeof(*z));
    ASSERT(z, "malloc(%zu) failed", sizeof(*z));

    z->xMin = xMin;
    z->xMax = xMax;
    z->yMin = yMin;
    z->yMax = yMax;
    z->xSlope = (xMax - xMin)/g->width;
    z->ySlope = (yMin - yMax)/g->height;

    // Add to the end of the list of zooms
    if(g->zoom) {
        DASSERT(g->top);
        // The current zoom (g->zoom) is always the last one.
        DASSERT(!g->zoom->next);
        g->zoom->next = z;
        z->prev = g->zoom;
    } else {
        DASSERT(!g->top);
        z->prev = 0;
        g->top = z;
    }
    z->next = 0;
    // The current zoom (g->zoom) is always the last one, is this one.
    g->zoom = z;

    return z;
}


// The width and/or height of the drawing Area changed and so must all the
// zoom scaling.  Create a zoom if there are none.
//
static inline void FixZooms(struct QsGraph *g, int width, int height) {

    DASSERT(g);
    DASSERT(width>=1);
    DASSERT(height>=1);
    DASSERT(g->xMin < g->xMax);
    DASSERT(g->yMin < g->yMax);

    bool wh_change = (g->width != width || g->height != height);

    if(wh_change) {
        g->width = width;
        g->height = height;
    }

    if(g->top && wh_change) {
        DASSERT(g->zoom);
        for(struct QsZoom *z = g->top; z; z = z->next) {
            z->xSlope = (z->xMax - z->xMin)/width;
            z->ySlope = (z->yMin - z->yMax)/height;
        }
    }

    if(!g->top) {
        DASSERT(!g->zoom);
        PushZoom(g, g->xMin, g->xMax, g->yMin, g->yMax);
    }

    DASSERT(g->top);
    DASSERT(g->zoom);
}


static gboolean drawingArea_configure_cb(GtkWidget *w,
        GdkEventConfigure *e, struct QsGraph *g) {

    DASSERT(g);
    DASSERT(g->drawingArea);
    DASSERT(w == GTK_WIDGET(g->drawingArea));

    //DSPEW();

    GtkAllocation a;
    gtk_widget_get_allocation(w, &a);
    FixZooms(g, a.width, a.height);

    if(g->bgSurface)
        cairo_surface_destroy(g->bgSurface);

    g->bgSurface = gdk_window_create_similar_surface(
            gtk_widget_get_window(w),
            // I tried using CAIRO_CONTENT_COLOR_ALPHA and it's buggy as
            // hell.  Looks like the GNOME compositor is buggy.
            //CAIRO_CONTENT_COLOR_ALPHA,
            CAIRO_CONTENT_COLOR,
            g->width, g->height);
    DASSERT(g->bgSurface);

    cairo_t *cr = cairo_create(g->bgSurface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, g->bgColor.r, g->bgColor.g, g->bgColor.b);
    cairo_paint(cr);

    DrawGrids(g, cr, true/*show subGrid*/);

    cairo_destroy(cr);

    return TRUE; // TRUE == done processing event
}


static gboolean drawingArea_draw_cb(GtkWidget *w,
        cairo_t *cr, struct QsGraph *g) {
    DASSERT(w);
    DASSERT(cr);
    DASSERT(g);
    DASSERT(g->bgSurface);
    DASSERT(w == GTK_WIDGET(g->drawingArea));

    // There seems to be far to many draw calls
    //DSPEW();

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

    g->tab = tab;
    g->vbox = vbox;
    g->drawingArea = drawingArea;
    g->controlbar = controlbar;
    g->statusbar = GTK_STATUSBAR(statusbar);

    g->controlbar_showing = true;
    g->statusbar_showing = true;


    // TODO: Find the extreme values that may be plotted.
    //
    // Setup graph/plot scale to start with:
    g->xMin = -2.24e5;
    g->xMax = 1.3e+5;
    g->yMin = -2.0e-1;
    g->yMax = 1.0e-1;

    // set default colors
    SetColor(&g->bgColor, 0, 0, 0);
    SetColor(&g->gridColor, 0, 0.8, 0.1);
    SetColor(&g->subGridColor, 0.4, 0.4, 0.6);
    SetColor(&g->axesLabelColor, 0.99, 0.99, 0.99);

    // Do this after the graph data is setup so we know how to draw.
    gtk_widget_show_all(vbox);
    gtk_notebook_append_page(w->gtkNotebook, vbox, tab);
}
