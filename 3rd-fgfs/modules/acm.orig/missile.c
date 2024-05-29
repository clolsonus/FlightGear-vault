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
#ifdef HAVE_DIS
#include "dis.h"
#endif

typedef struct _entry {
	double	time;
	double	min;
	craft	*c;
	VPoint	Sg;
	struct	_entry *next;
	}	entry;


extern craftType *lookupCraft();
extern int mdebug;
extern void lookForCannonImpacts();
extern void euler();
extern void transpose();
extern void buildEulerMatrix();
extern void newExplosion();

int
inCloud (c)
craft *c;
{
	int state;

	if (c->Sg.z < ctop) {
		state = 0;
	}
	else if (c->Sg.z < cbase) {
		state = 1;
	}
	else {
		state = 2;
	}
	return state;
}

int	fireMissile (c, ind)
craft	*c;
int	ind; {

	register craft	*m;
	register int	i;
	VPoint		s;
	VPoint		cY, mX, mY, mZ;
	double		v;
#ifdef HAVE_DIS
        double  disLocation[3];
	double  disZeroVec[3];
	double  disOrientation[3];
#endif 

	for ((i=0, m= &mtbl[0]); i<MAXPROJECTILES; (++i, ++m))
		if (m->type == CT_FREE) {
			m->type = CT_MISSILE;
			break;
		}

	if (i == MAXPROJECTILES)
		return -1;

	m->cinfo = lookupCraft ("aim-9m");
	m->fuel = m->cinfo->maxFuel;
	m->curThrust = m->cinfo->maxThrust;
	m->owner = c->pIndex;

/*
 *  Build trihedral based on the launching aircraft's current velocity vector
 *  rather than simply it's current direction vector.
 *
 *	(1)  build a unit velocity vector.
 *	(2)  calculate missiles local Z axis from
 *		plane's-y-axis CROSS missile's-unit-velocity-vector
 *	(3)  calculate missile's Y axis.
 */

	if ((v = mag (c->Cg)) < 1.0) {
		m->trihedral = c->trihedral;
		m->Itrihedral = c->Itrihedral;
		m->curRoll = c->curRoll;
		m->curPitch = c->curPitch;
		m->curHeading = c->curHeading;
	}
	else {
		mX = c->Cg;
		mX.x /= v;
		mX.y /= v;
		mX.z /= v;
		cY.x = c->trihedral.m[0][1];
		cY.y = c->trihedral.m[1][1];
		cY.z = c->trihedral.m[2][1];

		VCrossProd (&mX, &cY, &mZ);
		VCrossProd (&mZ, &mX, &mY);

		m->trihedral.m[0][0] = mX.x;
		m->trihedral.m[1][0] = mX.y;
		m->trihedral.m[2][0] = mX.z;
		m->trihedral.m[0][1] = mY.x;
		m->trihedral.m[1][1] = mY.y;
		m->trihedral.m[2][1] = mY.z;
		m->trihedral.m[0][2] = mZ.x;
		m->trihedral.m[1][2] = mZ.y;
		m->trihedral.m[2][2] = mZ.z;

		transpose (&m->trihedral, &m->Itrihedral);
		euler (m);
	}

	m->Cg = c->Cg;
	VTransform (&(c->cinfo->wStation[ind]), &(c->trihedral), &s);
	m->Sg.x = c->prevSg.x + s.x;
	m->Sg.y = c->prevSg.y + s.y;
	m->Sg.z = c->prevSg.z + s.z;
	m->armTimer = m->cinfo->armDelay;
	m->flags = FL_HAS_GYRO;
	m->createTime = curTime;

/*
 * kludge
 */

	m->curRadarTarget = c->curRadarTarget;

#ifdef HAVE_DIS

/*
 *  ACM missiles are DIS "tracked munitions", so we are
 *  responsible for sending entity state PDU's for them
 */

	if (disInUse) {
		disLocation[0] = m->Sg.x;
		disLocation[1] = m->Sg.y;
		disLocation[2] = m->Sg.z;
		disZeroVec[0] = 0.0;
		disZeroVec[1] = 0.0;
		disZeroVec[2] = 0.0;
		disOrientation[0] = m->curHeading;
		disOrientation[1] = m->curPitch;
		disOrientation[2] = m->curRoll;
		dis_entityEnter (c->team, m,
			&m->cinfo->entityType,
			&m->cinfo->altEntityType,
			disLocation, disZeroVec,
			disZeroVec, disOrientation,
			disZeroVec, &m->disId);
#ifdef DIS_DEBUG
		printf ("Missile Entering m%d %d\n", i, m->disId);
#endif
	}
#endif
	return 0;
}

