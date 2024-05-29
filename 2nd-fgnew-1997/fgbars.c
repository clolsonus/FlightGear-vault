/* fgbars.c -- This is a prototype FlightGear driver             
 * Draws colorbars in a bitmap 
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
 * $Id: fgbars.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $ 
 */

#include <stdio.h>
#include <time.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgtimer.h"
#include "fggrx.h"
#include "fgscreen.h"
#include "fgbars.h"

static FG_BITMAP *fgbars_bmp;

FG_VOID *fgbars_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgbars_init_struct=
                                    {sizeof(FGDRIVER_INIT_CALL),/* version id */
                                     0};         /* nothing to tell       */ 
  switch (callstruct->msg) {
    case FG_REINIT:        unhandle_fgtimer_event(10000,fgbars_event);
    case FG_INIT:          handle_fgtimer_event(10000,fgbars_event); 
    case FG_BITMAP_CHANGE: fgbars_bmp=callstruct->bitmap;
  }
  return (&fgbars_init_struct);
}

FG_VOID *fgbars_event(FGEVENT_CALL *event)
{ 
  FG_INT32 x;
  if (fgbars_bmp) {
    for (x=0;x<(fgbars_bmp->w);x++) {
      fg_vline(x,0,(fgbars_bmp->h)-1,(x*16)/(fgbars_bmp->w),fgbars_bmp);
    }
  }
  return (NULL);
}

void fgbars_export()
{
}

/* $Log: fgbars.c,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
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
