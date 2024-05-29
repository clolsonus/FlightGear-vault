/* flight.c -- This the ACM flight model
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
#include <sys/time.h>
#include <unistd.h>

#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgvars.h"
#include "fgtimer.h"
#include "flight.h"

#include "modelacm.h"


int flight_timer_event;

extern double deltaT, halfDeltaTSquared;
double curTime;
double targetspeed;
double lastTime = 0.0;
static int warned = 0;
craft *c;


FG_VOID *flight_init(FGDRIVER_INIT_CALL *callstruct) {
    static FGDRIVER_INIT_CALL flight_init_struct = {
	sizeof(FGDRIVER_INIT),  /* version id    */
	FGDRIVER_FLIGHT         /* I'm a flight model */
    };

    FGEVENT_CALL event = {0, NULL};

    int i;

    if (callstruct->msg == FG_INIT) {
	flight_timer_event=handle_fgtimer_event(50, flight_event);

	/* Compile the aircraft inventory */
	if (compileAircraftInventory() != 0) {
	    fprintf (stderr, "Fatal error\n");
	    exit (1);
	}

	i = newPlane("C-172");
	c = &ptbl[i];

	c->Sg.x = 0.0;
	c->Sg.y = 0.0;
	c->Sg.z = -5000;
	c->curHeading = 0.0;
	c->throttle = 32000;     /* Nearly full throttle */
	/* c->Se = 0.4; */       /* A little up elevator */
	targetspeed = 80.0;     /* knots */

	buildEulerMatrix (c->curRoll, c->curPitch, c->curHeading,
			  &(c->trihedral));
	transpose (&c->trihedral, &c->Itrihedral);

	return (&flight_init_struct);
    }
}


FG_VOID *flight_event(FGEVENT_CALL *event) {
    struct timeval tp;
    struct timezone tzp;
    static double lastTime = 0.0;
    double Vmag, knots;
    static double lastknots = 0.0, lastrate = 0.0;
    double diffknots, targetdiff;
    double diffrate;
    char junk[256];

    gettimeofday (&tp, &tzp);
    curTime = (double) tp.tv_sec + (double) tp.tv_usec / 1000000.0;

    printf ("tp.tv_sec = %d  tp.tv_usec = %d\n", tp.tv_sec, tp.tv_usec);

    if (lastTime == 0.0) {
	lastTime = curTime - 0.1;
    }

    /* Check if "curTime" ever equals "lastTime", if so, warn that
     *  REAL_DELTA_T probably should not be used on this system. */

    if (curTime == lastTime) {
	curTime += 0.0001;
	if (warned) {
	    warned = 1;
	    fprintf (stderr,
"Hmm. We seem to have performed a complete inner loop in zero elapsed time.\n\
This seems suspicious.  Perhaps gettimeofday() has a relatively coarse\n\
granularity on this system.\n\
This server should probably be recompiled without REAL_DELTA_T defined.\n");
	}
    }

    deltaT = curTime - lastTime;

    /* If the amount of real elapsed time is unreasonably long,
     * then make it one update interval.  */

    if (deltaT > (double)((UPDATE_INTERVAL / 1000000.0) * 4)) {
	deltaT = UPDATE_INTERVAL / 1000000.0;
    }

    lastTime = curTime;
    halfDeltaTSquared = 0.5 * deltaT * deltaT;

    flightCalculations(c);

    /* Ok, here's my first stab at a little autopilot-like code */

    Vmag = mag(c->Cg);
    knots = FPStoKTS(Vmag);

    diffknots = knots - lastknots;
    diffrate = c->Cg.z - lastrate;

    if ( c->Cg.z < 0 ) {
	/* ascending */
	if ( c->Cg.z > lastrate ) {
	    c->Se += 0.01;
	} else {
	    c->Se -= 0.01;
	}
    } else {
	/* descending */
	if ( c->Cg.z < lastrate ) {
	    c->Se -= 0.01;
	} else {
		c->Se += 0.01;
	}
    }

    /* c->Se += (c->Cg.z + diffrate) / 40; */

    /* if ( (c->Cg.z < -8.3333) || (c->Cg.z > 8.3333) ) { */
    /* decending or ascending faster than 500 ft/min */
    /* c->Se += (c->Cg.z + diffrate) / 20;
       } else {
       targetdiff = (knots + diffknots) - targetspeed;
       
       c->Se += targetdiff / 10.0;
       } */

    if ( c->Se < -1.0 ) {
	c->Se = -1.0;
    } else if ( c->Se > 1.0 ) {
	c->Se = 1.0;
    }
    
    lastknots = knots;
    lastrate = c->Cg.z;

    printf("--> c->Se = %.3f\n", c->Se);
    printf("continue ... ");
    /* gets(junk); */
    printf("\n");

    return(NULL);
}


void flight_export() {
}

/* $Log: fgslew.c,v $
 * */
