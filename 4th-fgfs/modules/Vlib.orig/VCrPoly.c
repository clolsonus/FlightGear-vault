#include "Vlib.h"

VPolygon *
VCreatePolygon (numVtces, vert, color)
int	numVtces;
VPoint	*vert;
VColor	*color;
{
	VPolygon	template;

	template.color = color;
	template.backColor = (VColor *) NULL;
	template.flags = 0;
	template.assignedDepth = -1;

	return VCreatePolygonFromTemplate (numVtces, vert, &template);
}

VPolygon *
VCreatePolygonFromTemplate (numVtces, vert, template)
int	numVtces;
VPoint	*vert;
VPolygon *template;
{
	VPolygon *p;
	VPoint	a, b;

	p = (VPolygon *) Vmalloc (sizeof (VPolygon));

	*p = *template;
	p->numVtces = numVtces;
	p->vertex = (VPoint *) Vmalloc (sizeof (VPoint) * numVtces);
	memcpy ((char *) p->vertex,
		(char *) vert, sizeof(VPoint) * p->numVtces);

	if ((p->flags & PolyNormalValid) == 0) {
		if ((p->flags & PolyClipBackface) != 0 ||
			p->backColor != (VColor *) NULL) {
			a.x = vert[0].x - vert[1].x;
			a.y = vert[0].y - vert[1].y;
			a.z = vert[0].z - vert[1].z;
			b.x = vert[2].x - vert[1].x;
			b.y = vert[2].y - vert[1].y;
			b.z = vert[2].z - vert[1].z;
			VCrossProd (&a, &b, &p->normal);
			p->flags |= PolyNormalValid;
		}
	}

	return p;
}
