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
 
#include <math.h>
#include "pm.h"
#include <stdio.h>
#include <string.h>

int select_aim9m();
int display_aim9m();
int update_aim9m();
int fire_aim9m();
int getIRTarget();
extern int fireMissile ();
extern FILE * acm_fopen ();
extern int countOrdinance();
extern int readyStation();

weaponDesc aim9mDesc = {
	WK_AIM9M,
	select_aim9m,		/* select */
	update_aim9m,		/* update */
	display_aim9m,		/* display procedure */
	fire_aim9m,		/* fire */
	(int (*)()) NULL,	/* fire button release */
	};

int	hasFired[MAXPLAYERS];
static	int count[MAXPLAYERS];

/*
 *  aim9m selection function
 *
 *  A selection function normally determines whether there are any weapons
 *  of this type on-board.  If so, and the weapon system is functional
 *  (in other words, undamaged) then return 1; otherwise return 0.
 */

/*ARGSUSED*/
int select_aim9m (c)
craft *c; {

	hasFired[c->pIndex] = 0;
	count[c->pIndex] = countOrdinance(c, "aim9m");
	return 1;

}

/*ARGSUSED*/
int update_aim9m (c)
craft *c; {

	register int i;

	if (hasFired[c->pIndex] && count[c->pIndex] > 0) {
		i = readyStation (c, "aim9m");
		if (i < 0)
			fprintf (stderr, "Oops. Can't find an AIM-9\n");
		else {
			if (arcadeMode == 0) {
				c->station[i].type = "";
				count[c->pIndex] --;
			}
			fireMissile (c, i);
			playSound (c, SoundMissileLaunch);
		}
		hasFired[c->pIndex] = 0;
	}
	return 1;
}

/*ARGSUSED*/
int fire_aim9m (c)
craft *c; {

	hasFired[c->pIndex] = 1;
	return 1;
}

/*
 *  aim9m display function
 *
 *  Update the HUD display strings associated with this weapon system.
 *
 *  This code may be called by drones.  Return a nonzero value if
 *  have a reasonable chance of scoring a kill.
 */

/*ARGSUSED*/
int display_aim9m (c, w, u)
craft *c;
craftType *w;
viewer *u; {

	char	s[16];
	double	d, a1, v, r, root1, root2, n, t;
	VPoint	tmp;
	int	target, result = 0;

	sprintf (s, "%d AIM-9M", count[c->pIndex]);
	strcpy (c->leftHUD[1], s);

	v = mag (c->Cg);
	a1 = (w->maxThrust - 0.5 * c->rho * w->CDOrigin * v * v)
		/ (w->emptyWeight + w->maxFuel) * a;

	if (c->curRadarTarget >= 0 && a1 >= 0.0) {

	d = c->targetDistance;
	r = c->targetClosure;

	n = r * r + 2.0 * a1 * d;
	if (n > 0) {
		n = sqrt (n);
		root1 = (- r + n) / a1;
		root2 = (- r - n) / a1;
		if (root1 >= 0.0)
			if (root2 >= 0.0)
				if (root1 < root2)
					t = root1;
				else
					t = root2;
			else
				t = root1;
		else if (root2 >= 0.0)
			t = root2;
		else
			t = -1.0;
	}
	else
		t = -1.0;
	}

	else
		t = -1.0;

/*
 *  See if the missiles can lock onto any target.  We'll constrain getIRTarget()
 *  so that it will only select target's in a twenty degree boresight cone.
 */

	if (count[c->pIndex] > 0) {
		target = getIRTarget (c, &tmp, 0.17633);
	}
	else {
		target = -1;
	}

	if (target >= 0 && t <= 15.0)
		sprintf (s, "LOCKED   %d", (int)(t+0.5));
	else if (t < 0.0)
		sprintf (s, "ARM      --");
	else if (t <= 15.0)
		sprintf (s, "IN RANGE %d", (int)(t+0.5));
	else 
		sprintf (s, "ARM      %d", (int)(t+0.5));

/*
 *  Set result equal to one if we are recommending a missile shot
 */

	if (target >= 0 && t <= 10.0 && t > (c->cinfo->armDelay + 1.0)) {
		result = 1;
	}

	strcpy (c->leftHUD[0], s);

	strcpy (c->leftHUD[2], "");

	return result;
}

extern craftType * newCraft ();

void initaim9()
{

	craftType	*c;
	FILE    	*f;
	dis_entity_type	em1 = { 2, 1, 225, 1, 1, 3, 0 };
	dis_entity_type	em2 = { 2, 1, 222, 1, 2, 1, 0 };

	c = newCraft();
	c->name = strdup("aim-9m");
	
	c->entityType    = em1;
	c->altEntityType = em2;

	wtbl[0] = aim9mDesc;
	wtbl[0].w = c;

	c->CDOrigin = 0.1963494;	/* 4" radius of body */
	c->CDFactor = -2.56694;

	c->CDBOrigin = 0.0;
	c->CDBFactor = 0.0;

	VIdentMatrix(&(c->I));
	c->I.m[0][0] =  0.0;
	c->I.m[1][1] = 0.0;
	c->I.m[2][2] = 0.0;
	c->cmSlope = -1.88;
	c->cmFactor = -1.00;

	c->wingS = 1.0;

/*
 *  Assume 150.0 lbs of weight is fuel and that it burns for 20 seconds.
 *  That yields a fuel burn rate of 7.5 lbs/second.
 */

	c->emptyWeight = 40.0;
	c->maxFuel = 150.0;
	c->maxThrust = 2000.0;
	c->spFuelConsump = 34.134;

/*
 *  Three second arming delay
 */

	c->armDelay = 3.0;

	c->groundingPoint.x = 0.0;
	c->groundingPoint.y = 0.0;
	c->groundingPoint.z = 0.0;

	c->viewPoint.x = 0.0;
	c->viewPoint.y = 0.0;
	c->viewPoint.z = 0.0;

	c->muStatic = 0.0;
	c->muKinetic = 0.0;
	c->muBStatic = 0.0;
	c->muBKinetic = 0.0;

	c->maxNWDef = 0.0;
	c->NWIncr = 0.0;
	c->maxNWS = 0.0;
	c->gearD1 = 0.0;
	c->gearD2 = 0.0;

	f = acm_fopen ("aim9.obj", "r");
	c->object = VReadObject(f);
	fclose (f);

}
