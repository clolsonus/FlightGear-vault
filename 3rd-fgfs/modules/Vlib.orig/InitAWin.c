#include <Alib.h>
#include <stdio.h>

#ifdef X11

#include <X11/Xlib.h>

extern char *malloc();

#ifdef HAS_FRAME_BUFFER
void
LocateWindowOrigin (display, w, xo, yo)
Display	*display;
Window  w;
int	*xo, *yo;
{
	XWindowAttributes win;
	Window		child;
	int		wx, wy;

	XGetWindowAttributes (display, w, &win);

	XTranslateCoordinates (display, w, win.root, 
		- win.border_width, - win.border_width, &wx, &wy, &child);

	*xo = wx;
	*yo = wy;

}
#endif

AWindow *
InitializeX11AWindow (dpy, screen, d, flags)
Display		*dpy;
int		screen;
Drawable	d;
int		flags; {


	register AWindow	*w;
	register int		i;
	int			x, y;
	Window			root;
	unsigned int		width, height, bwidth, depth;

	XGetGeometry (dpy, d, &root, &x, &y, &width, &height, &bwidth, &depth);

	if ((w = (AWindow *) malloc (sizeof(AWindow))) == (AWindow *) NULL)
		return w;

	w->width = width;
	w->height = height < 1024 ? 1024 : height;

	w->clip.x1 = 0;
	w->clip.y1 = 0;
	w->clip.x2 = width - 1;
	w->clip.y2 = height - 1;

	w->edges = (EdgeList *) malloc ((w->height + 1) * sizeof(EdgeList));
	w->lines = (EdgeList *) malloc ((w->height + 1) * sizeof(EdgeList));
	w->EPSize = 64 * 1024;
	w->edgePool = (Edge *) malloc (w->EPSize * sizeof(Edge));
	memset(w->edgePool, 0, w->EPSize * sizeof(Edge));

	w->CSSize0 = w->height * 32;
	w->csPool0 = (ColorSegment *) malloc (w->CSSize0 * sizeof(ColorSegment));
	memset(w->csPool0, 0, w->CSSize0 * sizeof(ColorSegment));

	w->CSSize1 = w->height * 32;
	w->csPool1 = (ColorSegment *) malloc (w->CSSize1 * sizeof(ColorSegment));
	memset(w->csPool1, 0, w->CSSize1 * sizeof(ColorSegment));

	w->scanLine = (ScanLine *) malloc ((w->height + 1) * sizeof(ScanLine));
	w->lastScanLine = (ScanLine *) malloc ((w->height + 1) * sizeof(ScanLine));

	w->EPTop = w->CSTop0 = w->CSTop1 = w->curPool = 0;

	w->ymin = 0x7FFF;
	w->ymax = -1;

	for (i=0; i<w->height + 1; ++i) {

		w->scanLine[i].count = 0;
		w->scanLine[i].head = 0;
		w->scanLine[i].tail = 0;
		w->lastScanLine[i].count = 0;
		w->lastScanLine[i].head = 0;
		w->lastScanLine[i].tail = 0;
		w->edges[i].head = (Edge *) NULL;
		w->lines[i].head = (Edge *) NULL;

	}

	w->bsegSize = 256; 

	for (i=0; i<AMaxPixelValue; ++i)
		w->bseg[i].count = -1;

	w->display = dpy;
	w->d = d;
	w->gc = XCreateGC (dpy, RootWindow (dpy, screen),
		0L, (XGCValues *) NULL);
	w->flags = flags;

	if (flags & AMonoMode)
		XSetFillStyle (dpy, w->gc, FillOpaqueStippled);

#ifdef HAS_FRAME_BUFFER
	FrameBufferIOInit ();
	LocateWindowOrigin (dpy, d, &x, &y);
	FrameBufferSetOrigin (x, y);
#endif

	return w;
}

#endif

