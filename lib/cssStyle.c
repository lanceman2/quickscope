#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"

static GtkCssProvider *provider = 0;


void CleanupCSS(void) {

    if(provider) {
        g_object_unref(provider);
        provider = 0;
    }
}


void AddCSS(void) {

    if(provider) return;

    provider = gtk_css_provider_new();
    DASSERT(provider);

    GError *error = 0;
    char *path = g_strdup_printf("%s/style.css", qsResourceDir);
    ASSERT(path);
    gtk_css_provider_load_from_path(provider, path, &error);
    ASSERT(error == 0, "Failed to parse GTK3 css file \"%s\"", path);
    g_free(path);

    gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER);
}


