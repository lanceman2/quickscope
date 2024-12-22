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

void PrintStatusbar(struct QsGraph *g, double xPix, double yPix) {

    // Record the pointer plot x y values in the status bar:
    //guint id = gtk_statusbar_get_context_id(g->statusbar, "info");
    const size_t LEN = 64;
    char text[LEN];
    if(isnormal(xPix) && isnormal(yPix))
        snprintf(text, LEN, "zoom[%"PRIu32"] %g  %g", g->zoomCount,
                pixToX(xPix, g->zoom), pixToY(yPix, g->zoom));
    else
        snprintf(text, LEN, "zoom[%"PRIu32"]", g->zoomCount);

    gtk_statusbar_push(g->statusbar, 0, text);
}


// Return true if there are still zooms to pop.
//
// We do not pop (free) the last one; unless we are destroying the graph.
// We need to keep the last one so we can draw something.
//
static inline bool _PopZoom(struct QsGraph *g) {

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
    --g->zoomCount;
 
    DZMEM(z, sizeof(*z));
    free(z);

    return (g->zoom == g->top)?false:true;
}

static inline void DrawBgSurface(struct QsGraph *g) {

    DASSERT(g);
    DASSERT(g->bgSurface);

    cairo_t *cr = cairo_create(g->bgSurface);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, g->bgColor.r, g->bgColor.g, g->bgColor.b);
    cairo_paint(cr);
    DrawGrids(g, cr, true/*show subGrid*/);
    cairo_destroy(cr);
}


// Returns true if there are zooms left to pop after this call.
//
bool PopZoom(struct QsGraph *g) {

    if(g->zoom == g->top)
        // No need to redraw.
        return false;

    bool ret = _PopZoom(g);
    DrawBgSurface(g);
    PrintStatusbar(g, NAN, NAN);
    gtk_widget_queue_draw(g->drawingArea);
    return ret;
}

static void Destroy_cb(GtkWidget *widget, struct QsGraph *g) {

    DASSERT(widget);
    DASSERT(g);

    DSPEW("Freeing graph=%p", g);

    if(g->bgSurface) {
        cairo_surface_destroy(g->bgSurface);
        g->bgSurface = 0;
    }

    // Free the added zooms without queue redraws.
    if(g->zoom)
        while(_PopZoom(g));

    if(g->top) {
        // Free the last zoom too.
        DASSERT(g->top == g->zoom);
        DZMEM(g->top, sizeof(*g->top));
        free(g->top);
    }

    DZMEM(g, sizeof(*g));
    free(g);
}

static inline void _PushZoom(struct QsGraph *g,
        double xMin, double xMax, double yMin, double yMax) {

    struct QsZoom *z = malloc(sizeof(*z));
    ASSERT(z, "malloc(%zu) failed", sizeof(*z));

    z->xSlope = (xMax - xMin)/g->width;
    z->ySlope = (yMin - yMax)/g->height;
    z->xShift = xMin - g->padX * z->xSlope;
    z->yShift = yMax - g->padY * z->ySlope;

    // Add to the end of the list of zooms
    if(g->zoom) {
        DASSERT(g->top);
        // The current zoom (g->zoom) is always the last one.
        DASSERT(!g->zoom->next);
        g->zoom->next = z;
        z->prev = g->zoom;
        ++g->zoomCount;
    } else {
        DASSERT(!g->top);
        z->prev = 0;
        g->top = z;
    }
    z->next = 0;
    // The current zoom (g->zoom) is always the last one, is this one.
    g->zoom = z;
}


void PushZoom(struct QsGraph *g,
        double xMin, double xMax, double yMin, double yMax) {

    DASSERT(xMin < xMax);
    DASSERT(yMin < yMax);

    // Zooming can fail if the numbers get small relative difference
    // values:
    double rdx = fabs(2.0*(xMax - xMin)/(xMax + xMin));
    double rdy = fabs(2.0*(yMax - yMin)/(yMax + yMin));
    if(rdx < 1.0e-8 || rdy < 1.0e-8) {
        // The double has about 15 decimal digits precision, and we need
        // some precision (bits) to use for converting numbers so we can
        // plot them.
        NOTICE("Failed to make a new zoom due to "
                "lack of relative number differences.\n"
                "The relative difference in x and y about: "
                "%.3g and %.3g", rdx, rdy);
        gtk_widget_queue_draw(g->drawingArea);
        return;
    }

    _PushZoom(g, xMin, xMax, yMin, yMax);
    DrawBgSurface(g);
    PrintStatusbar(g, NAN, NAN);
    gtk_widget_queue_draw(g->drawingArea);
}


static inline
void GetPadding(int width, int height, int *padX, int *padY) {

    if(width < screen_width/3)
        *padX = width;
    else
        *padX = screen_width/3 + (width - screen_width/3)/4;

    if(height < screen_height/3)
        *padY = height;
    else
        *padY = screen_height/3 + (height - screen_height/3)/4;

    //DSPEW("                padX=%d  padY=%d", *padX, *padY);
}


