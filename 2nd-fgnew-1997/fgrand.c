/* fgrand.c -- This is a prototype FlightGear driver             
 * Draws random lines in a bitmap 1000/sec.
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
 * $Id: fgrand.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $ 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgtimer.h"
#include "fggrx.h"
#include "fgrand.h"
#include "fgscreen.h"
#include "fgvars.h"

static FG_BITMAP *fgrand_bmp;
static FG_INT32 rtimer_event;
FG_PUBVARS *fgrand_public;

FG_VOID *fgrand_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgrand_init_struct=
                                    {sizeof(FGDRIVER_INIT_CALL),/* version id */
                                     0};         /* nothing to tell       */ 
  switch (callstruct->msg) {
    case FG_REINIT:        unhandle_fgtimer_event(0,fgrand_event);
    case FG_INIT:          rtimer_event=handle_fgtimer_event(0,fgrand_event);
                           fgrand_public=callstruct->public_vars;
#define public_vars fgrand_public
    case FG_BITMAP_CHANGE: fgrand_bmp=callstruct->bitmap;
  }
  srandom(time(0));
  return (&fgrand_init_struct);
}

FG_VOID *fgrand_event(FGEVENT_CALL *event)
{ 
  FG_INT32 x1,y1,x2,y2,color,i,elapsed;
  static FG_INT32 lastime=0;
  if (fgrand_bmp && (fgrand_bmp->struct_size == sizeof(FG_BITMAP))) 
  { 
    if (lastime)
		elapsed=AC_TIME_MS-lastime;
    else
      elapsed=0;
    for (i=0;i<(elapsed);i++) {
		x1=random() % fgrand_bmp->w;
		y1=random() % fgrand_bmp->h;
		x2=random() % fgrand_bmp->w;
		y2=random() % fgrand_bmp->h;
		color=random() % 16;
      fg_line(x1,y1,x2,y2,color,fgrand_bmp);
    }
	 lastime=AC_TIME_MS;
  }
  return (NULL);
}

void fgrand_export()
{
}

/* $Log: fgrand.c,v $
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
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * Started over
 * */
