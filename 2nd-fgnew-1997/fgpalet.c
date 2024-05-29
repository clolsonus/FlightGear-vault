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
 * $Id: fgpalet.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgpalet.h"
#include "fgscreen.h"

#if defined(USE_PM)
HPAL fg_hpal;
HDC  fg_hdc;
HPS  fg_hps;

#elif defined(USE_WIN32)
HPALETTE   fg_hpalette;
PLOGPALETTE fg_logpalette;
HBITMAP    fg_hbitmap;
HDC        fg_memdc;

#elif defined(USE_X11)
FG_UINT8 Xpal[NUM_COLORS];

#elif defined(USE_FB)
#include <sun/fbio.h>
#include <sys/ioctl.h>
unsigned char red[NUM_COLORS],green[NUM_COLORS],blue[NUM_COLORS];
struct fbcmap fb_cmap={0,NUM_COLORS,red,green,blue};

#endif

FGPAL_ENTRY StartupPalette[NUM_COLORS]={
  PALETTE_ENTRY(  0,  0,  0,PAL_REQUIRED),
  PALETTE_ENTRY(  0,  0,171,PAL_REQUIRED),
  PALETTE_ENTRY(  0,171,  0,PAL_REQUIRED), 
  PALETTE_ENTRY(  0,171,171,PAL_REQUIRED),
  PALETTE_ENTRY(171,  0,  0,PAL_REQUIRED),
  PALETTE_ENTRY(171,  0,171,PAL_REQUIRED),
  PALETTE_ENTRY(171, 85,  0,PAL_REQUIRED),
  PALETTE_ENTRY(171,171,171,PAL_REQUIRED),
  PALETTE_ENTRY( 85, 85, 85,PAL_REQUIRED),
  PALETTE_ENTRY( 85, 85,255,PAL_REQUIRED),
  PALETTE_ENTRY( 85,255, 85,PAL_REQUIRED),
  PALETTE_ENTRY( 85,255,255,PAL_REQUIRED),
  PALETTE_ENTRY(255, 85, 85,PAL_REQUIRED),
  PALETTE_ENTRY(255, 85,255,PAL_REQUIRED),
  PALETTE_ENTRY(255,255, 85,PAL_REQUIRED),
  PALETTE_ENTRY(255,255,255,PAL_REQUIRED)
};

PIX_GREY8 vga_grey8[16]={
  {GREYSCALE_ENTRY(  0,  0,  0)},
  {GREYSCALE_ENTRY(  0,  0,171)},
  {GREYSCALE_ENTRY(  0,171,  0)},
  {GREYSCALE_ENTRY(  0,171,171)},
  {GREYSCALE_ENTRY(171,  0,  0)},
  {GREYSCALE_ENTRY(171,  0,171)},
  {GREYSCALE_ENTRY(171, 85,  0)},
  {GREYSCALE_ENTRY(171,171,171)},
  {GREYSCALE_ENTRY( 85, 85, 85)},
  {GREYSCALE_ENTRY( 85, 85,255)},
  {GREYSCALE_ENTRY( 85,255, 85)},
  {GREYSCALE_ENTRY( 85,255,255)},
  {GREYSCALE_ENTRY(255, 85, 85)},
  {GREYSCALE_ENTRY(255, 85,255)},
  {GREYSCALE_ENTRY(255,255, 85)},
  {GREYSCALE_ENTRY(255,255,255)}
};

PIX_LUT8 vga_lut8[16]={{0},{1},{2},{3},{4},{5},{6},{7},
                       {8},{9},{10},{11},{12},{13},{14},{15}};

PIX_RGB565 vga_rgb565[16]={
  {RGB565_ENTRY(  0,  0,  0)},
  {RGB565_ENTRY(  0,  0,171)},
  {RGB565_ENTRY(  0,171,  0)},
  {RGB565_ENTRY(  0,171,171)},
  {RGB565_ENTRY(171,  0,  0)},
  {RGB565_ENTRY(171,  0,171)},
  {RGB565_ENTRY(171, 85,  0)},
  {RGB565_ENTRY(171,171,171)},
  {RGB565_ENTRY( 85, 85, 85)},
  {RGB565_ENTRY( 85, 85,255)},
  {RGB565_ENTRY( 85,255, 85)},
  {RGB565_ENTRY( 85,255,255)},
  {RGB565_ENTRY(255, 85, 85)},
  {RGB565_ENTRY(255, 85,255)},
  {RGB565_ENTRY(255,255, 85)},
  {RGB565_ENTRY(255,255,255)}
};