int	killMissile (c, target)
craft	*c;
craft	*target;
{
#ifdef HAVE_DIS
	double worldLocation[3], entityLocation[3];
	int	target_eid;


	if (c->type == CT_MISSILE || c->type == CT_CANNON) {

	if (target == NULL) {
		target_eid = DIS_ID_NONE;
	}
	else {
		target_eid = target->disId;
	}

	worldLocation[0] = FEETtoMETERS(c->Sg.x);
	worldLocation[1] = FEETtoMETERS(c->Sg.y);
	worldLocation[2] = FEETtoMETERS(c->Sg.z);

/*
 *  killMissile's calling sequence needs to be updated to allow for the
 *  entity detonation location to be passed.
 */

	entityLocation[0] = 0.0;
	entityLocation[1] = 0.0;
	entityLocation[2] = 0.0;
	if (disInUse) {
		dis_detonation (&c->cinfo->entityType,
			ptbl[c->owner].disId,
			target_eid,
			c->disId,
			worldLocation,
			entityLocation);
	}
	}
#endif	
	c->type = CT_FREE;
	return 0;
}

int	lookForImpacts () {

	craft	*c, *m;
	entry	p[MAXPLAYERS], *list, *q, *r, *rprev;
	VPoint	v, s0;
	double	t, d;
	int	i, j;

	for (m=mtbl, i=0; i<MAXPROJECTILES; ++i, ++m) {

		if (m->type == CT_CANNON) {
			lookForCannonImpacts (m);
			continue;
		}
		else if (m->type != CT_MISSILE || m->armTimer > 0.0)
			continue;

		list = (entry *) NULL;
		for (c=ptbl, j=0; j<MAXPLAYERS; ++j, ++c) {

			if (c->type == CT_FREE)
				continue;

/*
 * Reduce the relative motion of this object to a the parametric system
 * of equations:
 *		x(t) = vx * t + s0x
 *		y(t) = vy * t + s0y
 *		z(t) = vz * t + s0z
 *
 * We can then compute the time of perigee (closest pass) along with
 * the associated minimum distance.
 */

			v.x = c->Sg.x - c->prevSg.x - m->Sg.x + m->prevSg.x;
			v.y = c->Sg.y - c->prevSg.y - m->Sg.y + m->prevSg.y;
			v.z = c->Sg.z - c->prevSg.z - m->Sg.z + m->prevSg.z;
			s0.x = c->prevSg.x - m->prevSg.x;
			s0.y = c->prevSg.y - m->prevSg.y;
			s0.z = c->prevSg.z - m->prevSg.z;

/*
 * Compute time of minimum distance between the two objects (note that units
 * here are UPDATE_INTERVAL seconds).
 */

			t = - (v.x * s0.x + v.y * s0.y + v.z * s0.z) /
				(v.x * v.x + v.y * v.y + v.z * v.z);

			if (mdebug)
			printf ("perigee in %g seconds with player %d\n",
				t*deltaT, j);

/*
 *  If the closest pass occurs during this update interval, check for a hit.
 *  We'll build a linked list of all craft that this projectile may strike
 *  during this period, arranged in ascending order by time of "perigee"
 *  (closest pass).  We'll then test for strikes.  If a projectile misses
 *  the first object, then it may have struck subsequent objects in the
 *  list ...
 */

/*
 *  One special case occurs when a target or missile's turn suddenly
 *  changes the perigee time from positive to negative.  If the missile
 *  is within hitting range at t=0 and the time of perigee is negative,
 *  then zap 'em.
 */

			if (t < 0.0) {
				d = sqrt (s0.x *s0.x + s0.y * s0.y +
					s0.z * s0.z);
				if (isMissileHit (d, c)) {
					t = 0.0;
				}
			}

			if (t >= 0.0 && t <= 1.0) {
				q = &p[j];

				q->Sg = m->prevSg;
				q->Sg.x += (m->Sg.x - m->prevSg.x) * t;
				q->Sg.y += (m->Sg.y - m->prevSg.y) * t;
				q->Sg.z += (m->Sg.z - m->prevSg.z) * t;

				if (list == (entry *) NULL) {
					q->next = list;
					list = q;
				}
				else if (list->time > t) {
					q->next = list;
					list = q;
				}
				else {
					for (rprev=list, r=list->next; r != (entry *) NULL;) {
						if (r->time > t) break;
						rprev = r;
						r = r->next;
					}
					rprev->next = q;
					q->next = r;
				}
				q->time = t;
				q->c = c;
				q->min = sqrt (pow(v.x * t + s0.x, 2.0) +
					pow (v.y * t + s0.y, 2.0) +
					pow (v.z * t + s0.z, 2.0) );
#ifdef FLIGHTDEBUG
					if (mdebug)
					printf ("perigee %g feet; craft %d.\n",
						q->min, j);
#endif
			}
		}

/*
 *  Now look for missile hits in the list of perigees.
 */

		for (r=list; r != (entry *) NULL; r=r->next)
			if (isMissileHit (r->min, r->c)) {
				newExplosion (&(r->Sg), 15.0, 10.0, 3.0); 
				killMissile (m, r->c);
#ifdef HAVE_DIS
				/* can only damage local player */
				if (r->c->type != CT_DIS_PLANE)
#endif
				  if (absorbDamage (r->c, 20) == 0)
				    killPlayer (r->c);		  
				break;
			}
	}

	return(0);
}

