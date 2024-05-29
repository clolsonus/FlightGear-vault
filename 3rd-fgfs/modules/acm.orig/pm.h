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
#if defined(NETAUDIO)
#include <audio/audiolib.h>
#else
#if defined(HPAUDIO)
#include <audio/Alib.h>
#else
typedef	char	*AuServer;
typedef char	AuBucketID;
typedef char	AuFlowID;
typedef char	AuDeviceID;
#endif
#endif

#ifdef HAVE_DIS
#include <dis/dis.h>
#else

/*
 *  We need this typedef to store information from the inventory file,
 *  even if we are not using DIS.
 */

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

typedef struct _worldcoordinates {
	double	latitude;	/* [radians, north positive] */
	double  longitude;	/* [radians, east positive] */
	double  z;		/* above reference ellipsoid [meters] */
	}	WorldCoordinates;

/*
 *  Pre-defined sounds that can be played for the user
 */

#define SoundCrash		0
#define SoundGearUp		1
#define SoundGearDown		2
#define SoundMissileLaunch	3
#define SoundCannonFiring	4
#define SoundExplosion		5
#define SoundTouchdown		6
#define SoundStallWarning	7
#define SoundLockWarning	8
#define NUM_SOUNDS		9

typedef struct _viewer {
	struct _viewer *next;
	Display	        *dpy;		/* display for this user */
#if defined(HPAUDIO)
	Audio		*aserver;	/* audio server for this user */
	SBucket		*sound[NUM_SOUNDS];
	ATransID	flow[NUM_SOUNDS];
#else
/*
 *  These fields should be defined even if no audio driver is present
 */

	AuServer	*aserver;	/* audio server for this user */
	AuBucketID	sound[NUM_SOUNDS];
	AuFlowID	flow[NUM_SOUNDS];
	AuDeviceID	audioOutput[1];	/* audio output device ID */
#endif
	Window		win;		/* window for this user */
	GC		gc;		/* GC for drawing */
	GC		gauge_gc;	/* GC for RPM, TEWS, etc */
	XFontStruct	*font;		/* HUD font */
	XFontStruct	*rfont;		/* radar font */
	Viewport	*v;		/* Viewport for out-of-cockpit views */
	Pixmap		eng;		/* Engine "RPM" gauge */
	Pixmap		flap[5];	/* flap setting indicators */
	Pixmap		handle[2];	/* ldg gear handle indicators */
	Pixmap		gearLight[3];	/* ldg gear status lights */
	Atom		protocolsAtom;
	Atom		deleteWindowAtom;
	Atom		closedownAtom;
	int		cn;		/* X connection fd */
	int		width, height;	/* width & height of viewing window */
	int		xCenter, yCenter; /* center of viewing window */
	int		rftw, rfth;	/* radar font width & height */
	short		rx, ry;		/* radar x and y location */
	int		radarWidth;	/* width of radar screen (pixels) */
	int		radarHeight;	/* height of radar screen (pixels) */
	int		lastRPM;	/* last RPM fraction x 1000(0 .. 1000)*/
	int		lastFlap;	/* last flap setting rad x 1000 */
	int		redrew;		/* 1 = we redrew last cycle */
	scaleControl	altScale;	/* altitude scale control */
	scaleControl	velScale;	/* airspeed scale control */
	scaleControl	hdgScale;	/* heading scale control */
	int		TEWSx, TEWSy;	/* location of center of TEWS display */
	int		TEWSSize;	/* size of TEWS display (pixels) */
	int		panelx, panely;	/* location of top/right corner */
	double		scaleFactor;	/* overall scaling factor for display */
	ZInfo		z;		/* depth/color information for HUD */
	ZInfo		rz;		/* depth/color information for radar */
	}		viewer;

typedef struct _wctl {
	char		*type;		/* type of weapon at this station */
	int		info;		/* extra info (weapon specific) */
	int		info2;		/* "      " */
	int		info3;		/* "      " */
	}		weaponStation;

