#include "Vlib.h"

#define TICKtoRAD(a)	(a * M_PI / (180.0 * 64.0))
#define INCR		(30 * 64)

void
VDrawArc (v, x, y, width, height, angle1, angle2, color)
Viewport	*v;
int		x, y;
int		width, height;
int		angle1, angle2;
Color		color; {

	register ZInfo	*z;
	register double	w, h, xc, yc;
	register int	incr, x1, x2, y1, y2;

	if (v->ztop == v->zsize) {
		fprintf (stderr, "Z-information pool overflow\n");
		return;
	}

	z = &(v->zpool[(v->ztop)++]);
	z->depth = --v->depth;
	z->color = color;

	w = width / 2.0;
	h = height / 2.0;
	xc = x + width / 2.0;
	yc = y + height / 2.0;

	if (angle2 < 0) {
		incr = - INCR;
		angle2 = - angle2;
	}
	else
		incr = INCR;
	
	if (angle2 > 360*64)
		angle2 = 360*64;

	x1 = xc + (w * cos(TICKtoRAD(angle1)));
	y1 = yc - (h * sin(TICKtoRAD(angle1)));

	while (angle2 != 0) {
		angle1 += incr;
		angle2 -= INCR;
		if (angle2 < 0) {
			angle1 -= angle2;
			angle2 = 0;
		}		
		x2 = xc + (w * cos(TICKtoRAD(angle1)));
		y2 = yc - (h * sin(TICKtoRAD(angle1)));
		DrawLine (v->w, x1, y1, x2, y2, z);
		x1 = x2;
		y1 = y2;
	}
}
