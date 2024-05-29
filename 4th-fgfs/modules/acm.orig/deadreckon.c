/*
 * $Id: deadreckon.c,v 0.3 1995/01/13 14:16:09 mats Exp $
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

static char rcsid[] = "$Id: deadreckon.c,v 0.3 1995/01/13 14:16:09 mats Exp $";


#include <math.h>
#include "deadreckon.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#define EPSILON (1.5e-06)

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define PSI    0
#define THETA  1
#define PHI    2

#define UP (M_PI/2)
#define DOWN (-M_PI/2)


/*
 *  e u l e r T o Q u a t e r n i o n
 *
 *  Convert euler[psi, theta, phi] to quaternion q.
 *  Angles are defined in radians.
 */

void eulerToQuaternion(double euler[3], Quaternion *q)
{
    double coshpsi, coshtheta, coshphi, sinhpsi, sinhtheta, sinhphi;
    double halfpsi, halftheta, halfphi;

    halfpsi   = 0.5 * euler[PSI];
    halftheta = 0.5 * euler[THETA];
    halfphi   = 0.5 * euler[PHI];

    coshpsi   = cos(halfpsi);
    coshtheta = cos(halftheta);
    coshphi   = cos(halfphi);
    sinhpsi   = sin(halfpsi);
    sinhtheta = sin(halftheta);
    sinhphi   = sin(halfphi);

    q->scalar = coshpsi*coshtheta*coshphi + sinhpsi*sinhtheta*sinhphi;
    
    q->vector[X_AXIS] = coshpsi*coshtheta*sinhphi - sinhpsi*sinhtheta*coshphi;
    q->vector[Y_AXIS] = coshpsi*sinhtheta*coshphi + sinhpsi*coshtheta*sinhphi;
    q->vector[Z_AXIS] = sinhpsi*coshtheta*coshphi - coshpsi*sinhtheta*sinhphi;
}


/*
 *  a n g u l a r V e l o c i t y T o Q u a t e r n i o n
 *
 *  Convert avel[x-axis rotation, y ditto, z ditto] to quaternion q.
 *  Angular velocity is defined in radians per second.
 */

void angularVelocityToQuaternion(double avel[3], Quaternion *q)
{
  double scale;

  q->scalar = sqrt(avel[0]*avel[0] + avel[1]*avel[1] + avel[2]*avel[2]);

  if (q->scalar > EPSILON)
    {
      scale = 1.0 / q->scalar;
      q->vector[0] = avel[0] * scale;
      q->vector[1] = avel[1] * scale;
      q->vector[2] = avel[2] * scale;
    }
  else
    {
      q->scalar = 0.0;
      q->vector[0] = 1.0;
      q->vector[1] = 0.0;
      q->vector[2] = 0.0;
    }
}


/*
 *  q u a t e r n i o n T o E u l e r
 *
 *  Convert the quaternion q to euler[psi, theta, phi].
 *  Angles are defined in radians.
 */

void quaternionToEuler(Quaternion *q, double euler[3])
{
    double sin_theta;      /* sin of pitch */

    double R00, R01, R02;  /* rotation matrix elements */
    double      R11, R12;
    double      R21, R22;  

    R02 = 2.0 * ((q->vector[X_AXIS] * q->vector[Z_AXIS])
		 - (q->scalar * q->vector[Y_AXIS]));

    sin_theta = -R02;

    if (fabs(sin_theta) > (1.0 - EPSILON))
    {
        euler[PSI] = 0.0;  /* arbitrarily set yaw to 0.0 */

        if (sin_theta > 0)
          euler[THETA] = UP;
        else
          euler[THETA] = DOWN;

        R11 = (2.0 * ((q->scalar * q->scalar)
		      + (q->vector[Y_AXIS] * q->vector[Y_AXIS]))) - 1.0;
        R21 = 2.0 * ((q->vector[Y_AXIS] * q->vector[Z_AXIS]) - 
		     (q->scalar * q->vector[X_AXIS]));

        euler[PHI] = atan2(-R21, R11);
    }
    else
    {
        R00 = (2.0 * ((q->scalar * q->scalar)
		      + (q->vector[X_AXIS] * q->vector[X_AXIS]))) - 1.0;
        R01 =  2.0 * ((q->vector[X_AXIS] * q->vector[Y_AXIS]) + 
		      (q->scalar * q->vector[Z_AXIS]));
        R12 =  2.0 * ((q->vector[Y_AXIS] * q->vector[Z_AXIS]) + 
		      (q->scalar * q->vector[X_AXIS]));
        R22 = (2.0 * ((q->scalar * q->scalar)
		      + (q->vector[Z_AXIS] * q->vector[Z_AXIS]))) - 1.0;

        /* Determine angle psi */

        euler[PSI] = atan2(R01, R00);

        /* Determine angle theta */

        euler[THETA] = asin(sin_theta);

        /* Determine angle phi */

        euler[PHI] = atan2(R12, R22);
    }
}