/*ARGSUSED*/
int isMissileHit (min, c)
double min;
craft	*c; {

	return (min < 50.0) ? 1 : 0;
}

#define IRMaxRange	(15.0 * NM)

int isIRVisible (m, c, t, IRScanSlope)
craft *m, *c;
VPoint	*t;
double	IRScanSlope; {

	VPoint	relPos, tmp;
	int	cstate, mstate;

	if (c->type == CT_FREE)
		return 0;

/*
 *  If the seeker is in clouds, or the target is not at the same level
 *  (e.g seeker is above clouds, but target is below), then the target is
 *  not IR visible.
 */

	if ((mstate = inCloud (m)) == 1) {
		return 0;
	}

	if ((cstate = inCloud (c)) != mstate) {
		return 0;
	}

	tmp = c->prevSg;
	tmp.x -= m->Sg.x;
	tmp.y -= m->Sg.y;
	tmp.z -= m->Sg.z;

	VTransform (&tmp, &m->Itrihedral, t);

	if (sqrt(t->x*t->x + t->y*t->y + t->z*t->z) > IRMaxRange)
		return 0;

	if (t->x <= 0.0)
		return 0;

	relPos.z = t->z / (t->x * IRScanSlope);
	relPos.y = t->y / (t->x * IRScanSlope);

	return (sqrt(relPos.z*relPos.z + relPos.y*relPos.y) > 1.0) ? 0 : 1;
}

int getIRTarget (c, t, scanSlope)
craft	*c;
VPoint	*t;
double	scanSlope; {

	int	i, n;
	craft	*p;
	VPoint	tNew, tMin;
	double	m1, min;

	if (c->curRadarTarget != -1 &&
		isIRVisible(c, &ptbl[c->curRadarTarget], t, scanSlope))
		return c->curRadarTarget;

/*
 *  Look for a target.  Designate the closest one as a new target.
 */

	min = 1000000.0;
	n = -1;
	for (i=0, p=ptbl; i<MAXPLAYERS; ++i, ++p) {
		if (p == c)
			continue;
		if (p->type != CT_FREE)
		if (isIRVisible (c, p, &tNew, scanSlope)) {
			m1 = mag(tNew);
			if (m1 < min) {
				n = i;
				min = m1;
				tMin = tNew;
			}
		}
	}

	*t = tMin;
	return n;
}


