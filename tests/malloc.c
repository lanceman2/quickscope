#include <stdlib.h>

#include "../lib/debug.h"

int main(void) {

    void *ptr = malloc(1);
    ASSERT(ptr, "malloc(1) failed");

#ifndef NO_FREE
    free(ptr);
#endif

    return 0;
}
