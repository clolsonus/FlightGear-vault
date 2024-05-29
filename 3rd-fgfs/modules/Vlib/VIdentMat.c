#include "Vlib.h"

static VMatrix ident = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
	};

void
VIdentMatrix (Mtx)
VMatrix *Mtx;
{
#ifdef notdef
	short I, J;

	for (I=0; I<4; ++I)
		for (J=0; J<4; ++J)
			if (I == J)
				(*Mtx).m[I][J] = 1.0;
			else
				(*Mtx).m[I][J] = 0.0;
#endif
	*Mtx = ident;
}
