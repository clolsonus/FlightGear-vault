#include "Vlib.h"

void
VSetVisibility(d)
double	d;
{
	int	i;
	double	k;

	_VDefaultWorkContext->visibility = d;
	if (_VDefaultWorkContext->visTable) {
		free ((char *) _VDefaultWorkContext->visTable);
	}

	_VDefaultWorkContext->visTable = (double *) Vmalloc (sizeof(double) *
		_VDefaultWorkContext->depthCueSteps);

	k = log (1.0 / (double) _VDefaultWorkContext->depthCueSteps) /
		_VDefaultWorkContext->visibility;

#ifdef DEBUG
	fprintf (stderr, "k = %lg\n", k);
#endif
	
	for (i=0; i<_VDefaultWorkContext->depthCueSteps-1; ++i) {
	    _VDefaultWorkContext->visTable[i] =
		log (1.0 - (double)(i+1) /
		(double)_VDefaultWorkContext->depthCueSteps) / k;
#ifdef DEBUG
		fprintf (stderr, "next threshold is %lg units\n",
			_VDefaultWorkContext->visTable[i] );
#endif
	}
}

int
VComputePolygonColor (v, poly)
Viewport *v;
VPolygon *poly;
{
	VColor	*c;
	VPoint	*p;
	int	i;
	double	d;

/*
 *  First, are we seeing the front or the back of this polygon?
 */
 
	if (poly->flags & PolyUseBackColor) {
		c = poly->backColor;
	}
	else {
		c = poly->color;
	}

/*
 *  If depth queueing isn't turned on, or this color is not a depth-cueued
 *  color, then simply return the color index.
 */
 
	if ((v->flags & VPDepthCueing) == 0 || 
		(c->flags & ColorEnableDepthCueing) == 0) {
		return v->pixel[c->cIndex];
	}

/*
 *  Okay, it is a depth cueued color.  Check this polygon's distance against
 *  the different cutoff points for the current world visibility value.  If the
 *  distance is greater than any of the cutoff values, then render the object
 *  as a pure "haze" color.
 */

	else {
		if ((i = poly->assignedDepth) > -1) {
		    return v->pixel[(i<_VDefaultWorkContext->depthCueSteps-1) ?
			c->cIndex + i :
			_VDefaultWorkContext->depthCueColor->cIndex ];
		}
		p = &poly->vertex[0];
		d = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
		for (i=0; i<_VDefaultWorkContext->depthCueSteps-1; ++i) {
			if (d < _VDefaultWorkContext->visTable[i]) {
				return v->pixel[c->cIndex + i];
			}
		}
		return v->pixel[_VDefaultWorkContext->depthCueColor->cIndex];
	}
}

