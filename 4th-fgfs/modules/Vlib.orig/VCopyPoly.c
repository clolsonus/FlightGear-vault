#include "Vlib.h"

VPolygon *
VCopyPolygon (poly)
VPolygon *poly;
{
	return VCreatePolygonFromTemplate (poly->numVtces, poly->vertex, poly);
}
