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
#include <stdio.h>
#include <math.h>

#include "modelacm.h"

extern void euler(), buildEulerMatrix(), transpose();

/*
 *  Determine the time, relative to the beginning of this time interval,
 *  that the landing gear first make contact with the ground.  Return -1
 *  if they don't during this interval.
 */

extern double elevatorSetting();

#ifdef FLIGHTDEBUG
#define DEBUG1
extern int debug;
#endif

double
groundContactTime (c, contactSg)
register craft	*c;
VPoint	*contactSg;
{
	register craftType	*p = c->cinfo;
	register double		tm, tn;
	VPoint			v, s0, s0n, Sg, gear;

/*
 *  In this function, our computation of wheel position differs slightly
 *  with the one used in groundDynamics().  This can introduce a problem where
 *  this procedure thinks we are not in contact with the ground while
 *  groundDynamics() does.  We'll define an error margin to prevent that
 *  condition.
 */
#define EPSILON	0.01

#ifdef DEBUG1
	double cm, cn, cosTheta, sinTheta;
#endif

	tm = tn = 2.0;

	Sg = c->prevSg;
	v.x = c->Sg.x - c->prevSg.x;
	v.y = c->Sg.y - c->prevSg.y;
	v.z = c->Sg.z - c->prevSg.z;

/*
 *  If we're not on the ground now, and we're not descending, we won't touch
 *  down during this time interval.
 */

	if (v.z <= 0.0)
		return -1.0;

/*
 *  Determine the point in world axes relative to the CG that represents the
 *  "bottom" of the fully extended main gear with a fully extended oleo.
 *  The determine if (and when) it makes contact with the ground.
 *
 *  Remember, the main gear are considered one unit for purposes of ACM.
 */

	gear = p->rm;
	gear.z += p->Gm + p->cmMax;
	VTransform (&gear, &(c->trihedral), &s0);
	s0.x += c->prevSg.x;
	s0.y += c->prevSg.y;
	s0.z += c->prevSg.z;
	if (s0.z > (0.0 + EPSILON)) {
		*contactSg = c->prevSg;
		c->flags |= FL_GND_CONTACT;
#ifdef DEBUG1
		printf ("mains already in contact\n");
		cosTheta = cos (-c->curPitch);
		sinTheta = sin (-c->curPitch);
		cm = - (contactSg->z + p->rm.x * sinTheta) / cosTheta -
			p->Gm - p->rm.z;
		cn = - (contactSg->z + p->rn.x * sinTheta) / cosTheta -
			p->Gn - p->rn.z;
		printf ("cm/cn cmMax/cnMax %.4f %.4f %.4f %.4f\n\n",
			cm, cn , p->cmMax, p->cnMax);
#endif
		return 0.0;
	}

	if ((s0.z + v.z) > 0.0) {
		tm = - s0.z / v.z;
	}

/*
 *  Now do the same thing for the nose gear.
 */

	gear = p->rn;
	gear.z += p->Gn + p->cnMax;
	VTransform (&gear, &(c->trihedral), &s0n);
	s0n.x += c->prevSg.x;
	s0n.y += c->prevSg.y;
	s0n.z += c->prevSg.z;

	if (s0n.z > (0.0 + EPSILON)) {
		*contactSg = c->prevSg;
		c->flags |= FL_GND_CONTACT;
#ifdef DEBUG1
		printf ("nose already in contact\n");
#endif
		return 0.0;
	}

	if ((s0n.z + v.z) > 0.0) {
		tn = - s0n.z / v.z;
	}

/*
 *  Return the time of initial ground contact (or minus one).
 */

	if (tm != 2.0 || tn != 2.0) {
		c->flags |= FL_GND_CONTACT;
		if (tn < tm) {
			tm = tn;
		}

		if (tm < 0.0) {
			return -1.0;
		}
#ifdef DEBUG1
		contactSg->x = Sg.x + tm * v.x;
		contactSg->y = Sg.y + tm * v.y;
		contactSg->z = Sg.z + tm * v.z;

		printf ("Ground Contact, time t + %.5f sec\n", tm * deltaT);
		cosTheta = cos (-c->curPitch);
		sinTheta = sin (-c->curPitch);
		cm = - (contactSg->z + p->rm.x * sinTheta) / cosTheta -
			p->Gm - p->rm.z;
		cn = - (contactSg->z + p->rn.x * sinTheta) / cosTheta -
			p->Gn - p->rn.z;
		printf ("cm/cn cmMax/cnMax %.4f %.4f %.4f %.4f\n\n",
			cm, cn , p->cmMax, p->cnMax);
#endif
		return tm * deltaT;
	}
	else
		return -1.0;

}

