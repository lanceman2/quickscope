
struct QsWindow {

    //GActionGroup *actions;
    //GtkAccelGroup *accel_group;

    GtkWidget *menubar;   // Top Main Menu Bar
    GtkWidget *buttonbar; // H Box of the Main button bar

    // We keep our own state of "showing" because we don't know what the
    // fuck GTK is doing.  We just do not trust it.  Too many bugs.
    bool menubar_showing;
    bool buttonbar_showing;
    // gnore_showHide stops check_menu_item callback re-entrance.
    bool ignore_showHide;

    GtkCheckMenuItem *showMenubar_item;
    GtkCheckMenuItem *showButtonbar_item;

    GtkWindow *gtkWindow;
    GtkNotebook *gtkNotebook;

    // It unclear whither or not these, actions and accel_group, should be
    // here of just have a single instance of these for the app.  It seems
    // to work better here, but it could be because this
    // action/accelerator stuff is buggy as hell.  It may look like having
    // an app level version of these object works, until you hit a
    // combination of actions and main windows that fuck it.
    //
    // So ya, we have a action group and accelerator group for each main
    // window; even though that does not appear to be the intended way it
    // is supposed to be done.  Again working around GTK bugs here:
    GActionGroup *actions;
    GtkAccelGroup *accel_group;
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

