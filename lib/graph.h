
// The major grid lines get labeled with numbers.
//
// The closer together minor (sub) grid lines do not not get labeled with
// numbers.  There's an unusual case where grid line number labels can get
// large like for example 6.75684e-05 and the next grid line number is
// like 6.75685e-05; note the relative difference is 1.0e-10. Math round
// off errors start to kill the "plotting" at that point.
//
// TODO: We should consider using 
#define PIXELS_PER_MAJOR_GRID_LINE   (160)

#define SLIDE_BUTTON     1 /*1 -> left*/
#define BOX_BUTTON       3 /*3 -> right*/


#define SLIDE_ACTION    01  // sliding the graph with the pointer
#define BOX_ACTION      02  // sliding the graph with the pointer


enum QsChannelType {

    // These are things that can be plotted in a graph.
    QsChannelType_buffer = 0, // static buffer of values
    QsChannelType_stream,     // a sequence of values that flow
    QsChannelType_function    // 
};


struct QsSource {

    struct QsSource *prev, *next;
    // An array of channels:
    struct QsChannel *channels;
    uint32_t numChannels;
};


struct QsChannel {

    enum QsChannelType type;
    struct QsSource *source;
    uint32_t num; // index into the source channel array.
};


struct QsBuffer {

    // inherit channel
    struct QsChannel channel;

    double *values;
    size_t numValues;
};


struct QsFunction {

    // inherit channel
    struct QsChannel channel;
};

struct QsStream {

    // inherit channel
    struct QsChannel channel;
};


struct QsPlot {

    struct QsChannel *x, *y;
};




struct QsZoom {

    struct QsZoom *prev, *next; // To keep a list of zooms

    double xSlope/* = (xMax - xMin)/pixWidth */;
    double ySlope/* = (yMin - yMax)/pixHeight */;
    double xShift/* = xMin - g->padX * z->xSlope */;
    double yShift/* = yMax - g->padY * z->ySlope */;
};


struct QsColor {
  double r, g, b;
};


struct QsGraph {

    // Array of plots:
    struct QsPlot *plots;
    uint32_t numPlots;

    // Having this, window, should be unnecessary, but we do not want to
    // count on GTK not having events coming out of order, like for
    // example the focus changes to one window while another is still
    // processing a key press event.  We do use "the current window" in
    // some parts of the code, but not if there could be an event race
    // condition that could mix graphs with the wrong window.
    struct QsWindow *window;

    GtkWidget *tab;
    GtkWidget *vbox;
    GtkWidget *drawingArea;

    cairo_surface_t *bgSurface; // background with grid lines and labels
    cairo_surface_t *fgSurface; // foreground with plots

    GtkWidget *controlbar; // gtk entry widget
    GtkStatusbar *statusbar;

    // User values on the edges of the drawing area.
    double xMin, xMax, yMin, yMax;

    // Mark that we are in the process of doing a zoom thing.  Since there
    // is just one window focus we can use a global variable for this, but
    // a GTK bug forces this variable to not be reused between graphs.  We
    // tried making this global, but there are race conditions in GTK
    // handling point/mouse events which caused mouse focus fuck ups.
    // Windows got focus events out of order with the button release
    // events.
    uint32_t zoom_action;
    // mouse pointer positions at a zoom action.
    double x0, y0, x, y;

    // padX, padY is padding size added to drawingArea to make bgSuface
    // and fgSurface.  The padding is added to each of the 4 sides of
    // bgSuface and fgSurface.  The width and height of bgSuface and
    // fgSurface is width + 2 * padX and height + 2 * padY.
    //
    int padX, padY;
    // width and height of drawingArea widget.
    int width, height;

    struct QsZoom *top;  // first zoom level
    struct QsZoom *zoom; // current zoom level

    struct QsColor bgColor, gridColor, subGridColor, axesLabelColor;

    uint32_t zoomCount;
      
    // slideX and slideY are set when the graph is being moved via the
    // left (1) button press and pointer motion.  They go to zero when the
    // graph slide action is finished, the graph zooms are
    // recalculated, and the graph is redrawn.
    double slideX, slideY;

    bool controlbar_showing;
    bool statusbar_showing;
};


static inline struct QsGraph *GetCurrentGraph(void) {

    struct QsWindow *w = GetCurrentWindow();
    DASSERT(w->gtkNotebook);
    int page = gtk_notebook_get_current_page(w->gtkNotebook);
    //DSPEW("page=%d", page);
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

static inline void SetRGB(struct QsColor *c,
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


extern void PushZoom(struct QsGraph *g,
        double xMin, double xMax, double yMin, double yMax);
// Returns true if there are zooms left to pop after this call.
extern bool PopZoom(struct QsGraph *g);
extern void PrintStatusbar(struct QsGraph *g, double xPix, double yPix);

extern GtkWidget *CreateGraphControl(struct QsGraph *g);

// TODO: Can we use SIMD parallel processing for some of this
// arithmetic?


// These are mappings to (and from) pixels on the bgSurface and fgSurface.
// The bgSurface and fgSurface are padded to a larger size than the size
// of the drawing area, by plus padX to either size in X and plus padY to
// either side in Y.
//
static inline double xToPix(double x, struct QsZoom *z) {
    return (x - z->xShift)/z->xSlope;
}
//
static inline double pixToX(double p, struct QsZoom *z) {
    return p * z->xSlope + z->xShift;
}
//
static inline double yToPix(double y, struct QsZoom *z) {
    return (y - z->yShift)/z->ySlope;
}
//
static inline double pixToY(double p, struct QsZoom *z) {
    return p * z->ySlope + z->yShift;
}



extern void qsPlot_restart(struct QsPlot *p);

extern void qsPlot_getMinMax(struct QsPlot *p, double *min, double *max);

extern double *qsPlot_getNext(struct QsPlot *p);


extern void qsGraph_plot(struct QsGraph *g);

