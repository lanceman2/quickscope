// g_signal_connect() callback functions or functions that are called in
// g_signal_connect() callback functions, hence the file name
// commonGtkCallbacks.c.

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"



void NewGraphTab_cb(void) {

  DASSERT(app);

  GtkWindow *gtkWindow = gtk_application_get_active_window(app);
  ASSERT(gtkWindow);

  struct QsWindow *win = g_object_get_data(G_OBJECT(gtkWindow), "qsWindow");
  ASSERT(win);

  AddNewGraph(win, 0);
}

  
  
