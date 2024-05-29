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
 
#include "pm.h"
#include "alarm.h"
#include <X11/Xutil.h>

#ifdef REAL_DELTA_T
#include <sys/time.h>

double lastTime = 0.0, sum = 0.0;
long frameCount=0;

#endif

#ifdef HAVE_DIS
long sendCount=0;
#endif

double gameEmptyTime = 0.0;


extern int flightCalculations();
extern int missileCalculations();
extern void doEvents(), doViews(), flapControl(), resupplyCheck();
extern void alarmCheck();
extern void blackBoxInput(), blackBoxOutput();

int cur = 0;
static int warned = 0;


#ifdef HAVE_DIS

#include "dis.h"


#define FT_TO_M(x)	(0.3048*(x))
#define M_TO_FT(x)	((x)/0.3048)


void
disBroadcast (p)
craft	*p;
{
	double location[3], velocity[3], linearAcceleration[3];
	double orientation[3], angularVelocity[3];
	double dt;
	int res;
  	static double base = -1;
	if (base < 0) base = curTime;

	/* HACK: since we don't do any real coordinate conversions,
	   the acm aircrafts will fly around in the center of the
	   earth in the DIS coordinate system */
	location[0] = FT_TO_M(p->Sg.x);
	location[1] = FT_TO_M(p->Sg.y);
	location[2] = FT_TO_M(p->Sg.z);
	velocity[0] = FT_TO_M(p->Cg.x);
	velocity[1] = FT_TO_M(p->Cg.y);
	velocity[2] = FT_TO_M(p->Cg.z);

	dt = curTime - p->disLastTime;
	if (dt > 0.01) {
	    linearAcceleration[0] = FT_TO_M(p->Cg.x - p->disLastCg.x) / dt;
	    linearAcceleration[1] = FT_TO_M(p->Cg.y - p->disLastCg.y) / dt;
	    linearAcceleration[2] = FT_TO_M(p->Cg.z - p->disLastCg.z) / dt;
	} else {
	    linearAcceleration[0] = 0.0;
	    linearAcceleration[1] = 0.0;
	    linearAcceleration[2] = 0.0;
	}
	p->disLastTime = curTime;
	p->disLastCg = p->Cg;

	orientation[0] = p->curHeading;
	if (orientation[0] >= M_PI)
	  orientation[0] -= 2*M_PI;
	orientation[1] = p->curPitch;
	orientation[2] = p->curRoll;
	angularVelocity[0] = p->p;
	angularVelocity[1] = p->q;
	angularVelocity[2] = -p->r; /* acm seem to use z-axis pointing up */
	res = dis_entityState (p->disId, location, velocity, linearAcceleration,
			       orientation, angularVelocity);

#define R2D(x) ((x)*180/3.141593)
	
	if (res) {
	  sendCount++;
/*	  printf("%6.2f: h=%8.2f, p=%8.2f, r=%8.2f, p=%7.2f, q=%7.2f, r=%7.2f\n",
		 curTime-base,
		 R2D(orientation[0]), R2D(orientation[1]), R2D(orientation[2]),
		 R2D(angularVelocity[0]), R2D(angularVelocity[1]), R2D(angularVelocity[2]));*/
/*	  printf("%.2f %.1f %.1f %.1f %.1f %.1f %.1f\n",
		 curTime-base,
		 R2D(orientation[0]), R2D(orientation[1]), R2D(orientation[2]),
		 R2D(angularVelocity[0]), R2D(angularVelocity[1]), R2D(angularVelocity[2]));*/
	  fflush(stdout);
	}
}

void
disGetState (p)
craft	*p;
{
  	static double base = -1;
	double location[3];
	double velocity[3];
	double orientation[3];
	int res;
	double dt;

	if (base < 0) base = curTime;

	res = dis_getEntityState (p->disId, location, velocity, orientation);
	/* if (res != 0) should not happen? */

	p->prevSg = p->Sg;
	p->Sg.x = M_TO_FT(location[0]);
	p->Sg.y = M_TO_FT(location[1]);
	p->Sg.z = M_TO_FT(location[2]);
	p->Cg.x = M_TO_FT(velocity[0]);
	p->Cg.y = M_TO_FT(velocity[1]);
	p->Cg.z = M_TO_FT(velocity[2]);

	dt = curTime - p->disLastTime;
	
	p->curHeading = orientation[0];
	if (p->curHeading < 0.0)
	    p->curHeading += 2*M_PI;
	p->curPitch = orientation[1];
	p->curRoll = orientation[2];
	if (p->curRoll > M_PI)
	    p->curRoll -= 2*M_PI;

/*
 *  Aircraft trihedral is needed to calculate radar information; we
 *  probably do not need to update this every update frame.
 */

	buildEulerMatrix (p->curRoll, p->curPitch, p->curHeading,
		&(p->trihedral));
	transpose (&p->trihedral, &p->Itrihedral);

/*printf("%d: h=%.0f, p=%.0f, r=%.0f\n",
       p->disId, R2D(p->curHeading), R2D(p->curPitch), R2D(p->curRoll));*/
/*printf("%.2f %.1f %.1f %.1f\n",
       curTime-base, R2D(orientation[0]), R2D(p->curPitch), R2D(p->curRoll));*/
}

