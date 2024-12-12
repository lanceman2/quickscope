#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


// We would tend not to limit the number of app objects the user can make,
// but GTK limits it to one at a time.  It's the same shit in Qt.

bool looping = false;
static bool needCleanup = false;

uint32_t windowCount = 0;
GtkApplication *app = 0;


static
void Cleanup(void) {

    DASSERT(!looping);
    DASSERT(app);

    CleanupCSS();

    //CleanupTabPopupMenu();

    // It looks like I need to destroy the main windows because destroying
    // the GtkApplication does not destroy the main windows; WTF.
    GList *l = gtk_application_get_windows(app);
    // The GTK4 docs say that we do not own this GList; so I expect that
    // means that the GtkApplication will own/free it.
    while(l) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
        // Loop until no windows remain.
        l = gtk_application_get_windows(app);
    }

    // If things are consistent this window counter should be zero.
    DASSERT(windowCount == 0);

    g_object_unref(app);
    app = 0;

    needCleanup = false;

    DSPEW();
}


static void startup_cb (GApplication *application) {

    DASSERT(app);
    DASSERT(G_APPLICATION(app) == application);

    CreatePopoverMenu();
    AddActions(0, 0);
}


// Calling this can be automatic.
//
void qsApp_create(int argc, char **argv, GtkApplication *a) {

    DSPEW();


    // TODO: Maybe do nothing but parse args if there are any.
    // Or maybe it's best to not allow a useless case:
    ASSERT(!app);

    if(a)
        app = a;
    else if(!app) {
        // This calls gtk_init() if it needs to be called.
        app = gtk_application_new("org.gtk.quickscope",
                G_APPLICATION_DEFAULT_FLAGS);
        ASSERT(app);
    }

    g_signal_connect(app, "activate", G_CALLBACK(startup_cb), 0);

    // TODO: Parse arguments if they are non-zero.
    if(argv && argc > 0)
        WARN("Add argument parsing code here");
}


void qsApp_destroy(void) {

    // The underlying GTK g main loop object can't be destroyed (pisses me
    // off to no end).  The GTK Application object can be destroyed, but
    // it's kind-of misleading as to what is really destroyed (cleaned
    // up).  I wish GTK was robust, but it is not.  Neither is Qt.

    if(looping) {
        // We are in the qsApp_run() call.
        needCleanup = true;
        g_application_quit(G_APPLICATION(app));
    } else
        Cleanup();
}


int qsApp_run(int argc, char **argv) {

    GetApp();

    // We do not let this function call itself.
    DASSERT(!looping);

    if(!windowCount) {
        // We need at least one main window so we can run.
        INFO("Adding a main window");
        qsWindow_create();
    }
DSPEW();

    looping = true;
    int status = g_application_run(G_APPLICATION(app), 0, 0);
    looping = false;

    if(needCleanup)
        Cleanup();
    else {
        // Looks like a GtkApplication can only run once, so we must make
        // sure we do not run it again by destroying it now.
        g_object_unref(app);
        app = 0;
    }

    return status;
}
