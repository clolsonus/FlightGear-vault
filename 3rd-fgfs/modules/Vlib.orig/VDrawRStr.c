#include <math.h>
#include <Vlib.h>
#include <VFont.h>
#include <VRoman.h>

void VDrawRotatedStrokeString (v, x, y, str, len, scale, zinfo)
Viewport *v;
register int	x, y;
register char	*str;
register int	len;
register double	angle;
register int	scale;
ZInfo		*zinfo; {

	register int	c, i, k, m, sina, cosa;
	register VGlyphVertex *p;
	register int	x1, y1, x2, y2;

	sina = sin(angle) * 16384.0;
	cosa = cos(angle) * 16384.0;

	for ( ; len > 0; -- len) {

	    if ((c = *str++) < 128) {
		k = VRomanGlyph[c].path_start;
		for (i = 0; i < VRomanGlyph[c].path_count; ++ i, ++ k) {
			p = &VRomanVertex[VRomanPath[k].vertex_start];
			x1 = x + p->x * scale / 25600;
			y1 = y - p->y * scale / 25600;
			++ p;
			for (m=1; m < VRomanPath[k].vertex_count; ++m, ++p) {
				x2 = x + p->x * scale / 25600;
				y2 = y - p->y * scale / 25600;
				DrawLine (v->w, x1, y1, x2, y2, zinfo);
				x1 = x2;
				y1 = y2;
			}
		}

		x  += VRomanGlyph[c].glyph_width * scale / 25600;

	    }
	}
}
