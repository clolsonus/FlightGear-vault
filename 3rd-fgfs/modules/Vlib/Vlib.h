#ifndef __Vlib
#define __Vlib

#include <math.h>

#ifndef M_PI
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#endif

#define HAS_FRAME_BUFFER

#define VmaxVP	2048		/* max # of vertices in a polygon */

#define MAXCOLORS 8		/* max number of colors available */
				/* when double buffering */

#if (MAXCOLORS==4)
#define PLANES 2
#endif
#if (MAXCOLORS==8)
#define PLANES 3
#endif
#if (MAXCOLORS==16)
#define PLANES 4
#endif

#define NUM_ASPECTS 10

#include <X11/Xlib.h>
#include <Alib.h>
#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if !defined(__GNUC__)
#include <memory.h>
#endif

typedef struct _vcolor {
	char	*color_name;	/* text name */
	short	flags;
	short	cIndex;		/* index into pixel value or halftone table */
	struct _vcolor *next;
	} VColor;

/*
 *  VColor flag values
 */

#define ColorEnableDepthCueing	1

typedef struct {
	double	x, y, z;	/* A point or vector in 3-space */
	} VPoint;

typedef struct {
	double	s;
	VPoint	v;
	} VQuaternion;

typedef struct {
	short	flags;		/* flag word */
	short	numVtces;	/* vertex count */
	short	assignedDepth;	/* hard-assigned depth cue or -1 */
	VPoint	normal;		/* normal vector plus vertex[0] */
	VPoint	*vertex;	/* pointer to array of vertices (& normal) */
	VColor	*color;		/* pointer to color descriptor */
	VColor	*backColor;	/* pointer to back's color (sometimes NULL) */
	} VPolygon;

/*
 *  VPolygon Flags
 */

#define PolyClipBackface	0x01	/* Polygon is "one-sided" */
#define PolyUseBackColor	0x02	/* should be rendered using backColor*/
#define PolyNormalValid		0x04	/* "normal" contains valid info */

typedef struct {
	char	*name;		/* object name */
	VPoint	xaxis, yaxis, zaxis;
	VPoint	center;		/* average of all the object's points */
	double	extent;		/* distance from center to most distant point */
	int	numPolys;	/* polygon count */
	VPolygon **polygon;	/* pointer to array of polygon pointers */
	unsigned short *order;	/* 3-D relative polygon depth ordering */
	} VObject;

/*
 *  VObjects and VPolygons are an adequate model for defining simple
 *  objects.  If there are many vertices that are shared among an objects
 *  polygons, though, substantial time may be wasted re-transforming the
 *  same point.  Here we create the notion of a VTerseObject and terse polygon
 *  where all vertices are shared in a common array.
 */

typedef struct {
	short	flags;		/* flag word */
	short	numVtces;	/* vertex count for this polygon */
	VPoint	normal;		/* normal vector plus vertex[0] */
	struct _VSurfaceObject *parent; /* object that owns this polygon */
	unsigned long *vertex;	/* indices the vertices in this polygon */
	VColor	*color;		/* pointer to color descriptor */
	VColor	*backColor;	/* pointer to back's color (sometimes NULL) */
	} VTersePolygon;

typedef struct _VSurfaceObject {
	char	*name;
	VPoint	center;
	double	extent;
	long	numPolys;
	long	numVtces;	/* number of points in "point" array */
	VPoint	*point;		/* pts shared by all VTersePolygons in object */
	VPoint	*tpoint;	/* temporary (transformed) point storage */
	unsigned short *order;
	} VTerseObject;

typedef struct {
	double	m[4][4];
	} VMatrix;

typedef struct {
	AWindow	  *w;
	unsigned long depth;	/* polygon depth while plotting */
	unsigned long flags;	/* viewport flags */
	VMatrix	  eyeSpace;	/* transforms from world to eyeSpace system */
	VPolygon  *clipPoly;	/* planes to clip viewed polygons */
	VPoint    clipNormals[4]; /* normal vectors corresponding to clipPoly */
	double	  units;	/* world units expressed in meters */
	double	  dist;		/* distance in units from eye to screen */
	double	  xres;		/* x screen resolution in dots per unit */
	double	  yres;		/* y screen resolution in dots per unit */
	int	  width;	/* width of window in dots */
	int	  height;	/* height of window in dots */
	Point	  Middl;	/* center of the window */
	VPoint	  Scale;	/* scaling factor */
	Display	  *dpy;		/* Display associated with this viewport */
	int	  screen;	/* X screen number */
	Window	  win;
	Pixmap	  monoPixmap;	/* Pixmap used to buffer drawing */
	Pixmap	  halftone[8];	/* Pixmaps used for dithering images */

	int	  colors;	/* color count */
	int	  set;		/* id of buffer currently in use */
	unsigned long mask;	/* current plane mask */
	unsigned long aMask, bMask;
	unsigned long *pixel;	/* current pixel drawing values */
	unsigned long *aPixel;
	unsigned long bPixel[MAXCOLORS];
	XColor	  xdepthCueColor;
	XColor	  aColor[MAXCOLORS*MAXCOLORS];
	XColor	  bColor[MAXCOLORS*MAXCOLORS];
	Colormap  cmap;
	Visual	  *visual;
	ZInfo	  *zpool;
	long	  zsize;
	long	  ztop;
	void	  (*colorTweakProc)();
	} Viewport;

