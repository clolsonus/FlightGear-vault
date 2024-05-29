// Graphics.cc
//
// This file contains all graphics specific code.  Hopefully if this is
// ever ported to a different graphics system, only this file will need
// to be modified.  This file also contains the top level main().
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file)
//


#include "xarmsim.h"
#include "Graphics.h"
#include "./bitmaps/icon.bmp"


// Constructor
Graphics::Graphics() {
    printf("Creating a new instance of class Graphics\n");
}


// Initialize and setup the graphics display
void Graphics::init_graphics( int argc, char **argv, char sim_win_name[],
			      char sim_icon_name[] ) {
    unsigned int width, height;		// window size
    int x, y;				// window position
    unsigned int border_width = 4;	// four pixels
    unsigned int display_width, display_height;

    char *display_name = NULL;

    progname = argv[0];

    // connect to X server
    if ( (display=XOpenDisplay(display_name)) == NULL ) {
        printf( "%s: cannot connect to X server %s\n", argv[0], 
        	XDisplayName(display_name));
        exit( -1 );
    }

    // get screen size from display structure macro
    screen_num = DefaultScreen(display);
    display_width = DisplayWidth(display, screen_num);
    display_height = DisplayHeight(display, screen_num);

    // Note that in a real application, x and y would default to 0
    // but would be setable from the command line or resource database.  
    x = y = 0;

    // size window with enough room for text
    width =  X_SIZE;
    height = Y_SIZE;

    // create opaque window for simulation output
    sim_win = XCreateSimpleWindow(display, RootWindow(display,screen_num), 
                    x, y, width, height, border_width, black(), white());

    // Create pixmap of depth 1 (bitmap) for icon
    icon_pixmap = XCreateBitmapFromData(display, sim_win, icon_bits, 
                                        icon_width, icon_height);

    // These calls store window_name and icon_name into
    // XTextProperty structures and set their other 
    // fields properly.
    if (XStringListToTextProperty(&sim_win_name, 1, &sim_winName) == 0) {
        printf( "%s: structure allocation for sim_winName failed.\n", 
                progname);
        exit(-1);
    }

    if (XStringListToTextProperty(&sim_icon_name, 1, &sim_iconName) == 0) {
        printf( "%s: structure allocation for sim_iconName failed.\n", 
        	progname);
        exit(-1);
    }

    wm_hints.icon_pixmap = icon_pixmap;
    wm_hints.flags = StateHint | IconPixmapHint | InputHint;

    XSetWMProperties(display, sim_win, &sim_winName, &sim_iconName, 
                     argv, argc, &size_hints, &wm_hints, 
                     &class_hints);

    // Select event types wanted
    XSelectInput(display, sim_win, ExposureMask | StructureNotifyMask);

    load_font(&font_info);

    // create the needed GC for drawing
    Graphics::getGC(sim_win, &gc, GXcopy, font_info);
    Graphics::getGC(sim_win, &xor, GXxor, font_info);
    Graphics::getGC(sim_win, &or, GXor, font_info);

    // Display window
    XMapWindow(display, sim_win);

    // This will draw the initial window
    Graphics::flush_graphics();

    // printf("Black = %d  White = %d.\n", black(), white());
}


// Return the value of the color white
int Graphics::white() {
    #ifndef sgi
        return WhitePixel(display,screen_num);
    #else
	return 8;
    #endif
}


// Return the value of the color black
int Graphics::black() {
    #ifndef sgi
        return BlackPixel(display,screen_num);
    #else
	return 2;
    #endif
}


// Clear the display
void Graphics::clear_window() {
    XClearArea( display, sim_win, 0, 0, X_SIZE, Y_SIZE, False );
}


// Get a graphic context
void Graphics::getGC(Window sim_win, GC *gc, int function, XFontStruct 
		   *font_info) {
    /* unsigned long valuemask = 0; */	// ignore XGCvalues and use defaults
    unsigned long valuemask = GCFunction | GCGraphicsExposures;
    XGCValues values;
    unsigned int line_width = 1;
    int line_style = LineSolid;
    int cap_style = CapRound;
    int join_style = JoinRound;
    int dash_offset = 0;
    static char dash_list[] = {12, 24};
    int list_length = 2;

    // Preconfigure this GC
    values.function = function;
    values.graphics_exposures = False;

    // Create default Graphics Context
    *gc = XCreateGC(display, sim_win, valuemask, &values);

    // specify font
    XSetFont(display, *gc, font_info->fid);

    // specify black foreground since default window background is 
    // white and default foreground is undefined.
    XSetForeground(display, *gc, black());

    // set line attributes
    XSetLineAttributes(display, *gc, line_width, line_style, cap_style, 
                       join_style);

    // set dashes
    XSetDashes(display, *gc, dash_offset, dash_list, list_length);
}


// Load a font in case we ever want to draw text
void Graphics::load_font(XFontStruct **font_info) {
    char *fontname = "9x15";

    // Load font and get font information structure.
    if ((*font_info = XLoadQueryFont(display,fontname)) == NULL) {
        printf( "%s: Cannot open 9x15 font\n", progname);
        exit( -1 );
    }
}


// Draw a line
void Graphics::draw_line( int x1, int y1, int x2, int y2, int persists ) {
    if( persists ) {
        XDrawLine( display, sim_win, gc, x1, y1, x2, y2 );
    } else {
        XDrawLine( display, sim_win, xor, x1, y1, x2, y2 );
    }
}


// Draw a circle
void Graphics::draw_circle( int x, int y, int r, int persists ) {
    if( persists ) {
        XDrawArc( display, sim_win, gc, x-r, y-r, 2*r, 2*r, 0, 64*360 );
    } else {
        XDrawArc( display, sim_win, xor, x-r, y-r, 2*r, 2*r, 0, 64*360 );
    }
}


// Flush graphics
void Graphics::flush_graphics() {
    XFlush( display );
}


// Destructor
Graphics::~Graphics() {
    printf("Destroying an instance of class Graphics\n");
}


//
// $Log: graph_main.c,v $
//

