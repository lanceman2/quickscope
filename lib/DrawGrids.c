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



static inline double RoundUp(double x, uint32_t *subDivider,
                    int32_t *p_out) {

    // TODO: This could be rewritten by using an understanding of floating
    // point representation.  In this writing we did not even try to
    // understand how floating point numbers are stored, we just used math
    // functions that do the right thing.

    ASSERT(x > 0.0);
    ASSERT(isnormal(x));
    ASSERT(x < DBL_MAX/8.0); // max is about 1.0e308

    double pow = log10(x);
    // x = 10^pow
    int32_t p;
    if(pow > 0.0)
        p = (pow + 0.5);
    else
        p = (pow - 0.5);
    // x ~= 10^p  example if pow ==  1.3  p = 1
    //                    if pow == -1.3  p = -1
    double tenPow = exp10((double) p);
    double mant = x/tenPow;

    while(mant < 1.0) {
        mant *= 10.0;
        --p;
    }

    // I'm not sure how the mantissa is defined, but I don't care.  I just
    // have that:  
    //
    //       x = mant * 10 ^ p  [very close]
    //
    // Ya, it better be that this is so:
    DASSERT(x < 1.000001 * mant * exp10(p));
    DASSERT(x > 0.999999 * mant * exp10(p));

    //DSPEW("x = %lg = %lf * 10 ^(%d)", x, mant, p);

    // Now trim digits off of mant.  Like for example:
    //
    //    1.1234 => 2.0   or  4.6234 => 5.0

    uint32_t ix = (uint32_t) mant;

    // We make it larger, not smaller.  Just certain values look good in
    // plot grid lines.
    //
    switch(ix) {
        case 9:
        case 8:
        case 7:
        case 6:
        case 5:
            ix = 10;
            break;
        case 4:
        case 3:
        case 2:
            ix = 5;
            break;
        case 1:
            ix = 2;
            break;
        case 0:
            ASSERT(0, "Bad Code");
            break;
    }

    x = ix * exp10(p);

    //DSPEW("x = %lg <= %" PRIu32" * 10 ^(%d)", x, ix, p);

    *subDivider = ix;
    *p_out = p;

    return x;
}


static inline void DrawVSubGrid(cairo_t *cr, struct QsZoom *z,
        double start, double delta, double end, double height) {

    for(double x = start; x <= end; x += delta) {
        double pix = xToPix(x, z);
        cairo_move_to(cr, pix, 0);
        cairo_line_to(cr, pix , height);
        cairo_stroke(cr);
    }
}

static inline void DrawHSubGrid(cairo_t *cr, struct QsZoom *z,
        double start, double delta, double end, double width) {

    for(double y = start; y <= end; y += delta) {
        double pix = yToPix(y, z);
        cairo_move_to(cr, 0, pix);
        cairo_line_to(cr, width, pix);
        cairo_stroke(cr);
    }
}


// This will find a rounded up distance between lines that looks nice.
// For example a data scaled space between lines of 2.0000e-3 and not
// 1.8263e-3.
//
// Returns the distance between lines for a sub grid in plot
// coordinates.
//
static inline double GetVGrid(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        double pixelSpace/*minimum pixels between lines*/,
        struct QsZoom *z,
        double fontSize, double *start_out,
        uint32_t *subDivider, int32_t *pow) {

    DASSERT(lineWidth <= pixelSpace);

    // delta is in user values not pixels.
    double delta = z->xSlope * pixelSpace;
    delta = RoundUp(delta, subDivider, pow);

    // start a little behind pixel 0.
    double start = pixToX(0, z) - delta;
    // Make the start a integer number of deltX

    int32_t n;
    if(start < 0.0)
        n = (start/delta - 0.5);
    else
        n = (start/delta + 0.5);
    // Make start a multiple of delta
    start = n * delta;

    //DSPEW("V delta = %lg  start= %lg", delta, start);

    *start_out = start;
    return delta;
}

static inline double GetHGrid(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        double pixelSpace/*minimum pixels between lines*/,
        struct QsZoom *z, double width, double height,
        double fontSize, double *start_out,
        uint32_t *subDivider, int32_t *pow) {

    DASSERT(lineWidth <= pixelSpace);

    double delta = - z->ySlope * pixelSpace;
    delta = RoundUp(delta, subDivider, pow);

    // start a little behind pixel 0.
    double start = pixToY(height-1, z) - delta;
    // Make the start a integer number of delta

    int32_t n;
    if(start < 0.0)
        n = (start/delta - 0.5);
    else
        n = (start/delta + 0.5);
    // Make start a multiple of delta
    start = n * delta;

    //DSPEW("H delta = %lg  start= %lg", delta, start);

    *start_out = start;
    return delta;
}


