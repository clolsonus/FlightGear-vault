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
#include "bitmap.h"
#include <stdio.h>

extern char *ItoA();
extern void doDroneRadar(), doTEWS();
extern void placeCraft();
extern void doScale(), doCompassScale();
extern void doHUD(), doHSI();
extern void doPanel();

#define lim	((double) 200 * NM)

VPoint hex[6] =
{
    {1.00000, 0.00000, 0.0},
    {0.50000, 0.86603, 0.0},
    {-0.50000, 0.86603, 0.0},
    {-1.00000, 0.00000, 0.0},
    {-0.50000, -0.86603, 0.0},
    {0.50000, -0.86603, 0.0}
};

VPolygon **poly;
long polyCount;

static craft *sortList;

void insertCraft();

void
addGround(c, poly, count, color, z)
craft *c;
VPolygon **poly;
long *count;
VColor *color;
double z;
{
    VPoint hex1[6];
    double d;
    int i, j, k;
    long cnt = *count;

/*
 *  We should really be drawing all these hexagons if any of the viewers
 *  is in depth-cueued mode, not just the real player.
 */

    if (c->vl->v->flags & VPDepthCueing) {
	k = _VDefaultWorkContext->depthCueSteps - 1;
	for (i = 0; i < _VDefaultWorkContext->depthCueSteps; ++i, --k) {
	    d = (i == 0) ? lim : (_VDefaultWorkContext->visTable[k]);
	    for (j = 0; j < 6; ++j) {
		hex1[j].x = hex[j].x * d + c->Sg.x;
		hex1[j].y = hex[j].y * d + c->Sg.y;
		hex1[j].z = z;
	    }
	    poly[cnt] = VCreatePolygon(6, hex1, color);
	    poly[cnt++]->assignedDepth = k;
	}
    }
/*
 *  Otherwise draw just a single "ground" hexagon.
 */

    else {
	d = lim;
	for (j = 0; j < 6; ++j) {
	    hex1[j].x = hex[j].x * d + c->Sg.x;
	    hex1[j].y = hex[j].y * d + c->Sg.y;
	    hex1[j].z = z;
	}
	poly[cnt++] = VCreatePolygon(6, hex1, color);
    }
    *count = cnt;
}

