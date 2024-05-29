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
 * $Id: fgpalet.h,v 2.1 1996/10/31 18:20:21 korpela Exp korpela $
 */
#include "fgtypes.h"

#define PAL_NEAREST 0
#define PAL_REQUIRED 1

#if defined(USE_PM)

#define PALETTE_ENTRY(R,G,B,O) { (B), (G), (R), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT8 b,g,r,flags;
} FGPAL_ENTRY;

extern HPAL fg_hpal;
extern HDC  fg_hdc;
extern HPS  fg_hps; 

#elif defined(WIN32)
#define PALETTE_ENTRY(R,G,B,O) { (R), (G), (B), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT8 r,g,b,flags;
}

extern HPALETTE   fg_hpalette;
extern HDC        fg_hdc;
extern LOGPALETTE fg_logpalette;

#elif defined(__DJGPP__)
#define PALETTE_ENTRY(R,G,B,O) { (R), (G), (B), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT8 r,g,b,flags;
}  FGPAL_ENTRY;

#elif defined(USE_X11)
#define PALETTE_ENTRY(R,G,B,O) { (R), (G), (B), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT16 r,g,b,flags;
} FGPAL_ENTRY;

extern FG_UINT8 Xpal[256];

#elif defined(USE_FB)
#define PALETTE_ENTRY(R,G,B,O) { (R), (G), (B), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT8 r,g,b,flags;
} FGPAL_ENTRY;

#endif

extern FGPAL_ENTRY StartupPalette[NUM_COLORS];
extern FGPAL_ENTRY PaletteArray[NUM_COLORS];
extern int PaletteReferenceCount[NUM_COLORS];

void fg_set_startup_palette();
int  fg_find_closest_color(FGPAL_ENTRY *RGBColor);
