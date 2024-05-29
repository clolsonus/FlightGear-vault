/* fggrx.c -- flight 2D bitmap graphics routines 
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
 * $Id: fggrx.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <limits.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fggeom.h"
#include "fgscreen.h"
#include "fgpalet.h"
#include "fggrx.h"

FG_INT32 pix_size[]={
  sizeof(PIX_LUT8),    sizeof(PIX_GREY8),
  sizeof(PIX_RGB565),  sizeof(PIX_BGR565),
  sizeof(PIX_RGB555),  sizeof(PIX_BGR555),
  sizeof(PIX_RGB664),  sizeof(PIX_BGR466),
  sizeof(PIX_RGB888),  sizeof(PIX_BGR888),
  sizeof(PIX_RGBX8888),sizeof(PIX_XBGR8888)
};

void (*plot_fns[])(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp)= {
  fg_plot_8, fg_plot_8,
  fg_plot_16, fg_plot_16,
  fg_plot_16, fg_plot_16,
  fg_plot_16, fg_plot_16,
  fg_plot_24, fg_plot_24,
  fg_plot_32, fg_plot_32
};

void (*hline_fns[])(FG_INT32 x1, FG_INT32 x2, FG_INT32 y,
                    FG_UINT32 color, FG_BITMAP *bmp) ={
  fg_hline_8, fg_hline_8,
  fg_hline_16, fg_hline_16,
  fg_hline_16, fg_hline_16,
  fg_hline_16, fg_hline_16,
  fg_hline_24, fg_hline_24,
  fg_hline_32, fg_hline_32
};

FG_VOID *std_colors[]={
  vga_lut8,vga_grey8,
  vga_rgb565,vga_bgr565,
  vga_rgb555,vga_bgr555,
  vga_rgb664,vga_bgr466,
  vga_rgb888,vga_rgb888,
  vga_rgbx8888,vga_xbgr8888
};

FG_UINT32 (*getp_fns[])(FG_INT32 x, FG_INT32 y,
                    FG_BITMAP *bmp) ={
  fg_getpixel_8, fg_getpixel_8,
  fg_getpixel_16, fg_getpixel_16,
  fg_getpixel_16, fg_getpixel_16,
  fg_getpixel_16, fg_getpixel_16,
  fg_getpixel_24, fg_getpixel_24,
  fg_getpixel_32, fg_getpixel_32
};



FG_BITMAP *fg_create_bitmap(FG_UINT32 width, FG_UINT32 height, FG_UINT32 color_format)
{
  FG_BITMAP *bmp;
  FG_INT32 i;

  /* allocate the memory for the bitmap */
  if ((bmp=(FG_BITMAP *)calloc(1,sizeof(FG_BITMAP)+
                                  height*sizeof(FG_VOID *))) && 
      (bmp->lines=(FG_VOID **)((char *)bmp+sizeof(FG_BITMAP))) &&
      (bmp->lines[0]=(FG_VOID *)calloc(width*height,pix_size[color_format])))
  {
 
    /* if we got it, set up the structure */    
    bmp->struct_size=sizeof(FG_BITMAP);
    bmp->w=width;
    bmp->h=height;
    bmp->pix_size=pix_size[color_format];
    bmp->color_format=color_format;
    bmp->vga_colors=std_colors[color_format];


    /* set clipping */
    bmp->clip=1;
    bmp->cl=0; bmp->cr=width-1;
    bmp->ct=0; bmp->cb=height-1;
    bmp->plot=plot_fns[color_format];
    bmp->hline=hline_fns[color_format];
    bmp->getpixel=getp_fns[color_format];

    /* set pixel addresses */
    for (i=1;i<height;i++) {
      bmp->lines[i] = ((char *)(bmp->lines[i-1]))+width*bmp->pix_size;
    }
  } else {
    bmp=NULL;
  }
  return (bmp);
}

void fg_destroy_bitmap(FG_BITMAP *bmp)
{
  if ((bmp->type != FG_SUBBITMAP) && bmp->lines[0]) {
    free(bmp->lines[0]);
    bmp->lines[0]=NULL;
  }
  if (bmp) {
    free(bmp);
  }
}
    
FG_BITMAP *fg_create_subbitmap(FG_BITMAP *bmp, FG_INT32 x, FG_INT32 y,
                               FG_INT32 width, FG_INT32 height)
