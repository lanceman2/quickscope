
struct QsWindow {

    GtkWidget *menubar;   // Top Main Menu Bar
    GtkWidget *buttonbar; // H Box of the Main button bar

    // We keep our own state of "showing" because we don't know what the
    // fuck GTK is doing.  We just do not trust it.  Too many bugs.
    bool menubar_showing;
    bool buttonbar_showing;
    bool tabbar_showing;

    // gnore_showHide stops check_menu_item callback re-entrance that may
    // or may not happen by calling gtk_check_menu_item_set_active() to
    // change the check in the menu item.  Because we only get one
    // (pointer and keyboard) focus we only need one flag for all
    // GtkCheckMenuItem objects.  We can only effect them one at a time.
    bool ignore_showHide;

    GtkCheckMenuItem *showHideMenubar_item;
    GtkCheckMenuItem *showHideButtonbar_item;
    GtkCheckMenuItem *showHideTabbar_item;
    GtkCheckMenuItem *showHideControlbar_item;
    GtkCheckMenuItem *showHideStatusbar_item;

    // We need the window to not take the key entry events when
    // the graph entry widget needs the entry events.
    struct QsGraph *graphControlHasFocus;

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


// bit mask of modifier keys caught by the main window.
extern uint32_t mod_keys;
extern GdkCursor *hand_cursor;
extern gint screen_width, screen_height;

void CleanupCSS(void);
void AddCSS(void);


////////////////////////////////////////////////////////////
// Some callbacks.
//
extern void popZoom_cb(void);
extern void showHideMenubar_cb(void);   // toggle between show and hide
extern void showHideButtonbar_cb(void); // toggle between show and hide
extern void showHideTabbar_cb(void);    // toggle between show and hide
extern void showHideControlbar_cb(void);// toggle between show and hide
extern void showHideStatusbar_cb(void); // toggle between show and hide
extern void newTab_cb(void);
extern void newWindow_cb(void);
extern void closeTab_cb(void);
extern void closeWindow_cb(void);
extern void quit_cb(void);
