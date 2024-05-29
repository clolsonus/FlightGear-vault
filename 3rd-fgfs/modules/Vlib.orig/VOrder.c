#include "Vlib.h"
#include <math.h>

#define mag(x,y,z) sqrt ( (x) * (x) + (y) * (y) + (z) * (z) )

struct _euler {
	double	yaw;
	double	pitch;
	};

struct entry {
	int		id;
	double		d;
	struct entry	*next;
	};

/*
 *  Views corresponding to each of the eight defined major aspects
 */

static struct _euler view[] = {
	{ DEGtoRAD(  90.0), DEGtoRAD(  0.0) }, /* 0 - right */
	{ DEGtoRAD( -90.0), DEGtoRAD(  0.0) }, /* 1 - left */
	{ DEGtoRAD(  45.0), DEGtoRAD( 45.0) }, /* 2 - front right bottom */
	{ DEGtoRAD(  45.0), DEGtoRAD(-45.0) }, /* 3 - front right top    */
	{ DEGtoRAD( -45.0), DEGtoRAD( 45.0) }, /* 4 - front left bottom  */
	{ DEGtoRAD( -45.0), DEGtoRAD(-45.0) }, /* 5 - front left top     */
	{ DEGtoRAD( 135.0), DEGtoRAD( 45.0) }, /* 6 - aft right bottom   */
	{ DEGtoRAD( 135.0), DEGtoRAD(-45.0) }, /* 7 - aft right top	 */
	{ DEGtoRAD(-135.0), DEGtoRAD( 45.0) }, /* 8 - aft left bottom	 */
	{ DEGtoRAD(-135.0), DEGtoRAD(-45.0) }, /* 9 - aft left top	 */
	};

static char *aspect_name[] = {
	"left",
	"right",
	"front right bottom",
	"front right top",
	"front left bottom",
	"front left top",
	"aft right bottom",
	"aft right top",
	"aft left bottom",
	"aft left top"
	};

void
VComputePolygonOrdering (obj)
VObject	*obj;
{
	register int i, j, k, inserted;
	register double d, dn;
	VMatrix	 mtx;
	VPoint	pt1, pt2;
	VPolygon **p;
	struct entry ent[VmaxVP], *head, *e, *last_e;

	obj->order = (unsigned short *)
		Vmalloc (sizeof (unsigned short) * NUM_ASPECTS * obj->numPolys);

	for (i=0; i<NUM_ASPECTS; ++i) {

		VIdentMatrix (&mtx);
		VRotate (&mtx, YRotation, - view[i].pitch);
		VRotate (&mtx, ZRotation, view[i].yaw);
		pt1.x = obj->extent * 10.0;
		pt1.y = pt1.z = 0.0;
		VTransform (&pt1, &mtx, &pt2);
		head = (struct entry *) NULL;
		for (j=0, p=obj->polygon; j<obj->numPolys; ++j) {

/*
 *  Compute the distance to the farthest vertex on this polygon
 */

			dn = mag (p[j]->vertex[0].x - pt2.x,
				p[j]->vertex[0].y - pt2.y,
				p[j]->vertex[0].z - pt2.z);

			for (k=1; k<p[j]->numVtces; ++k) {
				d = mag (
					p[j]->vertex[k].x - pt2.x,
					p[j]->vertex[k].y - pt2.y,
					p[j]->vertex[k].z - pt2.z);
				if (d > dn) {
					dn = d;
				}
			}

			ent[j].id = j;
			ent[j].d = dn;

/*
 *  Insert this entry into the descending sorted list of polygons
 */

			if (!head) {
				ent[j].next = head;
				head = &ent[j];
			}
			else {
				last_e = (struct entry *) NULL;
				inserted = 0;
				for (e=head; e; e=e->next) {
					if (e->d < ent[j].d) {
					    if (last_e) {
						ent[j].next = last_e->next;
						last_e->next = &ent[j];
					    }
					    else {
						ent[j].next = e;
						head = &ent[j];
					    }
					    inserted = 1;
					    break;
					}
					last_e = e;
				}
				if (inserted == 0) {
					last_e->next = &ent[j];
					ent[j].next = (struct entry *) NULL;

				}
			}
		}

/*
 *  Copy this list as the hints for this quadrant.
 */

		k = obj->numPolys * i;
		for (j=0, e=head; j<obj->numPolys; ++j, e=e->next) {
			obj->order[k + j] = e->id;
		}

	}
}

/*
 *  Determine if an object should be ordered.  Objects that are not made of
 *  entirely the same color, or that have colored back-sides probably should
 *  be ordered.
 */

int
VObjectNeedsOrdering (obj)
VObject *obj;
{
	VPolygon	**p;
	VColor		*c;
	int		i;

	if (obj->order || obj->numPolys == 0) {
		return 0;
	}

	c = obj->polygon[0]->color;

	for (i=0, p=obj->polygon; i<obj->numPolys; ++i) {
#ifdef notdef
		if (p[i]->color != c) {
			return 1;
		}
#endif
		if (p[i]->backColor) {
			return 1;
		}
	}

	return 0;
}


int
VComputeObjectAspect(obj, loc)
VObject	*obj;
VPoint	*loc;
{
	register int q;
	register double a, b, c, m;

	m = mag (loc->x, loc->y, loc->z);

	a = VDotProd (loc, &obj->xaxis);
	b = VDotProd (loc, &obj->yaxis) / m;
	c = VDotProd (loc, &obj->zaxis);

/*
 *  If the absolute angle between the Y axis and viewing vector is less than 30
 *  degrees, then it is either a right or left aspect.
 */

	if (b > 0.866) {
		return 1;
	}
	else if (b < -0.866) {
		return 0;
	}

	if (a < 0.0 ) {
/* front */
		if (b < 0.0) {
/* front right */
			if (c < 0.0) {
/* front right bottom */
				q = 2;
			}
			else {
/* front right top */
				q = 3;
			}
		}
		else {
/* front left */
			if (c < 0.0) {
/* front left bottom */
				q = 4;
			}
			else {
/* front left top */
				q = 5;
			}
		}
	}
	else {
/* aft */
		if (b < 0.0) {
/* aft right */
			if (c < 0.0) {
/* aft right bottom */
				q = 6;
			}
			else {
/* aft right top */
				q = 7;
			}
		}
		else {
/* aft left */
			if (c < 0.0) {
/* aft left bottom */
				q = 8;
			}
			else {
/* aft left top */
				q = 9;
			}
		}
	}

	return q;	
}

char *
VGetAspectName (aspect)
int aspect;
{
	return aspect_name[aspect];
}

