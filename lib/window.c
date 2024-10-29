#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"

#include "display.h"
#include "window.h"


struct QsWindow *qsWindow_create(struct QsDisplay *d) {

    DSPEW();

    if(!d) {
        if(!lastDisplay)
            d = qsDisplay_create();
        else
            d = lastDisplay;
    }


    struct QsWindow *w = calloc(1, sizeof(*w));
    ASSERT(w, "calloc(1,%zu) failed", sizeof(*w));

    w->display = d;


    //////////////////////////////////////////////////////////////
    // Add this new main window to the list of main windows in the
    // display as the lastWindow.
    //
    if(d->lastWindow) {
        DASSERT(!d->lastWindow->next);
        DASSERT(d->firstWindow);
        DASSERT(!d->firstWindow->prev);
        d->lastWindow->next = w;
        w->prev = d->lastWindow;
    } else {
        DASSERT(!d->firstWindow);
        d->firstWindow = w;
    }
    d->lastWindow = w;

    return w;
}

void qsWindow_destroy(struct QsWindow *w) {

    DASSERT(w);
    struct QsDisplay *d = w->display;
    DASSERT(d);


    ////////////////////////////////////////////////////
    // Remove this window from the display windows list.
    //
    if(w->prev) {
        ASSERT(d->firstWindow != w);
        w->prev->next = w->next;
    } else {
        DASSERT(d->firstWindow == w);
        d->firstWindow = w->next;
    }
    //
    if(w->next) {
        DASSERT(d->lastWindow != w);
        w->next->prev = w->prev;
    } else {
        DASSERT(d->lastWindow == w);
        d->lastWindow = w->prev;
    }

    DZMEM(w, sizeof(*w));
    free(w);

    DSPEW();
}

