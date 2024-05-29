#include "Vlib.h"
#include <math.h>

void
VComputeObjectExtent(obj)
VObject *obj;
{
	VPoint	sum;
	register int	i, j, npts = 0;
	register double d;

	obj->extent = 0.0;
	sum.x = 0.0;
	sum.y = 0.0;
	sum.z = 0.0;

/*
 *  Add the xyz components of each point in the object so that we can
 *  determine the average location (i.e. the center).
 */

	for (i=0; i<obj->numPolys; ++i) {

		for (j=0; j<obj->polygon[i]->numVtces; ++j) {

			sum.x += obj->polygon[i]->vertex[j].x;
			sum.y += obj->polygon[i]->vertex[j].y;
			sum.z += obj->polygon[i]->vertex[j].z;
			++ npts;
		}
	}

	if (npts != 0) {

		obj->center.x = sum.x / npts;
		obj->center.y = sum.y / npts;
		obj->center.z = sum.z / npts;

/*
 *   Determine the most distant point from the center of the object
 */

		for (i=0; i<obj->numPolys; ++i) {
			for (j=0; j<obj->polygon[i]->numVtces; ++j) {
				sum.x = obj->polygon[i]->vertex[j].x -
					obj->center.x;
				sum.y = obj->polygon[i]->vertex[j].y -
					obj->center.y;
				sum.z = obj->polygon[i]->vertex[j].z -
					obj->center.z;
				d = sqrt (sum.x * sum.x + sum.y * sum.y +
					sum.z * sum.z);
				if (d > obj->extent)
					obj->extent = d;
			}
		}

	}
	else {
		obj->center.x = obj->center.y = obj->center.z = 0.0;
	}

}
