#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "modelacm.h"


extern double deltaT, halfDeltaTSquared;
double curTime;
double lastTime = 0.0;
static int warned = 0;


main() {
    int i;
    craft *c;
    struct timeval tp;
    struct timezone tzp;
    double lastTime = 0.0;
    double targetalt, error;
    char junk[256];

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
    targetalt = 5000.0;      /* feet */

    buildEulerMatrix (c->curRoll, c->curPitch, c->curHeading,
		      &(c->trihedral));
    transpose (&c->trihedral, &c->Itrihedral);

    for ( ; ; ) {
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

	/* Ok, here's my first stab at a little autopilot-like code
	 * First tries to keep climb rate within +/- 500 ft/min.  Then
	 * does a proportional control deal */

	error = targetalt + c->Sg.z;       /* Sg.z is negative */

	if ( error > 0 ) {
	    
	}
	if ( c->Cg.z < -8.3333 ) {
	    /*  ascending faster than 500 ft/min */
	    c->Se -= 0.1;
	} else if ( c->Cg.z > 8.3333 ) {
	    /*  descending faster than 500 ft/min */
	    c->Se += 0.1;
	} else {
	    c->Se = 0.001 * error;
	}

	c->Se = 0.01 * (c->Cg.z + 8.3333);

	if ( c->Se < -1.0 ) {
	    c->Se = -1.0;
	} else if ( c->Se > 1.0 ) {
	    c->Se = 1.0;
	}

	printf("--> c->Se = %.3f\n", c->Se);
	printf("continue ... ");
	gets(junk);
	printf("\n");
    }
}



