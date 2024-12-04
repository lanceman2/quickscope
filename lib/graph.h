

struct QsGraph {

    void *data;
    GtkWidget *tab;
    //GtkWidget *drawingArea;
    //GtkWidget *entry;
    //GtkStatusbar *statusbar;
    //GtkGesture *gesture;
};


static inline struct QsGraph *GetCurrentGraph(void) {

    struct QsWindow *w = GetCurrentWindow();
    DASSERT(w->gtkNotebook);
    int page = gtk_notebook_get_current_page(w->gtkNotebook);
    DASSERT(page >= 0);
    GtkWidget *widget = gtk_notebook_get_nth_page(w->gtkNotebook, page);
    DASSERT(widget);
    struct QsGraph *g = g_object_get_data(G_OBJECT(widget), "qsGraph");
    DASSERT(g);
    return g;
}

extern void ShowTabPopupMenu(struct QsGraph *g, int x, int y);
extern void CreatePopoverMenu(void);
extern void CleanupTabPopupMenu(void);

