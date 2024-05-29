/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */
 
#include "pm.h"
#include "eng.xbm"
#include "flaps0.xbm"
#include "flaps1.xbm"
#include "flaps2.xbm"
#include "flaps3.xbm"
#include "handleUp.xbm"
#include "handleDn.xbm"
#include "gearUp.xbm"
#include "gearTran.xbm"
#include "gearDown.xbm"
#include <stdio.h>
#include <X11/Xutil.h>
#ifndef __hpux
#include <X11/Xos.h>
#endif
#include <X11/cursorfont.h>
#ifdef HAVE_DIS
#include "dis.h"
#endif

#if defined(HAVE_STDLIB_H)
#ifndef bzero
#define bzero(s,n)      memset ((s), 0, (n))
#endif
#endif /* HAVE_STDLIB_H */

extern void resizePlayerWindow ();
extern void resizeChaserWindow();
extern void newExplosion ();
extern void blackBoxKillPlayer ();
extern void printValidAircraft ();
extern void buildEulerMatrix();
extern void transpose();

#define BORDER	1

#define	ARG_FONT		"font"
#define ARG_RADAR_FONT		"radarfont"
#define	ARG_BORDER_COLOR	"bordercolor"
#define ARG_BORDER		"borderwidth"
#define	ARG_GEOMETRY		"geometry"
/* #define DEFAULT_BACKGROUND	"#93bde4"	/* my version of sky blue */
/* #define DEFAULT_BACKGROUND	"#7491ae"	/* my version of sky blue */
#define DEFAULT_BACKGROUND	"#7c99b6"	/* my version of sky blue */
#define DEFAULT_BORDER		"black"
#define DEFAULT_RADAR_FONT	"-misc-fixed-medium-*-*-*-*-120-*-*-*-*-*-*"

#define SW_BORDER	1
#define SW_BACKGROUND	2
#define SW_HUDFONT	3
#define SW_GEOM		4
#define SW_RADARFONT	5
#define SW_TEAM		6
#define SW_CHASE	7
#define SW_OBSERVE	8
#define SW_DEFAULT_VISUAL 9
#define SW_PLANE	10
#define SW_LIST_PLAYER	11
#define SW_MONOCHROME	12

struct {
	char	*sw;
	int	value;
	}	swt[] = {
	"-bw", SW_BORDER,
	"-skycolor", SW_BACKGROUND,
	"-hudfont",  SW_HUDFONT,
	"-radarfont",SW_RADARFONT,
	"-geometry", SW_GEOM,
	"-team",     SW_TEAM,
	"-chase",    SW_CHASE,
	"-observe",  SW_OBSERVE,
	"-cmap",     SW_DEFAULT_VISUAL,
	"-mono",     SW_MONOCHROME,
	"-plane",    SW_PLANE,
	"-list",     SW_LIST_PLAYER,
	NULL, 0}, *swp;

void
recoverAcmArgv (args, argc, argv)
char	*args;
int	*argc;
char	*argv[];
{

	char *s;

	argv[0] = ACM;
	argv[1] = args;

	if (*args == '\0') {
		*argc = 1;
		argv[1] = (char *) NULL;
		return;
	}
	else
		*argc = 2;

	for (s=args; *s;) {
		if (*s == '|') {
			*s = '\0';
			argv[(*argc)++] = ++s;
		}
		else
			++s;
	}

	argv[*argc] = (char *) NULL;
}

Visual *
get_pseudo_visual (display, screen, depth)
Display *display;
int	screen;
unsigned int *depth;
{

	XVisualInfo	vTemplate;
	XVisualInfo	*visualList;
	int		i, visualsMatched;

/*
 *  Get all Visuals on this screen
 */

	vTemplate.screen = screen;
	visualList = XGetVisualInfo (display, VisualScreenMask,
		&vTemplate, &visualsMatched);

	if (visualsMatched == 0)
		return NULL;

/*
 *  look first for a suitable PseudoColor visual, then a GreyScale visual
 *  if no PseudoColor is present
 */

	for (i=0; i<visualsMatched; ++i) {
		if (visualList[i].visual->class == PseudoColor &&
			visualList[i].depth >= 4) {
			*depth = visualList[i].depth;
			XFree ((char *) visualList);
			return visualList[i].visual;
		}
	}

	for (i=0; i<visualsMatched; ++i) {
		if (visualList[i].visual->class == GrayScale &&
			visualList[i].depth >= 4) {
			*depth = visualList[i].depth;
			XFree ((char *) visualList);
			return visualList[i].visual;
		}
	}

	XFree ((char *) visualList);

	return NULL;
}

