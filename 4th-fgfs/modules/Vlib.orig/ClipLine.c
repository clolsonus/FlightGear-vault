#include <Alib.h>

int ClipLine (w, x1, y1, x2, y2)
AWindow	*w;
int	*x1, *y1, *x2, *y2; {

	register int dx, dy;

/*
 *  Top clip
 */

	dx = *x2 - *x1;
	dy = *y2 - *y1;

	if (*y1 < w->clip.y1 || *y2 < w->clip.y1) {
		if (*y1 < w->clip.y1)
			if (*y2 < w->clip.y1)
				return 1;
			else {
				*x1 = *x1 + dx * (w->clip.y1 - *y1) / dy;
				*y1 = w->clip.y1;
			}
		else {
			*x2 = *x1 + dx * (w->clip.y1 - *y1) / dy;
			*y2 = w->clip.y1;
		}
	}

/*
 *  Bottom clip
 */

	if (*y1 > w->clip.y2 || *y2 > w->clip.y2) {
		if (*y1 > w->clip.y2)
			if (*y2 > w->clip.y2)
				return 1;
			else {
				*x1 = *x1 + dx * (w->clip.y2 - *y1) / dy;
				*y1 = w->clip.y2;
			}
		else {
			*x2 = *x1 + dx * (w->clip.y2 - *y1) / dy;
			*y2 = w->clip.y2;
		}
	}

/*
 *  Left clip
 */

	if (*x1 < w->clip.x1 || *x2 < w->clip.x1) {
		if (*x1 < w->clip.x1)
			if (*x2 < w->clip.x1)
				return 1;
			else {
				*y1 = *y1 + dy * (w->clip.x1 - *x1) / dx;
				*x1 = w->clip.x1;
			}
		else {
			*y2 = *y1 + dy * (w->clip.x1 - *x1) / dx;
			*x2 = w->clip.x1;
		}

	}

/*
 *  Right clip
 */

	if (*x1 > w->clip.x2 || *x2 > w->clip.x2) {
		if (*x1 > w->clip.x2)
			if (*x2 > w->clip.x2)
				return 1;
			else {
				*y1 = *y1 + dy * (w->clip.x2 - *x1) / dx;
				*x1 = w->clip.x2;
			}
		else {
			*y2 = *y1 + dy * (w->clip.x2 - *x1) / dx;
			*x2 = w->clip.x2;
		}

	}

#ifdef DEBUG
	if (*y1 < w->clip.y1 || *y1 > w->clip.y2
		|| *y2 < w->clip.y1 || *y2 > w->clip.y2
		|| *x1 < w->clip.x1 || *x1 > w->clip.x2
		|| *x2 < w->clip.x1 || *x2 > w->clip.x2)
		printf ("clip error\n");
#endif

	return 0;
}
