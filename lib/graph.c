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


// TODO: Can we use SIMD parallel processing for all this arithmetic?

static inline double xToPix(double x, struct QsZoom *z) {
    return (x - z->xMin) / z->xSlope;
}

static inline double pixToX(double p, struct QsZoom *z) {
    return p * z->xSlope + z->xMin;
}

static inline double yToPix(double y, struct QsZoom *z) {
    return (y - z->yMax) / z->ySlope;
}

static inline double pixToY(double p, struct QsZoom *z) {
    return p * z->ySlope + z->yMax;
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
            z->ySlope = (z->xMin - z->xMax)/height;
        }
    }

    if(!g->top) {
        DASSERT(!g->zoom);
        PushZoom(g, g->xMin, g->xMax, g->yMin, g->yMax);
    }

    DASSERT(g->top);
    DASSERT(g->zoom);
}

static inline double Trim(double x) {

    // TODO: This could be rewritten by using an understanding of floating
    // point representation.  In this writing we did not even try to
    // understand how floating point numbers are stored, we just used math
    // functions that do the right thing.

    ASSERT(x > 0.0);
    ASSERT(isnormal(x));
    ASSERT(x < DBL_MAX/8.0); // max is about 1.0e308

    double pow = log10(x);
    // x = 10^pow
    int32_t p;
    if(pow > 0.0)
        p = (pow + 0.5);
    else
        p = (pow - 0.5);
    // x ~= 10^p  example if pow ==  1.3  p = 1
    //                    if pow == -1.3  p = -1
    double tenPow = exp10((double) p);
    // I'm not sure how the mantissa is defined, but I don't care.  I just
    // have that:  
    //
    //       x = mant * 10 ^ p  [very close]
    //
    double mant = x/tenPow;

    while(mant < 1.0) {
        mant *= 10.0;
        --p;
    }

    // Ya, it better be that this is so:
    DASSERT(x < 1.000001 * mant * exp10(p));
    DASSERT(x > 0.999999 * mant * exp10(p));

    //DSPEW("x = %lg = %lf * 10 ^(%d)", x, mant, p);

    // Now trim digits off of mant.  Like for example:
    //
    //    1.1234 => 2.0   or  4.6234 => 5.0

    uint32_t ix = (uint32_t) mant;


    // We make it larger, not smaller.  Just certain values look good in
    // plot grid lines.
    //
    switch(ix) {
        case 9:
        case 8:
        case 7:
        case 6:
        case 5:
            ix = 10;
            break;
        case 4:
        case 3:
        case 2:
            ix = 5;
            break;
        case 1:
            ix = 2;
            break;
        case 0:
            ASSERT(0, "Bad Code");
            break;
    }

    x = ix * exp10(p);

    DSPEW("x = %lg <= %" PRIu32" * 10 ^(%d)", x, ix, p);

    return x;
}


static inline void DrawVGrid(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        double pixelSpace/*minimum pixels between lines*/,
        struct QsZoom *z, double width, double height) {

    DASSERT(lineWidth <= pixelSpace);
    ASSERT(z->xMin < z->xMax, "%lg < %lg", z->xMin, z->xMax);

    double deltaX = (z->xMax - z->xMin) * pixelSpace/width;
    deltaX = Trim(deltaX);

    DSPEW("V grid spacing is %lg pixels (> %lg)",
            deltaX * width /  (z->xMax - z->xMin), pixelSpace);

    double start = pixToX(0, z);

    double end = pixToX(width, z) + deltaX;
    int i = 0;

    cairo_set_line_width(cr, lineWidth);

    for(double x=start; x<=end; ++i, x=i*deltaX) {
        double pix = xToPix(x, z);
        cairo_move_to(cr, pix, 0);
        cairo_line_to(cr, pix , height);
        cairo_stroke(cr);
    }
}

static gboolean drawingArea_configure_cb(GtkWidget *w,
        GdkEventConfigure *e, struct QsGraph *g) {

    DASSERT(g);
    DASSERT(g->drawingArea);
    DASSERT(w == GTK_WIDGET(g->drawingArea));

    DSPEW();

    GtkAllocation a;
    gtk_widget_get_allocation(w, &a);
    FixZooms(g, a.width, a.height);

    if(g->bgSurface)
        cairo_surface_destroy(g->bgSurface);

    g->bgSurface = gdk_window_create_similar_surface(
            gtk_widget_get_window(w),
            // I tried using CAIRO_CONTENT_COLOR_ALPHA and it's buggy as
            // hell.  Looks like the GNOME compositor is buggy as fuck.
            //CAIRO_CONTENT_COLOR_ALPHA,
            CAIRO_CONTENT_COLOR,
            a.width, a.height);
    DASSERT(g->bgSurface);



    cairo_t *cr = cairo_create(g->bgSurface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 1, 0, 0);
    DrawVGrid(cr, 0.7, 30, g->zoom, g->width, g->height);

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
    g->xMin = 0;
    g->xMax = 1;
    g->yMin = 0;
    g->yMax = 1;


    // Do this after the graph data is setup so we know how to draw.
    gtk_widget_show_all(vbox);
    gtk_notebook_append_page(w->gtkNotebook, vbox, tab);
}

