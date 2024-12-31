
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "../lib/debug.h"


int main(void) {

    for(uint32_t i=0; i<1000; ++i) {
        double x = i * M_PI/100.0;
        double y = sin(x);

        printf("%18.18g\n", y);
    }

    return 0;
}
