#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"


#include "display.h"
#include "window.h"


/** The list of all displays.   lastDisplay points to the last one that
 was created.
*/
struct QsDisplay *lastDisplay = 0;


struct QsDisplay *qsDisplay_create(void) {

    DSPEW();

    struct QsDisplay *d = calloc(1, sizeof(*d));
    ASSERT(d, "calloc(1,%zu) failed", sizeof(*d));

    ///////////////////////////////////////////////////////////////
    // Add this new display to the list of displays at lastDisplay,
    //
    d->prev = lastDisplay;

    if(lastDisplay) {
        DASSERT(!lastDisplay->next);
        lastDisplay->next = d;
    }
    lastDisplay = d;

    return d;
}

void qsDisplay_destroy(struct QsDisplay *d) {

    DASSERT(d);
    DASSERT(lastDisplay);
    DASSERT(!lastDisplay->next);

    /////////////////////////////////////////////////////
    // Destroy all the windows owned by this display.
    while(d->lastWindow)
        qsWindow_destroy(d->lastWindow);


    /////////////////////////////////////////////////////
    // Remove this display from the displays list.
    //
    if(d->prev) {
        d->prev->next = d->next;
    }
    // else we do not keep the first in the list.
    //
    if(d->next) {
        DASSERT(lastDisplay != d);
        d->next->prev = d->prev;
    } else {
        DASSERT(lastDisplay == d);
        lastDisplay = d->prev;
    }

    DZMEM(d, sizeof(*d));
    free(d);

    DSPEW();
}

