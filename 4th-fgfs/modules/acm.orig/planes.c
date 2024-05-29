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
#include <memory.h>
#ifdef HAVE_DIS
#include "dis.h"
#endif

static craftType ctype[MAXCRAFTTYPES];

extern FILE * acm_fopen ();
craftType *newCraft ();
craftType *readPlane ();
double genericThrust();
void genericResupply();

extern int radioReceptionCheck();
extern void initRadio();
extern void initDamage();
extern double efrandom();

craftType *
lookupCraft (name)
char * name; 
{
	int	i;
	craftType * c = NULL;

	for (i=0; i<MAXCRAFTTYPES; ++i) {
	    if (ctype[i].name != (char *) NULL) {
		if (strcmp(name, ctype[i].name) == 0)
			return &ctype[i];
	    }
	}

	return c;
}

#ifdef HAVE_DIS

craftType *
lookupCraftByEntityType (id)
dis_entity_type *id; 
{
	int	i;
	craftType * c = NULL;

	for (i=0; i<MAXCRAFTTYPES; ++i) {
	    if (ctype[i].name != (char *) NULL) {

/*
 * This search could be done better; we could stop when the entityType
 * field being tested is zero.
 */

		if (ctype[i].entityType.kind == id->kind &&
		ctype[i].entityType.domain == id->domain &&
		ctype[i].entityType.country == id->country &&
		ctype[i].entityType.category == id->category &&
		ctype[i].entityType.subcategory == id->subcategory) {
			return &ctype[i];
		}
	    }
	}
	return c;
}

#endif

void
printValidAircraft(s)
int	s;
{
	char	buf[256];
	int	i;

	sprintf (buf, "Valid aircraft types are:\n\n");
	write (s, buf, strlen(buf));

	for (i=0; i<MAXCRAFTTYPES; ++i) {
	    if (ctype[i].name != (char *) NULL) {
		if (ctype[i].CLift) {
			sprintf (buf, "        %s\n", ctype[i].name);
			write (s, buf, strlen(buf));
		}
	    }
	}
}

craftType *
newCraft ()
{
	int i;
	for (i=0; i<MAXCRAFTTYPES; ++i)
		if (ctype[i].name == (char *) NULL) {
			ctype[i].name = "*allocated*";
			return &ctype[i];
		}

	return (craftType *) NULL;
}


double
genericThrust (c)
craft *c;
{
	register double t, ts;

	if (c->flags & FL_AFTERBURNER) { 
		t = interpolate (c->cinfo->ABThrust, c->mach) * c->cinfo->maxABThrust;
	}
	else {
		t = interpolate (c->cinfo->Thrust, c->mach) * c->cinfo->maxThrust;
	}

	ts = c->throttle / 32768.0;
	c->rpm = (c->rpm - ts) * exp (deltaT / c->cinfo->engineLag) + ts;

	return t * c->rpm * c->rpm * c->rho / 0.002377;
}

void
genericResupply (c)
craft	*c;
{

	int i;
	c->fuel = c->cinfo->maxFuel;

	for (i=0; i<c->cinfo->sCount; i++)
		{
		c->station[i] = c->cinfo->station[i];
		}
	initDamage (c);
}


extern void initPanel();

