/*
 *  $Id: distest.c,v 0.10 1995/01/13 14:16:09 mats Exp $
 *
 *  Copyright (C) 1995  Mats Lofkvist  CelsiusTech Electronics AB
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
#include <signal.h>
#include <math.h>
#include <sys/time.h>

/*
 *  This is an ugly hack, for now ...
 */
#include "../npsdis/pdu.h"
#include "dis.h"

#define D2R(x) ((x)*3.141593/180.0)
#define R2D(x) ((x)*180.0/3.141593)


#define REALTIME
#define ABSOLUTETIME

#define DT 0.05
#define RSKIP 10

#define MAX_ENTITY 256


#ifndef DEVICE
#define DEVICE "le0"
#endif


static double sysTime(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec + tv.tv_usec/1000000.0;
}


#ifdef SOL2

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void usleep(unsigned useconds)
{
  struct timeval tv;
  fd_set myset;
  int seconds;

  seconds = 0;
  while (useconds >= 1000000)
    {
      seconds++;
      useconds -= 1000000;
    }

  tv.tv_sec = seconds;
  tv.tv_usec = useconds;

  FD_ZERO(&myset);

  select(1, &myset, &myset, &myset, &tv);
}

#endif


static double simtime = 0.0;

typedef struct {
  dis_entity_type etype;
  int force;
  int remote;
} Entity;

static Entity inUse[MAX_ENTITY];


static void entityEnterCb(int eid, dis_entity_type *etype, int force)
{
  printf("  %5.2f: enter %d <>\n", fmod(simtime, 60.0), eid);

  if (inUse[eid].force != -1)
    fprintf(stderr, "duplicate eid\n");

  if (eid >= MAX_ENTITY)
    fprintf(stderr, "too large eid\n");

  inUse[eid].etype = *etype;
  inUse[eid].force = force;
  inUse[eid].remote = 1;
}

static void entityExitCb(int eid)
{
  printf("  %4.1f: exit %d <>\n", fmod(simtime, 60.0), eid);
  
  inUse[eid].force = -1;
}

static void fireCb(int ftype, int firingEid, int targetEid,
		   double time, int rounds,
		   double location[3], double velocity[3],
		   double range)
{
}

static void detonationCb(int ftype, int firingEid, int targetEid,
			 double time, double worldLocation[3],
			 double entityLocation[3])
{
  printf("  %4.1f: detonation %d @ %.0f %.0f %.0f (%d from %d)\n",
	 fmod(simtime, 60.0),
	 targetEid, worldLocation[0], worldLocation[1], worldLocation[2],
	 ftype, firingEid);
}

/* euler = [psi, theta, phi] */
#define PSI	0
#define THETA	1
#define PHI	2

void eul2mat(double eul[3], double factor, double mat[3][3])
{
  double feul[3], sin_psi, cos_psi, sin_theta, cos_theta, sin_phi, cos_phi;

  feul[PSI] = factor * eul[PSI];
  feul[THETA] = factor * eul[THETA];
  feul[PHI] = factor * eul[PHI];

  sin_psi = sin(feul[PSI]);
  cos_psi = cos(feul[PSI]);
  sin_theta = sin(feul[THETA]);
  cos_theta = cos(feul[THETA]);
  sin_phi = sin(feul[PHI]);
  cos_phi = cos(feul[PHI]);

  mat[0][0] =  cos_theta * cos_psi;
  mat[0][1] =  sin_phi * sin_theta * cos_psi - cos_phi * sin_psi;
  mat[0][2] =  cos_phi * sin_theta * cos_psi + sin_phi * sin_psi;
  mat[1][0] =  cos_theta * sin_psi;
  mat[1][1] =  sin_phi * sin_theta * sin_psi + cos_phi * cos_psi;
  mat[1][2] =  cos_phi * sin_theta * sin_psi - sin_phi * cos_psi;
  mat[2][0] = -sin_theta;
  mat[2][1] =  sin_phi * cos_theta;
  mat[2][2] =  cos_phi * cos_theta;
}

#define EPSILON 0.000001

void mat2eul(double mat[3][3], double eul[3])
{
  double sin_theta;

  sin_theta = -mat[2][0];
  if (fabs(sin_theta > (1.0 - EPSILON)))
    {
      eul[PHI] = 0.0;
      if (sin_theta > 0)
	eul[THETA] = M_PI/2;
      else
	eul[THETA] = -M_PI/2;
      eul[PSI] = atan2(-mat[0][1], mat[1][1]);
    }
  else
    {
      eul[PSI] = atan2(mat[1][0], mat[0][0]);
      eul[THETA] = asin(sin_theta);
      eul[PHI] = atan2(mat[2][1], mat[2][2]);
    }
}

void matmul(double m1[3][3], double m2[3][3], double res[3][3])
{
  int r, c;
  double tmp[3][3];
  
  for (r = 0; r < 3; r++)
    for (c = 0; c < 3; c++)
      tmp[r][c] = m1[r][0]*m2[0][c] + m1[r][1]*m2[1][c] + m1[r][2]*m2[2][c];
  for (r = 0; r < 3; r++)
    for (c = 0; c < 3; c++)
      res[r][c] = tmp[r][c];
}


void cleanup(void)
{
  dis_close();

  exit(0);
}

