#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"



static void DestroyWindow_cb(GtkWindow *gtkWindow, struct QsWindow *w) {

    DASSERT(gtkWindow);
    DASSERT(w);
    DASSERT(w->gtkWindow == gtkWindow);

    DSPEW("freeing GTK main window=%p", w->gtkWindow);
    DZMEM(w, sizeof(*w));
    free(w);

    --windowCount;
}


static void CreateGTKWindow_cb(GtkApplication* gApp, struct QsWindow *w) {

    DASSERT(gApp == app);

    GtkBuilder *builder = Get_builder_from_file("window.ui");

    GtkWindow *gtkWindow = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    w->gtkWindow = gtkWindow;
    w->gtkNotebook = GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook"));
    DASSERT(w->gtkNotebook);

    AddActions(GTK_WIDGET(gtkWindow), builder);

    g_object_unref(builder);


    g_object_set_data(G_OBJECT(gtkWindow), "qsWindow", w);
    gtk_window_set_application(gtkWindow, app);
    gtk_window_set_title(gtkWindow, "Window");
    gtk_widget_show(GTK_WIDGET(gtkWindow));
    g_signal_connect(gtkWindow, "destroy", G_CALLBACK(DestroyWindow_cb), w);

    gtk_widget_show_all(GTK_WIDGET(gtkWindow));


    // Add a new graph tab.
    AddNewGraph(w, 0);
}


// We tried other ways, but this is the only one that works so far.
//
GtkBuilder *Get_builder_from_file(const char *filename) {

    DSPEW();
    GtkBuilder *builder = gtk_builder_new();
    DASSERT(builder);
    GError *error = 0;
    char *path = g_strjoin("/", qsResourceDir, filename, NULL);
    if(!gtk_builder_add_from_file(builder, path, &error)) {
        ASSERT(0, "gtk_builder_add_from_file(,\"%s\",) failed: %s",
                path, error->message);
        // Not that we get here.
        g_object_unref(error);
        g_free(path);
    }
    DASSERT(!error);
    DSPEW("Loaded GTK4 builder file \"%s\"", path);
    g_free(path);
    return builder;
}


struct QsWindow *qsWindow_create(void) {

    DSPEW();

    GetApp();

    struct QsWindow *w = calloc(1, sizeof(*w));
    ASSERT(w, "calloc(1,%zu) failed", sizeof(*w));

    if(looping)
        CreateGTKWindow_cb(GTK_APPLICATION(app), w);
    else
        g_signal_connect(app, "activate", G_CALLBACK(CreateGTKWindow_cb), w);

    ++windowCount;

    return w;
}


void qsWindow_destroy(struct QsWindow *w) {

    ASSERT(0, "This is never called");

    DASSERT(w);
    DASSERT(w->gtkWindow);

    gtk_widget_destroy(GTK_WIDGET(w->gtkWindow));
    // The QsWindow memory is freed in the GTK "destroy" callback.
}
