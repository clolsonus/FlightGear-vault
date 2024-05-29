/*
 * $Id: deadreckon.h,v 0.2 1995/01/13 14:16:09 mats Exp $
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

#ifndef DEADRECKON_H
#define DEADRECKON_H


typedef struct {
  double scalar;
  double vector[3];
} Quaternion;


/*
 *  e u l e r T o Q u a t e r n i o n
 *
 *  Convert euler[psi, theta, phi] to quaternion q.
 *  Angles are defined in radians.
 */

void eulerToQuaternion(double euler[3], Quaternion *q);

/*
 *  a n g u l a r V e l o c i t y T o Q u a t e r n i o n
 *
 *  Convert avel[x-axis rotation, y ditto, z ditto] to quaternion q.
 *  Angular velocity is defined in radians per second.
 */

void angularVelocityToQuaternion(double avel[3], Quaternion *q);

/*
 *  q u a t e r n i o n T o E u l e r
 *
 *  Convert the quaternion q to euler[psi, theta, phi].
 *  Angles are defined in radians.
 */

void quaternionToEuler(Quaternion *q, double euler[3]);

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
		   Quaternion *ori, Quaternion *angvel,
		   double newloc[3], double newvel[3], Quaternion *newori);

/*
 *  c h e c k L o c a t i o n T h r e s h o l d
 *
 *  Return 1 if the distance between the coordinates a and b differ
 *  more than specified by threshold, return 0 otherwise.
 */

int checkLocationThreshold(double threshold, double a[3], double b[3]);

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

int checkOrientationThreshold(double threshold, double a[3], double b[3]);

#endif
