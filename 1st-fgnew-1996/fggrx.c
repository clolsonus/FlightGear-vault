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
 * $Id: fggrx.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
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

FG_BITMAP *fg_create_bitmap(FG_UINT width, FG_UINT height)
{
  FG_BITMAP *bmp;
  int i;

  /* allocate the memory for the bitmap */
  if ((bmp=(FG_BITMAP *)calloc(1,sizeof(FG_BITMAP))) && 
      (bmp->lines=(FG_UINT8 **)calloc(height,sizeof(FG_UINT8 *))) &&
      (bmp->lines[0]=(FG_UINT8 *)calloc(width,height)))  {
 
    /* if we got it, set up the structure */    
    bmp->struct_size=sizeof(FG_BITMAP);
    bmp->w=width;
    bmp->h=height;

    /* set clipping */
    bmp->clip=1; 
    bmp->cl=0; bmp->cr=width-1; 
    bmp->ct=0; bmp->cb=height-1;

    /* set pixel addresses */
    for (i=1;i<height;i++) {
      bmp->lines[i] = (bmp->lines[i-1])+width;
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
  if (bmp->lines) {
    free(bmp->lines);
    bmp->lines=NULL;
  }
  if (bmp) {
    free(bmp);
  }
}
    
FG_BITMAP *fg_create_subbitmap(FG_BITMAP *bmp, FG_INT x, FG_INT y, 
                                 FG_INT width, FG_INT height)
/* creates a new bitmap that is a rectangular subsection of a larger bitmap */
{
  FG_BITMAP *newbmp;
  int i;

  if (bmp) {
    /* allocate the memory for the subbitmap structure */
    if ((newbmp=(FG_BITMAP *)calloc(1,sizeof(FG_BITMAP))) && 
        (newbmp->lines=(FG_UINT8 **)calloc(height,sizeof(FG_UINT8 *)))) {

      /* if we got it, set up the structure */    
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
    
      /* set clipping */
      newbmp->clip=1; 
      newbmp->cl=0; newbmp->cr=newbmp->w-1; 
      newbmp->ct=0; newbmp->cb=newbmp->h-1;

      /* set pixel addresses */
      for (i=0;i<newbmp->h;i++) {
        newbmp->lines[i] = bmp->lines[y+i]+x;
      }
    }  else newbmp=0;
  } else newbmp=0;
  return (newbmp);
}

inline void fg_plot(FG_INT x, FG_INT y, FG_INT color, FG_BITMAP *bmp)
{
  register int doit=1;
  if (bmp->clip) {
    doit=((y>=bmp->ct) && (y<=bmp->cb) && (x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      bmp->lines[y][x]=COLOR_INDEX(color);
  }
}

inline FG_INT fg_getpixel(FG_INT x, FG_INT y, FG_BITMAP *bmp)
{
  if (((unsigned)x < bmp->w) && ((unsigned)y < bmp->h)) {
    return ((FG_INT)bmp->lines[y][x]);
  } else {
    return (0);
  }
}

inline void fg_hline(FG_INT x1, FG_INT x2, FG_INT y, 
                  FG_INT color, FG_BITMAP *bmp)
{
  register int doit=1;
  if (bmp->clip) {
    x1=FG_MIN(FG_MAX(x1,bmp->cl),bmp->cr);
    x2=FG_MAX(FG_MIN(x2,bmp->cr),bmp->cl);
    doit=((y>=bmp->ct) && (y<=bmp->cb));
  } 
  if (doit) {
    memset(bmp->lines[y]+x1,COLOR_INDEX(color),x2-x1+1);
  }
}

inline void fg_drawHLINE(FG_HLINE *hline, FG_INT y, FG_INT color, 
              FG_BITMAP *bmp)
{
  fg_hline(hline->start,hline->end,y,color,bmp);
}

inline void fg_vline(FG_INT x, FG_INT y1, FG_INT y2, 
                  FG_UINT color, FG_BITMAP *bmp)
{
  register int doit=1;
  if (bmp->clip) {
      y1=FG_MIN(FG_MAX(y1,bmp->ct),bmp->cb);
      y2=FG_MAX(FG_MIN(y2,bmp->cb),bmp->ct);
      doit=((x<=bmp->cr) && (x>=bmp->cl)); 
  } 
  if (doit) {
      do {
        bmp->lines[y1++][x]=COLOR_INDEX(color);
      } while (y1<=y2);
  }
}

inline void fg_linex(FG_INT x1, FG_INT y1, FG_INT deltax, FG_INT deltay,
                     FG_INT color, FG_BITMAP *bmp)
{
  register int dx;
  register int dy;
  register int xstep;
  register int i;
  register int x;
  int x2;
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
      fg_hline(x/2048,(x+xstep)/2048,i,color,bmp);
      x+=xstep;
    }
    fg_hline(x/2048,x2,i,color,bmp);
  } else {
    for (i=y1;i<(y1+dy);i++) {
    fg_hline((x+xstep)/2048,x/2048,i,color,bmp);
    x+=xstep;
  }
  fg_hline(x2,x/2048,i,color,bmp);
  }
}

inline void fg_liney(FG_INT x1, FG_INT y1, FG_INT deltax, FG_INT deltay,
                     FG_INT color, FG_BITMAP *bmp)
{
  register int dx;
  register int dy;
  register int ystep;
  register int y;
  register int i;
  int y2;

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

void fg_line(FG_INT x1, FG_INT y1, FG_INT x2, FG_INT y2, FG_INT color, 
             FG_BITMAP *bmp)
{
  register int deltax, deltay;
  if (y1==y2) {
    fg_hline(FG_MIN(x1,x2),FG_MAX(x1,x2),y1,color,bmp);
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

inline int fg_reject_line(FG_INT x1, FG_INT y1, FG_INT x2, FG_INT y2, 
                     FG_BITMAP *bmp) 
{
  return ((FG_MIN((unsigned)x1,(unsigned)x2)>=bmp->w) 
           || (FG_MIN((unsigned)y1,(unsigned)y2)>=bmp->h));
}

inline int fg_line_in_bmp(FG_INT x1, FG_INT y1, FG_INT x2, FG_INT y2,
                    FG_BITMAP *bmp)
{
  return (!fg_reject_line(x1, y1, x2, y2, bmp));
}

void fg_rect(FG_INT x1, FG_INT y1, FG_INT x2, FG_INT y2, 
                   FG_INT color, FG_BITMAP *bmp)
{
  fg_hline(x1, x2, y1, color, bmp);
  fg_hline(x1, x2, y2, color, bmp);
  fg_vline(x1, y1+1, y2-1, color, bmp);
  fg_vline(x2, y1+1, y2-1, color, bmp);
}

void fg_RECT(FG_RECT *rect, FG_INT color, FG_BITMAP *bmp)
{
  fg_hline(rect->left,rect->right,rect->top, color, bmp);
  fg_hline(rect->left,rect->right,rect->bottom, color, bmp);
  fg_vline(rect->left,(rect->top)+1,(rect->bottom)-1, color, bmp);
  fg_vline(rect->right,(rect->top)+1,(rect->bottom)-1, color, bmp);
}

void fg_fillrect(FG_INT x1, FG_INT y1, FG_INT x2, FG_INT y2, 
                   FG_INT color, FG_BITMAP *bmp)
{
  do {
    fg_hline(x1, x2, y1, color, bmp);
    y1++;
  } while (y1<=y2);
}

void fg_fillRECT(FG_RECT *rect, FG_INT color, FG_BITMAP *bmp)
{
  int y1 = (rect->top);
  do {
    fg_hline(rect->left,rect->right,y1,color, bmp);
    y1++;
  } while (y1<=(rect->bottom));
}

void fg_clearbmp(FG_INT color, FG_BITMAP *bmp)
{
  int clip=bmp->clip;
  bmp->clip=0;
  fg_fillrect(0,0,bmp->w-1,bmp->h-1,color,bmp);
  bmp->clip=clip;
}

FG_POLYGON *fg_create_POLY(FG_INT num_vertices)
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
  int num_vertices=original->num_vertices;
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

inline void fg_scale_POLY(FG_POLYGON *poly, FG_INT x, FG_INT y, FG_INT32 scale) 
{
  int num_vertices=poly->num_vertices;
  register int i=0;
  register FG_I2VECT *vertices=poly->vertices;

  do {
    vertices->x=(vertices->x-x)*scale/(SHRT_MAX+1)+x;
    vertices->y=(vertices->y-y)*scale/(SHRT_MAX+1)+y;
    vertices++;
    i++;
  } while (i < num_vertices);
}  

void fg_scale_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT x,
                        FG_INT y, FG_INT32 scale)
{
  fg_copy_POLY(original,copy);
  fg_scale_POLY(*copy,x,y,scale);
}

inline void fg_translate_POLY(FG_POLYGON *poly, FG_INT dx, FG_INT dy)
{
  int num_vertices=poly->num_vertices;
  register int i=0;
  register FG_I2VECT *vertices=poly->vertices;

  do {
    vertices->x+=dx;
    vertices->y+=dy;
    vertices++;
    i++;
  } while (i<num_vertices);
}

void fg_translate_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT dx,
                            FG_INT dy)
{
  fg_copy_POLY(original,copy);
  fg_translate_POLY(*copy,dx,dy);
}



void fg_rotate_POLY(FG_POLYGON *poly, FG_INT x, FG_INT y, FG_INT32 angle)
{
  int num_vertices=poly->num_vertices;
  register int i=0;
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

void fg_rotate_copy_POLY(FG_POLYGON *original, FG_POLYGON **copy, FG_INT x,
                         FG_INT y, FG_INT32 angle)
{
  fg_copy_POLY(original,copy);
  fg_rotate_POLY(*copy,x,y,angle);
}


void fg_draw_POLY(FG_POLYGON *poly, FG_INT color, FG_BITMAP *bmp)
{
  int num_vertices=poly->num_vertices;
  register FG_I2VECT *vertices=poly->vertices;
  int xo=vertices->x,yo=vertices->y;
  register int i=1;

  while ((i++)<num_vertices) {
    vertices++; 
    fg_line(xo,yo,vertices->x,vertices->y,color,bmp);
    xo=vertices->x;
    yo=vertices->y;
  }  

  fg_line(xo,yo,poly->vertices->x,poly->vertices->y,color,bmp);
}

FG_RASTER_SET *fg_create_RASTER(FG_INT nlines)
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

void fg_display_RASTER(FG_INT y,FG_RASTER_SET *raster, FG_BITMAP *bmp)
{
  FG_INT i,j=y+raster->num_lines;
  FG_INT color=raster->color;
  FG_HLINE *hline=raster->hlines;

  for (i=y; i<j ; i++) {
    fg_drawHLINE(hline, i, color, bmp);
    hline++;
  }
}

void fg_rasterize_edge(FG_INT x1, FG_INT y1, FG_INT x2, FG_INT y2, 
                 FG_HLINE **hlines, FG_INT setstart)
{
  int nlines;
  int deltax=x2-x1;
  int deltay=y2-y1;

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
    register int dx, dy, xstep, i, x;
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
                              FG_INT color)
{
  int min_y=poly->vertices[0].y,max_y=poly->vertices[0].y
  int min_y_index=0,max_y_index=0;
  int min_x=poly->vertices[0].x,max_x=poly->vertices[0].x
  int min_x,max_x,min_x_index,max_x_index;
  int num_vertices=poly->num_vertices;
  int prev_index,next_index;
  int nlines;
  int i;


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
