#include <Alib.h>
#include <stdio.h>

/*
 *  makeET : build an edge table based on edge coherence as described by
 *	     Salmon and Slater, 1987.
 */

int MakeET (w, pts, npts, zinfo)
AWindow *w;
Point	*pts;
int	npts;
ZInfo	*zinfo; {

	register int	i, t, addedEdge = 0;
	int 	x1, y1, x2, y2;
	register long	ymin, ymax;
	register Edge	*e;

	ymin = 0x7FFFFFFF;
	ymax = 0x80000000;

	zinfo->next = NotAnElement;

/*
 *  Process each edge in this polygon
 */

	for (i=1; i<=npts; ++i) {

/*
 *  Determine the end-points of this edge (x1, y1), (x2, y2).
 */

		if (i == npts) {
			x2 = pts[0].x;
			y2 = pts[0].y;
		}
		else {
			x2 = pts[i].x;
			y2 = pts[i].y;
		}
		x1 = pts[i-1].x;
		y1 = pts[i-1].y;

		if (y1 > y2) {

			t = x1;
			x1 = x2;
			x2 = t;
			t = y1;
			y1 = y2;
			y2 = t;

		}

/*
 *  This hack allows 3d polygon clipping to function ...
 */

		if (y1 == -1) {
		    y1 = 0;
		}

		if (y1 < 0 || y2 >= w->height) {
			fprintf (stderr, "polygon edge y out %d %d\n", y1, y2);
			continue;
		}

#ifdef DEBUG
		fprintf (stderr, "edge (%d, %d) to (%d, %d)\n", x1, y1, x2, y2);
#endif

		if (y1 != y2) {

			if (w->EPTop == w->EPSize) {
				fprintf (stderr, "Edge Pool Overflow\n");
				return -1;
			}

			e = &(w->edgePool[(w->EPTop)++]);

			e->y2 = y2;
#ifdef FLOAT_SLOPE
			e->x1 = x1;
			e->Dx = (x2 - x1) / (y2 - y1);
#else
			e->x1 = x1 << 16;
			e->Dx = ((x2 - x1) << 16) / (y2 - y1);
#endif

			e->p = zinfo;
			e->nexte = w->edges[y1].head;
			w->edges[y1].head = e;

			addedEdge = 1;

			if (y1 < ymin)
				ymin = y1;

			if (y2 > ymax)
				ymax = y2;
		}

	}

	if (addedEdge == 1) {

		if (w->ymin > ymin)
			w->ymin = ymin;

		if (w->ymax < ymax)
			w->ymax = ymax;

#ifdef DEBUG
	fprintf (stderr, "ymin = %d, ymax = %d\n", ymin, ymax);
	for (i=ymin; i <=ymax; ++i) {
		if (w->edges[i].head == NULL)
			fprintf (stderr, "[%d] (null)\n", i);
		else {
			fprintf (stderr, "[%d]\n", i);
			for (e=w->edges[i].head; e != NULL; e=e->nexte)
			fprintf (stderr, "    x = %d, y2 = %d, Dx = 0x%x\n",
				e->x1 >> 16, e->y2, e->Dx);
		}
	}
#endif

	}

	return 0;

}
