#include "Vlib.h"
#include <string.h>

VColor *
VAllocColor (name)
char *name;
{
	return VAllocDepthCueuedColor (name, 0);
}


VColor *
VAllocDepthCueuedColor (name, flag)
char	*name;
int	flag;
{

	VColor	*p=_VDefaultWorkContext->VColorList, *prev=0, **q;

/*
 *  Search for this color among those already allocated.
 */

	while (p != (VColor *) 0) {
		if (strcmp(p->color_name, name) == 0) {
			if ((flag && (p->flags & ColorEnableDepthCueing)) ||
				(flag == 0 && p->flags == 0)) {
				return p;
			}
		}
		prev = p;
		p = p->next;
	}

/*
 *  The color was not in the list; allocate a new list element.
 */

	if (prev == (VColor *)0)
		q = &_VDefaultWorkContext->VColorList;
	else
		q = &(prev->next);

	*q = (VColor *) Vmalloc (sizeof(VColor));
	(*q)->color_name = strdup (name);
	(*q)->cIndex = 0;
	(*q)->flags = flag ? ColorEnableDepthCueing : 0;
	(*q)->next = 0;

	return *q;
}
