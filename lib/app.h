
struct QsWindow {

    //GActionGroup *actions;
    //GtkAccelGroup *accel_group;

    GtkWidget *menubar;   // Top Main Menu Bar
    GtkWidget *buttonbar; // H Box of the Main button bar

    GtkCheckMenuItem *showMenubar_item;
    GtkCheckMenuItem *showButtonbar_item;

    GtkWindow *gtkWindow;
    GtkNotebook *gtkNotebook;
};


extern void AddNewGraph(struct QsWindow *w, const char *title);


extern GtkApplication *app;


static inline void GetApp(void) {

    if(!app)
        qsApp_create(0, 0, 0);
    DASSERT(app);
}

extern uint32_t windowCount;

extern void AddActions(struct QsWindow *w, GtkBuilder *builder);
extern void FreeActions(struct QsWindow *window);
extern void FreeAccelGroup(void);

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
    struct QsWindow *w = g_object_get_data(G_OBJECT(gtkWindow),
            "qsWindow");
    DASSERT(w);
    return w;
}


void NewGraphTab_cb(void);

