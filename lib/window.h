
struct QsWindow {

    struct QsWindow *prev, *next;

    // display is the owner of this window thingy.
    struct QsDisplay *display;

    GtkWindow *gtkWindow;
};