AWindow *
InitializeAWindow (width, height)
unsigned int	width, height; {

	register AWindow	*w;
	register int		i;

	if ((w = (AWindow *) malloc (sizeof(AWindow))) == (AWindow *) NULL)
		return w;

	w->width = width;
	w->height = height;

	w->edges = (EdgeList *) malloc (w->height * sizeof(EdgeList));
	w->lines = (EdgeList *) malloc (w->height * sizeof(EdgeList));
	w->EPSize = 64 * 1024;
	w->edgePool = (Edge *) malloc (w->EPSize * sizeof(Edge));
	memset(w->edgePool, 0, w->EPSize * sizeof(Edge));

	w->CSSize0 = w->height * 10;
	w->csPool0 = (ColorSegment *) malloc (w->CSSize0 * sizeof(ColorSegment));
	memset(w->csPool0, 0, w->CSSize0 * sizeof(ColorSegment));

	w->CSSize1 = w->height * 10;
	w->csPool1 = (ColorSegment *) malloc (w->CSSize1 * sizeof(ColorSegment));
	memset(w->csPool1, 0, w->CSSize1 * sizeof(ColorSegment));

	w->scanLine = (ScanLine *) malloc (w->height * sizeof(ScanLine));
	w->lastScanLine = (ScanLine *) malloc (w->height * sizeof(ScanLine));

	w->EPTop = w->CSTop0 = w->CSTop1 = w->curPool = 0;

	w->ymin = 0x7FFF;
	w->ymax = -1;

	for (i=0; i<w->height; ++i) {

		w->scanLine[i].count = 0;
		w->scanLine[i].head = 0;
		w->scanLine[i].tail = 0;
		w->lastScanLine[i].count = 0;
		w->lastScanLine[i].head = 0;
		w->lastScanLine[i].tail = 0;
		w->edges[i].head = (Edge *) NULL;
		w->lines[i].head = (Edge *) NULL;

	}

	return w;
}

void /*ARGSUSED*/
ResizeAWindow (w, height, width)
AWindow *w;
int	height, width;
{
	register int i;

	if (w->edges) {
		free ((char *) w->edges);
	}
	if (w->lines) {
		free ((char *) w->lines);
	}
	if (w->edgePool) {
		free ((char *) w->edgePool);
	}
	if (w->scanLine) {
		free ((char *) w->scanLine);
	}
	if (w->lastScanLine) {
		free ((char *) w->lastScanLine);
	}

	w->width = width;
	w->height = height;
	w->clip.x2 = width - 1;
	w->clip.y2 = height - 1;

	w->edges = (EdgeList *) malloc ((w->height + 1) * sizeof(EdgeList));
	w->lines = (EdgeList *) malloc ((w->height + 1) * sizeof(EdgeList));
	w->EPSize = 64 * 1024;
	w->edgePool = (Edge *) malloc (w->EPSize * sizeof(Edge));
	memset(w->edgePool, 0, w->EPSize * sizeof(Edge));

	w->CSSize0 = w->height * 32;
	w->csPool0 = (ColorSegment *) malloc (w->CSSize0 * sizeof(ColorSegment));
	memset(w->csPool0, 0, w->CSSize0 * sizeof(ColorSegment));

	w->CSSize1 = w->height * 32;
	w->csPool1 = (ColorSegment *) malloc (w->CSSize1 * sizeof(ColorSegment));
	memset(w->csPool1, 0, w->CSSize1 * sizeof(ColorSegment));

	w->scanLine = (ScanLine *) malloc ((w->height + 1) * sizeof(ScanLine));
	w->lastScanLine = (ScanLine *) malloc ((w->height + 1) * sizeof(ScanLine));

	w->EPTop = w->CSTop0 = w->CSTop1 = w->curPool = 0;

	w->ymin = 0x7FFF;
	w->ymax = -1;

	for (i=0; i<w->height + 1; ++i) {

		w->scanLine[i].count = 0;
		w->scanLine[i].head = 0;
		w->scanLine[i].tail = 0;
		w->lastScanLine[i].count = 0;
		w->lastScanLine[i].head = 0;
		w->lastScanLine[i].tail = 0;
		w->edges[i].head = (Edge *) NULL;
		w->lines[i].head = (Edge *) NULL;

	}
}