/*
 *  q u a t e r n i o n M u l t i p l y
 *
 *  Multiply the quaternions q1 and q2 and return the result in qr.
 */

static void quaternionMultiply(Quaternion *q1, Quaternion *q2, Quaternion *qr)
{
  qr->scalar = q1->scalar*q2->scalar - q1->vector[0]*q2->vector[0]
    - q1->vector[1]*q2->vector[1] - q1->vector[2]*q2->vector[2];

  qr->vector[0] = q1->vector[0]*q2->scalar + q1->scalar*q2->vector[0]
    - q1->vector[2]*q2->vector[1] + q1->vector[1]*q2->vector[2];

  qr->vector[1] = q1->vector[1]*q2->scalar + q1->vector[2]*q2->vector[0]
    + q1->scalar*q2->vector[1] - q1->vector[0]*q2->vector[2];

  qr->vector[2] = q1->vector[2]*q2->scalar - q1->vector[1]*q2->vector[0]
    + q1->vector[0]*q2->vector[1] + q1->scalar*q2->vector[2];
}


/*
 *  d e a d R e c k o n R V W
 *
 *  Dead reckon from loc (location), vel (velocity), acc (acceleration),
 *  oriQ (orientation), angvelQ (angular velocity) deltatime seconds
 *  forward in time. The result is returned in newloc (new location),
 *  newvel (new velocity) and neworiQ (new orientation).
 *
 *  The dead reckoning is second order in location and first order
 *  in orientation with velocity and acceleration in world coordinates,
 *  hence the name RVW (dead reckoned rotation, velocity, world).
 */

void deadReckonRVW(double deltatime,
		   double loc[3], double vel[3], double acc[3],
		   Quaternion *oriQ, Quaternion *angvelQ,
		   double newloc[3], double newvel[3], Quaternion *neworiQ)
{
  int i;
  double halfDtSq = deltatime*deltatime/2.0;
  double halfAngle, sinHalfAngle;
  Quaternion moveQ;

  /* location */
  for (i = 0; i < 3; i++)
    {
      newloc[i] = loc[i] + deltatime*vel[i] + halfDtSq*acc[i];
      newvel[i] = vel[i] + deltatime*acc[i];
    }

  /* orientation */
  halfAngle = 0.5 * deltatime * angvelQ->scalar;
  moveQ.scalar = cos(halfAngle);
  sinHalfAngle = sin(halfAngle);
  moveQ.vector[0] = sinHalfAngle * angvelQ->vector[0];
  moveQ.vector[1] = sinHalfAngle * angvelQ->vector[1];
  moveQ.vector[2] = sinHalfAngle * angvelQ->vector[2];

  quaternionMultiply(oriQ, &moveQ, neworiQ);

#ifdef DEBUGIT  
{  
double xx[3], yy[3], zz[3];
quaternionToEuler(oriQ, xx);
quaternionToEuler(&moveQ, yy);
quaternionToEuler(neworiQ, zz);
#define R2D(x) ((x)*180.0/3.141593)
printf("orientation: %7.2f %7.2f %7.2f\n", R2D(xx[0]), R2D(xx[1]), R2D(xx[2]));
printf("move:        %7.2f %7.2f %7.2f\n", R2D(yy[0]), R2D(yy[1]), R2D(yy[2]));
printf("new orient.: %7.2f %7.2f %7.2f\n", R2D(zz[0]), R2D(zz[1]), R2D(zz[2]));
}
#endif
}


/*
 *  c h e c k L o c a t i o n T h r e s h o l d
 *
 *  Return 1 if the distance between the coordinates a and b differ
 *  more than specified by threshold, return 0 otherwise.
 */

int checkLocationThreshold(double threshold, double a[3], double b[3])
{
  if (((a[0]-b[0])*(a[0]-b[0])
       + (a[1]-b[1])*(a[1]-b[1])
       + (a[2]-b[2])*(a[2]-b[2]))
      > threshold*threshold)
    return 1;
  else
    return 0;
}


/*
 *  c h e c k O r i e n t a t i o n T h r e s h o l d
 *
 *  Return 1 if the angular distance between the orientations a and b differ
 *  more than specified by threshold, return 0 otherwise.
 *  The orientations should be defined as [psi, theta, phi] in radians.
 *
 *  BUG: the current implementation is wrong but will work well enough
 *  when used only to check if it is necessary to send an entity state PDU.
 */

int checkOrientationThreshold(double threshold, double a[3], double b[3])
{
  /* BUG: this is wrong */
  if (((a[0]-b[0])*(a[0]-b[0])
       + (a[1]-b[1])*(a[1]-b[1])
       + (a[2]-b[2])*(a[2]-b[2]))
      > threshold*threshold)
    return 1;
  else
    return 0;
}