int
newPlane (planeType)
char *planeType;
{

	int     i, j;
	craft	*c;

	for (i=0; i<MAXPLAYERS; ++i) {
		if (ptbl[i].type == CT_FREE) {

			c = &ptbl[i];
			if ((c->cinfo = lookupCraft (planeType)) == NULL) {
				return -2;
			}

			c->type = CT_PLANE;

			c->Cg.x = 0.0;
			c->Cg.y = 0.0;
			c->Cg.z = 0.0;
			c->Sg.x = 0.0;
			c->Sg.y = 0.0;
			c->Sg.z = - c->cinfo->groundingPoint.z;
			c->prevSg = c->Sg;

			c->p = c->q = c->r = 0.0;
			c->Se = c->Sr = c->Sa = 0.0;
			c->SeTrim = 0.0;

			c->throttle = 1024;
			c->curThrust = calcThrust (c);
			c->rpm = (double) c->throttle / 32768.0;
			c->throttle = 8192;
			c->curFlap = 0.0;
			c->flapSetting = 0.0;
			c->curGear[0] = c->curGear[1] =
				c->curGear[2] = pi / 2.0;
			c->curSpeedBrake = 0.0;
			c->speedBrakeSetting = 0.0;
			c->curHeading = c->curPitch = c->curRoll = 0.0;
			VIdentMatrix(&(c->trihedral));
			c->Itrihedral = c->trihedral;
			c->curNWDef = 0.0;
			c->flags = FL_NWS | FL_GHANDLE_DN | FL_GND_CONTACT;
			c->radarMode = RM_STANDBY;
			c->curRadarTarget = -1;
			c->viewDirection.x = 1.0;
			c->viewDirection.y = 0.0;
			c->viewDirection.z = 0.0;
			c->viewUp.x = 0.0;
			c->viewUp.y = 0.0;
			c->viewUp.z = -1.0;
			c->groundCgx = 0.0;
			for (j=0; j<3; ++j) {
				c->leftHUD[j] = Vmalloc (32);
				c->rightHUD[j] = Vmalloc (32);
			}

/*
 *  rearm and fuel the aircraft.
 */

			(*c->cinfo->resupply) (c);

			initPanel (c);

/* a trick to select a valid weapon */

			c->curWeapon = WEAPONTYPES - 1;
			selectWeapon (c);

/*
 *  Set up the radios
 */
			initRadio (&c->navReceiver[0]);
			initRadio (&c->navReceiver[1]);
			c->hsiSelect = &c->navReceiver[0];

			break;
			
		}
	}

	if (i == MAXPLAYERS)
		return -1;

	return i;
}

int newDrone(p)
craft *p; {


	int     i, j, droneTeam;
	craft	*c;
	char	*type;
	VPoint	s;
#ifdef HAVE_DIS
        double  disLocation[3];
	double  disZeroVec[3];
	double  disOrientation[3];
#endif    

	for (i=0; i<MAXPLAYERS; ++i) {
		if (ptbl[i].type == CT_FREE) {

			if (p->team == 1) {
				type = "MiG-29";
				droneTeam = 2;			
			}
			else {
				type = "F-16";
				droneTeam = 1;			
			}

			c = &ptbl[i];
			*c = *p;

			c->pIndex = i;
			c->team = droneTeam;
			c->curOpponent = -1;
			c->holdCount = 0;

			c->vl = NULL;
			c->type = CT_DRONE;
			c->cinfo = lookupCraft (type);

/*
 *  Position the drone 1000 feet ahead of the player's craft.
 */

			strncpy(c->name, "drone", sizeof(c->name));
			s.x = 2000.0 + 800.0 * (efrandom() + efrandom());
			s.y = 400.0 * (efrandom() + efrandom());
			s.z = 0.0;
			VTransform (&s, &(p->trihedral), &(c->Sg));
			c->Sg.x += p->Sg.x;
			c->Sg.y += p->Sg.y;
			c->Sg.z += p->Sg.z;
			c->prevSg = c->Sg;

			c->curThrust = calcThrust (c);

			c->curNWDef = 0.0;
			c->flags = p->flags & FL_AFTERBURNER;
			c->radarMode = RM_NORMAL;
			c->curRadarTarget = -1;

			for (j=0; j<3; ++j) {
				c->leftHUD[j] = Vmalloc (32);
				c->rightHUD[j] = Vmalloc (32);
			}

/*
 *  rearm and fuel the aircraft.
 */

			(*c->cinfo->resupply) (c);

			c->hsiSelect = NULL;
			
			selectNamedWeapon (c, WK_AIM9M);
			
			type = "";
			strcpy (c->lastTotal, type);
			strcpy (c->lastConsump, type);
			strcpy (c->lastFlap, type);
			strcpy (c->lastRPM, type);

#ifdef HAVE_DIS
			if (disInUse) {
			    disLocation[0] = c->Sg.x;
			    disLocation[1] = c->Sg.y;
			    disLocation[2] = c->Sg.z;
			    disZeroVec[0] = 0.0;
			    disZeroVec[1] = 0.0;
			    disZeroVec[2] = 0.0;
			    disOrientation[0] = c->curHeading;
			    disOrientation[1] = c->curPitch;
			    disOrientation[2] = c->curRoll;
			    dis_entityEnter (droneTeam, c,
			        &c->cinfo->entityType,
			        &c->cinfo->altEntityType,
			        disLocation, disZeroVec,
				disZeroVec, disOrientation,
				disZeroVec, &c->disId);
			    dis_setRadarMode (c, 1);
#ifdef DIS_DEBUG
			    printf ("Entering %d %d\n", c->pIndex, c->disId);
#endif
			}
#endif			

			break;			
		}
	}

	if (i == MAXPLAYERS)
		return -1;

	return i;
}
