#include "Vlib.h"

void VDrawString(v, win, gc, p, str, len)
Viewport *v;
Window	 win;
GC	 gc;
VPoint	 *p;
char	 *str;
int	 len; {

	register int	x, y;

	if (v->flags & VPPerspective) {
		x = (v->Middl.x + (int) (v->Scale.x * p->x / p->z)) >> 2;
		y = (v->Middl.y - (int) (v->Scale.y * p->y / p->z)) >> 2;
	}
	else {
		x = (v->Middl.x + (int) (v->Scale.x * p->x)) >> 2;
		y = (v->Middl.y - (int) (v->Scale.y * p->y)) >> 2;
	}

	XDrawString (v->dpy, win, gc, x, y, str, len);

}
