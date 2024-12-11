

struct QsGraph {

    GtkWidget *tab;
    GtkWidget *vbox;
    GtkWidget *drawingArea;

    cairo_surface_t *bgSurface; // background

    GtkWidget *controlbar; // gtk entry widget
    GtkStatusbar *statusbar;

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