#endif



int
redraw()
{

	int	i, gameEmpty;
	craft	*p;
	struct timeval tp;
	struct timezone tzp;

	gettimeofday (&tp, &tzp);
	curTime = (double) tp.tv_sec + (double) tp.tv_usec / 1000000.0;

	if (lastTime == 0.0) {
		lastTime = curTime - 0.1;
	}

/*
 *  Check if "curTime" ever equals "lastTime", if so, warn that REAL_DELTA_T
 *  probably should not be used on this system.
 */

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

#ifdef REAL_DELTA_T
	deltaT = curTime - lastTime;

/*
 *  If the amount of real elapsed time is unreasonably long, then
 *  make it one update interval.
 */

	if (deltaT > (double)((UPDATE_INTERVAL / 1000000.0) * 4)) {
		deltaT = UPDATE_INTERVAL / 1000000.0;
	}
#else
	deltaT = UPDATE_INTERVAL / 1000000.0;
#endif /* REAL_DELTA_T */

	lastTime = curTime;
	halfDeltaTSquared = 0.5 * deltaT * deltaT;


#ifdef HAVE_DIS
	if (disInUse) {
		dis_setTime (curTime);
		dis_receive ();
	}
#endif

	gameEmpty = 1;

	for ((i=0, p=ptbl); i<MAXPLAYERS; (++i, ++p)) {
		if ((p->type == CT_PLANE) && !(p->flags & FL_BLACK_BOX)) {
			gameEmpty = 0;
			doEvents (p);

			if (flightCalculations (p) == 1) {
				killPlayer (p);
			}
			doWeaponUpdate (p);
			flapControl (p);
#ifdef HAVE_DIS	  
			if (disInUse)
			    disBroadcast (p);
#endif	      
		}
	}

	for ((i=0, p=ptbl); i<MAXPLAYERS; (++i, ++p)) {
		if ((p->type == CT_DRONE) && !(p->flags & FL_BLACK_BOX)) {

			if (droneCalculations (p) == 1) {
				killPlayer (p);
			}

/*
 *  Calling flightCalculations makes drones adhere to the flight model.
 */
			if (flightCalculations (p) == 1) {
				killPlayer (p);
			}
			doWeaponUpdate (p);
			flapControl (p);
#ifdef HAVE_DIS	  
			if (disInUse)
			    disBroadcast (p);
#endif	      
		}
	}

#ifdef HAVE_DIS
	if (disInUse) {
	  for ((i=0, p=ptbl); i<MAXPLAYERS; (++i, ++p)) {
	      if (p->type == CT_DIS_PLANE)
	          disGetState (p);
	  }
	  for ((i=0, p=mtbl); i<MAXPROJECTILES; (++i, ++p)) {
	      if (p->type == CT_DIS_MUNITION)
	          disGetState (p);
	  }
	}
#endif	

	for ((i=0, p=ctbl); i<MAXPLAYERS; (++i, ++p)) {
	  if (p->type == CT_FREE) continue;
	  doEvents(p);
	}

	blackBoxInput();

	for ((i=0, p=mtbl); i<MAXPROJECTILES; (++i, ++p)) {
		if (p->type == CT_MISSILE) {
			if (missileCalculations (p) == 1) {
				killMissile (p, (craft *) NULL);
			}
#ifdef HAVE_DIS	  
			else if (disInUse) {
			    disBroadcast (p);
			}
#endif	
		}
		else if (p->type == CT_CANNON || p->type == CT_DIS_CANNON) {
			if (cannonCalculations (p) == 1) {
				killMissile (p, (craft *) NULL);
			}
		}
		else if (p->type == CT_EXPLOSION) {
			-- (p->flameDuration);
			if ((-- p->duration) <= 0)
				p->type = CT_FREE;
		}
	}

	lookForImpacts ();

#ifdef REAL_DELTA_T
	doViews();
#else
	if (cur++ % REDRAW_EVERY == 0)
		doViews ();
#endif

	blackBoxOutput();

	alarmCheck (deltaT);

#ifdef WATCH_FRAME_RATE
	sum += deltaT;
	frameCount++;
	if (frameCount % 100 == 0) {
		printf ("rate is %lf frames/second\n", (double)frameCount/sum );
#ifdef HAVE_DIS
		if (disInUse)
		  printf ("DIS send rate is %lf packets/second\n",
			  (double)sendCount/sum);
		sendCount = 0;
#endif	
		sum = deltaT;
		frameCount = 1;
	}
#endif	


	if (gameEmpty) {
		if ((gameEmptyTime += deltaT) > MAX_GAME_IDLE_SECONDS) {
			exit (0);
		}
	}
	else {
		gameEmptyTime = 0.0;
	}

	return 0;

}