/*
 *  This structure describes a class of aircraft (e.g. an F-16C).
 */

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
	dis_entity_type entityType;	/* craft type used in DIS */
	dis_entity_type altEntityType;	/* alternate craft type used in DIS */
	}	craftType;

typedef struct _wdsc {
	int		mask;		/* key mask */
	int		(*select)();	/* weapon select procedure */
	int		(*update)();	/* per tick update procedure */
	int		(*display)();	/* display update procedure */
	int		(*firePress)();	/* fire button pressed procedure */
	int		(*fireRelease)(); /* fire button released */
	craftType	*w;		/* description of this weapon */
	}		weaponDesc;

#define WK_M61A1	0x01		/* M-61A1 Vulcan 20mm cannon */
#define WK_AIM9M	0x02		/* Sidewinder missile */

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
 *  This structure describes a particular instance of aircraft.
 */

typedef struct _craft {
	int	pIndex;		/* index number of this element in the vector */
	int	type;		/* craft type */
	int	team;		/* team number, also used for chased plane if this is a chaser */
	double	createTime;	/* creation time of this craft */
	viewer	*vl;		/* list of viewers */
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
	}	craft;

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

/*
 *  Craft type definitions
 */

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

#define FL_RECORD	0x01	/* activate recording function */
#define FL_AFTERBURNER	0x02	/* afterburner state */
#define FL_BRAKES	0x04	/* wheel brake state */
#define	FL_NWS		0x08	/* nose-wheel steering mode */
#define FL_HAS_GYRO	0x10	/* missile is gyroscope equiped */
#define FL_GHANDLE_DN	0x20	/* landing gear handle state (1 = DOWN) */
#define FL_GEAR0_UP	0x40	/* set when nose gear is completely up */
#define FL_GEAR1_UP	0x80	/* set when right gear is completely up */
#define FL_GEAR2_UP	0x100	/* set when left gear is completely up */
#define FL_GND_CONTACT	0x200	/* set when we're touching the ground */
#define FL_FIXED_OBJECT 0x400	/* fixed (surface) object */
#define FL_CHASE_VIEW	0x800	/* chase plane view */
#define FL_BLACK_BOX	0x1000	/* object is from a black box recording */
#define FL_BALLISTIC	0x2000	/* munition has no target */

#define RM_OFF		0	/* radar is off */
#define RM_STANDBY	1	/* standby */
#define RM_GUN_DIRECTOR	2	/* gun director mode */
#define RM_HUD		3	/* 20 degree scan */
#define RM_NORMAL	4	/* 65 degree scan */
#define RM_ILS		5	/* instrument landing system mode */

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
_var_type  int		HUDPixel;	/* index of HUD color in viewport(s) */
_var_type  VColor	*HUDColor;
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

#define a	32.15		/* acceleration due to gravity (fps^2) */
#define pi	3.14159265358979323846
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

#define VIEW_WINDOW_HEIGHT	500
#define VIEW_WINDOW_WIDTH	1200
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
#define DESIGNATOR_SIZE		40
/* PANEL_X is determined based on the size of the panel and TEWS location */
#define PANEL_Y			(RADAR_Y)

/*
 *  Location of the center of the engine RPM gauge.
 */

#define ENG_X			(FS_WINDOW_WIDTH-300)
#define ENG_Y			(VIEW_WINDOW_HEIGHT+60+VISOR_MARGIN)

#define ALT_ORG_X		780
#define ALT_ORG_Y		360
#define ALT_LENGTH		219	
#define ALT_ORIENT		orientRight
#define ALT_SCALE		(1530.0 / (double) ALT_LENGTH)
#define ALT_INDEX_SIZE		32
#define ALT_MIN_INTERVAL	100
#define ALT_MIN_SIZE		9
#define ALT_MAJ_INTERVAL	500
#define ALT_MAJ_SIZE		17
#define ALT_DIVISOR		1000.0
#define ALT_FORMAT		"%4.3g"

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

extern int initializeAudio();
extern void shutdownAudio(), playSound(), playContinuousSound(), stopSound();
extern char * acm_find_file();


