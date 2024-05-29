#include <Vlib.h>

void VExposeViewport (v)
Viewport *v; {

	ForceWindowRedraw (v->w);
}
