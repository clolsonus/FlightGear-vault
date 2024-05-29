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
 
#include "pm.h"

static VPoint origin = { 0.0, 0.0, 0.0 };

extern buildEulerMatrix PARAMS((double, double, double, VMatrix *));

void
placeCraft (v, c, obj, poly, cnt)
Viewport *v;
craft	 *c;
craft	 *obj;
VPolygon **poly;
long	 *cnt; {

	int	 i, k, n;
	long	 j;
	VPoint	 *q, tmp, tmp1, center, nc;
	VMatrix	 mtx, mtx1;
	VPolygon **p, *p1;
	double	dist;
	int	aspect, offset;
	VObject	*op;

	j = *cnt;

/*
 *  Build a transformation matrix to convert from object to world coordinates.
 */
 
	if ((obj->flags & FL_FIXED_OBJECT) == 0) {
		buildEulerMatrix (obj->curRoll,
			obj->curPitch,
			obj->curHeading,
			&mtx);
		VTranslatePoint (&mtx, obj->Sg);
	}

/*
 *  If the object uses a special method to render itself, call that procedure.
 */
 
	if (obj->cinfo->placeProc != NULL) {
		(*obj->cinfo->placeProc)(v, obj, &mtx, poly, cnt);
		return;
	}

/*
 *  Build a matrix to transform from world to eye coordinate systems.
 */

	VMatrixMult (&mtx, &v->eyeSpace, &mtx1);

/*
 *  Reject the object if is completely outside any of the clipping planes
 */

	if (obj->flags & FL_FIXED_OBJECT)
		tmp = obj->cinfo->object->center;
	else
		VTransform (&obj->cinfo->object->center, &mtx, &tmp);
 
	VTransform (&tmp, &c->vl->v->eyeSpace, &center);
	for (i=0; i<4; ++i) {
		dist = VPointToClipPlaneDistance (&center,
			&(c->vl->v->clipNormals[i]));
		if (dist > obj->cinfo->object->extent)
			return;
	}

	op = obj->cinfo->object;
	n = op->numPolys;
	p = op->polygon;
	if (op->order) {
		VTransform (&origin, &mtx1, &nc);
		VTransform_ (&_VUnitVectorI, &mtx1, &op->xaxis);
		VTransform_ (&_VUnitVectorJ, &mtx1, &op->yaxis);
		VTransform_ (&_VUnitVectorK, &mtx1, &op->zaxis);
		aspect = VComputeObjectAspect (op, &nc);
#ifdef notdef
		printf ("%s: %s\n", op->name, VGetAspectName(aspect));
#endif
		offset = aspect * n;
	}

	for (i=0; i<n; ++i) {

/*
 *  Here is an opportunity to cull back-facing polygons without
 *  copying or transforming them. tmp1 becomes the transformed coordinates
 *  of the first vertex in the polygon.
 */

	    p1 = (op->order) ? p[op->order[offset + i]] : p[i];

	    VTransform (&p1->vertex[0], &mtx1, &tmp1);
	    if (p1->flags & PolyNormalValid) { 
	    	VTransform_ (&p1->normal, &mtx1, &tmp);
	    }
	    if (p1->flags & PolyClipBackface) {
		if (VDotProd(&tmp1, &tmp) >= 0.0) {
			continue;
		}
	    }

	    poly[j] = VCopyPolygon(p1);

/*
 *  First copy the normal vector and the transformed first point, then loop
 *  over all remaining points.
 */

	    if (poly[j]->flags & PolyNormalValid) { 
	    	poly[j]->normal = tmp;
	    }

	    q = poly[j]->vertex;

/*
 *  First, copy the previously transformed first vertex.
 */

	    *q = tmp1;
	    q++;

/*
 *  Now transform and copy the remaining vertices.
 */

	    for (k=1; k<poly[j]->numVtces; (++k, ++q)) {
		VTransform(q, &mtx1, &tmp);
		*q = tmp;
	    }
	    ++j;
	}

	*cnt = j;
}

/*
 *  OBSOLETE CODE: we no longer need this function.  We used to transform
 *  objects to their world coordinates and then transform them to eye
 *  coordinates.  Those are now consolidated into one step.
 *
 *  Place an object in world space based on the template object description,
 *  Euler angles, and [xyz] location.  This procedure is called only once
 *  for each fixed object (in init.c).
 */

#ifdef notdef

void
placeObject (obj)
craft	 *obj; {

	register int	 i, j, n;
	VPoint	 *q, tmp;
	VMatrix	 mtx;
	VPolygon **p;
	VObject	 *newObj;

	VIdentMatrix (&mtx);
	if (obj->curRoll != 0.0)
		VRotate (&mtx, XRotation, obj->curRoll);
	if (obj->curPitch != 0.0)
		VRotate (&mtx, YRotation, - obj->curPitch);
	if (obj->curHeading != 0.0)
		VRotate (&mtx, ZRotation, obj->curHeading);
	VTranslatePoint (&mtx, obj->Sg);

	if (obj->cinfo->placeProc != NULL) {
		fprintf (stderr, "This object cannot be placed this way.\n");
		exit (1);
	}

	newObj = VCopyObject (obj->cinfo->object);

	n = newObj->numPolys;
	p = newObj->polygon;
	for (i=0; i<n; ++i) {
		for ((j=0, q=p[i]->vertex); j<p[i]->numVtces; (++j, ++q)) {
			VTransform(q, &mtx, &tmp);
			*q = tmp;
		}
		if (p[i]->flags & PolyNormalValid) {
			VTransform (&p[i]->normal, &mtx, &tmp);
			p[i]->normal = tmp;
			p[i]->normal.x -= mtx.m[0][3];
			p[i]->normal.y -= mtx.m[1][3];
			p[i]->normal.z -= mtx.m[2][3];

		}
	}

	VTransform (&newObj->center, &mtx, &tmp);
	newObj->center = tmp; 

	obj->cinfo->object = newObj;

	obj->flags |= FL_FIXED_OBJECT;
}

#endif