void
doViews()
{

    long polyCount;
    register unsigned long curPixel;
    register craft *c, *p;
    register int i, j, k;
    register viewer *q;
    VPoint vp, fwd, up;
    static ZInfo z, panelz;
    double d;
    short bgIndex;

/*
 *  Build a vector of polygons for all objects in the scene.
 *
 *  This vector should be ordered from "most distant" to "closest" so that
 *  the final display will end up correct.  Rather than generalizing this
 *  to death, we'll use a few heuristics to get very close to what we need:
 *
 *  (0) Build a single polygon to represent the ground.
 *  (1) Objects on the surface (stbl) are collected first.
 *  (2) Planes and projectiles (ptbl and mtbl) are first sorted in descending
 *      order by their distance from the observer and then polygons are
 *      collected.
 */


    for ((i = 0, c = ptbl); i < MAXPLAYERS; (++i, ++c)) {

	if (c->type == CT_DRONE || c->type == CT_DIS_PLANE)
	    doDroneRadar(c);

	if (c->type != CT_PLANE || (c->flags & FL_BLACK_BOX))
	    continue;

/*
 *  Set up the eyespace transformation for this viewpoint
 */

	if (c->flags & FL_CHASE_VIEW) {

	    vp.x = c->Sg.x - c->prevSg.x;
	    vp.y = c->Sg.y - c->prevSg.y;
	    vp.z = c->Sg.z - c->prevSg.z;
	    d = mag(vp);

/*
 *  If our velocity is greater than 5 fps, point the view along the velocity
 *  vector (approximately); otherwise the view will be along the direction
 *  of aircraft orientation.
 */

	    if (d / deltaT > 2.0 || c->Sg.z < -100.0) {
		vp.x *= -50.0 / d;
		vp.y *= -50.0 / d;
		vp.z *= -50.0 / d;
		vp.x += c->Sg.x;
		vp.y += c->Sg.y;
		vp.z += c->Sg.z;
		fwd = c->Sg;
	    } else {
		fwd.x = -50.0;
		fwd.y = fwd.z = 0.0;
		VTransform(&fwd, &c->trihedral, &vp);
		vp.x += c->Sg.x;
		vp.y += c->Sg.y;
		vp.z += c->Sg.z;
		VTransform(&c->cinfo->viewPoint, &c->trihedral, &fwd);
		fwd.x += c->Sg.x;
		fwd.y += c->Sg.y;
		fwd.z += c->Sg.z;
	    }

	    up.x = 0.0 + vp.x;
	    up.y = 0.0 + vp.y;
	    up.z = -1.0 + vp.z;
	} else {

	    VTransform(&c->cinfo->viewPoint, &(c->trihedral), &vp);
	    vp.x += c->Sg.x;
	    vp.y += c->Sg.y;
	    vp.z += c->Sg.z;

	    VTransform(&c->viewDirection, &(c->trihedral), &fwd);
	    fwd.x += vp.x;
	    fwd.y += vp.y;
	    fwd.z += vp.z;

	    VTransform(&c->viewUp, &(c->trihedral), &up);
	    up.x += vp.x;
	    up.y += vp.y;
	    up.z += vp.z;
	}

	VGetEyeSpace(c->vl->v, vp, fwd, up);

	polyCount = 0;
	sortList = (craft *) NULL;

/*
 *  With our simple cloud model, we are either above, in, or below the clouds
 *
 *  What we display in the outside-cockpit view will depend of where we are.
 */

	bgIndex = 0;
	if (c->Sg.z < ctop) {

	    addGround(c, poly, &polyCount, cloudColor, ctop);
	    for (j = 0; j < polyCount; ++j) {
		VTransformPolygon(poly[j], &c->vl->v->eyeSpace);
	    }

	    for ((j = 0, p = ptbl); j < MAXPLAYERS; (++j, ++p)) {
		if (p->type != CT_FREE &&
		    (p != c || c->flags & FL_CHASE_VIEW) &&
		    p->Sg.z < ctop) {
		    insertCraft(c, p);
		}
	    }

	    for ((j = 0, p = mtbl); j < MAXPROJECTILES; (++j, ++p)) {
		if (p->type != CT_FREE && p->Sg.z < ctop) {
		    insertCraft(c, p);
		}
	    }
	} else if (c->Sg.z < cbase) {
	    bgIndex = cloudColor->cIndex;
	} else {

	    addGround(c, poly, &polyCount, groundColor, 0.0);
	    addGround(c, poly, &polyCount, cloudColor, cbase);
	    for (j = 0; j < polyCount; ++j) {
		VTransformPolygon(poly[j], &c->vl->v->eyeSpace);
	    }

	    for ((j = 0, p = stbl); j < MAXSURFACE; (++j, ++p)) {
		if (p->type != CT_FREE)
		    insertCraft(c, p);
	    }

	    for ((j = 0, p = ptbl); j < MAXPLAYERS; (++j, ++p)) {
		if (p->type != CT_FREE &&
		    (p != c || c->flags & FL_CHASE_VIEW) &&
		    p->Sg.z > cbase) {
		    insertCraft(c, p);
		}
	    }

	    for ((j = 0, p = mtbl); j < MAXPROJECTILES; (++j, ++p)) {
		if (p->type != CT_FREE && p->Sg.z > cbase) {
		    insertCraft(c, p);
		}
	    }
	}

	for (p = sortList; p != (craft *) NULL; p = (craft *) p->next)
	    placeCraft(c->vl->v, c, p, poly, &polyCount);

/*
 *  Clip all polygons
 */

	for (j = 0; j < polyCount; ++j) {
	    poly[j] = VClipSidedPolygon(c->vl->v, poly[j],
					c->vl->v->clipPoly);
	}

/*
 *  Display this image for each viewer associated with this craft
 */

	for (q = c->vl; q != NULL; q = q->next) {

/*
 *  Fill in the sky color
 */

	    z.depth = MaxDepth;
	    z.color = q->v->pixel[bgIndex];
	    FillRectangle(q->v->w, 0, 0, q->width, q->height, &z);
	    panelz.depth = MaxDepth;
	    panelz.color = q->v->pixel[blackPixel];
	    FillRectangle(q->v->w, q->rx, q->ry,
			  q->width - q->rx, q->radarWidth, &panelz);

/*
 *  Now the polygons ..
 */

	    curPixel = -1;
	    for (j = 0; j < polyCount; ++j) {
		if (poly[j]) {
		    VFillPolygon(q->v, q->win, q->gc, poly[j]);
		}
	    }

/*  Draw Head-Up Display and instrument panel */

	    doHUD(c, q);
	    doRadar(c, q);
	    doHSI(c, q);	/* doRadar must be called first */
	    doTEWS(c, q);	/* doRadar must be called first */
	    doRPM(c, q);

/*  Expose the completed drawing  */

	    VExposeBuffer(q->v, q->gc);

/*
 *  I moved this to avoid a conflict with the V library.
 */

	    doPanel(c, q);

	    switch (q->redrew) {
	    case 0:
		break;
	    case 1:
		q->redrew = 0;
		break;
	    case 2:
		q->redrew = 1;
		break;
	    }

	}

/*
 *  Release polygons
 */

	for (j = 0; j < polyCount; ++j)
	    if (poly[j])
		VDestroyPolygon(poly[j]);

    }

/*
 *  Now update chaser windows
 */

    for (i = 0, c = ctbl; i < MAXCHASERS; ++i, ++c) {
	craft *cp;		/* whom he is chasing */
	double cosPsi, sinPsi;

	if (c->type == CT_FREE)
	    continue;

	/* c->team stands for the chased airplane in this case */
	cp = &ptbl[c->team];

/*
 *  If plane has died, then kill the chaser
 */

	if (cp->type != CT_PLANE && cp->type != CT_DRONE) {
	    killChaser(c);
	    continue;
	}
/*
 *  Build trihedral matrix with only heading <g>
 */
	vp.x = cp->Sg.x - cp->prevSg.x;
	vp.y = cp->Sg.y - cp->prevSg.y;
	vp.z = 0;
	d = mag(vp);
	if (d != 0.) {		/* I think this should be always true<g>..T.R. */
	    sinPsi = vp.y / d;
	    cosPsi = vp.x / d;
	} else {
	    sinPsi = sin(cp->curHeading);
	    cosPsi = cos(cp->curHeading);
	}
	c->trihedral.m[0][0] = cosPsi;
	c->trihedral.m[0][1] = -sinPsi;
	c->trihedral.m[0][2] = 0.;
	c->trihedral.m[1][0] = sinPsi;
	c->trihedral.m[1][1] = cosPsi;
	c->trihedral.m[1][2] = 0.;
	c->trihedral.m[2][0] = 0.;
	c->trihedral.m[2][1] = 0.;
	c->trihedral.m[2][2] = 1.;
	c->trihedral.m[0][3] = c->trihedral.m[1][3] = c->trihedral.m[2][3] = 0.0;
	c->trihedral.m[3][0] = c->trihedral.m[3][1] = c->trihedral.m[3][2] = 0.0;
	c->trihedral.m[3][3] = 1.0;


/*
 *  Set up the eyespace transformation for this viewpoint
 */
	vp.x = cp->Sg.x
	    - c->p * (cosPsi * c->viewDirection.x - sinPsi * c->viewDirection.y);
	vp.y = cp->Sg.y
	    - c->p * (sinPsi * c->viewDirection.x + cosPsi * c->viewDirection.y);
	vp.z = cp->Sg.z;

	VTransform(&c->viewDirection, &(c->trihedral), &fwd);
	fwd.x += vp.x;
	fwd.y += vp.y;
	fwd.z += vp.z;

	VTransform(&c->viewUp, &(c->trihedral), &up);
	up.x += vp.x;
	up.y += vp.y;
	up.z += vp.z;

	VGetEyeSpace(c->vl->v, vp, fwd, up);

	polyCount = 0;
	sortList = (craft *) NULL;

	addGround(c, poly, &polyCount, groundColor, 0.0);

	for (j = 0; j < polyCount; ++j) {
	    VTransformPolygon(poly[j], &c->vl->v->eyeSpace);
	}

	for ((j = 0, p = stbl); j < MAXSURFACE; (++j, ++p)) {
	    if (p->type != CT_FREE)
		insertCraft(c, p);
	}

	for ((j = 0, p = ptbl); j < MAXPLAYERS; (++j, ++p)) {
	    if (p->type != CT_FREE && (p != c || c->flags & FL_CHASE_VIEW)) {
		insertCraft(c, p);
	    }
	}

	for ((j = 0, p = mtbl); j < MAXPROJECTILES; (++j, ++p)) {
	    if (p->type != CT_FREE)
		insertCraft(c, p);
	}

	for (p = sortList; p != (craft *) NULL; p = (craft *) p->next) {
	    placeCraft(c->vl->v, c, p, poly, &polyCount);
	}

/*
 *  Clip all polygons
 */

	for (j = 0; j < polyCount; ++j) {
	    poly[j] = VClipSidedPolygon(c->vl->v, poly[j],
					c->vl->v->clipPoly);
	}

/*
 *  Display this image for each viewer associated with this craft
 */

	for (q = c->vl; q != NULL; q = q->next) {

/*
 *  Fill in the sky color
 */
	    z.depth = MaxDepth;
	    z.color = *(q->v->pixel);
	    FillRectangle(q->v->w, 0, 0, q->width, q->height, &z);

/*
 *  Now the polygons ..
 */

	    for (j = 0; j < polyCount; ++j) {
		if (poly[j]) {
		    VFillPolygon(q->v, q->win, q->gc, poly[j]);
		}
	    }

/*  Expose the completed drawing  */

	    VExposeBuffer(q->v, q->gc);

	}

/*
 *  Release polygons
 */

	for (j = 0; j < polyCount; ++j)
	    if (poly[j])
		VDestroyPolygon(poly[j]);

    }

}

