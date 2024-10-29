

struct QsWindow;

// display is a top level object for the libquickscope.so library.
// We can make any number of displays; because why not.
//
// display is a container for windows (Main GTK Windows).
//
struct QsDisplay {

    // We keep a linked list of all displays so that we can clean them up.
    struct QsDisplay *prev, *next;

    // a list of GTK main windows
    struct QsWindow *firstWindow, *lastWindow;
};


extern struct QsDisplay *lastDisplay;

