

// display is a top level object for the libquickscope.so library.
//
struct QsDisplay {

    // We keep a linked list of all displays so that we can clean them up.
    struct QsDisplay *prev, *next;

};


extern struct QsDisplay *lastDisplay;


