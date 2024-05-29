#include <Alib.h>
#include <stdio.h>

extern void EdgeTableToScanLine (), ScanLineDifference();

void FrameComplete (w)
AWindow		*w; {

	register ScanLine	*tmp;
	register int		i;
#ifdef DEBUG
	register int		cstop;
#endif

/*
 *  Convert the edge table to scan line segments.
 */

	EdgeTableToScanLine (w);

/*
 *  Determine the differences between this frame and the previous one
 *  an plot the differences.
 */

	ScanLineDifference (w);

/*
 *  This frame now becomes the previous frame; clear the new current frame.
 */

	tmp = w->lastScanLine;
	w->lastScanLine = w->scanLine;
	w->scanLine = tmp;

	for (i=0; i<(w->height+1); ++i) {
		w->scanLine[i].count = 0;
		w->edges[i].head = (Edge *) NULL;
		w->lines[i].head = (Edge *) NULL;
	}

/*
 *  Release the allocated color segments for what was the last frame.
 */

	if (w->curPool == 0) {
		w->curPool = 1;
#ifdef DEBUG
		cstop = w->CSTop1;
#endif
		w->CSTop1 = 0;
	}
	else {
		w->curPool = 0;
#ifdef DEBUG
		cstop = w->CSTop0;
#endif
		w->CSTop0 = 0;
	}

/*
 *  Release the allocated elements of the edge pool.
 */

#ifdef DEBUG
	printf ("color segments = %d; edges = %d\n", cstop, w->EPTop);
#endif

	w->EPTop = 0;

	w->ymin = 0x7FFF;
	w->ymax = -1;

/*
 *  Perform any graphics-dependent finish-up
 */

#ifdef X11
	AX11FlushBufferedSegments (w);
#endif

#ifdef DEBUG
	printf ("End of Frame\n");
#endif

}
