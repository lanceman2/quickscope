
struct QsZoom {

    struct QsZoom *prev, *next; // To keep a list of zooms

    double xMin, xMax, xSlope/*=(xMax - xMin)/pixWidth*/;
    double yMin, yMax, ySlope/*=(yMin - yMax)/pixHeight*/;
};

struct QsColor {

  double r, g, b;
};


#define SLIDE_BUTTON     1 /*1 -> left*/

#define SLIDE_ACTION    01  // sliding the graph with the pointer


struct QsGraph {

    GtkWidget *tab;
    GtkWidget *vbox;
    GtkWidget *drawingArea;

    cairo_surface_t *bgSurface; // background
    cairo_surface_t *fgSurface; // foreground with plots

    cairo_surface_t *zoomBoxSurface;

    GtkWidget *controlbar; // gtk entry widget
    GtkStatusbar *statusbar;

    double xMin, xMax, yMin, yMax;

    int width, height; // drawing area in pixels

    struct QsZoom *top;  // first zoom level
    struct QsZoom *zoom; // current zoom level

    struct QsColor bgColor, gridColor, subGridColor, axesLabelColor;

    // Mark that we are doing a thing.  Since there is just one window
    // focus we can use a global variable like this:
    uint32_t zoom_action;
    double x0, y0; // pointer position at the start of an zoom action
    double x, y; // pointer position in current zoom action

    bool controlbar_showing;
    bool statusbar_showing;
    bool haveZoomBox;
};


static inline struct QsGraph *GetCurrentGraph(void) {

    struct QsWindow *w = GetCurrentWindow();
    DASSERT(w->gtkNotebook);
    int page = gtk_notebook_get_current_page(w->gtkNotebook);
    DSPEW("page=%d", page);
    DASSERT(page >= 0);
    GtkWidget *vbox = gtk_notebook_get_nth_page(w->gtkNotebook, page);
    DASSERT(vbox);
    struct QsGraph *g = g_object_get_data(G_OBJECT(vbox), "qsGraph");
    DASSERT(g);
    DASSERT(g->vbox == vbox);
    return g;
}

extern void ShowTabPopupMenu(struct QsGraph *g, int x, int y);
extern void CreatePopoverMenu(void);
extern void CleanupTabPopupMenu(void);

static inline void SetColor(struct QsColor *c,
    double r, double g, double b) {
  c->r = r;
  c->g = g;
  c->b = b;
}

extern void DrawGrids(struct QsGraph *g, cairo_t *cr, bool show_subGrid);

extern gboolean graph_buttonPress_cb(GtkWidget *drawingArea,
        GdkEventButton *e, struct QsGraph *g);
extern gboolean graph_buttonRelease_cb(GtkWidget *drawingArea,
        GdkEventButton *e, struct QsGraph *g);
extern gboolean graph_pointerMotion_cb(GtkWidget *drawingArea,
        GdkEventMotion *e, struct QsGraph *g);
extern gboolean graph_pointerEnter_cb(GtkWidget *drawingArea,
        GdkEvent *e, struct QsGraph *g);
extern gboolean graph_pointerLeave_cb(GtkWidget *drawingArea,
        GdkEvent *e, struct QsGraph *g);

extern void FixZoomsShift(struct QsGraph *g, double dx, double dy);

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