/*
 *  Manage pitch and translation while we're on the ground.
 */

int
groundDynamics (c, startT, CL, CD, CM, w, qS)
craft	*c;
double	startT, CL, CD, CM, w, qS;
{

	register double 	sinTheta, cosTheta;
	register craftType	*p;
	double		theta, theta_dot, dT, cm, cn, cm_dot, cn_dot, m;
	double		t, next_t, muStatic;
	double		muKinetic, Mu, nMu;
	double		lift, drag, pitch_moment, angle;
	double		v, sinAlphaP, cosAlphaP;
	VPoint		F, M, mt, Sg, Cg, Fm, Fn, FnMu, FmMu, r, tmpPt;
	VMatrix		turn, mtx;
	double		theta_damp;
	int		done = 0;


	Fm.x = Fm.y = Fm.z = 0.0;
	Fn.x = Fn.y = Fn.z = 0.0;

	m = w / 32.14;

	dT = 0.002;

	p = c->cinfo;

	theta_damp = 0.25 * p->wingS * c->rho * p->c * p->c * p->Cmq;

/*
 *  Grab initial conditions
 *
 *  We restrict the degrees of freedom here to x and z translations and
 *  pitch rotations.  Yawing induced by nosewheel steering is controlled
 *  by some special code at the end of this procedure.
 *
 *  Note that the pitch of the aircraft is the value theta, with a positive
 *  value representing a downward pitch.
 */

/*
 *  First, change the velocity into a two-demensional vector.
 */
 
	v = mag (c->Cg);
	Cg.z = c->Cg.z;
	Cg.y = 0.0;
	Cg.x = sqrt (v * v - Cg.z * Cg.z);

	Sg.x = Sg.y = 0.0;
	Sg.z = c->Sg.z;
	theta = - c->curPitch;
	theta_dot = - c->q;

#ifdef DEBUG
	sinTheta = sin (theta);
	cosTheta = cos (theta);
	cm = - (Sg.z + p->rm.x * sinTheta) / cosTheta - p->Gm - p->rm.z;
	cn = - (Sg.z + p->rn.x * sinTheta) / cosTheta - p->Gn - p->rn.z;
	printf ("start of groundDynamics\n");
	printf ("time    z     theta     cm      cn        Cg.x\n");
	printf ("%.3f  %.3f  %.2f  %f  %f   %f ft/sec\n", t, -Sg.z,
		theta * 180.0 / M_PI, cn, cm, Cg.x);
#endif

#ifdef notdef
	printf ("theta = %f, theta_dot = % f\n ", theta * 180.0 / M_PI,
		theta_dot);
#endif

/*
 *  Since the time interval is very short, we'll treat the aero forces
 *  as constants.
 */

	lift = CL * qS;
	drag = CD * qS;
	pitch_moment = CM * (c->alpha - elevatorSetting(c, qS, w) *
			p->effElevator) * p->c * qS;

	if (c->flags & FL_BRAKES) {
		muStatic = p->muBStatic;
		muKinetic = p->muBKinetic;
	}
	else {
		muStatic = p->muStatic;
		muKinetic = p->muKinetic;
	}

/*
 *  Since lift and drag are constants in this model, well make their x and z
 *  force components constant as well.
 */
 
	sinAlphaP = sin (c->alpha + theta);
	cosAlphaP = cos (c->alpha + theta);

	for (t = startT; !done; t = next_t) {

		next_t = t + dT;
		if (next_t >= deltaT) {
			done = 1;
			dT = deltaT - t;
		}

		sinTheta = sin (theta);
		cosTheta = cos (theta);

/*
 *  cm and cn are the current strut positions (i.e. how compressed an oleo
 *  currently is). Zero corresponds to a fully compressed oleo.  Positive
 *  values mean that the oleo is extended by that amount.  Units are feet.
 */

		cm = - (Sg.z + p->rm.x * sinTheta) / cosTheta - p->Gm - p->rm.z;
		cn = - (Sg.z + p->rn.x * sinTheta) / cosTheta - p->Gn - p->rn.z;

/*
 *  cm_dot and cn_dot are the instantaneous strut motion rates.  They are used
 *  to compute the damping forces in each strut.
 */

		cm_dot = -( p->rm.x * theta_dot + Cg.z / cosTheta +
			((Sg.z + p->rm.x * sinTheta) * sinTheta * theta_dot) /
			cosTheta * cosTheta );

		cn_dot = -( p->rn.x * theta_dot + Cg.z / cosTheta +
			((Sg.z + p->rn.x * sinTheta) * sinTheta * theta_dot) /
			cosTheta * cosTheta );

/*
 *  If cm or cn ever go negative, then that oleo -- and strut -- are considered
 *  to have collaped.
 */

		if (cm < 0.0) {
			return 1;
		}

		if (cn < 0.0) {
			return 1;
		}

/* 
 *  Compute the force at each landing gear strut location.  Note that damping
 *  only occurs "on the downstroke".  That's because the wheel isn't physically
 *  attached to the ground.
 */

		if (cm < p->cmMax) {
			Fm.z = -(p->Km * (p->cmMax - cm));
			if (cm_dot < 0.0)
			    Fm.z += p->Dm * cm_dot;
		}
		else
			Fm.z = 0.0;

		if (cn < p->cnMax) {
			Fn.z = -(p->Kn * (p->cnMax - cn));
			if (cn_dot < 0.0)
			    Fn.z += p->Dn * cn_dot;
			if (cn < 0.1)
			    /* add resistance in the last 0.1 feet */
			    Fn.z += -100*(p->Kn * (0.1 - cn));
		}
		else
			Fn.z = 0.0;


/*
 *  Add moments contributed by oleo compression and oleo movement
 */

		VCrossProd (&Fm, &p->rm, &M);

		VCrossProd (&Fn, &p->rn, &mt);
		M.x += mt.x;
		M.y += mt.y;
		M.z += mt.z;

/*
 *  Add moments contributed by tire friction (we'll use rm/rn for
 *  the sake of speed here instead of calculating a more accurate
 *  tire / ground contact position.  Note that FmMu and FnMu are forces
 *  relative to the body axes, not world axes.
 */

		if (fabs(Cg.x) < 0.5) {
			nMu = p->muStatic;
			Mu = muStatic;
		}
		else {
			nMu = p->muKinetic;
			Mu = muKinetic;
		}

		if (Cg.x < 0.0)
			Mu = -Mu;
		else if (Cg.x == 0.0)
			Mu = 0.0;

		if (cm < p->cmMax) {
			FmMu.x = Fm.z * cosTheta * cosTheta * Mu;
			FmMu.y = 0.0;
			FmMu.z = Fm.z * cosTheta * sinTheta * Mu;
			VCrossProd (&FmMu, &p->rm, &mt);
			M.x += mt.x;
			M.y += mt.y;
			M.z += mt.z;
		}


		if (cn < p->cnMax) {
			FnMu.x = Fn.z * cosTheta * cosTheta * nMu;
			FnMu.y = 0.0;
			FnMu.z = Fn.z * cosTheta * sinTheta * nMu;
			VCrossProd (&FnMu, &p->rn, &mt);
			M.x += mt.x;
			M.y += mt.y;
			M.z += mt.z;
		}

		M.y -= pitch_moment - theta_damp * v * theta_dot;

/*
 *  Sum forces in (simplified) world axes
 */

		F.x = - (Fm.z + Fn.z) * sinTheta;
		F.x += (Fm.z + Fn.z) * cosTheta * Mu;
		F.x += cosTheta * c->curThrust;
		F.x -= sinAlphaP * lift;
		F.x -= cosAlphaP * drag;
		F.y = 0.0;
		F.z = w + (Fm.z + Fn.z) * cosTheta;
		F.z += sinTheta * c->curThrust;
		F.z -= cosAlphaP * lift;
		F.z -= sinAlphaP * drag;

/*
 *  Update position and theta
 */

		Sg.x += Cg.x * dT + 0.5 * F.x / m * dT * dT;
		Sg.y += Cg.y * dT + 0.5 * F.y / m * dT * dT;
		Sg.z += Cg.z * dT + 0.5 * F.z / m * dT * dT;

		Cg.x += F.x / m * dT;
		Cg.y += F.y / m * dT;
		Cg.z += F.z / m * dT;

		theta += theta_dot * dT + 0.5 * M.y / p->I.m[1][1] * dT * dT;
		theta_dot += M.y / p->I.m[1][1] * dT;

/*
 *  Well, okay, it's time for a hack.  To simulate dragging the tail, we'll
 *  constrain theta to be less than 20 degrees (for now).  The right way to
 *  do this would be to determine if the tail contact point is positive and if
 *  it is, then limit theta.
 */

		if (theta < DEGtoRAD(-20.0)) {
			theta = DEGtoRAD(-20.0);
			theta_dot = 0.0;
		}

#ifdef DEBUG
		if (debug) {
		    printf ("%.3f  %.3f  %.2f  %f  %f", t+dT, -Sg.z,
			RADtoDEG(theta), cm, cn);
		    printf ("  %f ft/sec\n", Cg.x);
		}
#endif

	}

/*
 *  Well, now we've calculated how far we've moved relative to our starting
 *  point (Sg.x), what our CG's new altitude is (Sg.z), and our ending pitch
 *  (theta).  We'll use this information as an input to nose-wheel steering
 *  control.
 */

	c->curNWDef = c->Sa * c->cinfo->maxNWDef;

	if (c->curNWDef != 0.0 && v < p->maxNWS && cn < p->cnMax) {

		tmpPt.x = c->cinfo->gearD2;
		tmpPt.y = c->cinfo->gearD1 / tan(c->curNWDef);
		tmpPt.z = 0.0;
		angle = Sg.x / tmpPt.y;

/*
 *  Nose wheel steering mode.
 *  Relocate the aircraft and its trihedral (this code assumes that the
 *  plane is rolling on a flat surface (i.e. z is constant).
 */

		VTransform(&tmpPt, &(c->trihedral), &r);

		VIdentMatrix (&turn);
		turn.m[0][3] = - c->Sg.x - r.x;
		turn.m[1][3] = - c->Sg.y - r.y;
		turn.m[2][3] = - c->Sg.z;
		VRotate (&turn, ZRotation, angle);
		turn.m[0][3] = turn.m[0][3] + c->Sg.x + r.x;
		turn.m[1][3] = turn.m[1][3] + c->Sg.y + r.y;
		turn.m[2][3] = turn.m[2][3] + c->Sg.z;
		VTransform (&(c->Sg), &turn, &tmpPt);
		c->Sg = tmpPt;

		VIdentMatrix (&turn);
		VRotate (&turn, ZRotation, angle);
		mtx = c->trihedral;
		VMatrixMultByRank (&mtx, &turn, &(c->trihedral), 3);
		VTransform (&(c->Cg), &turn, &tmpPt);
		c->Cg = tmpPt;

		euler (c);

		VSetPoint (tmpPt, 1.0, 0.0, 0.0);
		VTransform (&tmpPt, &(c->trihedral), &r);

	}
	else {
		VSetPoint (tmpPt, 1.0, 0.0, 0.0);
		VTransform (&tmpPt, &(c->trihedral), &r);
		c->Sg.x += Sg.x * r.x;
		c->Sg.y += Sg.x * r.y;	/* yes, I really want Sg.x */

	}

	c->Cg.x = Cg.x * r.x;
	c->Cg.y = Cg.x * r.y;
	c->Cg.z = Cg.z;
	c->groundCgx = Cg.x;

	c->Sg.z = Sg.z;
	c->curPitch = - theta;
	c->curRoll = 0.0;
	c->q = - theta_dot;

	buildEulerMatrix (c->curRoll, c->curPitch, c->curHeading,
		&(c->trihedral));
	transpose (&c->trihedral, &c->Itrihedral);

/*
 *  Is the plane still in contact with the ground?
 */

	if ((cm >= p->cmMax) && (cn >= p->cnMax)) {
	    cosTheta = cos(theta);
	    sinTheta = sin(theta);
	    cm = - (Sg.z + p->rm.x * sinTheta) / cosTheta - p->Gm - p->rm.z;
	    cn = - (Sg.z + p->rn.x * sinTheta) / cosTheta - p->Gn - p->rn.z;
	    if ((cm >= p->cmMax) && (cn >= p->cnMax)) {
		c->flags &= ~FL_GND_CONTACT;
		c->groundCgx = 0.0;
	    }
	}

#ifdef DEBUG1
	if (debug) {
		printf ("Sg.z = %.3f  theta=%.2f  cm/cn %f  %f", -Sg.z,
			RADtoDEG(theta), cm, cn);
		printf ("  %f ft/sec\n", Cg.x);
	}
#endif


	return 0;

}
