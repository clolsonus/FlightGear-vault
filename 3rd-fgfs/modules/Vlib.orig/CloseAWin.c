#include <Alib.h>

/*
 *  I'd use #elif's except for that bizarre HP-UX C preprocessor bug
 *  that chokes on #elif's and long command lines .....
 */

#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#else
#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#else
extern void free();
#endif
#endif

void
CloseAWindow (w)
AWindow *w;
{
	free ((char *) w->edges);
	free ((char *) w->lines);
	free ((char *) w->edgePool);
	free ((char *) w->csPool0);
	free ((char *) w->csPool1);
	free ((char *) w->scanLine);
	free ((char *) w->lastScanLine);
	free ((char *) w);
}
