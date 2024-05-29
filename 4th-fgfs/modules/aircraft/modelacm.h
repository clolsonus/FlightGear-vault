/*
 *  modelacm.h -- acm flight model
 *
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


#ifndef MODELACM_H
#define MODELACM_H

/* from unsorted acm code */
#include <manifest.h>
#include <navaid.h>
#include <Vlib.h>

/* from sorted acm code */
#include "damage.h"
#include "interpolate.h"
#include "../radar/radar.h"
#include "../sound/sound.h"
#include "../weapon/weapon.h"


/* This structure describes a class of aircraft (e.g. an F-16C).  */

typedef struct {
    char	*name;		/* name of aircraft class */

    double	aspectRatio;	/* wing aspect ratio */

    double	CLOrigin, CLSlope; /* Defines the CL characteristic eqn */
    double	CLNegStall, CLPosStall;
    double	betaStall;	/* Stall angle for rudder */
    double  CDOrigin, CDFactor; /* Defines the CD Characteristic eqn */
    ITable	*CLift;		/* compute lift coefficent */
    ITable	*CDb;		/* compute body + wave drag coeff */
    ITable  *CnBeta;	/* compute yaw moment due to sideslip */	
    ITable	*ClBeta;	/* compute roll moment due to sideslip */
    double  CDBOrigin, CDBFactor; /* Defines the CD Characteristic eqn */
    double  CDBPhase;
    double	CYbeta;		/* Side-force from side-slip (dCY/dBeta) */
    double	Clda;		/* roll moment from aileron offset */
    double  Cldr;		/* roll moment from rudder offset */
    double  Clp;		/* roll damping */
    
    double	Cmq;		/* damping in pitch */
    double	Cnr;		/* damping in yaw */

    double  maxAileron;	/* maximum aileron offset */
    double  maxRudder;	/* maximum rudder offset */

    double	effElevator;	/* Elevator effectiveness */
    double	effRudder;	/* Rudder effectiveness */
    double	SeTrimTakeoff;	/* Takeoff elevator trim setting */

    VMatrix I;		/* Moments of Inertia about CG in [xyz] */
    double	cmSlope; 	/* CmAlpha curve slope */
    double  cmFactor;	/* CmAlpha factor when stalled */
    
    double	maxFlap;	/* maximum flap setting (radians) */
    double	cFlap;		/* lift coefficient of flaps */
    double	cFlapDrag;	/* drag coefficient of lowered flaps */
    double	flapRate;	/* flap movement rate (radians/sec) */

    double	cGearDrag;	/* drag coefficient of lowered gear */
    double  gearRate;	/* landging gear movement rate (rad/sec) */

    double	maxSpeedBrake;	/* maximum speed brake setting (radians) */
    double	cSpeedBrake;	/* drag coefficient of 90 degree speed brake */
    double	speedBrakeRate;	/* rate of speed brake movement (radians/sec) */
    double	speedBrakeIncr;	/* number of radians than one keystroke moves brake */
    
    double	wingS;		/* wing area (ft^2) */
    double	wings;		/* wing half-span (ft) */
    double	c;		/* mean aerodynamic chord (MAC) (ft) */
    double	emptyWeight;	/* empty weight (lbs.) */
    double	maxFuel;	/* maximum internal fuel (lbs.) */

    double  maxThrust;	/* max static thrust, military power (lb) */
    double  maxABThrust;	/* max static thrust, afterburner on  (lb) */
    double  (*thrust)();	/* computes current thrust */
    ITable	*Thrust;	/* Change in thrust due to mach number */
    ITable	*ABThrust;	/* Change in thrust due to mach number */
    double	engineLag;	/* controls lag between throttle and RPM */
    double  spFuelConsump;	/* specific fuel consump(lb fuel/lb T x hr)*/
    double	spABFuelConsump;
    VPoint	groundingPoint;	/* hypothetical single pt of contact w/ground */
    VPoint  viewPoint;	/* pilot's viewing location wrt CG */

    double	muStatic;	/* static coefficient of friction no-brakes */
    double	muKinetic;	/* moving coefficient of friction no-brakes */
    double	muBStatic;	/* static brakes-on */
    double  muBKinetic;	/* moving brakes-on */

    double  maxNWDef;	/* maximum nosewheel deflection (radians) */
    double  NWIncr;		/* deflection for each unit */
    double  maxNWS;		/* maximum NWS velocity */
    double	gearD1;		/* x station wrt nose gear of main gear */
    double  gearD2;		/* x station wrt CG of main gear */
    VPoint	rm, rn;		/* location if main/nose gear attachments*/
    double	Dm, Dn;		/* main/nose oleo damping factor */
    double	Km, Kn;		/* main/nose oleo spring factor */
    double  Gm, Gn;		/* main/nose strut length with tire */
    double  cmMax, cnMax;	/* main/nose maximum oleo extension distance */
    VPoint	tailExtent;	/* as we rotate, this part may drag */
    double	armDelay;	/* arming delay for missiles */

    long	damageBits;	/* initial bit mask of damaged systems */
    long	structurePts;	/* maximum structural damage */

    double	radarOutput;	/* radar output (watts) */
    double	radarTRange;	/* tracking radar range */
    double	radarDRange;	/* detection radar range */
    double	TEWSThreshold;

    long	sCount;		/* number of weapon stations */
    VPoint	wStation[STATIONS]; /* weapon's stations (launch points) */
    weaponStation station[STATIONS]; /* whatcan be at each weapon station */

    int	(*placeProc)();	/* object placement procedure (for special craft) */
    char	*objname;	/* name of file containing the object */
    VObject *object;	/* what it looks like */
    void	(*resupply)();	/* the plane's rearm & refuel procedure */

#ifdef HAVE_DIS
    dis_entity_type entityType;	/* craft type used in DIS */
    dis_entity_type altEntityType;	/* alternate craft type used in DIS */
#endif

} craftType;