PIX_BGR565 vga_bgr565[16]={
  {BGR565_ENTRY(  0,  0,  0)},
  {BGR565_ENTRY(  0,  0,171)},
  {BGR565_ENTRY(  0,171,  0)},
  {BGR565_ENTRY(  0,171,171)},
  {BGR565_ENTRY(171,  0,  0)},
  {BGR565_ENTRY(171,  0,171)},
  {BGR565_ENTRY(171, 85,  0)},
  {BGR565_ENTRY(171,171,171)},
  {BGR565_ENTRY( 85, 85, 85)},
  {BGR565_ENTRY( 85, 85,255)},
  {BGR565_ENTRY( 85,255, 85)},
  {BGR565_ENTRY( 85,255,255)},
  {BGR565_ENTRY(255, 85, 85)},
  {BGR565_ENTRY(255, 85,255)},
  {BGR565_ENTRY(255,255, 85)},
  {BGR565_ENTRY(255,255,255)}
};

PIX_RGB555 vga_rgb555[16]={
  {RGB555_ENTRY(  0,  0,  0)},
  {RGB555_ENTRY(  0,  0,171)},
  {RGB555_ENTRY(  0,171,  0)},
  {RGB555_ENTRY(  0,171,171)},
  {RGB555_ENTRY(171,  0,  0)},
  {RGB555_ENTRY(171,  0,171)},
  {RGB555_ENTRY(171, 85,  0)},
  {RGB555_ENTRY(171,171,171)},
  {RGB555_ENTRY( 85, 85, 85)},
  {RGB555_ENTRY( 85, 85,255)},
  {RGB555_ENTRY( 85,255, 85)},
  {RGB555_ENTRY( 85,255,255)},
  {RGB555_ENTRY(255, 85, 85)},
  {RGB555_ENTRY(255, 85,255)},
  {RGB555_ENTRY(255,255, 85)},
  {RGB555_ENTRY(255,255,255)}
};

PIX_BGR555 vga_bgr555[16]={
  {BGR555_ENTRY(  0,  0,  0)},
  {BGR555_ENTRY(  0,  0,171)},
  {BGR555_ENTRY(  0,171,  0)},
  {BGR555_ENTRY(  0,171,171)},
  {BGR555_ENTRY(171,  0,  0)},
  {BGR555_ENTRY(171,  0,171)},
  {BGR555_ENTRY(171, 85,  0)},
  {BGR555_ENTRY(171,171,171)},
  {BGR555_ENTRY( 85, 85, 85)},
  {BGR555_ENTRY( 85, 85,255)},
  {BGR555_ENTRY( 85,255, 85)},
  {BGR555_ENTRY( 85,255,255)},
  {BGR555_ENTRY(255, 85, 85)},
  {BGR555_ENTRY(255, 85,255)},
  {BGR555_ENTRY(255,255, 85)},
  {BGR555_ENTRY(255,255,255)}
};

PIX_RGB664 vga_rgb664[16]={
  {RGB664_ENTRY(  0,  0,  0)},
  {RGB664_ENTRY(  0,  0,171)},
  {RGB664_ENTRY(  0,171,  0)},
  {RGB664_ENTRY(  0,171,171)},
  {RGB664_ENTRY(171,  0,  0)},
  {RGB664_ENTRY(171,  0,171)},
  {RGB664_ENTRY(171, 85,  0)},
  {RGB664_ENTRY(171,171,171)},
  {RGB664_ENTRY( 85, 85, 85)},
  {RGB664_ENTRY( 85, 85,255)},
  {RGB664_ENTRY( 85,255, 85)},
  {RGB664_ENTRY( 85,255,255)},
  {RGB664_ENTRY(255, 85, 85)},
  {RGB664_ENTRY(255, 85,255)},
  {RGB664_ENTRY(255,255, 85)},
  {RGB664_ENTRY(255,255,255)}
};

