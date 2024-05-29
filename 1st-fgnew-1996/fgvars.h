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
 * $Id: fgvars.h,v 2.1 1996/10/31 18:20:21 korpela Exp korpela $
 */

#ifndef _FGVARS_H
#define _FGVARS_H

/* Aircraft position in units of 8.3819e-8 degrees (or 1/215 meters) */

#define FG_DEGREE 11930464
#define FG_ARCMINUTE 198841
#define FG_NMILE 198841
#define FG_ARCSECOND 3314
#define FG_R_EARTH 683565275
#define FG_RADIAN 683565275
#define FG_METER 215


/* Longitude */
#define ACPOS_ELON   *(FG_INT32 *)(public_vars+0*sizeof(FG_INT32))
#define ACPOS_ELON_MAX  INT_MAX 
#define ACPOS_ELON_MIN  INT_MIN

/* Latitude */
#define ACPOS_LAT    *(FG_INT32 *)(public_vars+1*sizeof(FG_INT32))
#define ACPOS_LAT_MAX   INT_MAX>>1
#define ACPOS_LAT_MIN   INT_MIN>>1 

/* Altitude */
#define ACPOS_ALTMSL *(FG_INT32 *)(public_vars+2*sizeof(FG_INT32))
#define ACPOS_ALTMSL_MAX INT_MAX
#define ACPOS_ALTMSL_MIN -1*FG_R_EARTH

#define ACPOS ((FG_INT32 *)public_vars)
#define ACPOSVEC *((FG_I3VECT *)public_vars)

/* Aircraft attitude */
/* A vector of length ( FG_ONE   ) describing the direction of the nose     */
/* of the aircraft                                                          */
#define ACATT_NLON    *(FG_INT32 *)(public_vars+3*sizeof(FG_INT32))
#define ACATT_NLAT    *(FG_INT32 *)(public_vars+4*sizeof(FG_INT32))
#define ACATT_NALT    *(FG_INT32 *)(public_vars+5*sizeof(FG_INT32))
#define ACATT_NOSE    (FG_INT32 *)(public_vars+3*sizeof(FG_INT32))
#define ACATT_NOSEVEC *(FG_I3VECT *)(public_vars+3*sizeof(FG_INT32))

/* A vector of length ( FG_ONE   ) describing the direction of the aircraft */
/* vertical axis                                                            */
#define ACATT_VLON    *(FG_INT32 *)(public_vars+6*sizeof(FG_INT32))
#define ACATT_VLAT    *(FG_INT32 *)(public_vars+7*sizeof(FG_INT32))
#define ACATT_VALT    *(FG_INT32 *)(public_vars+8*sizeof(FG_INT32))
#define ACATT_VERT    (FG_INT32 *)(public_vars+6*sizeof(FG_INT32))
#define ACATT_VERTVEC *(FG_I3VECT *)(public_vars+6*sizeof(FG_INT32))

/* A vector of length ( FG_ONE   ) describing the direction of the aircraft */
/* left wing (such that vert cross nose = wing )                            */
#define ACATT_WLON    *(FG_INT32 *)(public_vars+6*sizeof(FG_INT32))
#define ACATT_WLAT    *(FG_INT32 *)(public_vars+7*sizeof(FG_INT32))
#define ACATT_WALT    *(FG_INT32 *)(public_vars+8*sizeof(FG_INT32))
#define ACATT_WING    (FG_INT32 *)(public_vars+6*sizeof(FG_INT32))
#define ACATT_WINGVEC *(FG_I3VECT *)(public_vars+6*sizeof(FG_INT32))


/* Pitch above horizontal                                                 */
/* set by flight model to (iasin(ACATT_NALT))                             */
#define ACATT_PITCH   *(FG_INT32 *)(public_vars+12*sizeof(FG_INT32))

/* Bearing (TRUE)         */
/* set by flight model to (iatan2(ACATT_NLON,ACATT_NLAT)) */
#define ACATT_BEARING   *(FG_INT32 *)(public_vars+13*sizeof(FG_INT32))

/* Roll Angle             */
/* set by flight model    */
#define ACATT_ROLL      *(FG_INT32 *)(public_vars+14*sizeof(FG_INT32))

/* Aircraft velocity in units/second */
#define ACVEL_DLON    *(FG_INT32 *)(public_vars+15*sizeof(FG_INT32))
#define ACVEL_DLAT    *(FG_INT32 *)(public_vars+16*sizeof(FG_INT32))
#define ACVEL_DALT    *(FG_INT32 *)(public_vars+17*sizeof(FG_INT32))

#define ACVEL ((FG_INT32 *)(public_vars+15*sizeof(FG_INT32))
#define ACVELVEC (*(FG_I3VECT *)(public_vars+15*sizeof(FG_INT32)))

#define ACSPEED (i3mag(&ACVELVEC))
#define ACTAS (i3dot(&ACVELVEC,&ACATT_NOSEVEC)/FG_ONE) 

/* Control positions */
#define ACCONTROL_THROTTLE *(FG_INT32 *)(public_vars+18*sizeof(FG_INT32))
#define ACCONTROL_RUDDER   *(FG_INT32 *)(public_vars+19*sizeof(FG_INT32))
#define ACCONTROL_ELEVATOR *(FG_INT32 *)(public_vars+20*sizeof(FG_INT32))
#define ACCONTROL_AILERON  *(FG_INT32 *)(public_vars+21*sizeof(FG_INT32))

/* Simulation time of day in milliseconds */
#define AC_TIME_MS *(FG_INT32 *)(public_vars+22*sizeof(FG_INT32))

#endif

/* $Log: fgvars.h,v $
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
