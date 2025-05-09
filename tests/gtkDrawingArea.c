#include "../lib/debug.h"

#include <gtk/gtk.h>

static GtkWidget *window = NULL;
/* Pixmap for scribble area, to store current scribbles */
static cairo_surface_t *surface = NULL;

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
scribble_configure_event (GtkWidget         *widget,
                          GdkEventConfigure *event,
                          gpointer           data)
{
  GtkAllocation allocation;
  cairo_t *cr;

  if (surface)
    cairo_surface_destroy (surface);

  gtk_widget_get_allocation (widget, &allocation);
  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                               CAIRO_CONTENT_COLOR,
                                               allocation.width,
                                               allocation.height);

  /* Initialize the surface to white */
  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

/* Redraw the screen from the surface */
static gboolean
scribble_draw (GtkWidget *widget,
               cairo_t   *cr,
               gpointer   data)
{
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);
  // There's lots of needless draw calls.  Looks like a GTK3 bug to me.
DSPEW();

  return FALSE;
}

/* Draw a rectangle on the screen */
static void
draw_brush (GtkWidget *widget,
            gdouble    x,
            gdouble    y)
{
  GdkRectangle update_rect;
  cairo_t *cr;

  update_rect.x = x - 3;
  update_rect.y = y - 3;
  update_rect.width = 6;
  update_rect.height = 6;

  /* Paint to the surface, where we store our state */
  cr = cairo_create (surface);

  gdk_cairo_rectangle (cr, &update_rect);
  cairo_fill (cr);

  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gdk_window_invalidate_rect (gtk_widget_get_window (widget),
                              &update_rect,
                              FALSE);
}

static gboolean
scribble_button_press_event (GtkWidget      *widget,
                             GdkEventButton *event,
                             gpointer        data)
{
  if (surface == NULL)
    return FALSE; /* paranoia check, in case we haven't gotten a configure event */

  if (event->button == GDK_BUTTON_PRIMARY)
    draw_brush (widget, event->x, event->y);

  /* We've handled the event, stop processing */
  return TRUE;
}

static gboolean
scribble_motion_notify_event (GtkWidget      *widget,
                              GdkEventMotion *event,
                              gpointer        data)
{
  int x, y;
  GdkModifierType state;

  if (surface == NULL)
    return FALSE; /* paranoia check, in case we haven't gotten a configure event */

  /* This call is very important; it requests the next motion event.
   * If you don't call gdk_window_get_pointer() you'll only get
   * a single motion event. The reason is that we specified
   * GDK_POINTER_MOTION_HINT_MASK to gtk_widget_set_events().
   * If we hadn't specified that, we could just use event->x, event->y
   * as the pointer location. But we'd also get deluged in events.
   * By requesting the next event as we handle the current one,
   * we avoid getting a huge number of events faster than we
   * can cope.
   */

  gdk_window_get_device_position (event->window, event->device, &x, &y, &state);

  if (state & GDK_BUTTON1_MASK)
    draw_brush (widget, x, y);

  /* We've handled it, stop processing */
  return TRUE;
}


static gboolean
checkerboard_draw (GtkWidget *da,
                   cairo_t   *cr,
                   gpointer   data)
{
  gint i, j, xcount, ycount, width, height;

#define CHECK_SIZE 10
#define SPACING 2

  /* At the start of a draw handler, a clip region has been set on
   * the Cairo context, and the contents have been cleared to the
   * widget's background color. The docs for
   * gdk_window_begin_paint_region() give more details on how this
   * works.
   */

  xcount = 0;
  width = gtk_widget_get_allocated_width (da);
  height = gtk_widget_get_allocated_height (da);
  i = SPACING;
  while (i < width)
    {
      j = SPACING;
      ycount = xcount % 2; /* start with even/odd depending on row */
      while (j < height)
        {
          if (ycount % 2)
            cairo_set_source_rgb (cr, 0.45777, 0, 0.45777);
          else
            cairo_set_source_rgb (cr, 1, 1, 1);

          /* If we're outside the clip, this will do nothing.
           */
          cairo_rectangle (cr, i, j, CHECK_SIZE, CHECK_SIZE);
          cairo_fill (cr);

          j += CHECK_SIZE + SPACING;
          ++ycount;
        }

      i += CHECK_SIZE + SPACING;
      ++xcount;
    }

  /* return TRUE because we've handled this event, so no
   * further processing is required.
   */
  return TRUE;
}

static void
close_window (void)
{
  window = NULL;

  if (surface)
    cairo_surface_destroy (surface);
  surface = NULL;
}


int main(void) {
  
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *da;
  GtkWidget *label;

  gtk_init(0, 0);

      window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (window), "Drawing Area");

      g_signal_connect (window, "destroy",
                        G_CALLBACK (close_window), NULL);

      gtk_container_set_border_width (GTK_CONTAINER (window), 8);

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
      gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
      gtk_container_add (GTK_CONTAINER (window), vbox);

      /*
       * Create the checkerboard area
       */

      label = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label),
                            "<u>Checkerboard pattern</u>");
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
      gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

      da = gtk_drawing_area_new ();
      /* set a minimum size */
      gtk_widget_set_size_request (da, 100, 100);

      gtk_container_add (GTK_CONTAINER (frame), da);

      g_signal_connect (da, "draw",
                        G_CALLBACK (checkerboard_draw), NULL);

      /*
       * Create the scribble area
       */

      label = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label),
                            "<u>Scribble area</u>");
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
      gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

      da = gtk_drawing_area_new ();
      /* set a minimum size */
      gtk_widget_set_size_request (da, 100, 100);

      gtk_container_add (GTK_CONTAINER (frame), da);

      /* Signals used to handle backing surface */

      g_signal_connect (da, "draw",
                        G_CALLBACK (scribble_draw), NULL);
      g_signal_connect (da,"configure-event",
                        G_CALLBACK (scribble_configure_event), NULL);

      /* Event signals */

      g_signal_connect (da, "motion-notify-event",
                        G_CALLBACK (scribble_motion_notify_event), NULL);
      g_signal_connect (da, "button-press-event",
                        G_CALLBACK (scribble_button_press_event), NULL);


      /* Ask to receive events the drawing area doesn't normally
       * subscribe to
       */
      gtk_widget_set_events (da, gtk_widget_get_events (da)
                             | GDK_LEAVE_NOTIFY_MASK
                             | GDK_BUTTON_PRESS_MASK
                             | GDK_POINTER_MOTION_MASK
                             | GDK_POINTER_MOTION_HINT_MASK);


      gtk_widget_show_all (window);

      gtk_main();

  return 0;
}
