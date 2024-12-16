#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


static GdkWindow *window = 0;

#define SLIDE_BUTTON  1 /*1 -> left*/

#define SLIDE_ACTION    01  // sliding the graph with the pointer

// Mark that we are doing a thing.  Since there is just one window focus
// we can use a global variable like this:
static uint32_t action = 0;

static double x, y; // pointer position at the start of an action


gboolean graph_buttonPress_cb(GtkWidget *drawingArea,
        GdkEventButton *e, struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);

  if(action)
    // Do nothing if there are actions already underway.  Example:
    // pressing another button while one is already pressed.
    return TRUE;

  if(e->button == SLIDE_BUTTON) {
      // Mark the action as running.
      action = SLIDE_ACTION;
      window = gtk_widget_get_window(drawingArea);
      DASSERT(window);
      gdk_window_set_cursor(window, hand_cursor);
      // record starting position
      x = e->x;
      y = e->y;
  }

  return TRUE;
}

gboolean graph_buttonRelease_cb(GtkWidget *drawingArea,
        GdkEventButton *e, struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);
  // We assume there was a button press, where we got the window.
  DASSERT(window);

  if(!action)
    // Do nothing if there no actions currently underway.  It may be that
    // we got two buttons pressed and this is the release of the button
    // that got no action because another button was pressed while that
    // button was pressed.
    return TRUE;

  if(e->button == SLIDE_BUTTON && (action & SLIDE_ACTION)) {
      DASSERT(window);
      gdk_window_set_cursor(window, 0);
      window = 0;
      // Mark the action as done.
      action &= ~SLIDE_ACTION;
  }

  return TRUE;
}

gboolean graph_pointerMotion_cb(GtkWidget *drawingArea, GdkEventMotion *e,
        struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);
  DASSERT(e->type == GDK_MOTION_NOTIFY);

  // Record the pointer plot x y values in the status bar:
  guint id = gtk_statusbar_get_context_id(g->statusbar, "info");
  const size_t LEN = 32;
  char text[LEN];
  snprintf(text, LEN, "%g  %g",
          pixToX(e->x, g->zoom), pixToY(e->y, g->zoom)); 
  gtk_statusbar_push(g->statusbar, id, text);

  //DSPEW("x,y=%g,%g", e->x, e->y);

    if(action & SLIDE_ACTION) {

        DSPEW("SLIDE x,y=%g,%g", e->x, e->y);
    }


  return TRUE;
}

gboolean graph_pointerEnter_cb(GtkWidget *drawingArea, GdkEvent *e,
        struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);

DSPEW();

  return TRUE;
}

gboolean graph_pointerLeave_cb(GtkWidget *drawingArea, GdkEvent *e,
        struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);

DSPEW();

  return TRUE;
}
