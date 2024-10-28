#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../include/quickscope.h"

#include "debug.h"

#include "display.h"




static void __attribute__((constructor)) constructor(void) {

    DSPEW();
}


static void __attribute__((destructor)) destructor(void) {

    while(lastDisplay)
        qsDisplay_destroy(lastDisplay);

    DSPEW("libquickscope.so done");
}

