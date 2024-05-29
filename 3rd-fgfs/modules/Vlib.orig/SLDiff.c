#include <Alib.h>
#include <stdio.h>

extern char * malloc();

#ifdef X11

void AX11AllocBufferedSegments ();

#ifdef X11_DRAW_SEGMENTS
#define  OutputSegment(w, yp, x0p, x1p, c) \
	{ \
		register int		i; \
		register XSegment	*p; \
 \
		if (w->bseg[c].count == -1) \
			AX11AllocBufferedSegments (w, c); \
		i = ++(w->bseg[c].count); \
		p = &(w->bseg[c].head[i-1]); \
		p->x1 = x0p; \
		p->x2 = x1p; \
		p->y1 = p->y2 = yp; \
		if (i == w->bsegSize) { \
			if (w->flags & AMonoMode) \
				XSetStipple (w->display, w->gc, \
					w->stipple[c]); \
			else \
				XSetForeground (w->display, w->gc, c); \
			p = w->bseg[c].head; \
			XDrawSegments (w->display, w->d, w->gc, p, \
				w->bsegSize); \
			w->bseg[c].count = 0; \
		} \
	}
#else
#define  OutputSegment(w, yp, x0p, x1p, c) \
	{ \
		register int		i; \
		register XRectangle	*p; \
 \
		if (w->bseg[c].count == -1) \
			AX11AllocBufferedSegments (w, c); \
		i = ++(w->bseg[c].count); \
		p = &(w->bseg[c].head[i-1]); \
		p->x = x0p; \
		p->width = x1p - x0p + 1; \
		p->y= yp; \
		p->height = 1; \
		if (i == w->bsegSize) { \
			if (w->flags & AMonoMode) \
				XSetStipple (w->display, w->gc, \
					w->stipple[c]); \
			else \
				XSetForeground (w->display, w->gc, c); \
			p = w->bseg[c].head; \
			XFillRectangles (w->display, w->d, w->gc, p, \
				w->bsegSize); \
			w->bseg[c].count = 0; \
		} \
	}
#endif

void AX11FlushBufferedSegments (w)
AWindow	*w; {

	register int	i;

	for (i=0; i<AMaxPixelValue; ++i) {
		if (w->bseg[i].count > 0) {
			if (w->flags & AMonoMode) {
				XSetStipple (w->display, w->gc,
					w->stipple[i]);
			}
			else {
				XSetForeground (w->display, w->gc, i);
			}
#ifdef X11_DRAW_SEGMENTS
			XDrawSegments (w->display, w->d, w->gc,
				w->bseg[i].head, w->bseg[i].count);
#else
			XFillRectangles (w->display, w->d, w->gc,
				w->bseg[i].head, w->bseg[i].count);
#endif
			w->bseg[i].count = 0;
		}
	}
}

void AX11AllocBufferedSegments (w, c)
AWindow	*w;
Color	c; {

#ifdef X11_DRAW_SEGMENTS
	w->bseg[c].head = (XSegment *) malloc (w->bsegSize *
		sizeof (XSegment));
	if (w->bseg[c].head == (XSegment *) NULL) {
		fprintf (stderr, "Error allocating A library buffered segments\n");
		exit (1);
	}
#else
	w->bseg[c].head = (XRectangle *) malloc (w->bsegSize *
		sizeof (XRectangle));
	if (w->bseg[c].head == (XRectangle *) NULL) {
		fprintf (stderr, "Error allocating A library buffered segments\n");
		exit (1);
	}
#endif
	w->bseg[c].count = 0;

}

#endif

#ifdef DEBUG
void printScanLine (s, count)
ColorSegment	*s;
int		count; {

	for ( ; count > 0; --count, ++s)
		printf ("[%d %d %d ] ", s->x, s->length, s->color);

	printf ("\n");
}
#endif

