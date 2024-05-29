/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */

#include <math.h>
#include "pm.h"
#include "horizon.h"
#include <stdio.h>

extern void doScale(), doCompassScale();
extern void ShortTransform();
void doLadder();
extern void VMatrixToShortMatrix();
extern double magHeading();

#define FLIGHTDEBUG

void doHUD (c, u)
craft  *c;
viewer *u; {

	char	buffer[80];
	int	i, x, x1, y, scale, fth, scale1;
	double	vel;

	scale1 = u->width * 2048 / VIEW_WINDOW_WIDTH;

	scale = (int)(12.0 * u->scaleFactor + 0.5);
	fth = (int)(18.0 * u->scaleFactor + 0.5);

	if (c->viewDirection.x < 0.90)
		return;

#ifdef notdef
	vel = mag(c->Cg);
	sprintf (buffer, "a=%.1f  b=%.1f", RADtoDEG(c->alpha),
		RADtoDEG(c->beta));
#else
	if ((vel = mag(c->Cg)) < 50.0)
		sprintf (buffer, "a=0.0");
	else
		sprintf (buffer, "a=%.1f", RADtoDEG(c->alpha));
#endif

	x = 750 * scale1 / 2048;
	y = 120 * scale1 / 2048;
	VDrawStrokeString (u->v, x, y, buffer, strlen(buffer),
		scale, &u->z);

	x1 = 375 * scale1 / 2048;
	sprintf (buffer, "%4.1f", c->G.z);
	VDrawStrokeString (u->v, x1, y, buffer, strlen(buffer),
		scale, &u->z);

	x = 725 * scale1 / 2048;
	y = 390 * scale1 / 2048;
	sprintf (buffer, "%6d0", (int)(-c->Cg.z * 6.0));
	VDrawStrokeString (u->v, x, y, buffer, strlen(buffer),
		scale, &u->z);

	x = 375 * scale1 / 2048;
	doCompassScale (u->v, &(u->hdgScale), RADtoDEG(magHeading(c))*100.0);
	doScale (u->v, &(u->altScale), - (c->Sg.z + -6.0));
	doScale (u->v, &(u->velScale), FPStoKTS(mag(c->Cg)));

#ifdef notdef
	sprintf (buffer, "%3d%% %s", (int)(c->rpm * 100.0 + 0.5),
		(c->flags & FL_AFTERBURNER) ? "AB" : "");
	VDrawStrokeString (u->v, x, y, buffer, strlen(buffer),
		scale, &u->z);
#endif

	if (c->mach >= 0.7) {
		x = 415 * scale1 / 2048;
		y = 248 * scale1 / 2048;
		sprintf (buffer, "M%.2f", c->mach);
		VDrawStrokeString (u->v, x, y, buffer, strlen(buffer),
			scale * 7 / 10, &u->z);
	}

	x = 370 * scale1 / 2048;
	y = 450 * scale1 / 2048;

	doLadder (c, u, vel);

	for (i=0; i<3; ++i) {
		VDrawStrokeString (u->v, x, y, c->leftHUD[i],
			strlen(c->leftHUD[i]), scale, &u->z);
		y += fth;
	}

	return;

}


