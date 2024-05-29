#include <Vlib.h>
#include <VFont.h>
#include <VRoman.h>
/*ARGSUSED*/
int VFontWidthPixels (v, scale)
Viewport	*v;
int		scale; {

	return VRomanGlyph['A'].glyph_width * scale / 25600;

}

void VDrawStrokeString (v, x, y, str, len, scale, zinfo)
Viewport *v;
register int	x, y;
register unsigned char	*str;
register int	len;
register int	scale;
ZInfo		*zinfo; {

	register int	c, i, k, m;
	register VGlyphVertex *p;
	register int	x1, y1, x2, y2;

	for ( ; len > 0; -- len) {

	    if ((c = *str++) < 128) {
		k = VRomanGlyph[c].path_start;
		for (i = 0; i < VRomanGlyph[c].path_count; ++ i, ++ k) {
			p = &VRomanVertex[VRomanPath[k].vertex_start];
			x1 = p->x * scale / 25600 + x;
			y1 = y - p->y * scale / 25600;
			++ p;
			for (m=1; m < VRomanPath[k].vertex_count; ++m, ++p) {
				x2 = p->x * scale / 25600 + x;
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
