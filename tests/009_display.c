#include <quickscope.h>
#include <quickscope_debug.h>


int main(void) {

    // Make and destroy a few quickscope displays.
    //
    qsDisplay_create();
    struct QsDisplay *d1 = qsDisplay_create();
    qsDisplay_create();
    qsDisplay_create();
    qsDisplay_destroy(d1);
    d1 = qsDisplay_create();
    qsDisplay_destroy(d1);
    qsDisplay_create();
    d1 = qsDisplay_create();
    struct QsDisplay *d2 = qsDisplay_create();
    qsDisplay_create();
    qsDisplay_destroy(d2);
    qsDisplay_create();
    qsDisplay_create();
    qsDisplay_destroy(d1);
    qsDisplay_create();

    // The libquickstream.so library destructor should cleanup any
    // remaining displays.

    return 0;
}

