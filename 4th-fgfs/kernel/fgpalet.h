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
 * $Id: fgpalet.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */
#ifndef _FG_PALET_H
#define _FG_PALET_H
#include "fgtypes.h"

#define PAL_NEAREST 0
#define PAL_REQUIRED 1

#define GREYSCALE_ENTRY(R,G,B)  ((R)/3+(G)/6+(B)/2)
#define RGB565_ENTRY(R,G,B) {(R)/8,(G)/4,(B)/8}
#define BGR565_ENTRY(R,G,B) {(B)/8,(G)/4,(R)/8}
#define RGB555_ENTRY(R,G,B) {(R)/8,(G)/8,(B)/8}
#define BGR555_ENTRY(R,G,B) {(B)/8,(G)/8,(R)/8}
#define RGB664_ENTRY(R,G,B) {(R)/4,(G)/4,(B)/16}
#define BGR466_ENTRY(R,G,B) {(B)/16,(G)/4,(R)/4}
#define RGB888_ENTRY(R,G,B) {(R),(G),(B)}
#define BGR888_ENTRY(R,G,B) {(B),(G),(R)}
#define RGBX8888_ENTRY(R,G,B) {(R),(G),(B),0}
#define XBGR8888_ENTRY(R,G,B) {0,(B),(G),(R)}


#if defined(USE_PM)

#define PALETTE_ENTRY(R,G,B,O) { (B), (G), (R), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT8 b,g,r,flags;
} FGPAL_ENTRY;

extern HPAL fg_hpal;
extern HDC  fg_hdc;
extern HPS  fg_hps; 

#elif defined(USE_WIN32)
#ifndef PC_RESERVED
  #define PC_RESERVED 1
#endif
#define PALETTE_ENTRY(R,G,B,O) { (R), (G), (B), (O) }
typedef struct _FGPAL_ENTRY {
  FG_UINT8 r,g,b,flags;
} FGPAL_ENTRY;

#ifndef WINVER
  #ifndef OPEN32
	 #define WINVER 0x30a
  #else
	 #define WINVER 0x300
  #endif
#endif

extern HPALETTE   fg_hpalette;
extern PLOGPALETTE fg_logpalette;

#elif defined(USE_ALLEGRO)
#define PALETTE_ENTRY(R,G,B,O) { (R)>>2, (G)>>2, (B)>>2, (O) }
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

#ifndef USE_WIN32
extern FGPAL_ENTRY PaletteArray[NUM_COLORS];
#else
extern FGPAL_ENTRY *PaletteArray;
#endif
extern FGPAL_ENTRY StartupPalette[NUM_COLORS];
extern FG_UINT32 StandardColors[16];
extern FG_INT32 PaletteReferenceCount[NUM_COLORS];
#if defined(USE_PM) || defined(USE_WIN32)
  void fg_set_startup_palette(HWND hwnd);
#else
  void fg_set_startup_palette();
#endif

PIX_GREY8    vga_grey8[16];
PIX_LUT8     vga_lut8[16];
PIX_RGB565   vga_rgb565[16];
PIX_BGR565   vga_bgr565[16];
PIX_RGB555   vga_rgb555[16];
PIX_BGR555   vga_bgr555[16];
PIX_RGB664   vga_rgb664[16];
PIX_BGR466   vga_bgr466[16];
PIX_RGB888   vga_rgb888[16];
PIX_BGR888   vga_bgr888[16];
PIX_RGBX8888 vga_rgbx8888[16];
PIX_XBGR8888 vga_xbgr8888[16];

FG_INT32  fg_find_closest_color(FGPAL_ENTRY *RGBColor);
#endif