// The width and/or height of the drawing Area changed and so must all the
// zoom scaling.  Create a zoom if there are none.
//
// This assumes that the displayed user values did not change, just the
// drawing area changed size; i.e. there is a bigger or smaller user view
// but the plots did not shift in x or y direction.
//
static inline void FixZoomsScale(struct QsGraph *g, int width, int height) {

    DASSERT(g);
    DASSERT(g->top);
    DASSERT(g->zoom);
    DASSERT(g->xMin < g->xMax);
    DASSERT(g->yMin < g->yMax);

    if(g->width == width && g->height == height)
        return;

    int padX, padY;
    // Get the new padding for the new width and height.
    GetPadding(width, height, &padX, &padY);


    struct QsZoom *z = g->top;
    z->xSlope = (g->xMax - g->xMin)/width;
    z->ySlope = (g->yMin - g->yMax)/height;
    z->xShift = g->xMin - padX * z->xSlope;
    z->yShift = g->yMax - padY * z->ySlope;

    for(z = z->next; z; z = z->next) {
        z->xShift += g->padX * z->xSlope;
        z->xSlope *= ((double) g->width)/((double) width);
        z->xShift -= padX * z->xSlope;

        z->yShift += g->padY * z->ySlope;
        z->ySlope *= ((double) g->height)/((double) height);
        z->yShift -= padY * z->ySlope;
    }

    g->width = width;
    g->height = height;
    g->padX = padX;
    g->padY = padY;
}


static gboolean drawingArea_configure_cb(GtkWidget *w,
        GdkEventConfigure *e, struct QsGraph *g) {

    DASSERT(g);
    DASSERT(g->drawingArea);
    DASSERT(w == GTK_WIDGET(g->drawingArea));

    //DSPEW();

    GtkAllocation a;
    gtk_widget_get_allocation(w, &a);

    // TODO: WTF
    if(a.width < 2 || a.height < 2)
        // This seems to happen before a more reasonable size comes.
        return TRUE;

    if(!g->top) {
        DASSERT(!g->zoom);
        // Make the first zoom.
        GetPadding(a.width, a.height, &g->padX, &g->padY);
        g->width = a.width;
        g->height = a.height;
        _PushZoom(g, g->xMin, g->xMax, g->yMin, g->yMax);
    } else {
        DASSERT(g->zoom);
        FixZoomsScale(g, a.width, a.height);
    }

    if(g->bgSurface)
        cairo_surface_destroy(g->bgSurface);


    g->bgSurface = gdk_window_create_similar_surface(
            gtk_widget_get_window(w),
            // I tried using CAIRO_CONTENT_COLOR_ALPHA and it's buggy as
            // hell.  Looks like the GNOME compositor is buggy.
            //CAIRO_CONTENT_COLOR_ALPHA,
            CAIRO_CONTENT_COLOR,
            g->width + 2 * g->padX, g->height + 2 * g->padY);
    DASSERT(g->bgSurface);


    DrawBgSurface(g);

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
    cairo_set_source_surface(cr, g->bgSurface,
            - g->padX + g->slideX, - g->padY + g->slideY);
    cairo_paint(cr);

    if(g->zoom_action & BOX_ACTION) {
        cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_rectangle(cr, g->x0, g->y0, g->x - g->x0, g->y - g->y0);
        cairo_fill(cr);
    }

    return TRUE;
}


// Add a new graph and tab to the windows notebook
//
void AddNewGraph(struct QsWindow *w, const char *title) {

    DASSERT(w);

    struct QsGraph *g = calloc(1, sizeof(*g));
    ASSERT(g, "calloc(1,%zu) failed", sizeof(*g));

    g->window = w;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0/*spacing*/);
    g_signal_connect(vbox, "destroy", G_CALLBACK(Destroy_cb), g);
    gtk_widget_set_name(vbox, "vbox");

    GtkWidget *drawingArea = gtk_drawing_area_new();
    DASSERT(drawingArea);
    gtk_box_pack_start(GTK_BOX(vbox), drawingArea,
            TRUE/*expand*/, TRUE/*fill*/, 0/*padding*/);
    GtkWidget *controlbar = CreateGraphControl(g);
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
                | GDK_BUTTON_RELEASE_MASK
                | GDK_POINTER_MOTION_MASK
                | GDK_POINTER_MOTION_HINT_MASK
                | GDK_ENTER_NOTIFY_MASK
                | GDK_LEAVE_NOTIFY_MASK);

    g_signal_connect(drawingArea, "button-press-event",
            G_CALLBACK(graph_buttonPress_cb), g);
    g_signal_connect(drawingArea, "button-release-event",
            G_CALLBACK(graph_buttonRelease_cb), g);
    g_signal_connect(drawingArea, "motion-notify-event",
            G_CALLBACK(graph_pointerMotion_cb), g);
    g_signal_connect(drawingArea, "enter-notify-event",
            G_CALLBACK(graph_pointerEnter_cb), g);
    g_signal_connect(drawingArea, "leave-notify-event",
            G_CALLBACK(graph_pointerLeave_cb), g);

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
#if 0
    // Setup graph/plot scale to start with:
    g->xMin = -2.24e3;
    g->xMax = 1.3e+3;
    g->yMin = -1.0e-2;
    g->yMax = 1.0e-2;
#else
    g->xMin = -0.02;
    g->xMax = 1.02;
    g->yMin = -1.02;
    g->yMax = 1.02;
#endif

    // set default colors
    SetRGB(&g->bgColor, 0, 0, 0);
    SetRGB(&g->gridColor, 0, 0.8, 0.1);
    SetRGB(&g->subGridColor, 0.4, 0.4, 0.6);
    SetRGB(&g->axesLabelColor, 0.99, 0.99, 0.99);

    gint n = gtk_notebook_get_current_page(w->gtkNotebook);
    ++n;
    gtk_notebook_insert_page(w->gtkNotebook, vbox, tab, n);
    // GTK will not switch to the page if it is not showing yet.
    gtk_widget_show_all(vbox);
    gtk_notebook_set_current_page(w->gtkNotebook, n);
}
