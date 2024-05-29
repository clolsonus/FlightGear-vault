#include <Alib.h>
#include <stdio.h>

void DrawLine (w, x1, y1, x2, y2, zinfo)
AWindow	*w;
int	x1, y1, x2, y2;
ZInfo	*zinfo; {

	register int	t;
	register Edge	*e, *e1;

	zinfo->next = NotAnElement;

	if (ClipLine (w, &x1, &y1, &x2, &y2))
		return;

	if (y1 > y2) {
		t = x1;
		x1 = x2;
		x2 = t;
		t = y1;
		y1 = y2;
		y2 = t;
	}

	if ((x1 == x2) || (y1 == y2)) {

		if (w->EPTop >= w->EPSize - 1) {
			fprintf (stderr, "Edge Pool Overflow\n");
			return;
		}

		e = &(w->edgePool[(w->EPTop)++]);
		e1 = &(w->edgePool[(w->EPTop)++]);

		e->y2 = e1->y2 = y2 + 1;

#ifdef FLOAT_SLOPE
		if (x1 == x2)
			e->x1 = e1->x1 = x1;
		else {
			e->x1 = x1;
			e1->x1 = x2;
		}
		e->Dx = e1->Dx = 0;
#else
		if (x1 == x2)
			e->x1 = e1->x1 = x1 << 16;
		else {
			e->x1 = x1 << 16;
			e1->x1 = x2 << 16;
		}
		e->Dx = e1->Dx = 0;
#endif
		e->p = e1->p = zinfo;

		e1->nexte = w->edges[y1].head;
		e->nexte = e1;
		w->edges[y1].head = e;
	}

/*
 *  A non-trivial case.  The has a slope that is non-zero and not infinity.
 *  We will need one edge entry to hang off the line list at the strting y
 *  value of the line.  We will also pre-allocate two edge entries to be used
 *  as the line'e edges during the EdgeTableToScanLine procedure.  By our own
 *  convention, we'll place them in the two edge pool entries immediately
 *  following the line information entry. 
 */

	else {

		if (w->EPTop >= w->EPSize - 2) {
			fprintf (stderr, "Edge Pool Overflow\n");
			return;
		}

		e = &(w->edgePool[(w->EPTop)++]);
		w->EPTop += 2;

		e->y2 = y2;

#ifdef FLOAT_SLOPE
		e->x1 = x1;
		e->Dx = (x2 - x1) / (y2 - y1);
#else
		e->x1 = x1 << 16;
		e->Dx = ((x2 - x1) << 16) / (y2 - y1);
#endif

		e->p = (e+1)->p = (e+2)->p = zinfo;

		e->nexte = w->lines[y1].head;
		w->lines[y1].head = e;
	}

	if (y1 < w->ymin)
		w->ymin = y1;

	if (y2 > w->ymax)
		w->ymax = y2;
}
