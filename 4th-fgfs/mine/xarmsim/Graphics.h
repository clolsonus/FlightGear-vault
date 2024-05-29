// Graphics.h
//
// This file contains all graphics specific code contained within a
// class.  Hopefully if this is ever ported to a different graphics
// system, only this file will need to be modified.  This file also
// contains the top level main().
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file) 
//


#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xarmsim.h"


#define make_bitmap( x ) x = XCreatePixmapFromBitmapData( display, sim_win, \
x##_bits, x##_width, x##_height, black(), white(), \
DefaultDepth( display, DefaultScreen( display )))
#define OFF 0
#define ON 1


class Graphics {
    // These are used as arguments to nearly every Xlib routine, so it saves 
    // routine arguments to declare them global.  If there were 
    // additional source files, they would be declared extern there.
    Display *display;
    Window sim_win;
    int screen_num;
    XTextProperty sim_winName, sim_iconName;
    char *sim_win_name;   // = "navsim Simulation Output"
    char *sim_icon_name;  // = "navsim-out"

    XSizeHints size_hints;
    XWMHints wm_hints;
    XClassHint class_hints;
    XEvent report;

    /* static */ char *progname;	// name this program was invoked by

    GC gc, xor, or;			// graphic contexts
    XFontStruct *font_info;

    Pixmap icon_pixmap;

    // Get a graphic context
    void getGC(Window sim_win, GC *gc, int function, XFontStruct *font_info);

    // Load a font in case we ever want to draw text
    void load_font(XFontStruct **font_info);

  public:

    Graphics();

    // Initialize and setup the graphics display
    void init_graphics( int argc, char **argv, char sim_win_name[],
		        char sim_icon_name[] );

    // Return the value of the color white
    int white();

    // Return the value of the color black
    int black();

    // Clear the display
    void clear_window();

    // Draw a line
    void draw_line( int x1, int y1, int x2, int y2, int persists );

    // Draw a circle
    void draw_circle( int x, int y, int r, int persists );

    // Flush graphics
    void flush_graphics();

    ~Graphics();
};


#endif GRAPHICS_H


//
// $Log: graph_main.c,v $
//

