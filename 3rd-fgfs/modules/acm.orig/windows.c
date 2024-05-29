/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991,1992  Riley Rainey
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
#include "pm.h"

extern void redrawPanel(), resizePanel();

void resizePlayerWindow (c, u, width, height, initial_flag)
craft	*c;
viewer	*u;
int	width;
int	height;
int	initial_flag; {

	XWindowChanges	xwc;
	double		scale;

	scale = (double) width / (double) FS_WINDOW_WIDTH;

#ifdef notdef
	if (initial_flag == 0) {
		xwc.width = RADAR_WINDOW_WIDTH * scale + 0.5;
		xwc.height = RADAR_WINDOW_HEIGHT * scale + 0.5;
		xwc.x = RADAR_X * scale + 0.5;
		xwc.y = RADAR_Y * scale + 0.5;
		XConfigureWindow (u->dpy, u->rwin,
			CWX | CWY | CWWidth | CWHeight, &xwc);
	}
#endif

	u->width = VIEW_WINDOW_WIDTH * scale;
	u->height = VIEW_WINDOW_HEIGHT * scale;

	if (initial_flag == 0) {
		XSetPlaneMask (u->dpy, u->gc, AllPlanes);

		if (u->v->flags & VPMono) {
			XSetBackground (u->dpy, u->gc,
				VConstantColor (u->v, blackPixel));
			XSetForeground (u->dpy, u->gc,
				VConstantColor (u->v, whitePixel));
		}
		else {
			XSetForeground (u->dpy, u->gc,
				VConstantColor (u->v, blackPixel));
		}

        	XFillRectangle (u->dpy, u->win, u->gc, 0, u->height,
			u->width, height);
		XFlush (u->dpy);
	}

	if (initial_flag == 0) {
		XSetPlaneMask (u->dpy, u->gc, u->v->mask);
		redrawPanel (c, u);
	}

	u->xCenter = (u->width + 1) / 2;
	u->yCenter = (u->height + 1) / 2;

	u->rx = RADAR_X * scale + 0.5;
	u->ry = RADAR_Y * scale + 0.5;
	u->radarWidth = RADAR_WINDOW_WIDTH * scale + 0.5;
	u->radarHeight = RADAR_WINDOW_HEIGHT * scale + 0.5;
	u->rfth = 8.0 * scale + 0.5;
	u->rftw = VFontWidthPixels (u->v, u->rfth);

	u->TEWSx = TEWS_X * scale + 0.5;
	u->TEWSy = TEWS_Y * scale + 0.5;
	u->TEWSSize = TEWS_SIZE * scale + 0.5;
	u->scaleFactor = scale;

	ResizeAWindow (u->v->w, height, width);

	resizePanel (c, u);

/*
 *  Fill in the scale structures for the airspeed/altitude HUD scales.
 */

	u->altScale.xorg = ALT_ORG_X * scale;
	u->altScale.yorg = ALT_ORG_Y * scale;
	u->altScale.length = ALT_LENGTH * scale;
	u->altScale.orientation = ALT_ORIENT;
	u->altScale.scale = ALT_SCALE / scale;
	u->altScale.minorInterval = ALT_MIN_INTERVAL;
	u->altScale.minorSize = ALT_MIN_SIZE * scale;
	u->altScale.majorInterval = ALT_MAJ_INTERVAL;
	u->altScale.majorSize = ALT_MAJ_SIZE * scale;
	u->altScale.indexSize = ALT_INDEX_SIZE * scale;
	u->altScale.divisor = ALT_DIVISOR;
	u->altScale.format = ALT_FORMAT;
	u->altScale.pixel = u->v->pixel[HUDPixel];
	u->altScale.fontSize = (int) (14.0 * scale + 0.5);

	u->velScale.xorg = VEL_ORG_X * scale;
	u->velScale.yorg = VEL_ORG_Y * scale;
	u->velScale.length = VEL_LENGTH * scale;
	u->velScale.orientation = VEL_ORIENT;
	u->velScale.scale = VEL_SCALE / scale;
	u->velScale.minorInterval = VEL_MIN_INTERVAL;
	u->velScale.minorSize = VEL_MIN_SIZE * scale;
	u->velScale.majorInterval = VEL_MAJ_INTERVAL;
	u->velScale.majorSize = VEL_MAJ_SIZE * scale;
	u->velScale.indexSize = VEL_INDEX_SIZE * scale;
	u->velScale.divisor = VEL_DIVISOR;
	u->velScale.format = VEL_FORMAT;
	u->velScale.pixel = u->v->pixel[HUDPixel];
	u->velScale.fontSize = (int) (14.0 * scale + 0.5);

	u->hdgScale.xorg = HDG_ORG_X * scale;
	u->hdgScale.yorg = HDG_ORG_Y * scale;
	u->hdgScale.length = HDG_LENGTH * scale;
	u->hdgScale.orientation = HDG_ORIENT;
	u->hdgScale.scale = HDG_SCALE / scale;
	u->hdgScale.minorInterval = HDG_MIN_INTERVAL;
	u->hdgScale.minorSize = HDG_MIN_SIZE * scale;
	u->hdgScale.majorInterval = HDG_MAJ_INTERVAL;
	u->hdgScale.majorSize = HDG_MAJ_SIZE * scale;
	u->hdgScale.indexSize = HDG_INDEX_SIZE * scale;
	u->hdgScale.divisor = HDG_DIVISOR;
	u->hdgScale.format = HDG_FORMAT;
	u->hdgScale.pixel = u->v->pixel[HUDPixel];
	u->hdgScale.fontSize = (int) (14.0 * scale + 0.5);

}
