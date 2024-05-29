/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1995  Riley Rainey
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

#include <manifest.h>

#include <scale.h>
#include <damage.h>
#include <interpolate.h>
#include <imath.h>
#include <Vlib.h>
#include <navaid.h>

typedef struct _worldcoordinates {
	double	latitude;	/* [radians, north positive] */
	double  longitude;	/* [radians, east positive] */
	double  z;		/* above reference ellipsoid [meters] */
	}	WorldCoordinates;


/*
 *  Radar tracking information
 */

typedef struct _radarInfo {
	int	beamID;			/* index number of beam */
	VPoint	rel;			/* location relative to radar set */
	double	d;			/* distance to target (feet) */
	int	targetID;		/* craft id of target */
	int	locked;			/* 1=we have a "lock" */
	} radarInfo;

/*
 *  This structure describes a particular instance of observers.
 */

typedef struct _observer {
  VPoint  Sg;                  /* Position in Geoparallel system */
  VPoint  viewPoint;           /* Actual viewpoint */
  viewer  *vl;                 /* list of viewers (actually 1 struct) */

}      observer;

/*
 *  We'll use some defines to reduce the storage required for the craft
 *  (unions would be cleaner, perhaps).
 */

#define	rounds		curRadarTarget
#define	tracerMod	structurePts
#define tracerVal	radarMode
#define owner		curWeapon
#define duration	radarMode
#define flameDuration	flags
#define	escale		SeTrim
#define armTimer	curNWDef

#ifndef ALLOCATE_SPACE
#define _var_type extern
#else
#define _var_type
#endif

_var_type  int		recordCount;	/* number of players recording info */
_var_type  int		ptblCount;	/* number of active entries in ptbl */
_var_type  int          otblCount;      /* number of entries in otbl        */
_var_type  int          ctblCount;      /* number of chasers                */
_var_type  double	curTime;	/* current time */

_var_type  craft	stbl[MAXSURFACE]; /* table of surface objects */
_var_type  craft	ptbl[MAXPLAYERS]; /* table of player aircraft */
_var_type  craft	mtbl[MAXPROJECTILES]; /* table of missiles and cannon streams */
_var_type  craft        ctbl[MAXPLAYERS]; /* table of chasers */
_var_type  observer     otbl[MAXOBSERVERS]; /* table of observers not counted as players */
_var_type  weaponDesc	wtbl[WEAPONTYPES]; /* descriptions of different weapons */

_var_type  VPoint	teamLoc[3];	/* the centers of each team's airport */
_var_type  double	teamHeading[3]; /* heading of plane */

_var_type  double	deltaT;		/* Update interval in seconds */
_var_type  double	halfDeltaTSquared; /* 0.5 * deltaT * deltaT */
_var_type  int		whitePixel;	/* index of white in viewport(s) */
_var_type  VColor	*whiteColor;	/* white in viewport(s) */
_var_type  int		blackPixel;	/* index of black in viewport(s) */
_var_type  VColor	*blackColor;	/* black in viewport(s) */
_var_type  VColor	*groundColor;	/* ground color in viewport(s) */
_var_type  VColor	*radarColor;	/* radar color */
_var_type  VColor	*radarBackgroundColor;	/* radar CRT surface color */
_var_type  VColor	*cloudColor;	/* cloud color */
_var_type  int		arcadeMode;	/* set by -a switch */
_var_type  int          chaseOthers;    /* can one chase other's planes ? */
_var_type  double	droneAggressiveness;  /* how hard drones maneuver */
_var_type  double	visibility;	/* visibility (feet) */
_var_type  double	cbase, ctop;	/* cloud base and tops (feet) */

#ifdef HAVE_DIS
_var_type  int		disInUse;       /* true if DIS protocol is used */
#endif

#undef _var_type

#define CHASE_WINDOW_HEIGHT	400
#define CHASE_WINDOW_WIDTH	600
#define RADAR_WINDOW_WIDTH	200
#define RADAR_WINDOW_HEIGHT	200
#define VISOR_MARGIN		60
#define RADAR_X			(1200/2-200/2)
#define RADAR_Y			(VIEW_WINDOW_HEIGHT + VISOR_MARGIN)
#define TEWS_X			(1200/2-200/2-100)
#define TEWS_Y			(VIEW_WINDOW_HEIGHT + 10 + VISOR_MARGIN)
#define TEWS_SIZE		81
#define FS_WINDOW_WIDTH		(VIEW_WINDOW_WIDTH)	
#define FS_WINDOW_HEIGHT (VIEW_WINDOW_HEIGHT+RADAR_WINDOW_HEIGHT+VISOR_MARGIN)
#define	FLAP_X			(1200/2-200/2-95)
#define FLAP_Y			(FS_WINDOW_HEIGHT-53)
/* PANEL_X is determined based on the size of the panel and TEWS location */
#define PANEL_Y			(RADAR_Y)

/*
 *  Location of the center of the engine RPM gauge.
 */

#define ENG_X			(FS_WINDOW_WIDTH-300)
#define ENG_Y			(VIEW_WINDOW_HEIGHT+60+VISOR_MARGIN)

#define VEL_ORG_X		410
#define VEL_ORG_Y		360
#define VEL_LENGTH		ALT_LENGTH
#define VEL_ORIENT		0
#define VEL_SCALE		(153.0 / (double) VEL_LENGTH)
#define VEL_INDEX_SIZE		32
#define VEL_MIN_INTERVAL	10
#define VEL_MIN_SIZE		9
#define VEL_MAJ_INTERVAL	50
#define VEL_MAJ_SIZE		17
#define VEL_DIVISOR		10.0
#define VEL_FORMAT		"%3.3g"

#define HDG_ORG_X		(VIEW_WINDOW_WIDTH / 2 - (6*33+1) / 2)
#define HDG_ORG_Y		430
#define HDG_LENGTH		(6*33+1)
#define HDG_ORIENT		0
#define HDG_SCALE		(2727.0 / (double) VEL_LENGTH)
#define HDG_INDEX_SIZE		10
#define HDG_MIN_INTERVAL	500
#define HDG_MIN_SIZE		5
#define HDG_MAJ_INTERVAL	1000
#define HDG_MAJ_SIZE		10
#define HDG_DIVISOR		1000.0
#define HDG_FORMAT		"%2.2g"

#define NC_NOT_PLANE            -1
#define NC_CANNOT_CHASE         -2

extern char * acm_find_file();


