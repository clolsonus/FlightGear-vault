/*
 *  navaid --
 *
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991,1992  Riley Rainey
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


#ifndef NAVAID_H
#define NAVAID_H


#include <Vlib.h>


typedef struct {
	double	latitude;
	double	longitude;
	double	altitude_msl;
	}	location_t;

typedef unsigned short	freq_t;

typedef struct _navaid_t {
	struct _navaid_t *next;		/* links navaids on the same channel */
	char		id[4]; 		/* identifier (e.g. "IHNL") */
	freq_t		frequency;	/* channel number */
	unsigned short	flags;
	location_t	loc;		/* lat/long/altitude */
	location_t	gs_loc;		/* lat/long/altitude of glide-slope*/
	double		bearing;	/* orientation wrt true north(x axis) */
	double		beam_width;	/* beam width for localizers */
					/*see navaid.c on how to calculate it*/
	double		slope;		/* angle of glide slope */ 
	VPoint		Sg;		/* cartesian location */
	VPoint		gs_Sg;		/* cartesian location */
	VMatrix		gst;		/* transform world to gs coordinates */
	}		navaid_t;

typedef struct {
	freq_t	frequency;		/* active frequency */
	freq_t	standby_frequency;	/* alternate frequency */
	short	obs_setting;		/* omni bearing selector setting */
	char	dme_readout[16];	/* Current DME reading */
	char	freq_readout[8];	/* current printable freq */
	navaid_t *station;		/* station being received */
	}	radio_t;

#define VOR_CHANNEL_COUNT	180

/*
 *  Definition of the navaid flag word
 */

#define NAVAID_LOC	0x0001
#define NAVAID_VOR	0x0002
#define NAVAID_DME	0x0004
#define NAVAID_GS	0x0008
#define NAVAID_MARKER	0x0010
#define NAVAID_NDB	0x0020


#endif NAVAID_H
