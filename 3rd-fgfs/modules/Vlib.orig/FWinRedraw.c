#include <Alib.h>

/*
 *  ForceWindowRedraw :  Invalidate the previous frame's drawing information
 *			 so that the animation is completely redrawn at the
 *			 completion of the current frame.
 */
 
void
ForceWindowRedraw (w)
AWindow	*w; {

	register int i;

	for (i=0; i<w->height; ++i)
		w->lastScanLine[i].count = 0;

#ifdef HAS_FRAME_BUFFER
	LocateWindowOrigin (w->display, w->d, &x, &y);
	FrameBufferSetOrigin (x, y);
#endif
}