typedef struct {
	int	flags;			/* flag word */
	VColor	*VColorList;
	int	usePixmaps;
	int	nextPixel;		/* next pixel cell to allocate */
	int	depthCueSteps;		/* 0 = no haze */
	VColor	*depthCueColor;		/* haze color */
	double	visibility;		/* visibility (in feet) */
	double	*visTable;
	}	VWorkContext;

#define WCLocked		1	/* colors have been bound */

#define VSetPoint(p, xi, yi, zi)	{ p.x = xi; p.y = yi; p.z = zi; }
#define VGetPolygonPixel(p)		(p->color->xcolor.pixel)
#define VLookupViewportColor(v,n)	(v->pixel[n])
#define VGetViewportMask(v) 		(v->mask)
#define VConstantColor(v,cIndex)	(v->aPixel[cIndex] | v->bPixel[cIndex])
#define VUnitVectorI()			(& _VUnitVectorI)
#define VUnitVectorJ()			(& _VUnitVectorJ)
#define VUnitVectorK()			(& _VUnitVectorK)
#define VPointToClipPlaneDistance(pt, pl) \
	(- VDotProd(pl, pt) / ((pl)->x * (pl)->x + (pl)->y * (pl)->y + \
	(pl)->z * (pl)->z))
#define Vmagnitude(pt)	(sqrt((pt)->x*(pt)->x+(pt)->y*(pt)->y+(pt)->z*(pt)->z))
#define VSetDepthCue(color,s)	{ _VDefaultWorkContext->depthCueColor=(color); \
	_VDefaultWorkContext->depthCueSteps=(s); }

/*
 *  Globals
 */

extern VWorkContext	_v_default_work_context;
extern VWorkContext	*_VDefaultWorkContext;
extern VPoint		_VUnitVectorI, _VUnitVectorJ, _VUnitVectorK;

#ifndef PARAMS
#ifndef __STDC__
#define PARAMS(arglist) ()
#else
#define PARAMS(arglist)	arglist
#endif
#endif

/*
 *  V macros and function prototypes
 */

#define VDestroyPoints(a)	free((char *) a)
#define VDestroyPolygon(a)	{free((char *) a->vertex); free((char *) a);}

extern Viewport *VOpenViewport PARAMS((Display *, int, Window, Colormap,
	Visual *, double,  double, double, int, int));
extern void	 VResizeViewport PARAMS((Viewport *, double, double,
	double, int, int));
extern void	 VCloseViewport();	/* (Viewport *) */
extern VPolygon *VCreatePolygon PARAMS((int npts, VPoint *pts, VColor *));
extern VPolygon *VCreatePolygonFromTemplate PARAMS((int npts, VPoint *pts,
	VPolygon *template));
extern VPolygon *VCopyPolygon PARAMS((VPolygon *));
extern VObject	 *VCopyObject PARAMS((VObject *));
extern VPolygon *VClipPolygon PARAMS((VPolygon *, VPolygon *));
extern VPolygon *VClipSidedPolygon PARAMS((Viewport *, VPolygon *, VPolygon *));
extern VPoint	*VCreatePoints();	/* (int) */
extern VColor	*VAllocColor PARAMS((char *));
extern VColor	*VAllocDepthCueuedColor PARAMS((char *, int flag));
extern int	VBindColors PARAMS((Viewport *, char *));
extern void	VExposeBuffer PARAMS((Viewport *, GC));
extern VObject	*VReadObject PARAMS((FILE *f));
extern VObject	*VReadDepthCueuedObject PARAMS((FILE *f, int flag));
extern int	VWriteObject PARAMS((FILE *, VObject *));
extern VMatrix	*VRotate PARAMS((VMatrix *, int axis, double theta));
extern VMatrix  *VTranslate PARAMS((VMatrix *, double, double, double));
extern VMatrix  *VTranslatePoint();	/* (VMatrix *, VPoint) */
extern char	*Vmalloc();		/* (int) */
extern int	VEyeToScreen();
extern int	VWorldToScreen();
extern void	VMatrixMultByRank PARAMS((VMatrix *, VMatrix *,
		VMatrix *, int));
