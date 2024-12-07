
#include "../include/quickscope.h"


int main(void) {

    // This is optional:
    //qsApp_create(0, 0, 0);

    // Creating extra windows is optional:
    //qsWindow_create();
    //qsWindow_create();

    // If you do not call this you are not going to see any windows:
    //qsApp_run(0, 0);


    // TODO: When libquickscope.so is stable we'll want to add tests that
    // restart the "app" like so:  I just tested it and it works now (2024
    // Nov 12).
    //
    qsApp_run(0, 0);
    //qsApp_run(0, 0);


    return 0;
}