PIX_BGR466 vga_bgr466[16]={
  {BGR466_ENTRY(  0,  0,  0)},
  {BGR466_ENTRY(  0,  0,171)},
  {BGR466_ENTRY(  0,171,  0)},
  {BGR466_ENTRY(  0,171,171)},
  {BGR466_ENTRY(171,  0,  0)},
  {BGR466_ENTRY(171,  0,171)},
  {BGR466_ENTRY(171, 85,  0)},
  {BGR466_ENTRY(171,171,171)},
  {BGR466_ENTRY( 85, 85, 85)},
  {BGR466_ENTRY( 85, 85,255)},
  {BGR466_ENTRY( 85,255, 85)},
  {BGR466_ENTRY( 85,255,255)},
  {BGR466_ENTRY(255, 85, 85)},
  {BGR466_ENTRY(255, 85,255)},
  {BGR466_ENTRY(255,255, 85)},
  {BGR466_ENTRY(255,255,255)}
};

PIX_RGB888 vga_rgb888[16]={
  {RGB888_ENTRY(  0,  0,  0)},
  {RGB888_ENTRY(  0,  0,171)},
  {RGB888_ENTRY(  0,171,  0)},
  {RGB888_ENTRY(  0,171,171)},
  {RGB888_ENTRY(171,  0,  0)},
  {RGB888_ENTRY(171,  0,171)},
  {RGB888_ENTRY(171, 85,  0)},
  {RGB888_ENTRY(171,171,171)},
  {RGB888_ENTRY( 85, 85, 85)},
  {RGB888_ENTRY( 85, 85,255)},
  {RGB888_ENTRY( 85,255, 85)},
  {RGB888_ENTRY( 85,255,255)},
  {RGB888_ENTRY(255, 85, 85)},
  {RGB888_ENTRY(255, 85,255)},
  {RGB888_ENTRY(255,255, 85)},
  {RGB888_ENTRY(255,255,255)}
};

PIX_BGR888 vga_bgr888[16]={
  {BGR888_ENTRY(  0,  0,  0)},
  {BGR888_ENTRY(  0,  0,171)},
  {BGR888_ENTRY(  0,171,  0)},
  {BGR888_ENTRY(  0,171,171)},
  {BGR888_ENTRY(171,  0,  0)},
  {BGR888_ENTRY(171,  0,171)},
  {BGR888_ENTRY(171, 85,  0)},
  {BGR888_ENTRY(171,171,171)},
  {BGR888_ENTRY( 85, 85, 85)},
  {BGR888_ENTRY( 85, 85,255)},
  {BGR888_ENTRY( 85,255, 85)},
  {BGR888_ENTRY( 85,255,255)},
  {BGR888_ENTRY(255, 85, 85)},
  {BGR888_ENTRY(255, 85,255)},
  {BGR888_ENTRY(255,255, 85)},
  {BGR888_ENTRY(255,255,255)}
};

PIX_RGBX8888 vga_rgbx8888[16]={
  {RGBX8888_ENTRY(  0,  0,  0)},
  {RGBX8888_ENTRY(  0,  0,171)},
  {RGBX8888_ENTRY(  0,171,  0)},
  {RGBX8888_ENTRY(  0,171,171)},
  {RGBX8888_ENTRY(171,  0,  0)},
  {RGBX8888_ENTRY(171,  0,171)},
  {RGBX8888_ENTRY(171, 85,  0)},
  {RGBX8888_ENTRY(171,171,171)},
  {RGBX8888_ENTRY( 85, 85, 85)},
  {RGBX8888_ENTRY( 85, 85,255)},
  {RGBX8888_ENTRY( 85,255, 85)},
  {RGBX8888_ENTRY( 85,255,255)},
  {RGBX8888_ENTRY(255, 85, 85)},
  {RGBX8888_ENTRY(255, 85,255)},
  {RGBX8888_ENTRY(255,255, 85)},
  {RGBX8888_ENTRY(255,255,255)}
};

PIX_XBGR8888 vga_xbgr8888[16]={
  {XBGR8888_ENTRY(  0,  0,  0)},
  {XBGR8888_ENTRY(  0,  0,171)},
  {XBGR8888_ENTRY(  0,171,  0)},
  {XBGR8888_ENTRY(  0,171,171)},
  {XBGR8888_ENTRY(171,  0,  0)},
  {XBGR8888_ENTRY(171,  0,171)},
  {XBGR8888_ENTRY(171, 85,  0)},
  {XBGR8888_ENTRY(171,171,171)},
  {XBGR8888_ENTRY( 85, 85, 85)},
  {XBGR8888_ENTRY( 85, 85,255)},
  {XBGR8888_ENTRY( 85,255, 85)},
  {XBGR8888_ENTRY( 85,255,255)},
  {XBGR8888_ENTRY(255, 85, 85)},
  {XBGR8888_ENTRY(255, 85,255)},
  {XBGR8888_ENTRY(255,255, 85)},
  {XBGR8888_ENTRY(255,255,255)}
};