extern int	VFontWidthPixels();
extern void	VDrawStrokeString();
extern void	VDrawArc();
extern void	VDrawSegments();
extern void	VFillPolygon();
extern VPolygon	 * VTransformPolygon();
extern void	VGetEyeSpace();
extern VPolygon * VGetPlanes();
extern void	VIdentMatrix();
extern int	VComputeObjectAspect();
extern int	VObjectNeedsOrdering();
extern void	VComputePolygonOrdering();
extern char	*VGetAspectName();
extern void	VSetVisibility PARAMS ((double d));
extern int	VComputePolygonColor PARAMS((Viewport *v, VPolygon *poly));
extern int	ViewportSetDepthCueing PARAMS((Viewport *v, int flag));

#if defined(__GNUC__) && defined(__STDC__) && !defined(_NO_INLINE)
static inline void
VCrossProd (a, b, r)
VPoint	*a, *b, *r;
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
}

static inline double
VDotProd (a, b)
VPoint	*a, *b;
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

static inline void
VMatrixMult (Mt1, Mt2, R)
VMatrix *Mt1, *Mt2, *R;
{
	register int I, J, K;
	register double x;

	for (I=0; I<4; ++I)
		for (J=0; J<4; ++J) {
			x = 0.0;
			for (K=0; K<4; ++K)
				x += Mt1->m[K][J] * Mt2->m[I][K];
			R->m[I][J] = x;
		}
}

static inline void
VTransform (pt, mt, newPt)
VPoint	*pt;
VMatrix *mt;
VPoint	*newPt;
{

  newPt->x = pt->x * mt->m[0][0] + pt->y * mt->m[0][1]
		+ pt->z * mt->m[0][2] + mt->m[0][3];

  newPt->y = pt->x * mt->m[1][0] + pt->y * mt->m[1][1]
  		+ pt->z * mt->m[1][2] + mt->m[1][3];

  newPt->z = pt->x * mt->m[2][0] + pt->y * mt->m[2][1]
  		+ pt->z * mt->m[2][2] + mt->m[2][3];
}

static inline void
VTransform_ (pt, mt, newPt)
VPoint	*pt;
VMatrix *mt;
VPoint	*newPt;
{

  newPt->x = pt->x * mt->m[0][0] + pt->y * mt->m[0][1]
		+ pt->z * mt->m[0][2];

  newPt->y = pt->x * mt->m[1][0] + pt->y * mt->m[1][1]
  		+ pt->z * mt->m[1][2];

  newPt->z = pt->x * mt->m[2][0] + pt->y * mt->m[2][1]
  		+ pt->z * mt->m[2][2];
}
#else
extern void	VMatrixMult PARAMS((VMatrix *, VMatrix *, VMatrix *));
extern double	VDotProd PARAMS((VPoint *, VPoint *));
extern void	VCrossProd PARAMS((VPoint *, VPoint *, VPoint *));
extern void	VTransform PARAMS((VPoint *, VMatrix *, VPoint *));
extern void	VTransform_ PARAMS((VPoint *, VMatrix *, VPoint *));
#endif

/*
 *  Viewport flags (must be changed manually after VOpenViewport for now)
 */

#define	VPClip		1	/* polygons should be clipped before drawing */
#define VPPerspective	2	/* Z coordinate used for depth information */
#define VPMono		4	/* Monochrome environment */
#define VPPixmap	8	/* Use color Pixmap rather than double
					buffering */
#define VPFastAnimation 16
#define VPDepthCueing	32	/* Perform depth cueing */
#define VPDepthCueParsed 64	/* Pixel has been assigned to dc color */

/*
 *  VRotate options
 */

#define XRotation	1	/* rotate about X axis */
#define YRotation	2	/* rotate about Y axis */
#define ZRotation	3	/* rotate about Z axis */

/*
 *  Some units values for VOpenViewport()
 */

#define UNITS_METERS	1.0
#define UNITS_FEET	0.3048

/*
 *  Angle Conversions
 */

#define DEGtoRAD(a)	((a) * M_PI / 180.0)
#define RADtoDEG(a)	((a) * 180.0 / M_PI)

#ifndef SYSV
extern char *strdup();
#endif

#if (defined(_AIX) && defined(_IBMR2)) && !(defined(__GNUC__) \
     && (__GNUC__ == 2 ))
/* For some reason, the RS/6000 AIX header files don't declare strdup. */
extern char *strdup(const char *);
#endif

#endif __Vlib