static inline void StripZeros(char *label) {

    char *s = label + strlen(label) - 1;
    while(*s == '0' &&
        (*(s-1) >= '0' && *(s-1) <= '9')/*next is a 0 to 9 digit*/) {
        *s = '\0';
        --s;
    }
}


static inline void GetLabel(char *label, size_t LEN,
        double x, double exp10pow, int32_t pow) {

    //snprintf(label, LEN, "%1.1e", x);

    if(pow >= -1 && pow <= 3) {
        snprintf(label, LEN, "%3.4f", x);
        // Remove trailing zeros.
        StripZeros(label);
        return;
    }

    snprintf(label, LEN, "%4.4f", x/exp10pow);
    StripZeros(label);
    size_t l = strlen(label);
    if(!strcmp(label, "0.0"))
        // 0.0 is special
        return;
    snprintf(label+l, LEN-l, "e%+d", pow);
}


static inline void DrawVGrid(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        struct QsZoom *z, struct QsGraph *g,
        double fontSize, double start,
        double delta) {

    cairo_set_line_width(cr, lineWidth);

    double end = pixToX(g->width + 2*g->padX, z) + delta;

    // TODO: Optimize performance in this loop.
    //
    for(double x = start; x <= end; x += delta) {

        double pix = xToPix(x, z);
        cairo_move_to(cr, pix, 0);
        cairo_line_to(cr, pix , g->height + 2*g->padY);
        cairo_stroke(cr);
    }
}

static inline void DrawVGridLabels(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        struct QsZoom *z, struct QsGraph *g,
        double fontSize, double start,
        double delta, int32_t pow) {

    double end = pixToX(g->width + 2*g->padX, z) + delta;

    const size_t T_SIZE = 32;
    char label[T_SIZE];

    lineWidth *= 0.8;

    ++pow;
    double exp10pow = exp10(pow);

    for(double x = start; x <= end; x += delta) {

        if(fabs(x) < delta/100)
            // With infinite precision this would not happen, but we do
            // not have infinite precision so we'll just fix the value so
            // that we print 0 and not something like 1.3e-32.
            x = 0.0;

        double pix = xToPix(x, z);
        GetLabel(label, T_SIZE, x, exp10pow, pow);
        cairo_move_to(cr, pix + lineWidth, 
            g->height + g->padY - lineWidth);
        cairo_show_text(cr, label);

        if(g->height < 2*fontSize) continue;

        cairo_move_to(cr, pix + lineWidth, g->padY - lineWidth);
        cairo_show_text(cr, label);

        cairo_move_to(cr, pix + lineWidth,
            g->height + 2 * g->padY - lineWidth);
        cairo_show_text(cr, label);
    }
}


static inline void DrawHGrid(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        struct QsZoom *z, struct QsGraph *g,
        double fontSize, double start,
        double delta) {

    cairo_set_line_width(cr, lineWidth);

    double end = pixToY(0, z) + delta;

    // TODO: Optimize performance in this loop.
    //
    for(double y = start; y <= end; y += delta) {

        double pix = yToPix(y, z);
        cairo_move_to(cr, 0, pix);
        cairo_line_to(cr, g->width + 2*g->padX, pix);
        cairo_stroke(cr);
    }
}


static inline void DrawHGridLabels(cairo_t *cr,
        double lineWidth/*vertical line width in pixels*/, 
        struct QsZoom *z, struct QsGraph *g,
        double fontSize, double start,
        double delta, int32_t pow) {

    double end = pixToY(0, z) + delta;

    const size_t T_SIZE = 32;
    char label[T_SIZE];

    double textX = fontSize;

    lineWidth *= 0.8;

    ++pow;
    double exp10pow = exp10(pow);

    for(double y = start; y <= end; y += delta) {

        if(fabs(y) < delta/100)
            // With infinite precision this would not happen, but we do
            // not have infinite precision so we'll just fix the value so
            // that we print 0 and not something like 1.3e-32.
            y = 0.0;

        double pix = yToPix(y, z);
        GetLabel(label, T_SIZE, y, exp10pow, pow);
        cairo_move_to(cr, textX, pix - lineWidth - 1);
        cairo_show_text(cr, label);

        cairo_move_to(cr, textX + g->padX, pix - lineWidth - 1);
        cairo_show_text(cr, label);

        cairo_move_to(cr, textX + g->padX + g->width, pix - lineWidth - 1);
        cairo_show_text(cr, label);
     }
}


