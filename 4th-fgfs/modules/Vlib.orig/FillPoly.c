#include <Alib.h>

extern int MakeET();

void FillPolygon (w, pts, npts, zinfo)
AWindow	*w;
Point	*pts;
int	npts;
ZInfo	*zinfo; {

	MakeET (w, pts, npts, zinfo);
}
