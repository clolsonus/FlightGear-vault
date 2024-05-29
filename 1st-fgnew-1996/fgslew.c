/* fgslew.c -- This is a prototype FlightGear slew driver             
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
 * $Id: fgslew.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $ 
 */

#include <stdio.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fggeom.h"
#include "fgvars.h"
#include "fgslew.h"
#include "fgtimer.h"

FG_VOID *slew_public_vars;
int slew_timer_event;

FG_VOID *fgslew_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgslew_init_struct=
                                    {sizeof(FGDRIVER_INIT), /* version id    */
                                     FGDRIVER_FLIGHT}; /* I'm a flight model */
  FGEVENT_CALL event={0,NULL};
  if (callstruct->msg == FG_INIT) {
    slew_public_vars=callstruct->public_vars;

/********** access public variables ******************/
#define public_vars slew_public_vars 

    slew_timer_event=handle_fgtimer_event(50,fgslew_event);

/*********** get rid of this stuff later ************/
    /* Presumably at some point a "mode" driver will*/
    /* take care of this. */
    /* set us at 0,0,0 */
    ACPOS_ELON=0; ACPOS_LAT=0; ACPOS_ALTMSL=0;
    /* set speed to 0  */
    ACVEL_DLON=0; ACVEL_DLAT=0; ACVEL_DALT=0;
    /* point us north  */
    ACATT_NLON=0; ACATT_NLAT=FG_ONE; ACATT_NALT=0;
    /* turn us upright */
    ACATT_VLON=0; ACATT_VLAT=0; ACATT_VALT=FG_ONE;
    /* set the left wing on the horizon */
    ACATT_WLON=-FG_ONE; ACATT_WLAT=0; ACATT_WALT=0;


    event.event_id=get_fgevent("center_all_controls");
    send_fgevent(&event);
  }
  return (&fgslew_init_struct);
}

static FG_INT32 slew_time0,slew_time1;

FG_VOID *fgslew_event(FGEVENT_CALL *event)
{
  /* how long since we were here last */
  FG_INT32 elapsed,speed;
  slew_time1=AC_TIME_MS;
  elapsed=slew_time1-slew_time0;
  slew_time0=slew_time1;

  /* how fast are we supposed to go */
  /* speed=ACCONTROL_THROTTLE; */
  speed=50000;
  ACCONTROL_ELEVATOR=0;
  ACCONTROL_AILERON=0;
  ACCONTROL_RUDDER=FG_DEGREE;

  /* set the velocity accordingly */
  i3vect_scale(&ACATT_NOSEVEC,speed,&ACVELVEC);
  /* adjust for lattitude */
  ACVEL_DLON*=icos(ACPOS_LAT);
 
  /* move us */
  ACPOS_ELON+=(ACVEL_DLON * elapsed/1000); 
  ACPOS_LAT+=(ACVEL_DLAT * elapsed/1000);
  ACPOS_ALTMSL+=(ACVEL_DALT * elapsed/1000);

  /* turn us according to the control positions */
  if (ACCONTROL_ELEVATOR) {
    i3rot(&ACATT_NOSEVEC,&ACATT_WINGVEC,&ACATT_NOSEVEC,
           -1*ACCONTROL_ELEVATOR*elapsed/1000);
    i3cross(&ACATT_NOSEVEC,&ACATT_WINGVEC,&ACATT_VERTVEC);
  }

  if (ACCONTROL_AILERON) {
    i3rot(&ACATT_WINGVEC,&ACATT_NOSEVEC,&ACATT_WINGVEC,
           ACCONTROL_AILERON*elapsed/1000);
    i3cross(&ACATT_NOSEVEC,&ACATT_WINGVEC,&ACATT_VERTVEC);
  }

  if (ACCONTROL_RUDDER)  {
    i3rot(&ACATT_NOSEVEC,&ACATT_VERTVEC,&ACATT_NOSEVEC,
          ACCONTROL_RUDDER*elapsed/1000);
    i3cross(&ACATT_VERTVEC,&ACATT_NOSEVEC,&ACATT_WINGVEC);
  } 

  /* set variables for use by instruments */

  ACATT_PITCH=(ACATT_NALT);
  ACATT_BEARING=(FG_INT32)((float)ACATT_NLON/ACATT_NLAT);
  ACATT_ROLL=(ACATT_WALT);

  return(NULL);
  
}

/* $Log: fgslew.c,v $
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.1  1996/09/12  18:38:58  korpela
 * Initial revision
 * */
