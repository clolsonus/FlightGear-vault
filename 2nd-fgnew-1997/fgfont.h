/*                                      
 *
 * Copyright (C) 1996  Eric J. Korpela -- korpela@ssl.berkeley.edu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * $Id: fgfont.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */
#include "fgtypes.h"

#define MAX_FONT_WIDTH  1
#define MAX_FONT_HEIGHT 16

typedef struct _FGFONT_BITS {
  char name[30];
  FG_INT32 width, height;
  unsigned char bmp[256][MAX_FONT_HEIGHT];
} FGFONT_BITS;

typedef struct _FGFONT_PIXMAP {
  char *name;
  FG_INT32 width, height;
  char *pixmap[256];
} FGFONT_PIXMAP;

extern FGFONT_BITS fgfontb_8x8, fgfontb_9x14, fgfontb_10x16;
extern FGFONT_PIXMAP *fgfont_8x8, *fgfont_9x14, *fgfont_10x16;

FGFONT_PIXMAP *fg_scale_font(FGFONT_PIXMAP *font,FG_INT32 width, FG_INT32 height);
FGFONT_PIXMAP *fg_get_font(FG_INT32 width, FG_INT32 height);
FGFONT_PIXMAP *fg_load_font(FGFONT_BITS *fontbits);

void fgfont_init(void);
void fg_drawtext(char *text, FGFONT_PIXMAP *font, FG_INT32 x, FG_INT32 y, FG_INT32 color, FG_BITMAP *bmp);
/* $Log: fgfont.h,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 */
