/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
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
 
 /*
  *  The original "Smart Drone" code was created by
  *  Jason Nyberg (nyberg@ctron.com).  Enhancements added by Riley Rainey.
  */

#include "pm.h"
#include "alarm.h"

extern double efrandom();

/*
 *  convert target plane coords from world to drone's
 */

void
myCoordSys(c, p, pos, vel)
craft *c, *p;
VPoint *pos, *vel;
{
	VPoint tpos;

	tpos = p->prevSg;
	tpos.x -= c->Sg.x;
	tpos.y -= c->Sg.y;
	tpos.z -= c->Sg.z;
	VTransform (&tpos, &c->Itrihedral, pos);
	VTransform (&p->Cg, &c->Itrihedral, vel);
}

void
unholdFireAlarm (arg1, arg2)
char *arg1, *arg2;
{
	craft *c = (craft *) arg1;

	if (c->holdCount > 0) {
		c->holdCount --;
	}
}

/* 
 *  Dumbly choose the closest plane to be target
 */

int
pickTarget(c)
craft *c;
{
	int n,i,target=-1;
	craft *p;
	double d,min;
	VPoint pos, vel;

	min = 1000000.0;
	for (i=0, p=ptbl; i<MAXPLAYERS; ++i, ++p) {
        	if (p==c || p->type==CT_FREE)
        		continue;

		myCoordSys (c, p, &pos, &vel);

		d = sqrt (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);

		if (d < min) {
			min = d;
			target=p->pIndex;
		}
	}

	return target;
}

/*
 *  Drone flight management
 */

int
droneCalculations (c)
craft *c;
{

	double	d, phi, htime;
	VPoint	pos, vel;

	if (c->curOpponent == -1 ||
		ptbl[c->curOpponent].type == CT_FREE) {
		c->curOpponent = pickTarget(c);
		c->holdCount = 0;
	}

	if (c->curOpponent != -1) {

		myCoordSys (c, &(ptbl[c->curOpponent]), &pos, &vel);

/*
 *  To perform lag persuit, we'll need a spot behind the aircraft.  This spot
 *  is 0.5 seconds behind the target aircraft.
 */

#ifdef notdef 
		pos.x -= 0.5 * vel.x;
		pos.y -= 0.5 * vel.y;
		pos.z -= 0.5 * vel.z;
#endif

		d = sqrt (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z);

/*
 *  DRONE_FACTOR defines just how hard a drone will maneuver into position.
 *
 *  Change to a lower or higher value to make it easier or harder.
 *  Make sure the value is greater than 0.0 and 1.0
 *  0.2-0.3 makes for good gun practice, 1.0 is virtually impossible to shake.
 */

#define DRONE_MAX_Se	(droneAggressiveness)
#define DRONE_MAX_Sa 	(droneAggressiveness)
#define DRONE_MAX_Sr	((droneAggressiveness) * 0.1)

/*
 *  A lot of conventional 1V1 air combat involves keeping your lift
 *  vector on the target aircraft.  Phi is the computed angle between our
 *  target and the lift vector (simplified to be just the negative Z-axis).
 */

		phi = atan (pos.y / - pos.z);

/*
 *  If the target is behind our 3/9-line, we are defensive.  Pull maximum
 *  G's into the target (after rolling into him).
 *
 *  If we are behind of the target, perform pure pursuit (until the code gets
 *  a bit smarter).
 */
 
		if (pos.x < 0.0) {
/*
 *  Wait for lift vector to be close to where we want it before
 *  pulling G's.
 */
			if (phi < DEGtoRAD(10.0) &&
				phi > -DEGtoRAD(10.0)) {
				c->Se = DRONE_MAX_Se;
			}
		}
		else {
			c->Se = - pos.z / d * 3.0;
		}

/*
 *  Put the lift vector on the target.
 */

		if (pos.z == 0.0) {
			c->Sa = 0.0;
		}
		else {
			c->Sa = phi;
		}

/*
 *  Don't use the rudder, for now.
 */

		c->Sr = 0.0;

/*
 *  Constrain control surface positions to valid values.
 */
 
		if (c->Se > DRONE_MAX_Se)
			c->Se = DRONE_MAX_Se;
		else if (c->Se < -DRONE_MAX_Se)
			c->Se = -DRONE_MAX_Se;

		if (c->Sa > DRONE_MAX_Sa)
			c->Sa = DRONE_MAX_Sa;
		else if (c->Sa < -DRONE_MAX_Sa)
			c->Sa = -DRONE_MAX_Sa;

		if (c->Sr > DRONE_MAX_Sr)
			c->Sr = DRONE_MAX_Sr;
		else if (c->Sr < -DRONE_MAX_Sr)
			c->Sr = -DRONE_MAX_Sr;

/*
 *  Fire at the target, if appropriate.  The newDrone() code has
 *  already selected AIM-9's as our weapon.
 *
 *  We'll have to figure out a way to do lead pursuit in order to fire the
 *  cannon; we do pure pursuit now, which is the right thing to fire
 *  a missile.
 */

		if (c->holdCount == 0 &&
			doWeaponDisplay (c, (viewer *) NULL) == 1) {
			fireWeapon (c);
			htime = 10.0 + (efrandom() + efrandom()) * 5.0;
			addAlarm (htime, unholdFireAlarm, (char *) c, NULL);
			c->holdCount ++;
		}

	}

	return 0;
}