/* This structure describes a particular instance of aircraft.  */

typedef struct _craft {
    int	pIndex;		/* index number of this element in the vector */
    int	type;		/* craft type */
    int	team;		/* team number, also used for chased plane if this is a chaser */
    double	createTime;	/* creation time of this craft */
#ifdef HAVE_VIEWER
    viewer	*vl;		/* list of viewers */
#endif
    VMatrix trihedral;  	/* transforms [x, y, z] to Geoparallel */
				/* x = forward, y=right wing, z=down */
				/* does not include origin translation */
    VMatrix	Itrihedral;	/* transforms geoparallel to [x, y, z] */
    VPoint	Cg;		/* Velocity vector in Geoparallel system */
    VPoint	Sg;		/* Position in Geoparallel system */
    double	rho;		/* current rho value */
    double  mach1;		/* current value of mach 1.0 (fps) */
    double	mach;		/* current mach number */
    VPoint	G;		/* g-force vector in acft system */
    VPoint	prevSg;		/* last interval's Sg value (ft) */
    double	p, q, r;	/* roll, pitch, and yaw rates (rad/sec) */
    double	Se, Sa, Sr;	/* control settings (-1 <= x <= 1) */
    double	SeTrim;		/* elevator trim setting */
    double	curHeading, curPitch, curRoll; /* Euler angles for acft */
    double	curThrust;	/* Current thrust value (lbs) */
    double	curFlap;	/* current flap setting (radians) */
    double	flapSetting;	/* current target flap setting (radians) */
    double	curSpeedBrake;	/* current speed brake position (radians) */
    double	speedBrakeSetting; /* current target speed brake setting(rad) */
    double	curGear[3];	/* current ldg gear location (0.0 = up) */
				/* [0] - nose, [1] - right, [2] - left */
    int	throttle;	/* thrust setting 0 - 32768 */
    double	rpm;		/* actual engine RPM (0.0 .. 1.0) */
    double	alpha, beta;	/* angles of attack and sideslip (rad) */
    double	fuel;		/* current fuel on board (lbs) */
    int	flags;		/* flag word */

    long	damageBits;	/* bit flags of damaged subsystems */
    long	structurePts;	/* damage pts that can be absorbed */
    double	leakRate;	/* fuel leakage rate (lbs/second) */
    double	damageCL;	/* damage induced roll */
    double	damageCM;	/* damage induced pitch */

    double	groundCgx;	/* groundspeed */

    int	radarMode;	/* radar mode */
    double  curNWDef;	/* Current nosewheel deflection (radians) */
    craftType *cinfo;	/* General craft information */
    double	order;		/* temporary value used to sort craft */
    struct  _craft *next;	/* next craft in sorted list */
    VPoint	viewDirection;	/* where the pilot is currently looking */
    VPoint	viewUp;		/* the "up" direction of the pilot's view */

    char	name[32];	/* logname of player */
    char	display[32];	/* display name of player */

    short	curRadarTarget;	/* our primary "threat" */
    double	targetDistance; /* distance to primary target [feet] */
    double	targetClosure;	/* closure rate on primary target [fps] */
    short	relValid[MAXPLAYERS];
    short	curOpponent;	/* who this drone is trying to kill */
    short	holdCount;	/* non-zero when drones holding fire */
    VPoint	relPos[MAXPLAYERS];
    double	rval[MAXPLAYERS];/* radar strength seen by us from other craft */
				/* relative positions of possible targets */
    char	*leftHUD[3];	/* strings in lower left corner of HUD */
				/* (reserved for weapons' status */
    char	*rightHUD[3];	/* strings in lower right corner of HUD */
				/* (reserved for future use) */
    int	curWeapon;	/* index of currently selected weapon */
    weaponStation station[STATIONS]; /* what's at each weapon station */
    radio_t navReceiver[2];
    radio_t *hsiSelect;	/* pointer to radio being used for hsi */

    char	lastConsump[32];
    char	lastTotal[32];
    char	lastFlap[16];
    char	lastRPM[16];
    radarInfo rinfo[32];	/* radar target information */
    int	rtop;		/* number of entries used in rinfo */
#ifdef HAVE_DIS
    int     disId;		/* DIS identity */
    VPoint	disLastCg;	/* last velocity vector (used for local) */
    double	disLastTime;	/* last time (used for local) */
#endif	
} craft;