void ScanLineDifference (w)
AWindow	*w; {

	register ColorSegment	*cur, *last;
	register int	len, curX = 0, curLength = 0, curCount, lastCount, y;

	for (y=0; y<w->height; ++y) {

	cur = w->scanLine[y].head;
	curCount = w->scanLine[y].count;

	if (curCount > 0) {
		curX = cur->x;
		curLength = cur->length;
	}

	last = w->lastScanLine[y].head;
	lastCount = w->lastScanLine[y].count;

#ifdef DEBUG
	if (curCount > 1 || lastCount > 1) {
		printf ("\ny = %d\nLast scan line:\n", y);
		printScanLine (last, lastCount);
		printf ("Current scan line:\n");
		printScanLine (cur, curCount);
	}
#endif

	while (curCount != 0) {

		if (lastCount == 0) {
#ifdef DEBUG
			printf ("row: %d, %d to %d color %d\n",
				y, curX, curX + curLength - 1,
				cur->color);
#endif
#ifdef HAS_FRAME_BUFFER
			FrameBufferRun (curX, y, curLength,
				cur->color);
#else
			OutputSegment (w, y, curX, curX + curLength - 1,
				cur->color);
#endif
			++ cur;
			curX = cur->x;
			curLength = cur->length;
			-- curCount;
		}

		else if (curX == last->x) {

			if (curLength == last->length) {

/*
 *  The trivial case:  Both segments have the same length.  If the colors are
 *  different, then output the correct new color.
 */

				if (cur->color != last->color) {
#ifdef HAS_FRAME_BUFFER
					FrameBufferRun (curX, y, curLength,
						cur->color);
#else
					OutputSegment (w, y, curX,
						curX + curLength - 1,
						cur->color);
#endif
#ifdef DEBUG
					printf ("row: %d, %d to %d color %d\n",
						y, curX,
						curX + curLength - 1,
						cur->color);
#endif
				}
				++ last;
				++ cur;
				curX = cur->x;
				curLength = cur->length;
				-- curCount;
				-- lastCount;
			}

			else if (curLength < last->length) {

/*
 *  The current segment is shorter than the last one.  Output a new line segment
 *  if the colors differ; delete the current segment and update the last one's
 *  x value and length.
 */
				if (cur->color != last->color) {
#ifdef HAS_FRAME_BUFFER
					FrameBufferRun (curX, y, curLength,
						cur->color);
#else
					OutputSegment (w, y, curX,
						curX + curLength - 1,
						cur->color);
#endif
#ifdef DEBUG
					printf ("row: %d, %d to %d color %d\n",
						y, curX,
						curX + curLength - 1,
						cur->color);
#endif
				}
				last->x += curLength;
				last->length -= curLength;
				++ cur;
				curX = cur->x;
				curLength = cur->length;
				-- curCount;
			}

/*
 *  The current segment is longer than the last one.  Output, if necessary;
 *  delete the last segment and update the current segment's x value and length.
 */

			else {
				if (cur->color != last->color) {
#ifdef HAS_FRAME_BUFFER
					FrameBufferRun (curX, y, last->length,
						cur->color);
#else
					OutputSegment (w, y, curX,
						curX + last->length - 1,
						cur->color);
#endif
#ifdef DEBUG
					printf ("row: %d, %d to %d color %d\n",
						y, curX,
						curX + lastLength - 1,
						cur->color);
#endif
				}
				curX += last->length;
				curLength -= last->length;
				++ last;
				-- lastCount;
			}

		}

/*
 *  The current segment starts before any previous segments.
 */

		else if (curX < last->x) {
			if (curX + curLength > last->x)
				len = last->x - curX;
			else
				len = curLength;

#ifdef HAS_FRAME_BUFFER
			FrameBufferRun (curX, y, len,
				cur->color);
#else
			OutputSegment (w, y, curX, curX + len - 1,
				cur->color);
#endif
#ifdef DEBUG
			printf ("row: %d, %d to %d color %d\n",
				y, curX, curX + len - 1, cur->color);
#endif
			curX += len;
			curLength -= len;
			if (curLength == 0) {
				++ cur;
				curX = cur->x;
				curLength = cur->length;
				-- curCount;
			}
		}

/*
 *  The previous segment begins before the first current segment.
 */

		else {
			if (last->x + last->length > curX)
				len = curX - last->x;
			else
				len = last->length;

			last->x += len;
			last->length -= len;
			if (last->length == 0) {
				++ last;
				-- lastCount;
			}
		}
	}
	} /* for (y=... */
}
