#include <Vlib.h>


int	HUDPixel;	/* index of HUD color in viewport(s) */
VColor	*HUDColor;


typedef struct _viewer {
    struct _viewer *next;
    /* Display *dpy; */		        /* display for this user */
#if defined(HPAUDIO)
	Audio		*aserver;	/* audio server for this user */
	SBucket		*sound[NUM_SOUNDS];
	ATransID	flow[NUM_SOUNDS];
#else
/*
 *  These fields should be defined even if no audio driver is present
 */

	AuServer	*aserver;	/* audio server for this user */
	AuBucketID	sound[NUM_SOUNDS];
	AuFlowID	flow[NUM_SOUNDS];
	AuDeviceID	audioOutput[1];	/* audio output device ID */
#endif
	Window		win;		/* window for this user */
	GC		gc;		/* GC for drawing */
	GC		gauge_gc;	/* GC for RPM, TEWS, etc */
	XFontStruct	*font;		/* HUD font */
	XFontStruct	*rfont;		/* radar font */
	Viewport	*v;		/* Viewport for out-of-cockpit views */
	Pixmap		eng;		/* Engine "RPM" gauge */
	Pixmap		flap[5];	/* flap setting indicators */
	Pixmap		handle[2];	/* ldg gear handle indicators */
	Pixmap		gearLight[3];	/* ldg gear status lights */
	Atom		protocolsAtom;
	Atom		deleteWindowAtom;
	Atom		closedownAtom;
	int		cn;		/* X connection fd */
	int		width, height;	/* width & height of viewing window */
	int		xCenter, yCenter; /* center of viewing window */
	int		rftw, rfth;	/* radar font width & height */
	short		rx, ry;		/* radar x and y location */
	int		radarWidth;	/* width of radar screen (pixels) */
	int		radarHeight;	/* height of radar screen (pixels) */
	int		lastRPM;	/* last RPM fraction x 1000(0 .. 1000)*/
	int		lastFlap;	/* last flap setting rad x 1000 */
	int		redrew;		/* 1 = we redrew last cycle */
	scaleControl	altScale;	/* altitude scale control */
	scaleControl	velScale;	/* airspeed scale control */
	scaleControl	hdgScale;	/* heading scale control */
	int		TEWSx, TEWSy;	/* location of center of TEWS display */
	int		TEWSSize;	/* size of TEWS display (pixels) */
	int		panelx, panely;	/* location of top/right corner */
	double		scaleFactor;	/* overall scaling factor for display */
	ZInfo		z;		/* depth/color information for HUD */
	ZInfo		rz;		/* depth/color information for radar */
} viewer;


#define ALT_ORG_X		780
#define ALT_ORG_Y		360
#define ALT_LENGTH		219	
#define ALT_ORIENT		orientRight
#define ALT_SCALE		(1530.0 / (double) ALT_LENGTH)
#define ALT_INDEX_SIZE		32
#define ALT_MIN_INTERVAL	100
#define ALT_MIN_SIZE		9
#define ALT_MAJ_INTERVAL	500
#define ALT_MAJ_SIZE		17
#define ALT_DIVISOR		1000.0
#define ALT_FORMAT		"%4.3g"

