/* fgvars.h -- flight gear public variable definitions
 *
 * Copyright (C) 1996  Eric J. Korpela -- korpela@ssl.berkeley.edu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * $Id: fgvars.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGVARS_H
#define _FGVARS_H

/* Aircraft position in units of 8.3819e-8 degrees (or 1/215 meters) */

#define FG_DEGREE 11930464L
#define FG_ARCMINUTE 198841L
#define FG_NMILE 198841L
#define FG_ARCSECOND 3314
#define FG_R_EARTH 683565275L
#define FG_RADIAN 683565275L
#define FG_METER 215

typedef struct {
	FG_I3VECT acpos;
/* Longitude */
#define ACPOS_ELON   (public_vars->acpos.x)
#define ACPOS_ELON_MAX  FG_INT32_MAX
#define ACPOS_ELON_MIN  FG_INT32_MIN

/* Latitude */
#define ACPOS_LAT    (public_vars->acpos.y)
#define ACPOS_LAT_MAX   FG_INT32_MAX>>1
#define ACPOS_LAT_MIN   FG_INT32_MIN>>1

/* Altitude */
#define ACPOS_ALTMSL (public_vars->acpos.z)
#define ACPOS_ALTMSL_MAX FG_INT32_MAX
#define ACPOS_ALTMSL_MIN -1*FG_R_EARTH

#define ACPOS ((FG_INT32 *)(&(public_vars->acpos)))
#define ACPOSVEC (public_vars->acpos)

/* Aircraft attitude */
/* A vector of length ( FG_ONE   ) describing the direction of the nose     */
/* of the aircraft														*/
	struct {
		FG_I3VECT nosevec;
#define ACATT_NLON    (public_vars->acatt.nosevec.x)
#define ACATT_NLAT    (public_vars->acatt.nosevec.y)
#define ACATT_NALT    (public_vars->acatt.nosevec.z)
#define ACATT_NOSE    ((FG_INT32 *)(&(publicvars->acatt.nosevec)))
#define ACATT_NOSEVEC (public_vars->acatt.nosevec)

/* A vector of length ( FG_ONE   ) describing the direction of the aircraft */
/* vertical axis                                                            */
		FG_I3VECT vertvec;
#define ACATT_VLON    (public_vars->acatt.vertvec.x)
#define ACATT_VLAT    (public_vars->acatt.vertvec.y)
#define ACATT_VALT    (public_vars->acatt.vertvec.z)
#define ACATT_VERT    ((FG_INT32 *)(&(publicvars->acatt.vertvec)))
#define ACATT_VERTVEC (public_vars->acatt.vertvec)


/* A vector of length ( FG_ONE   ) describing the direction of the aircraft */
/* left wing (such that vert cross nose = wing )                            */
		FG_I3VECT wingvec;
#define ACATT_WLON    (public_vars->acatt.wingvec.x)
#define ACATT_WLAT    (public_vars->acatt.wingvec.y)
#define ACATT_WALT    (public_vars->acatt.wingvec.z)
#define ACATT_WING    ((FG_INT32 *)(&(publicvars->acatt.wingvec)))
#define ACATT_WINGVEC (public_vars->acatt.wingvec)


/* Pitch above horizontal                                                 */
/* set by flight model to (iasin(ACATT_NALT))                             */
		FG_INT32 pitch;
#define ACATT_PITCH (public_vars->acatt.pitch)

/* Bearing (TRUE)         */
/* set by flight model to (iatan2(ACATT_NLON,ACATT_NLAT)) */
		FG_INT32 bearing;
#define ACATT_BEARING   (public_vars->acatt.bearing)

/* Roll Angle             */
/* set by flight model    */
		FG_INT32 roll;
#define ACATT_ROLL      (public_vars->acatt.roll)
	} acatt;

/* Aircraft velocity in units/second */
	FG_I3VECT acvel;
#define ACVEL_DLON    (public_vars->acvel.x)
#define ACVEL_DLAT    (public_vars->acvel.y)
#define ACVEL_DALT    (public_vars->acvel.z)

#define ACVEL ((FG_INT32 *)(&(public_vars->acvel)))
#define ACVELVEC (public_vars->acvel)

#define ACSPEED (i3mag(&ACVELVEC))
#define ACTAS (i3dot(&ACVELVEC,&ACATT_NOSEVEC)/FG_ONE)

/* Control positions */
	struct {
		FG_INT32 throttle;
		FG_INT32 rudder;
		FG_INT32 elevator;
		FG_INT32 aileron;
	} accontrol;

#define ACCONTROL_THROTTLE (public_vars->accontrol.throttle)
#define ACCONTROL_RUDDER   (public_vars->accontrol.rudder)
#define ACCONTROL_ELEVATOR (public_vars->accontrol.elevator)
#define ACCONTROL_AILERON  (public_vars->accontrol.aileron)

/* Simulation time of day in milliseconds */
	FG_UINT32 ac_time_ms;
#define AC_TIME_MS (public_vars->ac_time_ms)
        FG_UINT32 fg_is_paused;
#define FG_IS_PAUSED (public_vars->fg_is_paused)
} FG_PUBVARS;

extern FG_PUBVARS *public_vars;

#endif

/* $Log: fgvars.h,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 * Revision 2.1  1996/10/31  18:20:21  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.4  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * minor changes.
 * */