/* creates a new bitmap that is a rectangular subsection of a larger bitmap */
{
  FG_BITMAP *newbmp;
  FG_INT32 i;

  if (bmp) 
  {
    /* allocate the memory for the subbitmap structure */
    if ((newbmp=(FG_BITMAP *)calloc(1,sizeof(FG_BITMAP)+
	                                    height*sizeof(FG_VOID *))))
    {

      /* if we got it, set up the structure */    
      newbmp->lines=(FG_VOID **)((char *)newbmp+sizeof(FG_BITMAP));
      newbmp->struct_size=sizeof(FG_BITMAP);
      if ((x+width)<=(bmp->w)) {
        newbmp->w=width;
      } else {
        newbmp->w=(bmp->w-x);
      }
      if ((y+height)<=(bmp->h)) {
        newbmp->h=height;
      } else {
        newbmp->h=(bmp->h-y);
      }
      newbmp->type=FG_SUBBITMAP;
      newbmp->color_format=bmp->color_format;
      newbmp->pix_size=bmp->pix_size;
      newbmp->plot=bmp->plot;
      newbmp->hline=bmp->hline;
      newbmp->getpixel=bmp->getpixel;
      newbmp->vga_colors=bmp->vga_colors;
    
      /* set clipping */
      newbmp->clip=1; 
      newbmp->cl=0; newbmp->cr=newbmp->w-1; 
      newbmp->ct=0; newbmp->cb=newbmp->h-1;

      /* set pixel addresses */
      for (i=0;i<newbmp->h;i++) {
        newbmp->lines[i] = ((char *)bmp->lines[y+i])+x*newbmp->pix_size;
      }
    }  else newbmp=0;
  } else newbmp=0;
  return (newbmp);
}