FG_INT32 PaletteReferenceCount[NUM_COLORS]={INT_MAX/2,INT_MAX/2,INT_MAX/2,INT_MAX/2,
													INT_MAX/2,INT_MAX/2,INT_MAX/2,INT_MAX/2,
													INT_MAX/2,INT_MAX/2,INT_MAX/2,INT_MAX/2,
													INT_MAX/2,INT_MAX/2,INT_MAX/2,INT_MAX/2};

#ifndef USE_WIN32
FGPAL_ENTRY PaletteArray[NUM_COLORS];
#else
FGPAL_ENTRY *PaletteArray;
#endif

#if !(defined(USE_PM) || defined(USE_WIN32))
void fg_set_startup_palette()
#else
void fg_set_startup_palette(HWND hwnd)
#endif
{
  #if defined(USE_WIN32)
  {
	 FG_INT32 i;
	 HDC hdc=GetDC(hwnd);
	 fg_logpalette=(LOGPALETTE *)calloc(1,sizeof(LOGPALETTE)
                                           +(NUM_COLORS)*sizeof(FGPAL_ENTRY));
	 PaletteArray=(FGPAL_ENTRY *)((char *)fg_logpalette+sizeof(LOGPALETTE));
	 for (i=0;i<16;i++) {
		fg_logpalette->palPalEntry[i].peRed=StartupPalette[i].r;
		fg_logpalette->palPalEntry[i].peGreen=StartupPalette[i].g;
		fg_logpalette->palPalEntry[i].peBlue=StartupPalette[i].b;
		fg_logpalette->palPalEntry[i].peFlags=PC_RESERVED;
	 }
	 fg_logpalette->palVersion=0x300;
	 fg_logpalette->palNumEntries=NUM_COLORS;
	 fg_hpalette=CreatePalette(fg_logpalette);
	 SelectPalette(hdc,fg_hpalette,FALSE);
	 RealizePalette(hdc);
	 ReleaseDC(hwnd,hdc);
  }
  #else
	 memmove(PaletteArray,StartupPalette,sizeof(StartupPalette));

	 #if defined(USE_FB)
	 {
		FG_INT32 i;
		ioctl(fb_fd,FBIOGETCMAP,&fb_cmap);
		for (i=0;i<NUM_COLORS;i++) {
		  fb_cmap.red[i]=PaletteArray[i].r;
		  fb_cmap.green[i]=PaletteArray[i].g;
		  fb_cmap.blue[i]=PaletteArray[i].b;
		}
		ioctl(fb_fd,FBIOPUTCMAP,&fb_cmap);
	 }


	 #elif defined(USE_ALLEGRO)
	 set_pallete(PaletteArray);

	 #elif defined(USE_PM)
	 {
		FG_INT32 ncolors=NUM_COLORS;
		static SIZEL sizel={0,0};
		if (!fg_hdc) fg_hdc=WinOpenWindowDC(hwnd);
		if (!fg_hps) fg_hps=GpiCreatePS(hab,fg_hdc,&sizel,
									  PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC );
		if ((fg_hpal=GpiCreatePalette(fg_hab,LCOL_PURECOLOR,LCOLF_CONSECRGB,ncolors,
												PaletteArray)))
		{
		  WinPostMsg(hwnd,WM_COMMAND,MPFROM2SHORT(CMD_PALETTE_CHANGE,0),0);
		}
	 }

	 #elif defined(USE_X11)
	 {
		 XColor Color;
		 FG_INT32 i;

		 for (i=0;i<NUM_COLORS;i++) Xpal[i]=Black;
		 for (i=0;i<16;i++)
		 {
			Color.flags=DoRed|DoGreen|DoBlue;
			Color.red=PaletteArray[i].r<<8;
			Color.green=PaletteArray[i].g<<8;
			Color.blue=PaletteArray[i].b<<8;
			if(XAllocColor(Dsp,DefaultCMap,&Color)) Xpal[i]=Color.pixel;
		 }
	 }
	 #endif
  #endif
}

