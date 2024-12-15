
struct QsZoom {

    struct QsZoom *prev, *next; // To keep a list of zooms

    double xMin, xMax, xSlope/*=(xMax - xMin)/pixWidth*/;
    double yMin, yMax, ySlope/*=(yMin - yMax)/pixHeight*/;
};

struct QsColor {

  double r, g, b;
};

struct QsGraph {

    GtkWidget *tab;
    GtkWidget *vbox;
    GtkWidget *drawingArea;

    cairo_surface_t *bgSurface; // background

    GtkWidget *controlbar; // gtk entry widget
    GtkStatusbar *statusbar;

    double xMin, xMax, yMin, yMax;

    int width, height; // drawing area in pixels

    struct QsZoom *top;  // first zoom level
    struct QsZoom *zoom; // current zoom level

    struct QsColor bgColor, gridColor, subGridColor, axesLabelColor;

    bool controlbar_showing;
    bool statusbar_showing;
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