inline void fg_plot_8(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  if (bmp->clip) {
    doit=((y>=bmp->ct) && (y<=bmp->cb) && (x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      ((unsigned char *)bmp->lines[y])[x]=COLOR_INDEX(color);
  }
}

inline void fg_plot_16(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  if (bmp->clip) {
    doit=((y>=bmp->ct) && (y<=bmp->cb) && (x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      ((unsigned short *)bmp->lines[y])[x]=color;
  }
}

inline void fg_plot_24(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  if (bmp->clip) {
    doit=((y>=bmp->ct) && (y<=bmp->cb) && (x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      ((PIX_RGB888 *)bmp->lines[y])[x].ival=color;
  }
}

inline void fg_plot_32(FG_INT32 x, FG_INT32 y, FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  if (bmp->clip) {
    doit=((y>=bmp->ct) && (y<=bmp->cb) && (x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      ((FG_UINT32 *)bmp->lines[y])[x]=color;
  }
}

inline FG_UINT32 fg_getpixel_8(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp)
{
  if (((unsigned)x < bmp->w) && ((unsigned)y < bmp->h)) {
    return ((FG_UINT32)((unsigned char *)bmp->lines[y])[x]);
  } else {
    return (0);
  }
}

inline FG_UINT32 fg_getpixel_16(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp)
{
  if (((unsigned)x < bmp->w) && ((unsigned)y < bmp->h)) {
    return ((FG_UINT32)((unsigned short *)bmp->lines[y])[x]);
  } else {
    return (0);
  }
}

inline FG_UINT32 fg_getpixel_24(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp)
{
  if (((unsigned)x < bmp->w) && ((unsigned)y < bmp->h)) {
    return (((PIX_RGB888 *)bmp->lines[y])[x]).ival;
  } else {
    return (0);
  }
}

inline FG_UINT32 fg_getpixel_32(FG_INT32 x, FG_INT32 y, FG_BITMAP *bmp)
{
  if (((unsigned)x < bmp->w) && ((unsigned)y < bmp->h)) {
    return ((FG_UINT32)((FG_UINT32 *)bmp->lines[y])[x]);
  } else {
    return (0);
  }
}

inline void fg_hline_8(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                  FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  if (bmp->clip) {
    x1=FG_MIN(FG_MAX(x1,bmp->cl),bmp->cr);
    x2=FG_MAX(FG_MIN(x2,bmp->cr),bmp->cl);
    doit=((y>=bmp->ct) && (y<=bmp->cb));
  } 
  if (doit) {
    memset(((char *)bmp->lines[y])+x1,COLOR_INDEX(color),x2-x1+1);
  }
}

inline void fg_hline_16(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                  FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  register FG_UINT16 *p=((FG_UINT16 *)bmp->lines[y])+x1;
  register FG_UINT16 *q=((FG_UINT16 *)bmp->lines[y])+x2;
  if (bmp->clip) {
    x1=FG_MIN(FG_MAX(x1,bmp->cl),bmp->cr);
    x2=FG_MAX(FG_MIN(x2,bmp->cr),bmp->cl);
    doit=((y>=bmp->ct) && (y<=bmp->cb));
  } 
  if (doit) {
    while (p<=q) *p++ =color;
  }
}

inline void fg_hline_24(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                  FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  register PIX_RGB888 *p=((PIX_RGB888 *)bmp->lines[y])+x1;
  register PIX_RGB888 *q=((PIX_RGB888 *)bmp->lines[y])+x2;
  if (bmp->clip) {
    x1=FG_MIN(FG_MAX(x1,bmp->cl),bmp->cr);
    x2=FG_MAX(FG_MIN(x2,bmp->cr),bmp->cl);
    doit=((y>=bmp->ct) && (y<=bmp->cb));
  } 
  if (doit) {
    while (p<=q) (p++)->ival =color;
  }
}

inline void fg_hline_32(FG_INT32 x1, FG_INT32 x2, FG_INT32 y, 
                  FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  register FG_UINT32 *p,*q;
  if (bmp->clip) {
    x1=FG_MIN(FG_MAX(x1,bmp->cl),bmp->cr);
    x2=FG_MAX(FG_MIN(x2,bmp->cr),bmp->cl);
    doit=((y>=bmp->ct) && (y<=bmp->cb));
  } 
  p=((FG_UINT32 *)bmp->lines[y])+x1;
  q=((FG_UINT32 *)bmp->lines[y])+x2;
  if (doit) {
    while (p<=q) *p++ =color;
  }
}

inline void fg_drawHLINE(FG_HLINE *hline, FG_INT32 y, FG_UINT32 color, 
              FG_BITMAP *bmp)
{
  bmp->hline(hline->start,hline->end,y,color,bmp);
}

inline void fg_vline(FG_INT32 x, FG_INT32 y1, FG_INT32 y2, 
                  FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 doit=1;
  if (bmp->clip) {
      y1=FG_MIN(FG_MAX(y1,bmp->ct),bmp->cb);
      y2=FG_MAX(FG_MIN(y2,bmp->cb),bmp->ct);
      doit=((x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      do {
        bmp->plot(x,y1++,color,bmp);
      } while (y1<=y2);
  }
}

inline void fg_linex(FG_INT32 x1, FG_INT32 y1, FG_INT32 deltax, FG_INT32 deltay,
                     FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 dx;
  register FG_INT32 dy;
  register FG_INT32 xstep;
  register FG_INT32 i;
  register FG_INT32 x;
  FG_INT32 x2;
  dx=deltax;
  dy=deltay;
  if (dy<0) {
    y1=y1+dy;
    x1=x1+dx;
    dy=-1*dy;
    dx=-1*dx;
  }
  x2=x1+dx;
  xstep=dx*2048/dy;
  x=x1*2048;
  if (xstep>0) {
    for (i=y1;i<(y1+dy);i++) {
      bmp->hline(x/2048,(x+xstep)/2048,i,color,bmp);
      x+=xstep;
    }
    bmp->hline(x/2048,x2,i,color,bmp);
  } else {
    for (i=y1;i<(y1+dy);i++) {
    bmp->hline((x+xstep)/2048,x/2048,i,color,bmp);
    x+=xstep;
  }
  bmp->hline(x2,x/2048,i,color,bmp);
  }
}

inline void fg_liney(FG_INT32 x1, FG_INT32 y1, FG_INT32 deltax, FG_INT32 deltay,
                     FG_UINT32 color, FG_BITMAP *bmp)
{
  register FG_INT32 dx;
  register FG_INT32 dy;
  register FG_INT32 ystep;
  register FG_INT32 y;
  register FG_INT32 i;
  FG_INT32 y2;

  dx=deltax;
  dy=deltay;
  if (dx<0) {
    x1=x1+dx;
    y1=y1+dy;
    dx=-1*dx;
    dy=-1*dy;
  }
  y2=y1+dy;
  ystep=dy*2048/dx;
  y=y1*2048;
  if (ystep>0) {
    for (i=x1;i<(x1+dx);i++) {
      fg_vline(i,y/2048,(y+ystep)/2048,color,bmp);
      y+=ystep;
    }
    fg_vline(i,y/2048,y2,color,bmp);
  } else {
    for (i=x1;i<(x1+dx);i++) {
      fg_vline(i,(y+ystep)/2048,y/2048,color,bmp);
      y+=ystep;
    }
    fg_vline(i,y2,y/2048,color,bmp);
  }
}

void fg_line(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, FG_UINT32 color, 
             FG_BITMAP *bmp)
{
  register FG_INT32 deltax, deltay;
  if (y1==y2) {
    bmp->hline(FG_MIN(x1,x2),FG_MAX(x1,x2),y1,color,bmp);
  } else if (x1==x2) {
    fg_vline(x1,FG_MIN(y1,y2),FG_MAX(y1,y2),color,bmp);
  } else {
    deltax=x2-x1;
    deltay=y2-y1;
    if (FG_ABS(deltax)>FG_ABS(deltay)) {
      fg_linex(x1,y1,deltax,deltay,color,bmp);
    } else {
      fg_liney(x1,y1,deltax,deltay,color,bmp);
    }
  }
}

inline FG_INT32 fg_reject_line(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, 
                     FG_BITMAP *bmp) 
{
  return ((FG_MIN((unsigned)x1,(unsigned)x2)>=bmp->w) 
           || (FG_MIN((unsigned)y1,(unsigned)y2)>=bmp->h));
}

inline FG_INT32 fg_line_in_bmp(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2,
                    FG_BITMAP *bmp)
{
  return (!fg_reject_line(x1, y1, x2, y2, bmp));
}

void fg_rect(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, 
                   FG_UINT32 color, FG_BITMAP *bmp)
{
  bmp->hline(x1, x2, y1, color, bmp);
  bmp->hline(x1, x2, y2, color, bmp);
  fg_vline(x1, y1+1, y2-1, color, bmp);
  fg_vline(x2, y1+1, y2-1, color, bmp);
}

void fg_RECT(FG_RECT *rect, FG_UINT32 color, FG_BITMAP *bmp)
{
  bmp->hline(rect->left,rect->right,rect->top, color, bmp);
  bmp->hline(rect->left,rect->right,rect->bottom, color, bmp);
  fg_vline(rect->left,(rect->top)+1,(rect->bottom)-1, color, bmp);
  fg_vline(rect->right,(rect->top)+1,(rect->bottom)-1, color, bmp);
}

void fg_fillrect(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, 
                   FG_UINT32 color, FG_BITMAP *bmp)
{
  do {
    bmp->hline(x1, x2, y1, color, bmp);
    y1++;
  } while (y1<=y2);
}

void fg_fillRECT(FG_RECT *rect, FG_UINT32 color, FG_BITMAP *bmp)
{
  FG_INT32 y1 = (rect->top);
  do {
    bmp->hline(rect->left,rect->right,y1,color, bmp);
    y1++;
  } while (y1<=(rect->bottom));
}

void fg_clearbmp(FG_UINT32 color, FG_BITMAP *bmp)
{
  FG_INT32 clip=bmp->clip;
  bmp->clip=0;
  fg_fillrect(0,0,bmp->w-1,bmp->h-1,color,bmp);
  bmp->clip=clip;
}

FG_POLYGON *fg_create_POLY(FG_INT32 num_vertices)
{
  FG_POLYGON *ply;
  if ((ply=(FG_POLYGON *)calloc(1,sizeof(FG_POLYGON)+
                                 num_vertices*sizeof(FG_I2VECT))))
  {
    ply->vertices=(FG_I2VECT *)(((char *)ply)+sizeof(FG_POLYGON));
    ply->num_vertices=num_vertices;
  }
  return (ply);
}

void fg_destroy_POLY(FG_POLYGON *ply)
{
  if (ply) free(ply);
}

void fg_RECT_to_POLY(FG_RECT *rect, FG_POLYGON **poly)
{
  register FG_I2VECT *vertices;

  if (!(*poly)) {
    *poly=fg_create_POLY(4);
  } else {
    if ((*poly)->num_vertices < 4) {
      fg_destroy_POLY(*poly);
      *poly=fg_create_POLY(4);
    } else {
      (*poly)->num_vertices=4;
    }
  }
 
  vertices=(*poly)->vertices;
  vertices[0].x=vertices[3].x=rect->left;
  vertices[1].x=vertices[2].x=rect->right;
  vertices[0].y=vertices[1].y=rect->bottom;
  vertices[2].y=vertices[3].y=rect->top;
}

inline void fg_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy)
{
  FG_INT32 num_vertices=original->num_vertices;
  if (!(*copy)) {
    *copy=fg_create_POLY(num_vertices);
  } else {
    if ((*copy)->num_vertices < num_vertices) {
      fg_destroy_POLY(*copy);
      *copy=fg_create_POLY(num_vertices);
    } else {
      (*copy)->num_vertices=num_vertices;
    }
  }

  memcpy((*copy)->vertices,original->vertices,num_vertices*sizeof(FG_I2VECT));
}    

inline void fg_scale_POLY(FG_POLYGON *poly, FG_INT32 x, FG_INT32 y, FG_INT32 scale) 
{
  FG_INT32 num_vertices=poly->num_vertices;
  register FG_INT32 i=0;
  register FG_I2VECT *vertices=poly->vertices;

  do {
    vertices->x=(vertices->x-x)*scale/(SHRT_MAX+1)+x;
    vertices->y=(vertices->y-y)*scale/(SHRT_MAX+1)+y;
    vertices++;
    i++;
  } while (i < num_vertices);
}  

void fg_scale_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT32 x,
                        FG_INT32 y, FG_INT32 scale)
{
  fg_copy_POLY(original,copy);
  fg_scale_POLY(*copy,x,y,scale);
}

inline void fg_translate_POLY(FG_POLYGON *poly, FG_INT32 dx, FG_INT32 dy)
{
  FG_INT32 num_vertices=poly->num_vertices;
  register FG_INT32 i=0;
  register FG_I2VECT *vertices=poly->vertices;

  do {
    vertices->x+=dx;
    vertices->y+=dy;
    vertices++;
    i++;
  } while (i<num_vertices);
}

void fg_translate_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT32 dx,
                            FG_INT32 dy)
{
  fg_copy_POLY(original,copy);
  fg_translate_POLY(*copy,dx,dy);
}



void fg_rotate_POLY(FG_POLYGON *poly, FG_INT32 x, FG_INT32 y, FG_INT32 angle)
{
  FG_INT32 num_vertices=poly->num_vertices;
  register FG_INT32 i=0;
  register FG_I2VECT *vertices=poly->vertices;

  do {
    vertices->x-=x;
    vertices->y-=y;
    i2rot(vertices,vertices,angle);
    vertices->x+=x;
    vertices->y+=y;
    vertices++;
    i++;
  } while (i<num_vertices);
}

void fg_rotate_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT32 x,
                         FG_INT32 y, FG_INT32 angle)
{
  fg_copy_POLY(original,copy);
  fg_rotate_POLY(*copy,x,y,angle);
}


void fg_draw_POLY(FG_POLYGON *poly, FG_UINT32 color, FG_BITMAP *bmp)
{
  FG_INT32 num_vertices=poly->num_vertices;
  register FG_I2VECT *vertices=poly->vertices;
  FG_INT32 xo=vertices->x,yo=vertices->y;
  register FG_INT32 i=1;

  while ((i++)<num_vertices) {
    vertices++; 
    fg_line(xo,yo,vertices->x,vertices->y,color,bmp);
    xo=vertices->x;
    yo=vertices->y;
  }  

  fg_line(xo,yo,poly->vertices->x,poly->vertices->y,color,bmp);
}

FG_RASTER_SET *fg_create_RASTER(FG_INT32 nlines)
{
  FG_RASTER_SET *rv=calloc(1,sizeof(FG_RASTER_SET)+nlines*sizeof(FG_HLINE));
  if (rv) {
    rv->num_lines=nlines;
    rv->hlines=(FG_HLINE *)(((char *)rv)+sizeof(FG_RASTER_SET));
    return (rv);
  } else {
    return (NULL);
  }
}

void fg_destroy_RASTER(FG_RASTER_SET *raster)
{
  if (raster) free(raster);
}

void fg_display_RASTER(FG_INT32 y,FG_RASTER_SET *raster, FG_BITMAP *bmp)
{
  FG_INT32 i,j=y+raster->num_lines;
  FG_UINT32 color=raster->color;
  FG_HLINE *hline=raster->hlines;

  for (i=y; i<j ; i++) {
    fg_drawHLINE(hline, i, color, bmp);
    hline++;
  }
}

void fg_rasterize_edge(FG_INT32 x1, FG_INT32 y1, FG_INT32 x2, FG_INT32 y2, 
                 FG_HLINE **hlines, FG_INT32 setstart)
{
  FG_INT32 nlines;
  FG_INT32 deltax=x2-x1;
  FG_INT32 deltay=y2-y1;

/* make sure the memory is allocated */  
  nlines=(y2-y1)+1;
  if (!(*hlines)) *hlines=(FG_HLINE *)calloc(nlines,sizeof(FG_HLINE));

/* check for horizontal lines */
  if (y2==y1) {
    if (setstart) {
      (*hlines)->start=FG_MIN(x1,x2)+1;  /* skip the left pixel so our  */
                                         /* polygons fit together       */
                                         /* without breaks or overlaps  */
    } else {
      (*hlines)->end=FG_MAX(x1,x2);
    }
    return;
  }

  if (hlines) {
    FG_HLINE *hlinep=*hlines; 
    register FG_INT32 dx, dy, xstep, i, x;
    dx=deltax;
    dy=deltay;
    if (dy<0) {
      y1=y1+dy;
      i=x1;
      x1=x2;
      x2=i;
      dy=-1*dy;
      dx=-1*dx;
    }

	 xstep=dx*2048/dy;  /* Just say no to floating point */
    x=x1*2048;

    if (xstep>0) {
      for (i=y1;i<(y1+dy);i++) {
        if (setstart) {
          hlinep->start=x/2048+1;  /* remember to skip that left pixel */
        } else {
          hlinep->end=(x+xstep)/2048;
        }
        hlinep++;
        x+=xstep;
      }
      if (setstart) {
        hlinep->start=x/2048+1;
      } else {
		  hlinep->end=x2;
      }
    } else {
      for (i=y1;i<(y1+dy);i++) {
       if (setstart) {
         hlinep->start=(x+xstep)/2048+1; /* remember to skip that left pixel */
       } else {
         hlinep->end=x/2048;
       }
       hlinep++;
       x+=xstep;
      }
      if (setstart) {
        hlinep->start=x2+1;
      } else {
        hlinep->end=x/2048;
		}
    }
  }
}

/*

#define NEXT_VERTEX(n) (((n) + 1) % num_vertices)
#define PREV_VERTEX(n) (((n) - 1 + num_vertices) % num_vertices)

void fg_rasterize_convex_POLY(FG_POLYGON *poly, FG_RASTER_SET **raster, 
FG_UINT32 color)
{
  FG_INT32 min_y=poly->vertices[0].y,max_y=poly->vertices[0].y
  FG_INT32 min_y_index=0,max_y_index=0;
  FG_INT32 min_x=poly->vertices[0].x,max_x=poly->vertices[0].x
  FG_INT32 min_x,max_x,min_x_index,max_x_index;
  FG_INT32 num_vertices=poly->num_vertices;
  FG_INT32 prev_index,next_index;
  FG_INT32 nlines;
  FG_INT32 i;


  if (num_vertices>2) {
*/
    /* find the top and bottom of the polygon */ /*
    for (i=1;i<num_vertices;i++) {
      if (poly->vertices[i].y>max_y) {
        max_y=poly->vertices[i].y;
        max_y_index=i;
      }
      if (poly->vertices[i].y>min_y) {
		  min_y=poly->vertices[i].y;
        min_y_index=i;
      }
    }

    nlines=max_y-min_y+1;

    if (nlines !=1) {

      */ /* make sure our raster set is present and big enough */ /*
      if (!(*raster)) {
        *raster=fg_create_RASTER(nlines);
      } else {
        if (*raster->num_lines < nlines ) {
          fg_destroy_RASTER(*raster);
          *raster=fg_create_RASTER(nlines);
		  } else {
          *raster->num_lines=nlines;
        }
      }

      *raster->color=color;
  
      */ /* find out whether we go CW or CCW around this polygon */ /*
      do {
        prev_index=PREV_INDEX(min_y_index);
      } while ((min_y == poly->vertices[prev_index].y) && 
               (prev_index != max_y_index));
      next_index=NEXT_INDEX(min_y_index);
  
      if (prev_index == next_index) prev_index = NEXT_INDEX(prev_index);
        */ /* damn flat topped triangles */ /*

      if (poly->vertices[next_index].x > poly->vertices[prev_index].x) {
        direction=1; 
      } else {
        if (poly->vertices[next_index].x == poly->vertices[prev_index.x) {
          if (poly->vertices[next_index].y > poly->vertices[prev_index].y) {
            direction=1;
          }
        } else {
          direction=-1;
        }
      }

*/

void fggrx_export()
{
}
  

/* $Log: fggrx.c,v $
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
 * Revision 1.6  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.5  1996/05/06  20:20:38  korpela
 * Added some polygon functions.
 *
 * Revision 1.4  1996/05/05  22:01:26  korpela
 * minor bug fixes
 * */
