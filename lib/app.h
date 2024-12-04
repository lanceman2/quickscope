
struct QsWindow {

    // The GActionGroup callbacks are called from a more "global"
    // perspective (scope) and do not have user_data pointer arguments passed
    // that point to specific objects; so we need to put pointers
    // here in this structure which we can get access to in the code with
    // the function gtk_application_get_active_window(app) where we
    // set the GtkWindow user data via the call:
    // g_object_set_data(G_OBJECT(gtkWindow), "qsWindow", w)
    // where w is a pointer to this structure.
    //

    GtkWindow *gtkWindow;

    GtkNotebook *gtkNotebook;

    GtkStatusbar *gtkStatusbar;
};


extern void AddNewGraph(struct QsWindow *w, const char *title);


extern GtkApplication *app;


static inline void GetApp(void) {

    if(!app)
        qsApp_create(0, 0, 0);
    DASSERT(app);
}

extern uint32_t windowCount;

extern void MakeGtkActionGroup(void);
extern void AddActions(GtkWidget *w);


extern bool looping;

extern GtkBuilder *Get_builder_from_file(const char *filename);

// Get the quicksocpe thingys that are currently in focus:
//
static inline GtkWindow *GetCurrentGtkWindow(void) {

    DASSERT(app);
    GtkWindow *gtkWindow = gtk_application_get_active_window(app);
    DASSERT(gtkWindow);
    return gtkWindow;
}
static inline struct QsWindow *GetCurrentWindow(void) {

    GtkWindow *gtkWindow = GetCurrentGtkWindow();
    struct QsWindow *w = g_object_get_data(G_OBJECT(gtkWindow), "qsWindow");
    DASSERT(w);
    return w;
}


void NewGraphTab_cb(void);