/*
 *  insertCraft :  Insert a craft into sortList in descending order.
 */

void
insertCraft(c, p)
craft *c, *p;
{

    double dist, offset;
    VPoint ds;
    craft *q, *prev;

/*
 *  Here's a kludge for you:  to avoid polygon clipping, I'm going to 
 *  cheat and hack a way to get ground objects to display properly.
 *  if the Z coordinate of an object is zero (i.e. on ground objects),
 *  I'll add a huge offset to their distance values to force them to be
 *  plotted first -- and in roughly their correct drawing order.
 *
 *  To support automated world outline maps, stbl[0] is considered to be
 *  the world map, and will always be plotted in the background.
 */

    offset = (p->Sg.z == 0.0) ? 1000.0 * NM : 0.0;
    if (p == &stbl[0]) {
	offset += 500.0 * NM;
    }
    ds.x = p->Sg.x - c->Sg.x;
    ds.y = p->Sg.y - c->Sg.y;
    ds.z = p->Sg.z - c->Sg.z;
    p->order = dist = mag(ds) + offset;

    if (sortList != (craft *) NULL) {
	for ((q = sortList, prev = (craft *) NULL); q != (craft *) NULL;) {
	    if (q->order < dist) {
		p->next = (struct _craft *) q;
		if (prev == (craft *) NULL)
		    sortList = p;
		else
		    prev->next = (struct _craft *) p;
		break;
	    }
	    prev = q;
	    q = (craft *) q->next;
	}
	if (q == (craft *) NULL) {
	    prev->next = (struct _craft *) p;
	    p->next = (struct _craft *) NULL;
	}
    } else {
	sortList = p;
	p->next = (struct _craft *) NULL;
    }
}