void	doLadder (c, u, vel)
craft *c;
viewer *u;
double vel;
{

	int		a1, a2, a3, a4, a5, plotFPM, npath;
	XSegment	seg[400];
	XRectangle	rect[1];
	int		i, j, k, kmax, windX, windY, w, tx, ty, scale;
	VPoint		tmp, tmp1, t1;
	VMatrix		m;
	double		v;
	short_matrix_t	im;
	short_point_t	it1, it2;
	short_fixed_t	limit;
	register short	xscale, yscale;
	extern void	buildEulerMatrix();

	limit = 0.965 * UNITY;

	scale = (int) (12.0 * u->scaleFactor + 0.5);

	rect[0].x = u->xCenter - (int) (137.5 * u->scaleFactor + 0.5);
	rect[0].y = u->yCenter - (int) (116.0 * u->scaleFactor + 0.5);
	rect[0].width = 275.0 * u->scaleFactor + 0.5;
	rect[0].height = 252.0 * u->scaleFactor + 0.5;

	npath = sizeof(horizon_path) / sizeof(horizon_path[0]);

/*
 *  Build a transformation matrix to be used to display the flight
 *  path ladder (artificial horizon).
 *
 *  One thing we do is to keep the ladder centered on the flight path
 *  marker.  The correction angle "v" is calculated to make that happen.
 */

	if (fabs(c->Cg.x) < 5.0 && fabs(c->Cg.y) < 5.0) {
		v = 0.0;
	}
	else {
		t1.x = cos(c->curHeading) * c->Cg.x +
			sin(c->curHeading) * c->Cg.y;
		t1.y = - sin(c->curHeading) * c->Cg.x +
			cos(c->curHeading) * c->Cg.y;
		v = atan2 (t1.y, t1.x);
	}

	VIdentMatrix (&m);
	VRotate (&m, ZRotation, v);
	VRotate (&m, YRotation, - c->curPitch);
	VRotate (&m, XRotation, c->curRoll);
	VMatrixToShortMatrix (&m, &im);

	xscale = u->v->Scale.x / 4;
	yscale = u->v->Scale.y / 4;

/*
 *  Our horizon structure is a list of point paths on a unit circle.  Traverse
 *  that structure, transforming each set of points into screen coordinates,
 *  and add a segment entry for each one that'll be displayed.
 */

	for (i=0, j=0; j < npath; ++j) {
	    k = horizon_path[j].vertex_start;
	    ShortTransform (&horizon_vertex[k], &im, &it1);
	    if (it1.x > limit) {
	        kmax = k + horizon_path[j].vertex_count;
		for (++ k; k < kmax; ++k) {
		    ShortTransform (&horizon_vertex[k], &im, &it2);
		    if (it2.x > limit) {
		        seg[i].x1 = u->xCenter + (xscale * it1.y / it1.x);
		        seg[i].y1 = u->yCenter - (yscale * it1.z / it1.x);
		        seg[i].x2 = u->xCenter + (xscale * it2.y / it2.x);
		        seg[i++].y2 = u->yCenter - (yscale * it2.z / it2.x);
		    }
		    it1 = it2;
		}
	    }
	}

	u->v->w->clip.x1 = rect[0].x;
	u->v->w->clip.y1 = rect[0].y;
	u->v->w->clip.x2 = rect[0].x + rect[0].width - 1;
	u->v->w->clip.y2 = rect[0].y + rect[0].height - 1;

/*
 *  Determine the location of the flight path marker
 */

	VTransform (&c->Cg, &c->Itrihedral, &tmp);

	plotFPM = 1;

	if (vel < 50.0 || tmp.x == 0.0) {
		windX = u->xCenter;
		windY = u->yCenter;
	}
	else if (tmp.x > 0.0) {
		windX = u->xCenter + ((int)(tmp.y * u->v->Scale.x / tmp.x) >> 2);
		windY = u->yCenter + ((int)(tmp.z * u->v->Scale.y / tmp.x) >> 2);
	}
	else
		plotFPM = 0;

	if (plotFPM) {

		a1 = 18.0 * u->scaleFactor + 0.5;
		a2 = 9.0 * u->scaleFactor + 0.5;
		a3 = 15.0 * u->scaleFactor + 0.5;

		seg[i].x1 = windX - a1;
		seg[i].y1 = windY;
		seg[i].x2 = windX - a2;
		seg[i++].y2 = windY;

		seg[i].x1 = windX + a1;
		seg[i].y1 = windY;
		seg[i].x2 = windX + a2;
		seg[i++].y2 = windY;

		seg[i].x1 = windX;
		seg[i].y1 = windY - a2;
		seg[i].x2 = windX;
		seg[i++].y2 = windY - a1;
	}

/*
 *  Gather weapon display info (and maybe draw a reticle).
 */

	doWeaponDisplay (c, u);

/*
 *  Draw a target designator around the current primary radar target.
 */

	if (c->curRadarTarget >= 0) {

		w = DESIGNATOR_SIZE * u->scaleFactor * 0.5;

		tmp1 = ptbl[c->curRadarTarget].Sg;
		tmp1.x -= c->Sg.x;
		tmp1.y -= c->Sg.y;
		tmp1.z -= c->Sg.z;
		VTransform (&tmp1, &c->Itrihedral, &tmp);
#ifdef notdef
		VTransform (&(ptbl[c->curRadarTarget].Sg),
			&u->v->eyeSpace, &tmp);
#endif

/* radar target is assumed to be ahead of us (tmp.z > 0.0) */
		
		tx = (u->v->Middl.x + ((int)(tmp.y * u->v->Scale.x / tmp.x)) >> 2);
		ty = (u->v->Middl.y + ((int)(tmp.z * u->v->Scale.y / tmp.x)) >> 2);

		seg[i].x1 = tx - w;
		seg[i].y1 = ty - w;
		seg[i].x2 = tx + w;
		seg[i++].y2 = ty - w;

		seg[i].x1 = tx + w;
		seg[i].y1 = ty - w;
		seg[i].x2 = tx + w;
		seg[i++].y2 = ty + w;

		seg[i].x1 = tx + w;
		seg[i].y1 = ty + w;
		seg[i].x2 = tx - w;
		seg[i++].y2 = ty + w;

		seg[i].x1 = tx - w;
		seg[i].y1 = ty + w;
		seg[i].x2 = tx - w;
		seg[i++].y2 = ty - w;
	}

	VDrawSegments (u->v, seg, i, u->v->pixel[HUDPixel]);

	VDrawArc (u->v, u->xCenter-2, u->yCenter-2, 4, 4, 0, 360*64,
		u->v->pixel[HUDPixel]);

	if (plotFPM) {
		a4 = 16.0 * u->scaleFactor + 0.5;
		a5 = (a4 + 1) / 2;
		VDrawArc (u->v, windX-a5, windY-a5, a4, a4, 0, 360*64,
			u->v->pixel[HUDPixel]);
	}

	u->v->w->clip.x1 = 0;
	u->v->w->clip.y1 = 0;
	u->v->w->clip.x2 = u->v->w->width - 1;
	u->v->w->clip.y2 = u->v->w->height - 1;

	return;
}

