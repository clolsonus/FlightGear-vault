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
#include "hsi.h"
#include <stdio.h>

#define CDI_DOT_SPACE	0.25
#define GS_DOT_SPACE	0.35
#define FREQ_X		0.8
#define FREQ_Y		1.1
#define DME_X		-1.0
#define DME_Y		-1.1

#define SCALE_1		7.0

void doHSICompassCard();
extern double radial();
extern double glideSlopeOffset();
extern void ShortTransform();
extern void VMatrixToShortMatrix();
extern double magHeading();

void
doHSI (c, u)
craft  *c;
viewer *u;
{

	if (c->radarMode != RM_ILS)
		return;

	doHSICompassCard (c, u);
	return;

}


void
doHSICompassCard (c, u)
craft *c;
viewer *u;
{

	XSegment	seg[1024];
	XRectangle	rect[1];
	int		npath, i, j, k, kmax, xc, yc, h, x, y;
	int		vor_orientation;
	VMatrix		m;
	short_matrix_t	im;
	short_point_t	it1, it2;
	VGlyphVertex3	jt1, jt2;
	int		xscale, yscale, xoffset, icdi_offset, igs_offset;
	double		cdi_offset, gs_offset, obs, r, diff, dx, dy, cb, sb;
	VPoint		hd, rd;

	rect[0].x = u->rx;
	rect[0].y = u->ry;
	rect[0].width = u->radarWidth;
	rect[0].height = u->radarWidth;

	xoffset = (- 15.0 * u->scaleFactor) + 0.5;
	xc = u->rx + (u->radarWidth + 1) / 2 + xoffset;
	yc = u->ry + (u->radarWidth + 1) / 2;

/*
 *  Build a transformation matrix to be used to display the compass card.
 */

	VIdentMatrix (&m);
	VRotate (&m, XRotation, magHeading(c));
	VMatrixToShortMatrix (&m, &im);

	xscale = u->v->Scale.x / (SCALE_1 * 4);
	yscale = u->v->Scale.y / (SCALE_1 * 4);

	h = 8.0 * u->scaleFactor + 0.5;
	x = FREQ_X * u->v->Scale.x / (SCALE_1 * 4);
	y = FREQ_Y * u->v->Scale.y / (SCALE_1 * 4);
	VDrawStrokeString (u->v, x + xc, y + yc, c->hsiSelect->freq_readout,
		strlen(c->hsiSelect->freq_readout), h, &u->z);

	if (c->hsiSelect->station &&
		(c->hsiSelect->station->flags & NAVAID_DME)) {
		x = DME_X * u->v->Scale.x / (SCALE_1 * 4);
		y = DME_Y * u->v->Scale.y / (SCALE_1 * 4);
		VDrawStrokeString (u->v, x + xc, y + yc,
			c->hsiSelect->dme_readout,
			strlen(c->hsiSelect->dme_readout), h, &u->z);
	}

/*
 *  Our compass card structure is a list of point paths on a unit circle.
 *  Traverse that structure, transforming each set of points into screen
 *  coordinates, and add a segment entry for each one that'll be displayed.
 */

	npath = sizeof(heading_path) / sizeof(heading_path[0]);

	for (i=0, j=0; j < npath; ++j) {
	    k = heading_path[j].vertex_start;
	    ShortTransform (&heading_vertex[k], &im, &it1);
	    kmax = k + heading_path[j].vertex_count;
	    for (++ k; k < kmax; ++k) {
		    ShortTransform (&heading_vertex[k], &im, &it2);
		    seg[i].x1 = xc + (xscale * it1.y / it1.x);
		    seg[i].y1 = yc - (yscale * it1.z / it1.x);
		    seg[i].x2 = xc + (xscale * it2.y / it2.x);
		    seg[i++].y2 = yc - (yscale * it2.z / it2.x);
		    it1 = it2;
	    }
	}

/*
 *  Draw the legend
 */

	npath = sizeof(legend_path) / sizeof(legend_path[0]);

	for (j=0; j < npath; ++j) {
	    k = legend_path[j].vertex_start;
	    jt1 = legend_vertex[k];
	    kmax = k + legend_path[j].vertex_count;
	    for (++ k; k < kmax; ++k) {
		    jt2 = legend_vertex[k];
		    seg[i].x1 = xc + (xscale * jt1.y / jt1.x);
		    seg[i].y1 = yc - (yscale * jt1.z / jt1.x);
		    seg[i].x2 = xc + (xscale * jt2.y / jt2.x);
		    seg[i++].y2 = yc - (yscale * jt2.z / jt2.x);
		    jt1 = jt2;
	    }
	}

/*
 *  Build a transformation matrix to be used to display the selected course
 *  pointer.
 */

	VIdentMatrix (&m);
	obs = DEGtoRAD(c->hsiSelect->obs_setting);
	VRotate (&m, XRotation, magHeading(c) - obs);
	VMatrixToShortMatrix (&m, &im);

/*
 *  Draw the Selected Course Pointer
 */

	npath = sizeof(scp_path) / sizeof(scp_path[0]);

	for (j=0; j < npath; ++j) {
	    k = scp_path[j].vertex_start;
	    ShortTransform (&scp_vertex[k], &im, &it1);
	    kmax = k + scp_path[j].vertex_count;
	    for (++ k; k < kmax; ++k) {
		    ShortTransform (&scp_vertex[k], &im, &it2);
		    seg[i].x1 = xc + (xscale * it1.y / it1.x);
		    seg[i].y1 = yc - (yscale * it1.z / it1.x);
		    seg[i].x2 = xc + (xscale * it2.y / it2.x);
		    seg[i++].y2 = yc - (yscale * it2.z / it2.x);
		    it1 = it2;
	    }
	}

/*
 *  Draw the Course Deviation Indicator
 */

	if (c->hsiSelect->station != NULL) {

		r = radial (c->hsiSelect->station, c);
		if (r > pi) {
			r -= 2.0 * pi;
		}

		if ((c->hsiSelect->station->flags & NAVAID_LOC) == 0) {
			hd.x = cos (obs);
			hd.y = sin (obs);
			hd.z = 0.0;
		}
		if (obs > pi) {
			obs -= 2.0 * pi;
		}

		diff = obs - r;

		if (diff > pi / 4.0) {
			diff = pi - diff;
		}
		else if (diff < - pi / 4.0) {
			diff = - pi - diff;
		}

		if (c->hsiSelect->station->flags & NAVAID_LOC) {
			vor_orientation = 0;
			cdi_offset = RADtoDEG(r) /
				c->hsiSelect->station->beam_width;
		}
		else {
			sb = sin (c->hsiSelect->station->bearing);
			cb = cos (c->hsiSelect->station->bearing);
			dx = c->Sg.x - c->hsiSelect->station->Sg.x;
			dy = c->Sg.y - c->hsiSelect->station->Sg.y;
			rd.x =   cb * dx + sb * dy;
			rd.y = - sb * dx + cb * dy;
			rd.z = 0.0;
			vor_orientation = VDotProd (&hd, &rd) < 0.0 ? 1 : -1;
			cdi_offset = RADtoDEG(diff) / 1.7;
		}

		if (cdi_offset > 2.5)
			icdi_offset = UNITY * 2.5 * CDI_DOT_SPACE;
		else if (cdi_offset < -2.5)
			icdi_offset = UNITY * -2.5 * CDI_DOT_SPACE;
		else
			icdi_offset = UNITY * cdi_offset * CDI_DOT_SPACE;

		npath = sizeof(cdi_path) / sizeof(cdi_path[0]);

		for (j=0; j < npath; ++j) {
		    k = cdi_path[j].vertex_start;
		    jt1 = cdi_vertex[k];
		    jt1.y += icdi_offset;
		    ShortTransform (&jt1, &im, &it1);
		    kmax = k + cdi_path[j].vertex_count;
	 	    for (++ k; k < kmax; ++k) {
			    jt1 = cdi_vertex[k];
			    jt1.y += icdi_offset;
			    ShortTransform (&jt1, &im, &it2);
			    seg[i].x1 = xc + (xscale * it1.y / it1.x);
			    seg[i].y1 = yc - (yscale * it1.z / it1.x);
			    seg[i].x2 = xc + (xscale * it2.y / it2.x);
			    seg[i++].y2 = yc - (yscale * it2.z / it2.x);
		    	it1 = it2;
		    }
		}

		if (vor_orientation != 0) {

			npath = sizeof(vor_to_from_path) /
				sizeof(vor_to_from_path[0]);

			for (j=0; j < npath; ++j) {
	    		    k = vor_to_from_path[j].vertex_start;
			    jt1 = vor_to_from_vertex[k];
			    jt1.z *= vor_orientation;
	    		    ShortTransform (&jt1, &im, &it1);
	    		    kmax = k + vor_to_from_path[j].vertex_count;
	    		    for (++ k; k < kmax; ++k) {
				jt1 = vor_to_from_vertex[k];
				jt1.z *= vor_orientation;
				ShortTransform (&jt1, &im, &it2);
				seg[i].x1 = xc + (xscale * it1.y / it1.x);
				seg[i].y1 = yc - (yscale * it1.z / it1.x);
			    	seg[i].x2 = xc + (xscale * it2.y / it2.x);
				seg[i++].y2 = yc - (yscale * it2.z / it2.x);
			 	it1 = it2;
			    }
			}
		}

/*
 *  Draw the Glide Slope scale
 */

		if (c->hsiSelect->station->flags & NAVAID_GS) {

		    npath = sizeof(gs_scale_path) / sizeof(gs_scale_path[0]);

		    for (j=0; j < npath; ++j) {
			k = gs_scale_path[j].vertex_start;
			jt1 = gs_scale_vertex[k];
		 	kmax = k + gs_scale_path[j].vertex_count;
		 	for (++ k; k < kmax; ++k) {
			    jt2 = gs_scale_vertex[k];
			    seg[i].x1 = xc + (xscale * jt1.y / jt1.x);
			    seg[i].y1 = yc - (yscale * jt1.z / jt1.x);
			    seg[i].x2 = xc + (xscale * jt2.y / jt2.x);
			    seg[i++].y2 = yc - (yscale * jt2.z / jt2.x);
			    jt1 = jt2;
			}
		    }

/*
 *  Draw the Glide Slope Deviation Indicator
 */

		    gs_offset = RADtoDEG(
		    	glideSlopeOffset(c->hsiSelect->station,c)) / 0.35;

		    if (gs_offset > 2.5)
			igs_offset = UNITY * (2.5 * GS_DOT_SPACE);
		    else if (gs_offset < -2.5)
			igs_offset = UNITY * (-2.5 * GS_DOT_SPACE);
		    else
			igs_offset = UNITY * (gs_offset * GS_DOT_SPACE);

		    npath = sizeof(gs_pointer_path) /
		    	sizeof(gs_pointer_path[0]);

		    for (j=0; j < npath; ++j) {
			k = gs_pointer_path[j].vertex_start;
			jt1 = gs_pointer_vertex[k];
			jt1.z += igs_offset;
		 	kmax = k + gs_pointer_path[j].vertex_count;
			for (++ k; k < kmax; ++k) {
			    jt2 = gs_pointer_vertex[k];
			    jt2.z += igs_offset;
			    seg[i].x1 = xc + (xscale * jt1.y / jt1.x);
			    seg[i].y1 = yc - (yscale * jt1.z / jt1.x);
			    seg[i].x2 = xc + (xscale * jt2.y / jt2.x);
			    seg[i++].y2 = yc - (yscale * jt2.z / jt2.x);
			    jt1 = jt2;
			}
		    }

		}
	}

	u->v->w->clip.x1 = rect[0].x;
	u->v->w->clip.y1 = rect[0].y;
	u->v->w->clip.x2 = rect[0].x + rect[0].width - 1;
	u->v->w->clip.y2 = rect[0].y + rect[0].height - 1;

	VDrawSegments (u->v, seg, i, u->v->pixel[HUDPixel]);

	u->v->w->clip.x1 = 0;
	u->v->w->clip.y1 = 0;
	u->v->w->clip.x2 = u->v->w->width - 1;
	u->v->w->clip.y2 = u->v->w->height - 1;

	return;
}

