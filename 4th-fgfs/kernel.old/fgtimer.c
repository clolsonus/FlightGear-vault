/* fgtimer.c -- set up timer events                                  
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
 * $Id: fgtimer.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $ 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgvars.h"
#include "fgtimer.h"

struct _FG_TIMER {
  int event;     /* event id */
  int counter;   /* remaining time */
  int value;     /* timer count value */
  char name[MAX_FGEVENT_NAME]; /* event name */
} timer[NUM_TIMERS];

int master_timer,timer_incr;
int n_timers;
struct timeb timer_time0,timer_time1;
int timer_paused;
FG_VOID *timer_public_vars;

FG_VOID *fgtimer_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgtimer_init_struct=
                                    {sizeof(FGDRIVER_INIT), /* version id */
                                     0};         /* nothing to tell       */ 
  unsigned short t1;
  if (callstruct->msg == FG_INIT) {
    master_timer=handle_fgevent("master",fgtimer_master);
    handle_fgevent("pause_notify",fgtimer_pause);
    handle_fgevent("unpause_notify",fgtimer_unpause);
    timer_public_vars=callstruct->public_vars;
/************** use public variables **************************/
#define public_vars timer_public_vars 

    /* determine the minimum timer increment */
    timer_incr=-1;
    while (timer_incr<0) {
      ftime(&timer_time0);
      t1=timer_time0.millitm;
      while (t1==timer_time1.millitm) ftime(&timer_time1);
      timer_incr=timer_time1.millitm-t1;
    }

    printf("timer_incr=%d ms\n",timer_incr);

    AC_TIME_MS=(timer_time0.time % 86400)*1000+timer_time0.millitm;
  }
  
  return (&fgtimer_init_struct);
}

FG_VOID *fgtimer_pause(FGEVENT_CALL *event)
{
  timer_paused=1;
  return(NULL);
}

FG_VOID *fgtimer_unpause(FGEVENT_CALL *event)
{
  timer_paused=0;
  return(NULL);
}

FG_VOID *fgtimer_master(FGEVENT_CALL *my_event)
{
  static FGEVENT_CALL event;
  int i;
  FG_INT32 elapsed;

  /* Are we paused??? If not update the time */
  if (!timer_paused) {
    ftime(&timer_time1);
  } else {
    memcpy(&timer_time1,&timer_time0,sizeof(timer_time1));
  }

  /* calculate time elasped since last update */
  elapsed=(timer_time1.time-timer_time0.time)*1000+
          (timer_time1.millitm-timer_time0.millitm);

  /* update the aircraft clock */
  AC_TIME_MS+=elapsed;

  /* update the timers and generate events as necessary */ 
  for (i=0;i<n_timers;i++) {
    if ((timer[i].counter-=elapsed)<0) {
      event.event_id=timer[i].event;
      send_fgevent(&event);
      timer[i].counter=timer[i].value;
    }
  }

  /* Are we paused???*/
  if (timer_paused) {
    ftime(&timer_time1);
  } 
  memcpy(&timer_time0,&timer_time1,sizeof(timer_time0));

  return (NULL);
}


int handle_fgtimer_event(int time_ms,FG_EVENT_FN function)
{
  int i,diff,mindiff=10000,besttimer=-1;
  char name[MAX_FGEVENT_NAME];
  time_ms=FG_MAX(timer_incr,((time_ms+timer_incr/2)/timer_incr)*timer_incr);

  /* find the closest existing timer */
  for (i=0;i<n_timers;i++) {
    diff=FG_ABS(time_ms-timer[i].value);
    if (diff<mindiff) {
      mindiff=diff;
      besttimer=i;
    }
  }
  /* is it close enough or have we used up all the timers? */
  if (((mindiff*16)<time_ms) || (n_timers==NUM_TIMERS)) {
    /* Yes, get the closest timer */
    sprintf(name,"%s",timer[besttimer].name);
    return (handle_fgevent(name,function)); 
  } else {
    /* No, create a timer */
    timer[n_timers].counter=random() % time_ms;
    timer[n_timers].value=time_ms;
    sprintf(timer[n_timers].name,"%7d_ms_timer",time_ms);
    timer[n_timers].event=handle_fgevent(timer[n_timers].name,function);
    return (timer[n_timers++].event);
  }
}
    
int unhandle_fgtimer_event(int time_ms,FG_EVENT_FN function)
{
  int i,diff,mindiff=10000,besttimer=-1;
  char name[MAX_FGEVENT_NAME];
  time_ms=FG_MAX(timer_incr,((time_ms+timer_incr/2)/timer_incr)*timer_incr);
  /* find the closest existing timer */
  for (i=0;i<n_timers;i++) {
    diff=FG_ABS(time_ms-timer[i].value);
    if (diff<mindiff) {
      mindiff=diff;
      besttimer=i;
    }
  }
  /* is it close enough or have we used up all the timers? */
  if ((mindiff*16)<time_ms) {
    /* Yes, get the closest timer */
    sprintf(name,"%7d_ms_timer",besttimer);
    return (unhandle_fgevent(name,function)); 
  } else {
    return(0);
  }
}
    
/* $Log: fgtimer.c,v $
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.2  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.1  1996/05/21  17:02:49  korpela
 * Initial revision
 * */