FG_INT32 fg_get_color(FGPAL_ENTRY *RGBColor)
{
  FG_INT32 i=0, FoundColor=0;
  if (RGBColor->r | RGBColor->g | RGBColor->b) {
	 i++;

	 /* Find a matching or blank Palette entry. */
	 while (
		memcmp(RGBColor,&(PaletteArray[i]),sizeof(FGPAL_ENTRY)*3/4) &&
		PaletteReferenceCount[i] &&
		i<NUM_COLORS
    ) i++;

    /* If we found one set the palette appropriately.      */
    if (i<NUM_COLORS) 
    {
		if (!PaletteReferenceCount[i])
      {
        memmove(&(PaletteArray[i]),RGBColor,sizeof(FGPAL_ENTRY));

        #if defined(USE_X11)
        {
          XColor Color;
          Color.flags=DoRed|DoGreen|DoBlue;
          Color.red=RGBColor->r<<8;
          Color.green=RGBColor->g<<8;
          Color.blue=RGBColor->b<<8;
          if(XAllocColor(Dsp,DefaultCMap,&Color)) {
            Xpal[i]=Color.pixel;
          } else {
            i=(fg_find_closest_color(RGBColor));
          }
        }
        #elif defined(USE_FB)
	{
          fb_cmap.red[i]=PaletteArray[i].r;  
	  fb_cmap.green[i]=PaletteArray[i].g;
	  fb_cmap.blue[i]=PaletteArray[i].b;
	  ioctl(fb_fd,FBIOPUTCMAP,&fb_cmap);
	}
        #elif defined(USE_PM)
        if (GpiSetPaletteEntries(fg_hpal,LCOLF_CONSECRGB,i
                                 ,1,&(PaletteArray[i]))) 
        {
          WinPostMsg(fg_hwndClient, WM_COMMAND,
                     MPFROM2SHORT(CMD_PALETTE_CHANGE,0),0);
        } else {
          i=(fg_find_closest_color(RGBColor));
        }

        #elif defined(USE_ALLEGRO)
        set_pallete(PaletteArray);

        #elif defined(USE_WIN32)
        fg_logpalette->palPalEntry[i].peRed=RGBColor->r;
        fg_logpalette->palPalEntry[i].peGreen=RGBColor->g;
        fg_logpalette->palPalEntry[i].peBlue=RGBColor->b;
        fg_logpalette->palPalEntry[i].peFlags=PC_RESERVED;
        if (SetPaletteEntries(fg_hpalette,i,1,&(fg_logpalette->palPalEntry[i])))
        {
          HDC hdc=GetDC(fg_hwnd);
          RealizePalette(hdc);
          ReleaseDC(fg_hwnd,hdc);
        } else {
          i=(fg_find_closest_color(RGBColor));
        }
        #endif
      } 
    } else {
      i=fg_find_closest_color(RGBColor);
    }
  }
  PaletteReferenceCount[i]++;
  return(i);
}

FG_INT32 fg_free_color(FG_INT32 color)
{
  if (PaletteReferenceCount[color]) {
    return (--PaletteReferenceCount[color]);
  } else {
    return (0);
  }
}

FG_INT32 fg_find_closest_color(FGPAL_ENTRY *RGBColor)
/* really should convert this to HSV distance */
{
  FG_INT32 i,mindiff=INT_MAX,diff,min_index=0,dr,dg,db;
  for (i=0;i<NUM_COLORS;i++) {
    if (PaletteReferenceCount[i]) {
      dr=(FG_INT32)(RGBColor->r)-PaletteArray[i].r;
      dg=(FG_INT32)(RGBColor->g)-PaletteArray[i].g;
      db=(FG_INT32)(RGBColor->b)-PaletteArray[i].b;
      diff=dr*dr+dg*dg+db*db;
      if (diff<mindiff) min_index=i;
    }
  }
  return(min_index);
}
    
FG_INT32 fg_fade_color(FG_INT32 Color, FG_UINT32 mul, FG_UINT32 add)
{
  FGPAL_ENTRY RGBColor=PaletteArray[Color];
  fg_free_color(Color);
  RGBColor.r=(((unsigned)RGBColor.r*mul)>>8)+add;
  RGBColor.g=(((unsigned)RGBColor.g*mul)>>8)+add;
  RGBColor.b=(((unsigned)RGBColor.b*mul)>>8)+add;
  return (fg_get_color(&RGBColor));
}


