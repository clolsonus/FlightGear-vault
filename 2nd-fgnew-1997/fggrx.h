/* fggrx.h -- flight 2D bitmap graphics routines
 *
 * Copyright (C) 1996  Eric J. Korpela -- korpeLa@ssl.berkeley.edu
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
 * $Id: fggrx.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGGRX_H
#define _FGGRX_H

#include "fgdefs.h"
#include "fgtypes.h"

extern FG_INT32 pix_size[];
extern void (*plot_fns[])(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp);
extern void (*hline_fns[])(FG_INT32 x1, FG_INT32 x2, FG_INT32 y,
                    FG_UINT32 color, FG_BITMAP *bmp); 
extern FG_UINT32 (*getp_fns[])(FG_INT32 x, FG_INT32 y,
                    FG_BITMAP *bmp);

FG_BITMAP *fg_create_bitmap(FG_UINT32 width, FG_UINT32 height, 
                            FG_UINT32 color_format);
void fg_destroy_bitmap(FG_BITMAP *bmp);
FG_BITMAP *fg_create_subbitmap(FG_BITMAP *bmp, FG_INT32 x, FG_INT32 y,
                               FG_INT32 width, FG_INT32 height);

inline static void fg_plot(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp)
{
  bmp->plot(x,y,color,bmp);
}
inline void fg_plot_8(FG_INT32 x, FG_INT32  y, FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_plot_16(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_plot_24(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_plot_32(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp);

inline static FG_UINT32 fg_getpixel(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp)
{
  return bmp->getpixel(x,y,bmp);
}
inline FG_UINT32 fg_getpixel_8(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp);
inline FG_UINT32 fg_getpixel_16(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp);
inline FG_UINT32 fg_getpixel_24(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp);
inline FG_UINT32 fg_getpixel_32(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp);

inline static void fg_hline(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                       FG_UINT32 color, FG_BITMAP *bmp)
{
  bmp->hline(x1,x2,y,color,bmp);
}
inline void fg_hline_8(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                       FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_hline_16(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                       FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_hline_24(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                       FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_hline_32(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                       FG_UINT32 color, FG_BITMAP *bmp);

inline void fg_vline(FG_INT32 x, FG_INT32 y1, FG_INT32 y2, 
                       FG_UINT32 color, FG_BITMAP *bmp);

inline void fg_linex(FG_INT32 x1, FG_INT32 y1, FG_INT32 deltax, FG_INT32 deltay,
                     FG_UINT32 color, FG_BITMAP *bmp);
inline void fg_liney(FG_INT32 x1, FG_INT32 y1, FG_INT32 deltax, FG_INT32 deltay,
                     FG_UINT32 color, FG_BITMAP *bmp);
void fg_line(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, FG_UINT32 color, 
             FG_BITMAP *bmp);

void fg_rect(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, 
                   FG_UINT32 color, FG_BITMAP *bmp);
void fg_RECT(FG_RECT *rect, FG_UINT32 color, FG_BITMAP *bmp);

void fg_fillrect(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, 
                   FG_UINT32 color, FG_BITMAP *bmp);
void fg_fillRECT(FG_RECT *rect, FG_UINT32 color, FG_BITMAP *bmp);

FG_POLYGON *fg_create_polygon(FG_INT32 vertices);
void fg_destroy_polygon(FG_POLYGON *ply);
void fg_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy);
void fg_scale_POLY(FG_POLYGON *poly,FG_INT32 x, FG_INT32 y, FG_INT32 scale);
void fg_scale_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT32 x,
                        FG_INT32 y, FG_INT32 scale);
void fg_translate_POLY(FG_POLYGON *poly, FG_INT32 dx, FG_INT32 dy);
void fg_translate_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT32 dx,
                        FG_INT32 dy);
void fg_rotate_POLY(FG_POLYGON *poly, FG_INT32 x, FG_INT32 y, FG_INT32 angle);
void fg_rotate_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT32 x,
                         FG_INT32 y, FG_INT32 angle);
void fg_draw_POLY(FG_POLYGON *poly, FG_UINT32 color, FG_BITMAP *bmp);

FG_RASTER_SET *fg_create_RASTER(FG_INT32 nlines);
void fg_display_RASTER(FG_INT32 y,FG_RASTER_SET *raster, FG_BITMAP *bmp);
void fg_rasterize_edge(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2,
					  FG_HLINE **hlines, FG_INT32 setstart);
void fg_rasterize_convex_POLY(FG_POLYGON *poly, FG_RASTER_SET **raster,
                              FG_UINT32 color);
void fg_fill_convex_POLY(FG_POLYGON *poly, FG_UINT32 color, FG_BITMAP *bmp);


void fggrx_export(void);
void fg_clearbmp(FG_UINT32 color, FG_BITMAP *bmp);

#endif

/* $Log: fggrx.h,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.4  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.3  1996/09/04  19:03:09  korpela
 * More polygon stuff...
 *
 * Revision 1.2  1996/05/06  20:20:38  korpela
 * Added some polygon functions.
 *
 * Revision 1.1  1996/05/05  22:01:26  korpela
 * Initial revision
 * */
