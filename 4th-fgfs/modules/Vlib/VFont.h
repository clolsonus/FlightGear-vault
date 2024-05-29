#ifndef _VFont_h
#define _VFont_h

typedef struct {
	short	path_count;
	short	path_start;
	short	glyph_width;
	}	VGlyph;

typedef struct {
	short	vertex_count;
	short	vertex_start;
	}	VGlyphPath;

typedef struct {
	short	x;
	short	y;
	}	VGlyphVertex;

typedef struct {
	short	x, y, z;
	}	VGlyphVertex3;

#endif