int main(int argc, char *argv[])
{
  int sendresult, eid, loop, teid, err;
  double position[3] = {0,0,0};
  double velocity[3] = {100,0,0};
  double acceleration[3] = {0,1,0};
  double attitude[3] = {D2R(90.0),0,0};
  /* rotation = angular velocity */
  double rotation[3] = {D2R(60.0),D2R(10.0),D2R(-13.0)};
  double roteul[3];
  double curmat[3][3], rotmat[3][3];
  double rpos[3], rvel[3], ratt[3];
  double lasttime, dt, halfdtsq;
  double wloc[3], eloc[3];

  eul2mat(attitude, 1.0, curmat);

  for (eid = 0; eid < MAX_ENTITY; eid++)
    inUse[eid].force = -1;

  if (dis_init(DEVICE, 3000, 1, 1, argc, entityEnterCb, entityExitCb,
	       fireCb, detonationCb) != 0)
    {
      fprintf(stderr, "dis_init failed\n");
      exit(17);
    }

  signal(SIGINT, cleanup);

  dis_setDRThresholds(5.0, D2R(0.5));

#ifdef REALTIME  
  simtime = sysTime();
#else
  simtime = 0.0;
#endif
#ifdef ABSOLUTETIME
  dis_setTimeAbsolute();
#else  
  dis_setTime(simtime);
#endif  

  if (argc > 1)
    {
      dis_entity_type f16 = { 1, 2, 225, 1, 3, 3, 0 };
      dis_entity_type MiG29 = { 1, 2, 222, 1, 2, 5, 0 };
      int team_one = 1;

      if (dis_entityEnter(team_one, &f16, &MiG29,
      		position, velocity, acceleration,
		attitude, rotation, &eid) == 0)
	fprintf(stderr, "T %5.2f: %d <> enter ok\n",
		fmod(simtime, 60.0), eid);
      if (eid >= MAX_ENTITY)
	{
	  fprintf(stderr, "to big eid\n");
	  exit(17);
	}
      inUse[eid].etype = f16;
      inUse[eid].force = team_one;
      inUse[eid].remote = 0;
    }

  loop = 0;
  while (1)
    {
      loop++;
      usleep((int) (1000000*DT));
      lasttime = simtime;
#ifdef REALTIME  
      simtime = sysTime();
#else
      simtime += DT;
#endif  
      dt = simtime - lasttime;
#ifdef ABSOLUTETIME
      dis_setTimeAbsolute();
#else  
      dis_setTime(simtime);
#endif
      err = dis_receive();
      if (err != 0)
	printf("receive err = %d\n", err);
      for (eid = 0; eid < MAX_ENTITY; eid++)
	if (inUse[eid].force != -1)
	  {
	    if (inUse[eid].remote == 1)
	      {
		/* remote */
		if (loop % RSKIP == 0)
		  {
		    if (dis_getEntityState(eid, rpos, rvel, ratt) != 0)
		      {
			printf("bar! %d\n", eid);
			exit(17);
		      }
		    printf("R %5.2f: %d @ %.0f %.0f %.0f [ %.2f %.2f %.2f ]\n",
			   fmod(simtime, 60.0), eid, rpos[0], rpos[1], rpos[2],
			   R2D(ratt[0]), R2D(ratt[1]), R2D(ratt[2]));
		  }
	      } /* if remote == 1 */
	    else
	      {
		/* local */
		halfdtsq = dt*dt/2;
		position[0] += dt*velocity[0] + halfdtsq*acceleration[0];
		position[1] += dt*velocity[1] + halfdtsq*acceleration[1];
		position[2] += dt*velocity[2] + halfdtsq*acceleration[2];
		velocity[0] += dt*acceleration[0];
		velocity[1] += dt*acceleration[1];
		velocity[2] += dt*acceleration[2];
		
		roteul[0] = rotation[2];
		roteul[1] = rotation[1];
		roteul[2] = rotation[0];
		eul2mat(roteul, dt, rotmat);
		matmul(curmat, rotmat, curmat);
		mat2eul(curmat, attitude);
		
		sendresult = dis_entityState(eid, position, velocity,
					     acceleration, attitude, rotation);
		if (sendresult == 1)
		  printf("T %5.2f: %d @ %5.0f %5.0f %5.0f"
			 "     [ %8.2f %8.2f %8.2f ]\n",
			 fmod(simtime, 60.0),
			 eid, position[0], position[1], position[2],
			 R2D(attitude[0]), R2D(attitude[1]), R2D(attitude[2]));
		else if (sendresult < 0)
		  {
		    fprintf(stderr, "  %5.2f foo? %d\n",
			    fmod(simtime, 60.0), eid);
		    exit(17);
		  }
		
		if ((loop % 100) == 0)
		  {
		    for (teid = 0; teid < MAX_ENTITY; teid++)
		      if (inUse[teid].force != -1 && inUse[teid].remote == 1)
			break;
		    if (teid < MAX_ENTITY)
		      {
			/* found a target */
			wloc[0] = 1000;
			wloc[1] = 2000;
			wloc[2] = 3000;
			eloc[0] = 4;
			eloc[1] = 5;
			eloc[2] = 6;
			if ((loop % 200) == 0)
			  {
			    printf("  %5.2f %d detonating M61A1 at %d ",
				   fmod(simtime, 60.0), eid, teid);
			    err = dis_detonation(DIS_FIRE_M61A1, eid, teid,
						 0, 0, wloc, eloc);
			    printf("%d\n", err);
			  }
			else
			  {
			    printf("  %5.2f %d detonating AIM9M at %d ",
				   fmod(simtime, 60.0), eid, teid);
			    err = dis_detonation(DIS_FIRE_AIM9M, eid, teid,
						 0, 0, wloc, eloc);
			    printf("%d\n", err);
			  }
		      } /* if found target */
		  } /* if loop % 100 == 0 */
		
		/* 1000 = 50s with DT = 0.05 */
		if (loop == 1000)
		  {
		    printf("  %5.2f exiting %d\n", fmod(simtime, 60.0), eid);
		    dis_entityExit(eid);
		    inUse[eid].force = -1;
		  }
	      } /* if remote != 1 */
	  } /* if etype != -1 */
    } /* while 1 */

  return 0;
}