int
newPlayer(s, display, logname, switches)
int	s;
char	*display;
char	*logname;
char	*switches;
{

    char       *fontName;	/* Name of font for string */
    XSizeHints	xsh;		/* Size hints for window manager */
    Colormap	cmap;
    XGCValues	gcv;
    unsigned long pad;		/* Font size parameters */
    unsigned long bd;		/* Pixel values */
    unsigned long bw;		/* Border width */
    char       *tempstr;	/* Temporary string */
    XColor      color;		/* Temporary color */
    char       *geomSpec;	/* Window geometry string */
    XWMHints	xwmh;		/* Window manager hints */
    Cursor	cursor;
    char	**c;
    char	err[256];
    static char	*background = NULL;
    int		borderWidth = -1;
    int		player;
    viewer	*u;
    craft	*cf;
    int		argc;
    char	*argv[32];
    int		screen;
    char	*hudfont = NULL, *radarfont = NULL;
    int		team = 1;
    char	*plane = NULL;	/* name of plane type */
    int		obsrver = -1;
    int		chaser = -1;
    int		width, height;	/* dimensions of main window */
    double	scale;
    unsigned	depth, mono = 0;
    long	win_attr_mask;
    Visual	*theVisual;
    XSetWindowAttributes window_attributes;
    int		useDefaultVisual = 1;
    Atom	atom[2];
#ifdef HAVE_DIS
    double      disLocation[3];
    double      disZeroVec[3];
    double      disOrientation[3];
    int		disType;
#endif    

    recoverAcmArgv (switches, &argc, argv);

    geomSpec = NULL;
    u = (viewer *) malloc (sizeof(viewer));

/*
 *  Parse command line
 */

    for (c = &argv[1]; *c != (char *) NULL; ++c)
	if (**c == '-') {
	    for (swp = &swt[0]; swp->value != 0; ++swp)
		if (strcmp (swp->sw, *c) == 0) {

			switch (swp->value) {

			case SW_GEOM: 
				geomSpec = *(++c);
				break;

			case SW_BORDER:
				borderWidth = atoi (*(++c));
				break;

			case SW_BACKGROUND:
				background = *(++c);
				break;

			case SW_HUDFONT:
				hudfont = *(++c);
				break;

			case SW_RADARFONT:
				radarfont = *(++c);
				break;

			case SW_TEAM:
				team = atoi (*(++c));
				break;

			case SW_CHASE:
				chaser = atoi (*(++c));
				break;

			case SW_OBSERVE:
				obsrver = atoi (*(++c));
				break;

			case SW_PLANE:
				plane = *(++c);
				break;

			case SW_MONOCHROME:
				mono = 1;
				break;

			case SW_LIST_PLAYER:
				sprintf (err, "\nname\t\tnumber\n");
				write (s, err, strlen(err));
				sprintf (err, "-------------------------\n");
				write (s, err, strlen(err));
				for (team = 0; team < MAXPLAYERS; team++) {
				    if (ptbl[team].type == CT_PLANE) {
					sprintf (err, "%-16s  %d\n",
						ptbl[team].name, team);
					write (s, err, strlen(err));
				    }
				}
				return -1;

			case SW_DEFAULT_VISUAL:
				useDefaultVisual = 0;
				break;
			}
		        break;
		}
	    if (swp->value == 0) {
		free ((char *) u);
		sprintf (err, "%s: invalid switch %s", ACM, *c);
		write (s, err, strlen(err));
		return -1;
	    }
	}
  
     if (chaser != -1) {
	sprintf (err, "You are chasing player %d\n", chaser);
	write (s, err, strlen (err));
	if (!chaseOthers) { }
	switch (player = newChaser (chaser, logname)) {

	case NC_NOT_PLANE:
		sprintf (err, "You have selected unknown player %d\n", chaser);
		write (s, err, strlen (err));
		plane = "chaser";
		return -1;

	case NC_CANNOT_CHASE:
		sprintf (err, "You cannot chase player \"%s\".\n", ptbl[chaser].name);
		write (s, err, strlen (err));
		plane = "chaser";
		return -1;

	}
	ctbl[player].team = chaser;
    }

    if (chaser == -1) {

	if (!plane) {
		plane = (team == 1) ? "F-16" : "MiG-29";
	}

	if ((player = newPlane (plane)) < 0) {
		if (player == -1) {
	    		sprintf (err,
		    "Sorry, no room for any more players at this moment.\n");
			write (s, err, strlen(err));
		}
		else {
			sprintf (err,
				"You have selected an unknown plane type.\n");
			write (s, err, strlen(err));
			printValidAircraft(s);
		}
		return -1;
	}

	cf = &ptbl[player];

/*
 *  assign an initial location basewd on the player's team.
 */

	if (team != 2) {
		team = 1;
	}

	cf->Sg.x = teamLoc[team].x;
	cf->Sg.y = teamLoc[team].y;
	/* cf->Sg.z = teamLoc[team].z; */
	cf->curHeading = teamHeading[team];

	buildEulerMatrix (cf->curRoll, cf->curPitch, cf->curHeading,
		&(cf->trihedral));
	transpose (&cf->trihedral, &cf->Itrihedral);
    }
    else {				/* adding a new chaser */
	cf = &ctbl[player];
	cf->type = CT_CHASER;
    }

    if ((u->dpy = XOpenDisplay(display)) == (Display *) NULL) {
	free ((char *) u);
	cf->type = CT_FREE;
	sprintf(err, "%s: can't open %s\n", ACM, display);
	write (s, err, strlen(err));
	return -1;
    }
    screen = DefaultScreen (u->dpy);

    if (radarfont)
	 fontName = radarfont;
    else if ((fontName = XGetDefault(u->dpy, ACM, ARG_RADAR_FONT)) == NULL) {
	fontName = DEFAULT_RADAR_FONT;
    }
    if ((u->rfont = XLoadQueryFont(u->dpy, fontName)) == NULL) {
	XCloseDisplay (u->dpy);
	free ((char *) u);
	cf->type = CT_FREE;
	sprintf(err, "%s: display %s doesn't know font %s\n",
		ACM, display, fontName);
	write (s, err, strlen(err));
	return -1;
    }

/*
 *  If the player has specified that the want the default Visual, simply
 *  give 'em that along with the default Colormap.
 */

    if (useDefaultVisual) {

	theVisual = DefaultVisual (u->dpy, screen);
	cmap = DefaultColormap (u->dpy, screen);
	depth = DisplayPlanes (u->dpy, screen);

    }

/*
 *  Look for a visual; if we can't find one,
 *  fall back to monochrome mode.
 */

    else {
	if ((theVisual = get_pseudo_visual (u->dpy, screen, &depth)) == NULL) {
		theVisual = DefaultVisual (u->dpy, screen);
		cmap = DefaultColormap (u->dpy, screen);
		depth = DefaultDepth(u->dpy, screen);
	}
	else {
		cmap = XCreateColormap (u->dpy, RootWindow(u->dpy, screen),
			theVisual, AllocNone);
	}
    }

/*
 * Select colors for the border, the window background, and the
 * foreground.  We use the default colormap to allocate the colors in.
 */

    if (background == NULL)
	background = DEFAULT_BACKGROUND;

    if ((tempstr = XGetDefault(u->dpy, ACM, ARG_BORDER_COLOR)) == NULL)
	tempstr = DEFAULT_BORDER;
    if (XParseColor(u->dpy, cmap, tempstr, &color) == 0) {
	XCloseDisplay (u->dpy);
	free ((char *) u);
	cf->type = CT_FREE;
	sprintf (err, "Can't get border color %s\n", tempstr);
	write (s, err, strlen(err));
	return -1;
    }

    if (depth == 1)
	bd = BlackPixel (u->dpy, screen);
    else {
        if (XAllocColor(u->dpy, cmap, &color) == 0) {
	    XCloseDisplay (u->dpy);
	    free ((char *) u);
	    cf->type = CT_FREE;
	    sprintf (err, "Cannot allocate color cells\n");
	    write (s, err, strlen(err));
	    return -1;
	}
	bd = color.pixel;
    }

/*
 * Set the border width of the window, and the gap between the text
 * and the edge of the window, "pad".
 */

    pad = BORDER;
    if (borderWidth >= 0)
	bw = borderWidth;
    else if ((tempstr = XGetDefault(u->dpy, ACM, ARG_BORDER)) == NULL)
	bw = 1;
    else
	bw = atoi(tempstr);

/*
 * Deal with providing the window with an initial position & size.
 * Fill out the XSizeHints struct to inform the window manager.
 */

    if (geomSpec == NULL)
        geomSpec = XGetDefault(u->dpy, ACM, ARG_GEOMETRY);

/*
 * If the defaults database doesn't contain a specification of the
 * initial size & position, locate it in the center of the screen.
 */

    if (geomSpec == NULL) {
	xsh.flags = PPosition | PSize;
	if (chaser == -1) {
	    xsh.height = FS_WINDOW_HEIGHT * DisplayWidth(u->dpy, screen) / 1280;
	    xsh.width =  FS_WINDOW_WIDTH * DisplayWidth(u->dpy, screen) / 1280;
	}
	else {
	    xsh.height = CHASE_WINDOW_HEIGHT * DisplayWidth(u->dpy, screen) / 1280;
	    xsh.width =  CHASE_WINDOW_WIDTH * DisplayWidth(u->dpy, screen) / 1280;
	}
	xsh.x = (DisplayWidth(u->dpy, screen) - xsh.width) / 2;
	xsh.y = (DisplayHeight(u->dpy, screen) - xsh.height) / 2;
    }
    else {
	int         bitmask;

	bzero((char *) &xsh, sizeof(xsh));
	bitmask = XParseGeometry (geomSpec, &xsh.x, &xsh.y,
		&xsh.width, &xsh.height);
	if (bitmask & (XValue | YValue)) {
	    xsh.flags |= USPosition;
	}
	if (bitmask & (WidthValue | HeightValue)) {
	    xsh.flags |= USSize;
	}
    }

    width = xsh.width;
    height = xsh.height;

/*
 * Create the Window with the information in the XSizeHints, the
 * border width,  and the border & background pixels.
 */

    win_attr_mask = CWColormap | CWBitGravity | CWBackPixel;
    window_attributes.colormap = cmap;
    window_attributes.bit_gravity = NorthWestGravity;
    window_attributes.background_pixel = BlackPixel(u->dpy, screen);

    u->win = XCreateWindow (u->dpy, RootWindow (u->dpy, screen),
		xsh.x, xsh.y, xsh.width, xsh.height, bw, depth,
		InputOutput, theVisual, win_attr_mask, &window_attributes);

    scale = (double) xsh.width / (double) FS_WINDOW_WIDTH;

/*
 * Create a pixmap of the engine RPM gauge and flap indicators.
 */

    u->eng = XCreateBitmapFromData (u->dpy, u->win, eng_bits, eng_width,
	eng_height);
    u->flap[0] = XCreateBitmapFromData (u->dpy, u->win, flaps0_bits,
	flaps0_width, flaps0_height);
    u->flap[1] = XCreateBitmapFromData (u->dpy, u->win, flaps1_bits,
	flaps1_width, flaps1_height);
    u->flap[2] = XCreateBitmapFromData (u->dpy, u->win, flaps2_bits,
	flaps2_width, flaps2_height);
    u->flap[3] = XCreateBitmapFromData (u->dpy, u->win, flaps3_bits,
	flaps3_width, flaps3_height);
    u->handle[0] = XCreateBitmapFromData (u->dpy, u->win, handleUp_bits,
	handleUp_width, handleUp_height);
    u->handle[1] = XCreateBitmapFromData (u->dpy, u->win, handleDn_bits,
	handleDn_width, handleDn_height);
    u->gearLight[0] = XCreateBitmapFromData (u->dpy, u->win, gearUp_bits,
	gearUp_width, gearUp_height);
    u->gearLight[1] = XCreateBitmapFromData (u->dpy, u->win, gearTran_bits,
	gearTran_width, gearTran_height);
    u->gearLight[2] = XCreateBitmapFromData (u->dpy, u->win, gearDown_bits,
	gearDown_width, gearDown_height);

/*
 * Set the standard properties and hints for the window managers.
 */

    XSetStandardProperties(u->dpy, u->win, ACM, ACM, None, argv, argc, &xsh);
    xwmh.flags = InputHint | StateHint;
    xwmh.input = True;
    xwmh.initial_state = NormalState;
    XSetWMHints(u->dpy, u->win, &xwmh);
    if (chaser == -1) {
	cursor = XCreateFontCursor (u->dpy, XC_tcross);
	XDefineCursor (u->dpy, u->win, cursor);
    }

/*
 *  Tell the window manager that we'd like to participate in the
 *  WM_CLOSEDOWN and WM_DELETE_WINDOW protocols.
 */

    u->protocolsAtom = XInternAtom (u->dpy, "WM_PROTOCOLS", False);
    atom[0] =
	u->closedownAtom = XInternAtom (u->dpy, "WM_CLOSEDOWN", False);
    atom[1] =
	u->deleteWindowAtom = XInternAtom (u->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols (u->dpy, u->win, atom, 2);

/*
 *  Fill-in the viewer structure
 */
    if (chaser == -1) {
      XSelectInput (u->dpy, u->win, KeyPressMask | ButtonPressMask |
   		  StructureNotifyMask | ButtonReleaseMask | ExposureMask);
      cf->team = team;
    }
    else {
      XSelectInput (u->dpy, u->win, KeyPressMask | StructureNotifyMask |
  		  ExposureMask);
    }

    cf->vl = u;
    strncpy (cf->name, logname, sizeof (cf->name));
    strncpy (cf->display, display, sizeof (cf->display));
    u->next = (viewer *) NULL;

/*
 *  We use the "redrew" flag to allow Vlib to optimize areas that we'll
 *  be drawing using X, as well.
 */

    u->redrew = 2;

/*
 *  COnnect to the audio server
 */
   
    if (initializeAudio(cf, u, display) != 0) {
    	sprintf (err, "Sound is not available for server %s\n", display);
	write (s, err, strlen(err));
    }
 
#ifdef USE_PIXMAP_ANIMATION
    if (depth != 1)
	_VDefaultWorkContext->usePixmaps = 1;
#endif

    _VDefaultWorkContext->usePixmaps = mono ? 1 : 0;

    if (chaser == -1) {
	u->v = VOpenViewport (u->dpy, screen, u->win, cmap, theVisual,
  			  UNITS_FEET, scale, 0.50,
  			  width * VIEW_WINDOW_WIDTH / FS_WINDOW_WIDTH,
  			  height * VIEW_WINDOW_HEIGHT / FS_WINDOW_HEIGHT);
    }
    else {
	u->v = VOpenViewport (u->dpy, screen, u->win, cmap, theVisual,
  			  UNITS_FEET, scale, 0.50, width, height);
    }

/*
 *  If this is a color-rich visual, then enable depth-cueing mode
 */
 
    if (depth >= 6) {
    	ViewportSetDepthCueing (u->v, 1);
    }

    if (VBindColors (u->v, background) < 0) {
	XCloseDisplay (u->dpy);
	free ((char *) u);
	ptbl[player].type = CT_FREE;
	sprintf (err, "Error in binding colors.\n");
	write (s, err, strlen(err));
	return -1;
    }

    whitePixel = whiteColor->cIndex;
    blackPixel = blackColor->cIndex;
    HUDPixel = HUDColor->cIndex;

/*
 * Create the GC for drawing the picture.
 */

    gcv.graphics_exposures = False;
    gcv.font = u->rfont->fid;
    u->gc = XCreateGC(u->dpy, u->win, GCGraphicsExposures | GCFont, &gcv);

    gcv.graphics_exposures = False;
    gcv.foreground = u->v->flags & VPMono ?
		WhitePixel(u->v->dpy, u->v->screen) :
		u->v->pixel[whitePixel];
    gcv.background = u->v->flags & VPMono ?
		BlackPixel(u->v->dpy, u->v->screen) :
		u->v->pixel[blackPixel];
    gcv.line_width = 2;
    gcv.font = u->rfont->fid;
    u->gauge_gc = XCreateGC(u->dpy, u->win,
		GCGraphicsExposures | GCForeground | GCBackground |
		GCLineWidth | GCFont, &gcv);

    if (chaser == -1) {
	resizePlayerWindow (cf, u, width, height, 1);
    }
    else {
	resizeChaserWindow (cf, u, width, height, 1);
    }
									       
/*
 * Map the window to make it visible.
 */

    XMapWindow(u->dpy, u->win);
    if (depth != 1) {
#ifdef notdef
	register int curPixel;

    	VExposeBuffer (u->v, u->gc);
	curPixel = *(u->v->pixel);
	XSetForeground (u->dpy, u->gc, curPixel);
	XFillRectangle (u->dpy, u->win, u->gc, 0, 0, u->width, u->height);
#endif
   }

    if (depth == 1)
	XSetWindowBackground (u->dpy, u->win, BlackPixel(u->dpy, screen));
    else
	XSetWindowBackground (u->dpy, u->win, VConstantColor(u->v, blackPixel));

    u->z.color = u->v->pixel[HUDPixel];
    u->z.depth = 1;

    u->rz.color = u->v->pixel[radarColor->cIndex];
    u->rz.depth = 2;

    if (chaser == -1) {
	++ ptblCount;
    }
    else {
	++ ctblCount;
    }

#ifdef HAVE_DIS
	if (disInUse) {
	    disLocation[0] = cf->Sg.x;
	    disLocation[1] = cf->Sg.y;
	    disLocation[2] = teamLoc[team].z;
	    disZeroVec[0] = 0.0;
	    disZeroVec[1] = 0.0;
	    disZeroVec[2] = 0.0;
	    disOrientation[0] = cf->curHeading;
	    disOrientation[1] = 0.0;
	    disOrientation[2] = 0.0;

	    dis_entityEnter (team, cf,
	    	&cf->cinfo->entityType,
	    	&cf->cinfo->altEntityType,
	    	disLocation, disZeroVec, disZeroVec,
		disOrientation, disZeroVec, &cf->disId);
	}
#endif	

    return 0;

}

int
killPlayer (c)
craft	*c;
{

	viewer	*v, *vn;
	int	i;
	
	playSound(c, SoundCrash);

/*
 *  Decrement the player count, iff this is a real person that just got
 *  killed.
 */

	if (c->type == CT_PLANE && (c->flags & FL_BLACK_BOX) == 0) {
		-- ptblCount;
	}

/*
 *  Erase our radar emissions
 */

	for (i=0; i<MAXPLAYERS; ++i)
		ptbl[i].rval[c->pIndex] = 0.0;

/*
 *  Replace the plane with an explosion.
 */

	newExplosion (&(c->Sg), 100.0, 15.0, 4.0);

/*
 *  Free HUD string storage
 */

	if (c->leftHUD[0] != (char *) NULL)
		for (i=0; i<3; ++i) {
			free (c->leftHUD[i]);
			free (c->rightHUD[i]);
		}

/*
 *  Close viewers' display
 */

	for (v=c->vl; v != (viewer *) NULL;) {
		VCloseViewport (v->v);
		XCloseDisplay (v->dpy);
		shutdownAudio (c, v);
		vn = (viewer *) v->next;
		free ((char *) v);
		v = vn;
	}

	if (c->flags && FL_RECORD)
		-- recordCount;

	if (c->flags & FL_BLACK_BOX)
		blackBoxKillPlayer (c->pIndex);

#ifdef HAVE_DIS
	if (c->type == CT_PLANE || c->type == CT_DRONE) {
	        /* active (broadcasting) player, announce its death */
#ifdef DIS_DEBUG
		printf("Killed local player/drone: %d %d\n", c->pIndex, c->disId);
#endif
	        dis_entityExit (c->disId);
	}
#endif

	c->type = CT_FREE;
	return 0;
}