extern double deltaT;
extern double curTime;	/* current time */

extern  int	ptblCount;	/* number of active entries in ptbl */
extern  craft	ptbl[MAXPLAYERS]; /* table of player aircraft */


/* Craft type definitions */

#define CT_FREE		0	/* an unused craft entry */
#define CT_PLANE	1	/* a player */
#define CT_MISSILE	2	/* an air to air missile */
#define CT_CANNON	3	/* a stream of cannon fire */
#define CT_SURFACE	4	/* surface object (e.g. a runway) */
#define CT_CHASER	5	/* a non-player "watcher" chasing a plane */
/*#define CT_OBSERVER	5	/* a non-player "watcher" */
#define CT_DRONE	6	/* a target drone */
#define CT_EXPLOSION	7	/* an explosion */
#ifdef HAVE_DIS
#define CT_DIS_PLANE	8	/* external player (on different server) */
#define CT_DIS_MUNITION 9	/* external tracked munition */
#define CT_DIS_CANNON	10	/* external untracked munition (cannon) */
#endif

#define RM_OFF		0	/* radar is off */
#define RM_STANDBY	1	/* standby */
#define RM_GUN_DIRECTOR	2	/* gun director mode */
#define RM_HUD		3	/* 20 degree scan */
#define RM_NORMAL	4	/* 65 degree scan */
#define RM_ILS		5	/* instrument landing system mode */

#define FL_RECORD       0x01    /* activate recording function */
#define FL_AFTERBURNER  0x02    /* afterburner state */
#define FL_BRAKES       0x04    /* wheel brake state */
#define FL_NWS          0x08    /* nose-wheel steering mode */
#define FL_HAS_GYRO     0x10    /* missile is gyroscope equiped */
#define FL_GHANDLE_DN   0x20    /* landing gear handle state (1 = DOWN) */
#define FL_GEAR0_UP     0x40    /* set when nose gear is completely up */
#define FL_GEAR1_UP     0x80    /* set when right gear is completely up */
#define FL_GEAR2_UP     0x100   /* set when left gear is completely up */
#define FL_GND_CONTACT  0x200   /* set when we're touching the ground */
#define FL_FIXED_OBJECT 0x400   /* fixed (surface) object */
#define FL_CHASE_VIEW   0x800   /* chase plane view */
#define FL_BLACK_BOX    0x1000  /* object is from a black box recording */
#define FL_BALLISTIC    0x2000  /* munition has no target */

#define DESIGNATOR_SIZE		40

#define a	32.15		/* acceleration due to gravity (fps^2) */

#ifndef M_PI                                    
#define M_PI        3.14159265358979323846      /* pi */
#endif                                                           
#ifndef M_PI_2                                                     
#define M_PI_2      1.57079632679489661923      /* pi/2 */           
#endif                       
#ifndef PI2                                               
#define PI2  (M_PI + M_PI)                      /* pi*2 */               
#endif        

/* Number of microseconds between each update (when not in "real time"
 *  mode) */

#ifndef UPDATE_INTERVAL 
#define UPDATE_INTERVAL		125000
#endif

#define a	32.15		/* acceleration due to gravity (fps^2) */
#define mag(v)	(sqrt (v.x * v.x + v.y * v.y + v.z * v.z))

#define NM		6076.115
#define SM		5280.0
#define FPStoMPH(v)	((v) / 5280.0 * 3600.0)
#define FPStoKTS(v)	((v) / 6076.115 * 3600.0)
#define KTStoFPS(v)	((v) * 6076.115 / 3600.0)
#define FEETtoMETERS(v)	((v) * 0.3048)
#define METERStoFEET(v)	((v) * 3.28084)
#define fsign(d)	(d < 0 ? -1.0 : 1.0)
#define calcThrust(c)	((((c->flags & FL_AFTERBURNER) ? \
	c->cinfo->maxABThrust : c->cinfo->maxThrust) \
	* (c->rho / 0.002377)) * \
	pow ((double) c->throttle / 32768.0, 2.0))
#define fuelUsed(c)	(((c->flags & FL_AFTERBURNER) ? \
	c->cinfo->spABFuelConsump : c->cinfo->spFuelConsump) * c->curThrust * \
	deltaT / 3600.0)


#ifdef HAVE_DIS
#include <dis/dis.h>
#else

/* We need this typedef to store information from the inventory file,
 * even if we are not using DIS.  */

struct dis_entity_type {
	unsigned char kind;
	unsigned char domain;
	unsigned short country;
	unsigned char category;
	unsigned char subcategory;
	unsigned char specific;
	unsigned char extra;
};
typedef struct dis_entity_type dis_entity_type;

#endif /* HAVE_DIS */


#endif MODELACM_H
