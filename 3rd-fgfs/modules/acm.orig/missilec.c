/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991,1992  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */
 
#include <stdio.h>
#include <math.h>
#include "pm.h"

int mdebug = 0;
extern double calcRho();
extern void trackTarget();
extern void craftToGround();

int  missileCalculations (c)
craft *c; {

	double	q, rho;
	double	FLift, FDrag, FWeight;
	double	Vmag;
	VPoint	F, Fg;

/*
 *  Check for ground impact.  We do this at the beginning to permit us to
 *  kill ground targets.
 */

	if (c->Sg.z > 0.0) {
		q =  - c->prevSg.z / (c->Sg.z - c->prevSg.z);
		c->Sg.x = c->prevSg.x + q * (c->Sg.x - c->prevSg.x);
		c->Sg.y = c->prevSg.y + q * (c->Sg.y - c->prevSg.z);
		c->Sg.z = 0.0;
		return 1;
	}

	trackTarget (c);

/*
 *  If we haven't armed the missile, yet.  Decrement the delay timer.
 *  If the FL_BALLISTIC flag is set, we have no target; self-destruct
 *  if the timer expires.
 */

	if (c->armTimer != 0.0) {
		if ((c->armTimer -= deltaT) < 0.0) {
			if (c->flags & FL_BALLISTIC) {
				return 1;
			}
			c->armTimer = 0.0;
		}
	}

/*
 *  Re-orient the body of the missile towards it's intended target.
 */

	c->prevSg = c->Sg;

	rho = calcRho (-(c->Sg.z), &q);

/*
 *  Compute the resultant force vector on the missile.
 */

	Vmag = mag(c->Cg);
	q = rho * c->cinfo->wingS * Vmag * Vmag * 0.5;
	FLift = 0.0;
	FDrag = c->cinfo->CDOrigin * q;

#ifdef FLIGHTDEBUG
	if (mdebug) {
		printf ("rho = %g, FLift = %g, FDrag = %g\n", rho, FLift, FDrag);
		printf ("FThrust = %g\n", c->curThrust);
	}
#endif

	F.x = c->curThrust - FDrag;
	F.y = 0.0; 
	F.z = 0.0; 

/*
 *  Now calculate changes in position (Sg) and velocity (Cg).
 */

	if ((c->fuel -= fuelUsed(c)) <= 0.0) {
		if (c->curThrust > 0.0)
			if (mdebug)
			printf ("Missile burnout; velocity = %g fps (%g kts)\n", Vmag,
				FPStoKTS(Vmag));
		c->fuel = 0.0;
		c->curThrust = 0.0;
	}

/*
 *  The missile's trihedral and Itrihedral matrices are managed by 
 *  trackTarget().
 */

	craftToGround (c, &F, &Fg);
	FWeight = c->cinfo->emptyWeight + c->fuel;
	Fg.z += FWeight;

#ifdef FLIGHTDEBUG
	if (mdebug) {
	    	printf ("v = %g kts, Fg = { %g, %g, %g }\n", FPStoKTS(Vmag),
			Fg.x, Fg.y, Fg.z);
	    	printf ("F = { %g, %g, %g }\n", F.x, F.y, F.z);
	}
#endif



/*
 *  Update the missile's position and velocity.
 */


	c->Sg.x += c->Cg.x * deltaT + Fg.x / FWeight * a * halfDeltaTSquared;
	c->Sg.y += c->Cg.y * deltaT + Fg.y / FWeight * a * halfDeltaTSquared;
	c->Sg.z += c->Cg.z * deltaT + Fg.z / FWeight * a * halfDeltaTSquared;

	c->Cg.x += Fg.x / FWeight * a * deltaT;
	c->Cg.y += Fg.y / FWeight * a * deltaT;
	c->Cg.z += Fg.z / FWeight * a * deltaT;

#ifdef FLIGHTDEBUG
	if (mdebug) {
		printf ("Altitude = %g\n", -c->Sg.z);
		printf ("Euler angles { %g, %g, %g }\n", RADtoDEG(c->curRoll),
			RADtoDEG(c->curPitch), RADtoDEG(c->curHeading));
		printf ("Cg = { %g, %g, %g }\n", c->Cg.x, c->Cg.y, c->Cg.z);
		printf ("Sg = { %g, %g, %g }\n", c->Sg.x, c->Sg.y, c->Sg.z);
	}
#endif

	return 0;
}
