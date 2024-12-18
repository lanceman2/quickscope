#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <float.h>

#include <gtk/gtk.h>

#include "../include/quickscope.h"

#include "debug.h"
#include "app.h"
#include "graph.h"


#define SLIDE_BUTTON     1 /*1 -> left*/
#define BOX_BUTTON       3 /*3 -> right*/


#define SLIDE_ACTION    01  // sliding the graph with the pointer
#define BOX_ACTION      02  // sliding the graph with the pointer



static GdkWindow *window = 0;
static double x0, y0; // pointer position at the start of an zoom action

// Mark that we are doing a thing.  Since there is just one window
// focus we can use a global variable like this:
uint32_t zoom_action;

gboolean graph_buttonPress_cb(GtkWidget *drawingArea,
        GdkEventButton *e, struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);

  if(zoom_action)
    // Do nothing if there are actions already underway.  Example:
    // pressing another button while one is already pressed.
    return TRUE;

  if(e->button == SLIDE_BUTTON) {
      // Mark the action as running.
      zoom_action = SLIDE_ACTION;
      window = gtk_widget_get_window(drawingArea);
      DASSERT(window);
      gdk_window_set_cursor(window, hand_cursor);
      // record starting position
      x0 = e->x;
      y0 = e->y;
  }

  if(e->button == BOX_BUTTON) {
      // Mark the action as running.
      zoom_action = BOX_ACTION;
      // record starting position
      x0 = e->x;
      y0 = e->y;
  }


  return TRUE; // TRUE -> eat the event
}

gboolean graph_buttonRelease_cb(GtkWidget *drawingArea,
        GdkEventButton *e, struct QsGraph *g) {

  DASSERT(g);
  DASSERT(drawingArea);
  DASSERT(g->drawingArea == drawingArea);

  if(!zoom_action)
    // Do nothing if there no actions currently underway.  It may be that
    // we got two buttons pressed and this is the release of the button
    // that got no action because another button was pressed while that
    // button was pressed.
    return TRUE;

  if(e->button == SLIDE_BUTTON && (zoom_action & SLIDE_ACTION)) {
      DASSERT(window);
      gdk_window_set_cursor(window, 0);
      window = 0;
      // Mark the action as done.
      zoom_action &= ~SLIDE_ACTION;

      double dx, dy;
      dx = x0 - e->x;
      dy = y0 - e->y;

      if(dx > g->padX)
          dx = g->padX;
      else if(dx < - g->padX)
          dx = - g->padX;
      if(dy > g->padY)
          dy = g->padY;
      else if(dy < - g->padY)
          dy = - g->padY;

      x0 = y0 = g->slideX = g->slideY = 0;

      FixZoomsShift(g, dx, dy);
      DASSERT(!zoom_action, "We have multi button press actions??");
  }

  if(e->button == BOX_BUTTON && (zoom_action & BOX_ACTION)) {

      DSPEW("Zoom box finished");
      zoom_action &= ~BOX_ACTION;
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

    if(zoom_action & SLIDE_ACTION) {

        g->slideX = e->x - x0;
        g->slideY = e->y - y0;
  
        if(g->slideX > g->padX)
            g->slideX = g->padX;
        else if(g->slideX < - g->padX)
            g->slideX = - g->padX;
        if(g->slideY > g->padY)
            g->slideY = g->padY;
        else if(g->slideY < - g->padY)
            g->slideY = - g->padY;

        gtk_widget_queue_draw(drawingArea);
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
