#include <Alib.h>
#include <stdio.h>

void DrawPoint (w, x, y, zinfo)
AWindow	*w;
int	x, y;
ZInfo	*zinfo; {

	register Edge	*e, *e1;

	if (x < w->clip.x1 || x >w->clip.x2 || y < w->clip.y1 || y > w->clip.y2)
		return;

	zinfo->next = NotAnElement;

	if (w->EPTop >= w->EPSize - 1) {
		fprintf (stderr, "Edge Pool Overflow\n");
		return;
	}

	e = &(w->edgePool[(w->EPTop)++]);
	e1 = &(w->edgePool[(w->EPTop)++]);

	e->y2 = e1->y2 = y + 1;
#ifdef FLOAT_SLOPE
	e->x1 = e1->x1 = x;
	e->Dx = 0.0;
#else
	e->x1 = e1->x1 = x << 16;
	e->Dx = 0;
#endif
	e->p = e1->p = zinfo;

	e1->nexte = w->edges[y].head;
	e->nexte = e1;
	w->edges[y].head = e;

	if (y < w->ymin)
		w->ymin = y;

	if (y > w->ymax)
		w->ymax = y;
}
