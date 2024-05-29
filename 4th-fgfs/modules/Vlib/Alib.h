#define X11

#include <stdio.h>

#ifdef X11
#undef HAS_FRAME_BUFFER		/* use direct frame buffer output */
#define X11_DRAW_SEGMENTS	/* draw segments instead of rectangles */
#include <X11/Xlib.h>
#endif

#ifdef HAS_FRAME_BUFFER
extern void FrameBufferIOInit(), FrameBufferRun();
#endif

typedef unsigned short Color;

#ifdef X11

typedef XPoint Point;
typedef XSegment Segment;
typedef XRectangle Rectangle;

#else /* X11 */

typedef struct {
	short	x, y;
	} Point;

typedef struct {
	short	x1, y1, x2, y2;
	} Segment;

typedef struct {
	short	x, y;
	unsigned short width, height;
	} Rectangle;

#endif /* X11 */

typedef struct _ZInfo {
	Color	color;		/* color of this polygon */
	unsigned long depth;	/* depth of this polygon */
	struct	_ZInfo *next;	/* next polygon in plane sweep set */
	struct	_ZInfo *prev;	/* previous polygon in plane sweep set */	
	}	ZInfo;

#define NotAnElement		((ZInfo *) -1)

#define MaxDepth		0xFFFFFFFF
#define MinDepth		0

/*
 *  Polygons are built from edges
 */

typedef struct _edge {
	short	y2;		/* Ending y location */
#ifdef FLOAT_SLOPE
	float	x1;		/* Starting x location */
	float	Dx;		/* Inverse slope of edge line */
#else
	long	x1;		/* Starting x location */
	long	Dx;		/* Inverse slope of edge line */
#endif
	ZInfo	*p;		/* depth and color information */
	struct _edge	*nexte;	/* next edge on this edge list */ 
	struct _edge	*next;	/* next edge on active edge list */
	} Edge;

/*
 *  An edge list contains a pointer to a list of Edges.
 */

typedef struct {
	Edge	*head;		/* pointer to first edge in list */
	} EdgeList;

/*
 *  A ColorSegment is a row of adjacent pixels all of the same color.
 */

typedef struct {
	short		x;	/* starting x location */
	unsigned short	length;	/* number of pixels of this color */
	Color		color;	/* color of this segment */
	} ColorSegment;

/*
 *  A ScanLine is composed of a vector of ColorSegment's.
 */

typedef struct {
	ColorSegment	*head;	/* first element in the list */
	ColorSegment	*tail;	/* last element in the list */
	int		count;
	} ScanLine;

#ifdef X11

/*
 *  Randomly changing drawing colors in X would require that we add an XChangeGC
 *  request before virtually every line segment draw.
 *  Once the scan line segment output has been optimized by ScanLineDifference,
 *  we can add a further optimization to reduce the number of XChangeGC
 *  requests that are required.  We'll add a structure to group drawing
 *  requests by color.  When some watermark is past, an XChangeCG request may
 *  be issued, followed by all requests for that color.  This request buffer
 *  will be flushed with a call to FrameComplete().
 */

typedef struct {
#ifdef X11_DRAW_SEGMENTS
	XSegment	*head;	/* pointer to a vector of BufferedSegment's */
#else
	XRectangle	*head;	/* pointer to a vector of BufferedSegment's */
#endif
	int		count;	/* number of seg's currently in use */
	}	SegmentGroup;

#define AMaxPixelValue	255	/* we'll only handle up-to 8-bit Visuals */

#endif	

/*
 *  To perform animation in a window, we associate an AWindow structure
 *  with it.
 */

typedef struct {
#ifdef X11
	Display		*display;	/* X display for this window */
	Drawable	d;		/* dest drawable for this window */
	GC		gc;
	int		bsegSize;	/* Count of elements in each bseg vec */
	SegmentGroup	bseg[AMaxPixelValue+1];
	Pixmap		*stipple;	/* pointer to a list of pixmaps */
	int		flags;
#endif
	short		width, height;	/* dimensions of this window */
	short		ymin, ymax;	/* range of EdgeList's with polygons */
	Segment		clip;		/* clipping bounds */
	ScanLine	*scanLine;	/* a vector with height elements */
	ScanLine	*lastScanLine;	/* a vector with height elements */
	EdgeList	*edges;		/* a vector with height elements */
	EdgeList	*lines;		/* a vector with height elements */
	Edge		*edgePool;	/* a pool of polygon edges */
	unsigned int	EPSize;		/* number of entries in edgePool */
	unsigned int	EPTop;		/* index of first free edge */
	unsigned int	curPool;	/* selects csPool (0 or 1) */	
	ColorSegment	*csPool0;
	unsigned int	CSSize0;	/* number of entries in csPool0 */
	unsigned int	CSTop0;		/* index of first free color seg */
	ColorSegment	*csPool1;
	unsigned int	CSSize1;	/* number of entries in csPool1 */
	unsigned int	CSTop1;		/* index of first free color seg */
	} AWindow;

#ifdef X11
extern AWindow *InitializeX11AWindow();

#define AMonoMode	1		/* monochrome flag */
#endif
extern AWindow *InitializeAWindow();
extern void CloseAWindow();
extern void DrawPolygon ();
extern void FillPolygon ();
extern void FillRectangle ();
extern void DrawLine ();
extern void DrawPoint();
extern void DrawLines ();
extern void DrawArc ();
extern void FrameComplete ();
extern void ForceWindowRedraw ();
extern int  ClipLine();
extern void AX11FlushBufferedSegments(); 