/*
 *  Track target using proportional navigation guidance (N = 5).
 */

#define AIM9SLOPE	0.57735

void trackTarget (c)
craft *c; {

	VMatrix mtx, mtx1;
	VPoint	t, t1, v, vrel;
	double	vmag, h, m, maxTurnRate, aMax = 30.0 * a, omegap, omegay;
	double	hs;
	double	deltaRoll, deltaPitch, deltaYaw;
	craft	*target;

/*
 *  Now let's get to target tracking;  the missile won't start tracking until
 *  0.60 seconds has elapsed.  Then, if we don't already have a target
 *  designated, get one.
 */

	if (curTime - c->createTime < 0.60) {
		deltaPitch = 0.0;
		deltaYaw = 0.0;
		goto change;
	}
	else if ((c->curRadarTarget = getIRTarget(c, &t, AIM9SLOPE)) == -1) {

/*
 *  Not target; missile goes ballistic
 */

		deltaPitch = 0.0;
		deltaYaw = 0.0;
		goto change;
#ifdef notdef
#ifdef FLIGHTDEBUG
		if (mdebug)
			printf ("Missile elects to self-destruct\n");
#endif
		newExplosion (&(c->Sg), 15.0, 10.0, 3.0); 
		killMissile (c, (craft *) NULL);
		return;
#endif
	}

/*
 *  We'll steer towards the target at a rate proportional to the
 *  rate of change of the target's position in the missile's XZ (pitch)
 *  and XY (yaw) planes.
 */

	target = &ptbl[c->curRadarTarget];

	v.x = target->Cg.x - c->Cg.x;
	v.y = target->Cg.y - c->Cg.y;
	v.z = target->Cg.z - c->Cg.z;

	VTransform (&v, &c->Itrihedral, &vrel);

	hs = t.x * t.x + t.y * t.y;

	omegay = (vrel.y * t.x - vrel.x * t.y) / hs;

	omegap = (vrel.z * hs - t.z * (vrel.x * t.x + vrel.y * t.y)) /
		(sqrt (hs) * (hs + t.z * t.z));

	vmag = mag (c->Cg);

	deltaPitch = omegap * 5.0 * deltaT;
	deltaYaw   = omegay * 5.0 * deltaT;

	h = sqrt (deltaPitch * deltaPitch + deltaYaw * deltaYaw);

/*
 *  We'll constrain missile turns to about 20 degree/second unless it's velocity
 *  would make that greater than a 30g load factor.
 */

	if ((m=(vmag*vmag - aMax*aMax/4.0)) > 0.0)
		maxTurnRate = atan (aMax / (2.0 * sqrt (m))) * deltaT;
	else
		maxTurnRate = DEGtoRAD(20.0 * deltaT);

#ifdef FLIGHTDEBUG
	if (mdebug)
		printf ("\nturn rate = %g; maxTurnRate = %g\n", h, maxTurnRate);
#endif

	if (h > maxTurnRate) {
		omegap *= maxTurnRate / h;
		omegay *= maxTurnRate / h;
	}

/*
 *  Re-orient the missile and velocity vector.
 */

change:

	deltaRoll = 0.0;

#ifdef FLIGHTDEBUG
	if (mdebug) {
		printf ("Missile changes: pitch/yaw: %g %g (deg).\n",
			RADtoDEG(deltaPitch), RADtoDEG(deltaYaw));
		printf ("position [%g, %g, %g]\n", t.x, t.y, t.z);
		printf ("target pitch/yaw rates: %g, %g (deg/sec)\n",
			RADtoDEG(omegap), RADtoDEG(omegay));
	}
#endif

	buildEulerMatrix (deltaRoll, - deltaPitch, deltaYaw, &mtx);

	VTransform (&(c->Cg), &(c->Itrihedral), &t);
	VTransform (&t, &mtx, &t1);
	VTransform (&t1, &(c->trihedral), &(c->Cg));

	VMatrixMultByRank (&mtx, &c->trihedral, &mtx1, 3);
	c->trihedral = mtx1;
	transpose (&c->trihedral, &c->Itrihedral);
	euler (c);

}
