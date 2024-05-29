#include <Alib.h>
#include <stdio.h>

static ColorSegment *CSOverflow () {

	fprintf (stderr, "Color Segment pool overflow\n");
	return (ColorSegment *) NULL;

}

#define mallocCS(w)	( (w->curPool) ? \
	((w->CSTop1 == w->CSSize1) ? \
	CSOverflow() : &(w->csPool1[(w->CSTop1)++])) : \
	((w->CSTop0 == w->CSSize0) ? \
	CSOverflow() : &(w->csPool0[(w->CSTop0)++])) )

#define freeLastCS(w)    ( (w->curPool) ? -- (w->CSTop1) : -- (w->CSTop0) )


/*
 * DrawScanLine : draw a line segment.  Successive calls on the same scan line
 *		must be ordered by x0 value.
 */

void DrawScanLine (w, y, x0, x1, color)
AWindow		*w;
int		y, x0, x1;
Color		color; {

	register ColorSegment	*p;
	register int		lastx;

#ifdef DEBUG
	printf ("DrawScanLine (w, %d, %d, %d, %d)\n", y, x0, x1, color);
#endif

	if (w->scanLine[y].count == 0) {

/*
 *  This is the first segment on this scan line.
 */

		if ((p = mallocCS(w)) != (ColorSegment *) NULL) {
			p->x = x0;
			p->length = x1 - x0 + 1;
			p->color = color;
			w->scanLine[y].head = w->scanLine[y].tail = p;
			w->scanLine[y].count = 1;
		}

	}

/*
 *  The scan line wasn't empty.  Check the previous scan line entry for
 *  any overlap with this one.
 */

	else {
		p = w->scanLine[y].tail;
		lastx = p->x + p->length - 1;

		if (lastx >= x0) {

/*
 *  The segments overlap.
 *
 *  If the overlapping segments have the same color, then simply
 *  extend the previous segment's length.
 */

			if (p->color == color)
				p->length = x1 - p->x + 1;

/*
 *  The overlapping segment's colors are different.  Shorten the previous
 *  segment and allocate an entry for the current one.  If the shortened
 *  segment is eliminated, use it to store this segment's information.
 */

			else {
			    if ((p->length = x0 - p->x) > 0) {
				if ((p = mallocCS(w)) != (ColorSegment *) NULL) {
					++ w->scanLine[y].count;
					w->scanLine[y].tail = p;
				}
			    }

/*
 *  If the shortened segment's length went to zero, we may need to merge
 *  this segment with the last one.
 */

			    if (((p-1)->color == color) &&
				(w->scanLine[y].count > 1)) {
				freeLastCS (w);
				p = w->scanLine[y].tail = p-1;
				-- w->scanLine[y].count;
				p->length = x1 - p->x + 1;
			    }
			    else {
			    	p->x = x0;
			    	p->length = x1 - x0 + 1;
			    	p->color = color;
			    }
			}
		}

/*
 *  The segments do not overlap.
 *
 *  If the segments are adjacent and the colors are the same, extend the
 *  last segment.
 *
 *  Otehrwise, create a new segment and append it to this line.
 */

		else if ((lastx == x0 - 1) && (p->color == color))

			p->length = x1 - p->x + 1;

		else if ((p = mallocCS(w)) != (ColorSegment *) NULL) {
			++ w->scanLine[y].count;
			w->scanLine[y].tail = p;
			p->x = x0;
			p->length = x1 - x0 + 1;
			p->color = color;
		}

	}		

}
