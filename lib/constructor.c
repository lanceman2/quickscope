#define _GNU_SOURCE
#include <link.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"

#include "app.h"

#define DIRSTR  "/"
#define DIRCHR  ('/')


// GTK resource_base_path
const char *qsResourceDir = 0;


// This gets called with different DSO (dynamic shared object) library
// filenames.  It is hoped that one of the calls will be with a path
// ending in /lib/libquickscope.so which is the file that has this code.
//
// If there happens to be another library ending with that path we may
// have a problem.
//
// We wish to have the gtk builder files associated with the library
// and not the binary program.
//
static int dl_callback(struct dl_phdr_info *info,
                        size_t size, void *data) {

    // We use the system dl_iterate_phdr() call to get the path to
    // the library that this code is running from and than go back
    // two directories and add "/share/quickscope/run" to that
    // to be the directory that has gtkbuilder files in it.

    const char *LibName = "/lib/libquickscope.so";
    // If someone changes the DSO (dynamic shared object) library
    // filename, LibName, lots of things will be broken.
 
    char *libDir = 0;
    size_t LibNameLen = strlen(LibName);

    size_t len = strlen(info->dlpi_name);
    if(len < LibNameLen) return 0;
    const char *end = info->dlpi_name + len - LibNameLen;

    for(const char *c = info->dlpi_name; c <= end; ++c) {
        if(0 == strncmp(c, LibName, LibNameLen)) {
            // realpath() will allocate a string.
            libDir = realpath(info->dlpi_name, 0);
            ASSERT(libDir);
            // Better be a full path.
            ASSERT(*libDir == DIRCHR);
            break;
        }
    }

    if(libDir) {

        // Truncate the path by removing the "/lib/libquickscope.so".
        *(libDir + strlen(libDir) - LibNameLen) = '\0';

        const char *addToPath = "/share/quickscope/run";
        len = strlen(libDir) + strlen(addToPath) + 1;
        char *s = malloc(len);
        strcpy(s, libDir);
        strcpy(s + strlen(libDir), addToPath);
        free(libDir);

        // This extra const-ifying may save us some pain in the long run.
        // If some code tries to change it, at least they get a compile
        // error.
        qsResourceDir = (const char *) s;
        return 1; // done
    }

    return 0;
}


static void __attribute__((constructor)) constructor(void) {

    // For each library:
    ASSERT(dl_iterate_phdr(dl_callback, 0));

    // We used the DSO (dynamic shared object) library path to find the
    // path to the GTK UI resource (XML) files.  This running code in
    // effect finds where it is running from in order to find files
    // relative to that path.
    //
    // We can put essential quickscope run files in the directory that is
    // qsResourceDir; that is the directory that this libquickscope.so
    // file loaded from and back one: lib/../share/
    if(!qsResourceDir) {
        WARN("The runtime Resource Directory was"
            " not found, setting it to \".\"");
        qsResourceDir = strdup(".");
        ASSERT(qsResourceDir, "strdup() failed");
    }

    DSPEW("qsResourceDir=\"%s\"", qsResourceDir);
}


static void __attribute__((destructor)) destructor(void) {

    if(app)
        g_object_unref(app);

    free((char *) qsResourceDir);

    DSPEW("libquickscope.so done");

    // Unfortunately the GTK libraries do not cleanup themselves,
    // and so there will be lots of system resource leaks.  Hence
    // we can't make an unloadable DSO (dynamic shared object)
    // module that does not leak.
}

