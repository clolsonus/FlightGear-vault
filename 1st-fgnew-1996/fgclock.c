/* fgclock.c -- This is a prototype FlightGear driver             
 * Draws a digital clock in a bitmap.
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
 * $Id: fgclock.c,v 2.1 1996/10/31 18:05:22 korpela Exp korpela $ 
 */

#include <stdio.h>
#include <time.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgvars.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgtimer.h"
#include "fggrx.h"
#include "fgfont.h"
#include "fgscreen.h"
#include "fgclock.h"

static FG_BITMAP *fgclock_bmp;
void *fgclock_public; 
FGFONT_PIXMAP *clockfont;

FG_VOID *fgclock_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgclock_init_struct=
                                    {sizeof(FGDRIVER_INIT_CALL),/* version id */
                                     0};         /* nothing to tell       */ 
  switch (callstruct->msg) {
    case FG_REINIT:        unhandle_fgtimer_event(1000,fgclock_event);
    case FG_INIT:          handle_fgtimer_event(1000,fgclock_event); 
                           fgclock_public=callstruct->public_vars;
#define public_vars fgclock_public
    case FG_BITMAP_CHANGE: fgclock_bmp=callstruct->bitmap;
                           clockfont=fg_get_font((fgclock_bmp->w-2)/8,fgclock_bmp->h-2);
                           
  }
  return (&fgclock_init_struct);
}

FG_VOID *fgclock_event(FGEVENT_CALL *event)
{ 
  int hours,minutes,sec;
  char text[10];
  if (fgclock_bmp) {
    hours=AC_TIME_MS/3600000L;
    minutes=(AC_TIME_MS%3600000L)/60000L;
    sec=(AC_TIME_MS%60000L)/1000L;
    sprintf(text,"%02d:%02d:%02d",hours,minutes,sec);
    fg_drawtext(text,clockfont,1,1,12,fgclock_bmp);
  }
  return (NULL);
}

void fgclock_export()
{
}

/* $Log: fgclock.c,v $
 * Revision 2.1  1996/10/31  18:05:22  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:53:04  korpela
 * *** empty log message ***
 *
 * Revision 1.4  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * Started over
 * */
