#include "Vlib.h"

void VCloseViewport (v)
Viewport *v; {

	CloseAWindow (v->w);

	VDestroyPolygon (v->clipPoly);
	free ((char *) v->zpool);
	free ((char *) v);
}
