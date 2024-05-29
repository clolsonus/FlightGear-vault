#include "Vlib.h"

VObject*
VCopyObject (obj)
VObject	*obj;
{

	register int		i;
	register VObject	*newObj;

	newObj = (VObject *) Vmalloc (sizeof (VObject));
	newObj->name = obj->name;
	newObj->extent = obj->extent;
	newObj->center = obj->center;
	newObj->numPolys = obj->numPolys;
	newObj->polygon =
		(VPolygon **) Vmalloc (sizeof(VPolygon *) * newObj->numPolys);
	if (obj->order) {
		newObj->order = (unsigned short *)
			Vmalloc (sizeof(unsigned short) * NUM_ASPECTS *
			newObj->numPolys);
		memcpy ((char *) newObj->order, (char *) obj->order,
			sizeof(unsigned short) * NUM_ASPECTS *
			newObj->numPolys);
	}
	else {
		newObj->order = (unsigned short *) NULL;
	}

	for (i=0; i <obj->numPolys; ++i) {
		if ((newObj->polygon[i] = VCopyPolygon (obj->polygon[i]))
			== (VPolygon *) NULL) {
			fprintf (stderr, "VCopyObject: can't copy polygons\n");
			exit (1);
		}
	}

	return newObj;
}
