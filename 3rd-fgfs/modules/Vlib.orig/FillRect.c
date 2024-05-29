#include <Alib.h>

extern int MakeET();

void FillRectangle (w, x, y, width, height, zinfo)
AWindow	*w;
int	x, y;
int	width, height;
ZInfo	*zinfo; {

	Point	pts[4];
	register int yextent, xextent;

/*
 *  (Minimally) clip the box.
 */

	xextent = x + width - 1;
	yextent = y + height - 1;

	if (x > w->clip.x2 || y > w->clip.y2 ||
		xextent < w->clip.x1 || yextent < w->clip.y1) {
		return;
	}
	if (x < w->clip.x1) {
		x = w->clip.x1;
	}
	if (y < w->clip.y1) {
		y = w->clip.y1;
	}
	if (xextent > w->clip.x2) {
		xextent = w->clip.x2;
	}
	if (yextent > w->clip.y2) {
		yextent = w->clip.y2;
	}

	pts[0].x = x;
	pts[0].y = y;
	pts[1].x = xextent;
	pts[1].y = y;
	pts[2].x = xextent;
	pts[2].y = yextent;
	pts[3].x = x;
	pts[3].y = yextent;

	MakeET (w, pts, 4, zinfo);
}