void DrawGrids(struct QsGraph *g, cairo_t *cr, bool show_subGrid) {

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_NORMAL);
    const double fontSize = 20;
    cairo_set_font_size(cr, fontSize);

    double startX, deltaX, startY, deltaY;
    uint32_t subDividerX, subDividerY;
    double lineWidth = 5.7;
    int32_t powX, powY;

    deltaX = GetVGrid(cr, lineWidth, 140, g->zoom,
            fontSize, &startX, &subDividerX, &powX);

    deltaY = GetHGrid(cr, lineWidth, 140, g->zoom,
            g->width + 2*g->padX, g->height + 2*g->padY,
            fontSize, &startY, &subDividerY, &powY);


    if(!show_subGrid)
      goto drawGrid;

    cairo_set_source_rgb(cr,
            g->subGridColor.r, g->subGridColor.g, g->subGridColor.b);

    switch(subDividerX) {
        case 10:
            cairo_set_line_width(cr, 1.5);
            DrawVSubGrid(cr, g->zoom, startX, deltaX/10.0,
                    pixToX(g->width + 2*g->padX, g->zoom) + deltaX,
                    g->height + 2*g->padY);
            cairo_set_line_width(cr, 4.0);
            DrawVSubGrid(cr, g->zoom, startX + deltaX/2, deltaX,
                    pixToX(g->width + 2*g->padX, g->zoom) + deltaX,
                    g->height + 2*g->padY);
            break;
        case 5:
            cairo_set_line_width(cr, 4.0);
            DrawVSubGrid(cr, g->zoom, startX, deltaX/5.0,
                    pixToX(g->width + 2*g->padX, g->zoom) + deltaX,
                    g->height + 2*g->padY);
            cairo_set_line_width(cr, 0.7);
            DrawVSubGrid(cr, g->zoom, startX, deltaX/10.0,
                    pixToX(g->width + 2*g->padX, g->zoom) + deltaX,
                    g->height + 2*g->padY);
            break;
        case 2:
            cairo_set_line_width(cr, 4.2);
            DrawVSubGrid(cr, g->zoom, startX + deltaX/2, deltaX,
                    pixToX(g->width + 2*g->padX, g->zoom) + deltaX,
                    g->height + 2*g->padY);
            cairo_set_line_width(cr, 1.5);
            DrawVSubGrid(cr, g->zoom, startX, deltaX/10.0,
                    pixToX(g->width+ 2*g->padX, g->zoom) + deltaX,
                    g->height + 2*g->padY);
            break;
        default:
            ASSERT(0, "Bad Code");
            break;
    }


    switch(subDividerY) {
        case 10:
            cairo_set_line_width(cr, 1.5);
            DrawHSubGrid(cr, g->zoom, startY, deltaY/10.0,
                    pixToY(0, g->zoom) + deltaY,
                    g->width + 2*g->padX);
            cairo_set_line_width(cr, 4.0);
            DrawHSubGrid(cr, g->zoom, startY + deltaY/2, deltaY,
                    pixToY(0, g->zoom) + deltaY,
                    g->width + 2*g->padX);
            break;
        case 5:
            cairo_set_line_width(cr, 4.0);
            DrawHSubGrid(cr, g->zoom, startY, deltaY/5.0,
                    pixToY(0, g->zoom) + deltaY,
                    g->width + 2*g->padX);
            cairo_set_line_width(cr, 0.7);
            DrawHSubGrid(cr, g->zoom, startY, deltaY/10.0,
                    pixToY(0, g->zoom) + deltaY,
                    g->width + 2*g->padX);
            break;
        case 2:
            cairo_set_line_width(cr, 4.2);
            DrawHSubGrid(cr, g->zoom, startY + deltaY/2, deltaY,
                    pixToY(0, g->zoom) + deltaY,
                    g->width + 2*g->padX);
            cairo_set_line_width(cr, 1.5);
            DrawHSubGrid(cr, g->zoom, startY, deltaY/10.0,
                    pixToY(0, g->zoom) + deltaY,
                    g->width + 2*g->padX);
            break;
        default:
            ASSERT(0, "Bad Code");
            break;
    }

drawGrid:

    cairo_set_source_rgb(cr,
            g->gridColor.r, g->gridColor.g, g->gridColor.b);
    DrawVGrid(cr, lineWidth, g->zoom, g,
            fontSize, startX, deltaX);
    DrawHGrid(cr, lineWidth, g->zoom, g,
            fontSize, startY, deltaY);

    cairo_set_source_rgb(cr,
            g->axesLabelColor.r, g->axesLabelColor.g,
            g->axesLabelColor.b);
    DrawVGridLabels(cr, lineWidth, g->zoom, g,
            fontSize, startX, deltaX, powX);
    DrawHGridLabels(cr, lineWidth, g->zoom, g,
            fontSize, startY, deltaY, powY);
}