int
doRPM(c, u)
craft *c;
viewer *u;
{

    int x, y, x0, y0, x1, y1, len, i, ffth, fftw, last;
    double percent, p, sp, cp;
    char s[32], s1[32];

    x = u->xCenter + (u->radarWidth + 1) / 2 +
	(int) (20.0 * u->scaleFactor + 0.5);

    y = u->height +
	(int) ((VISOR_MARGIN + 80.0) * u->scaleFactor + 0.5);

    x += handleDn_width + 100 + (int) (10.0 * u->scaleFactor + 0.5);

    len = eng_x_hot - 6;
    ffth = u->rfont->max_bounds.ascent + u->rfont->max_bounds.descent;
    fftw = u->rfont->max_bounds.width;

    if (isFunctioning(c, SYS_ENGINE1))
	percent = (double) c->rpm * 100.0;
    else
	percent = 0.0;

/*
 *  We only redraw the RPM gauge when we're pretty sure that the gauge moved.
 *  (forced redraws are triggered in panel.c).
 */

    sprintf(s, "%3d%s", (int) (c->rpm * 100.0 + 0.5),
	    (c->flags & FL_AFTERBURNER) ? "AB" : "  ");
    if (strcmp(s, c->lastRPM) != 0) {

	XCopyPlane(u->dpy, u->eng, u->win, u->gauge_gc,
		   0, 0, eng_width, eng_height,
		   x - eng_x_hot, y - eng_y_hot, 1);

	if (percent < 60.0) {
	    p = (60.0 - percent) / 120.0 * pi;
	    sp = sin(p);
	    cp = cos(p);
	    x0 = x - (len >> 3) * cp;
	    y0 = y + (len >> 3) * sp;
	    x1 = x + len * cp;
	    y1 = y - len * sp;
	} else {
	    p = (percent - 60.0) / 40.0 * pi;
	    sp = sin(p);
	    cp = cos(p);
	    x0 = x - (len >> 3) * cp;
	    y0 = y - (len >> 3) * sp;
	    x1 = x + len * cp;
	    y1 = y + len * sp;
	}

	XDrawImageString(u->dpy, u->win,
			 u->gauge_gc,
			 x - eng_x_hot + 3 * fftw,
			 y - eng_y_hot + ffth + ffth,
			 s, strlen(s));
	strcpy(c->lastRPM, s);

	XDrawLine(u->dpy, u->win, u->gauge_gc, x0, y0, x1, y1);
    }
/*
 *  Print Fuel total and Fuel consumption rate
 */

    x = x - eng_x_hot;
    y = y + eng_x_hot + ffth + (ffth >> 1);

    sprintf(s, "%7s lbs Total", ItoA(((int) c->fuel) / 10 * 10, s1));
    if (strcmp(s, c->lastTotal) != 0 || u->redrew > 0) {
	XDrawImageString(u->dpy, u->win, u->gauge_gc,
			 x, y, s, strlen(s));
	strcpy(c->lastTotal, s);
    }
    sprintf(s, "%7s lbs/hour",
	    ItoA(((int) (fuelUsed(c) * 3600.0 / deltaT)) / 10 * 10, s1));
    if (strcmp(s, c->lastConsump) != 0 || u->redrew > 0) {
	XDrawImageString(u->dpy, u->win, u->gauge_gc,
			 x, y + ffth + 2, s, strlen(s));
	strcpy(c->lastConsump, s);
    }
/*
 *  Display Flap Setting
 */

    x = u->xCenter - (u->radarWidth + 1) / 2 - flaps0_width -
	(int) (20.0 * u->scaleFactor);
    y = FLAP_Y * u->width / FS_WINDOW_WIDTH;

/*
 *  We only adjust the flap indicator when we're pretty sure it moved
 */

    if ((last = (int) (c->flapSetting * 1000.0)) != u->lastFlap) {

	if (c->flapSetting > DEGtoRAD(29.0))
	    i = 3;
	else if (c->flapSetting > DEGtoRAD(19.0))
	    i = 2;
	else if (c->flapSetting > DEGtoRAD(9.0))
	    i = 1;
	else
	    i = 0;

	XCopyPlane(u->dpy, u->flap[i], u->win, u->gauge_gc,
		   0, 0, flaps0_width, flaps0_height, x, y, 1);
    }
    sprintf(s, "Flaps: %d ", (int) RADtoDEG(c->curFlap));
    if (strcmp(s, c->lastFlap) || u->redrew > 0) {
	XDrawImageString(u->dpy, u->win, u->gauge_gc,
		       x + flaps0_x_hot, y + flaps0_y_hot, s, strlen(s));
    }
    u->lastFlap = last;

    return 0;
}
