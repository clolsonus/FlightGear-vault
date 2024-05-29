/*

 *  Copyright (C) 1995  Mats Lofkvist  CelsiusTech Electronics AB
 *
 *  With additions by Riley Rainey
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

static char rcsid[] = "$Id: from dis_nps.c,v 0.8 1995/01/13 14:16:09 mats Exp $";

#include <pm.h>
#undef a
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <assert.h>
#include <deadreckon.h>
#include <dis/dis.h>
#include <dis.h>

#define DEFAULT_LOCATION_THRESHOLD      1.0
#define DEFAULT_ORIENTATION_THRESHOLD   (3.0 * M_PI / 180.0)

#define SEND_TIMEOUT_SECONDS	5.0
#define RECV_TIMEOUT_SECONDS	12.0


static dis_EntityEnterCb entityEnterCb = NULL;
static dis_DetonationCb detonationCb = NULL;

static double locationThreshold = DEFAULT_LOCATION_THRESHOLD;
static double orientationThreshold = DEFAULT_ORIENTATION_THRESHOLD;

static int exercise;
static int site;
static int application;

static double theTime;
static int absoluteTime = 0;

static DISxApplicationInfo *app;

extern int killPlayer (craft *c);

#define MARKINGS_LEN 11

typedef struct {
    int mode;			/* current mode */
    int cur_target;		/* current radar target (ptbl index) */
    double lastTime;		/* time of last update */
    dis_em_emission_pdu em;
    dis_em_system_info *s;	/* systems passed to us */
    dis_beam_info *b;		/* total active beams */
    dis_track_info *target;	/* track/jam target info */
} EntityEM;

typedef struct {
    int local;			/* -1: free, 0: remote, 1: local */
    craft *c;			/* pointer to craft structure  in ptbl */
    double lastTime;		/* last send for local, receive for remote */
    u_char forceId;
    dis_entity_id entityId;
    dis_entity_type entityType;
    dis_entity_type altEntityType;
    EntityEM *em;		/* EM emission PDU information, or NULL */
    unsigned char markings[MARKINGS_LEN];
    unsigned int appearance;
    double location[3];
    double velocity[3];
    double linearAcceleration[3];
    double orientation[3];
    double angularVelocity[3];
    double lastLocation[3];
    double lastVelocity[3];
    double lastLinearAcc[3];
    double lastOrientation[3];
    double lastAngularVel[3];
    Quaternion lastOrientationQ;
    Quaternion lastAngularVelQ;
} Entity;

#define MAX_ENTITIES 128

static Entity *entities;
static int entity_top = -1;


/*
 *  These coordinate system conversion routines don't do very much today,
 *  but, eventually, we must support a spheroid world (WGS84 standard).
 */

void
ACMtoDISWorld(VPoint * in, dis_world_coordinates * out)
{
    out->x = FEETtoMETERS(in->x);
    out->y = FEETtoMETERS(in->y);
    out->z = FEETtoMETERS(in->z);
}

void
DIStoACMWorld(dis_world_coordinates * in, VPoint * out)
{
    out->x = METERStoFEET(in->x);
    out->y = METERStoFEET(in->y);
    out->z = METERStoFEET(in->z);
}

void
ACMtoDISVelocity(VPoint * in, dis_float_vector * out)
{
    out->x = FEETtoMETERS(in->x);
    out->y = FEETtoMETERS(in->y);
    out->z = FEETtoMETERS(in->z);
}

void
DIStoACMVelocity(dis_float_vector * in, VPoint * out)
{
    out->x = METERStoFEET(in->x);
    out->y = METERStoFEET(in->y);
    out->z = METERStoFEET(in->z);
}

/*
 *  f i n d E n t i t y
 *
 *  Find the entity with Dis id id in the local entities table
 *  and return its index in the table.
 *
 *  The id (index) is returned on success, -1 is returned on failure.
 */

static int
findEntity(dis_entity_id * id)
{
    int i;
    Entity *p = entities;

    for (i = 0; i <= entity_top; i++) {
	if (p->local != -1
	    && p->entityId.entity_id == id->entity_id
	&& p->entityId.sim_id.application_id == id->sim_id.application_id
	    && p->entityId.sim_id.site_id == id->sim_id.site_id) {
	    return i;
	}
	++p;
    }

    return -1;
}


/*
 *  d i s _ i n i t
 *
 *  Initialize the DIS library.
 *  The broadcast device used is specified with device (e.g. "le0" on a Sun).
 *  The port for receiving DIS packets is specified with port.
 *     (The port number is currently hardwired to 3000.)
 *  The DIS exercise number, site number and application number are specified
 *  by the corresponding arguments.
 *     (The application number is currently hardwired to a part of the local
 *      hostid. This will ensure uniqe application numbers on a local network.)
 *  User callbacks for entity enter, entity exit, fire and detonation are
 *  specified by the last arguments. NULL callbacks may be used for fire
 *  and detonation.
 *
 *  Zero is returned on success.
 */

int
dis_init(int xexercise, int xsite, int xapplication,
	 dis_EntityEnterCb xentityEnterCb,
	 dis_DetonationCb xdetonationCb)
{
    int i;
    dis_simulation_addr addr;

    entityEnterCb = xentityEnterCb;
    detonationCb = xdetonationCb;

    exercise = xexercise;

    if ((app = DISxInitializeApplication(xexercise, xsite, xapplication))
	== NULL) {
	return -1;
    }
/*
 *  Get the actual simulation address assigned to us.
 */

    DISxGetSimulationAddress(app, &addr);
    site = addr.site_id;
    application = addr.application_id;

/*
 *  Allocate storage for the entity table and initialize it.
 */

    entities = (Entity *) malloc(sizeof(Entity) * MAX_ENTITIES);
    for (i = 0; i < MAX_ENTITIES; i++) {
	entities[i].local = -1;
    }

    return 0;
}


/*
 *  d i s _ c l o s e
 *
 *  Close down the DIS library.
 *
 *  Zero is returned on success.
 */

int
dis_close(void)
{
    return 0;
}


/*
 *  d i s _ s e t D R T h r e s h o l d s
 *
 *  Set the dead reckoning thresholds for location and orientation.
 *  The values shall be given in meters and radians (psi, theta, phi).
 *
 *  Zero is returned on success.
 */

int
dis_setDRThresholds(double location, double orientation)
{
    locationThreshold = location;
    orientationThreshold = orientation;

    return 0;
}


static const double timeFactor = 596523.235556;		/* 2^31/3600 bits per second */


/*
 *  t i m e D I S T o D o u b l e
 *
 *  Convert a DIS timestamp to a double in UNIX format (seconds since 1970).
 *  The local time 'theTime' is used for the hours part since the DIS
 *  timestamp only specifies part of hour. The returned value will be
 *  the closest possible to 'theTime'.
 */

static double
timeDISToDouble(dis_timestamp disTime)
{
    double seconds;		/* seconds into the current hour */
    double myseconds;		/* ditto for 'theTime' */
    double diffseconds;
    double myhour;		/* hour part of 'theTime' */

    if (disTime.type == 0)	/* HACK: return current local time */
	return theTime;

    seconds = disTime.time / timeFactor;
    myseconds = fmod(theTime, 3600.0);
    myhour = rint((theTime - myseconds) / 3600.0);

    diffseconds = myseconds - seconds;
    if (diffseconds > 1800.0)
	return 3600.0 * (myhour + 1) + seconds;
    else if (diffseconds < -1800.0)
	return 3600.0 * (myhour - 1) + seconds;
    else
	return 3600.0 * myhour + seconds;
}


/*
 *  t i m e D o u b l e T o D I S
 *
 *  Convert a double in UNIX format (seconds since 1970) to a DIS timestamp.
 *  If reference is 0, the time will be marked relative.
 *  If reference is 1, the time will be marked absolute, i.e. true UTC time.
 */

static dis_timestamp
timeDoubleToDIS(double time, int reference)
{
    unsigned long tmp;
    dis_timestamp res;

    tmp = (unsigned long) (fmod(time, 3600.0) * timeFactor);
    if (tmp > 2147483647L)	/* 2^31 - 1 */
	res.time = 2147483647L;
    else
	res.time = tmp;
    res.type = reference;

    return res;
}


/*
 *  d i s _ s e t T i m e
 *
 *  Set the current time in the DIS library.
 *
 *  Zero is returned on success.
 */

int
dis_setTime(double time)
{
    theTime = time;
    absoluteTime = 0;

    return 0;
}


/*
 *  d i s _ s e t T i m e A b s o l u t e
 *
 *  Set the current time in the DIS library using the system time,
 *  and mark the time as absolute, i.e. true UTC time.
 *  This will improve the dead reckoning performance on networks
 *  with significant delays between players _iff_ all hosts have
 *  true UTC time (with millisecond precision!).
 *
 *  If this is used when players do _not_ have synchronized clocks,
 *  the result will be very strange positions for external players!!
 *
 *  Zero is returned on success.
 */

int
dis_setTimeAbsolute(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    theTime = tv.tv_sec + tv.tv_usec / 1000000.0;
    absoluteTime = 1;

    return 0;
}

double
getCurrentTime()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;
}


/*
 *  g e t E n t i t y S t a t e D a t a
 *
 *  Read in the entity state data from the entity state PDU es
 *  and write it to the local entity with id (index) eid.
 */

static void
getEntityStateData(int eid, dis_entity_state_pdu * es)
{

/*
 *  Rather than relying on time syncronization here, let's use the local
 *  time ...  this completely ignores network time delays (for now).
 entities[eid].lastTime = timeDISToDouble(es->hdr.time_stamp);
 */
    entities[eid].lastTime = getCurrentTime();
    entities[eid].lastLocation[0] = es->pos.x;
    entities[eid].lastLocation[1] = es->pos.y;
    entities[eid].lastLocation[2] = es->pos.z;
    entities[eid].lastVelocity[0] = es->vel.x;
    entities[eid].lastVelocity[1] = es->vel.y;
    entities[eid].lastVelocity[2] = es->vel.z;
    entities[eid].lastOrientation[0] = es->orientation.psi;
    entities[eid].lastOrientation[1] = es->orientation.theta;
    entities[eid].lastOrientation[2] = es->orientation.phi;
    entities[eid].lastLinearAcc[0] = es->dr_parm.linear_acc.x;
    entities[eid].lastLinearAcc[1] = es->dr_parm.linear_acc.y;
    entities[eid].lastLinearAcc[2] = es->dr_parm.linear_acc.z;
    entities[eid].lastAngularVel[0] = es->dr_parm.angular_vel.psi;
    entities[eid].lastAngularVel[1] = es->dr_parm.angular_vel.theta;
    entities[eid].lastAngularVel[2] = es->dr_parm.angular_vel.psi;
    eulerToQuaternion(entities[eid].lastOrientation,
		      &entities[eid].lastOrientationQ);
    angularVelocityToQuaternion(entities[eid].lastAngularVel,
				&entities[eid].lastAngularVelQ);
    if (es->marking.charset == DISCharSetASCII)
	strcpy(entities[eid].markings, es->marking.marking);
    else
	entities[eid].markings[0] = '\0';
}


/*
 *  e n t i t y E n t e r
 *
 *  Process the entity state PDU esPDU for a new (currently unknown)
 *  entity.
 *
 *  Zero is returned on success.
 */

static int
entityEnter(dis_entity_state_pdu * esPDU)
{
    int eid;
    craft *c = NULL;

    for (eid = 0; eid < MAX_ENTITIES; eid++) {
	if (entities[eid].local == -1) {
	    break;
	}
    }
    if (eid >= MAX_ENTITIES) {
	return -1;
    }
    if (eid > entity_top) {
	entity_top = eid;
    }
    entities[eid].local = 0;
    entities[eid].forceId = esPDU->force_id;
    entities[eid].entityId = esPDU->id;
    entities[eid].entityType = esPDU->type;
    entities[eid].altEntityType = esPDU->alt_type;
    entities[eid].em = (EntityEM *) NULL;
    getEntityStateData(eid, esPDU);

    if (esPDU->type.kind == DISKindPlatform
	&& esPDU->type.domain == DISDomainAir) {
	(entityEnterCb) (eid, &esPDU->type, esPDU->force_id, &c);
    }
    else if (esPDU->type.kind == DISKindMunition) {
	(entityEnterCb) (eid, &esPDU->type, esPDU->force_id, &c);
    }
    else			/* nothing we are interested in */
	entities[eid].local = -1;
    entities[eid].c = c;

    return 0;
}


/*
 *  e n t i t y E x i t
 *
 *  Remove the entity with id (index) eid from the local table.
 */

static void
entityExit(int eid)
{
    craft *c;

#ifdef DIS_DEBUG
    printf("entityExit (%d)\n", eid);
#endif

    c = entities[eid].c;
    if (c != NULL && c->type == CT_DIS_PLANE) {
#ifdef DIS_DEBUG
	printf("killing ptbl player index %d in entityExit\n", c->pIndex);
#endif
	killPlayer(c);
    }
    entities[eid].local = -1;
    if (eid == entity_top) {
	entity_top--;
    }
}


/*
 *  e n t i t y S t a t e P D U
 *
 *  Process an incoming entity state PDU.
 *
 *  Zero is returned on success.
 */

static int
entityStatePDU(dis_entity_state_pdu * esPDU)
{
    int eid;

    eid = findEntity(&esPDU->id);

    /* NPSBUG: should define AppearanceGen_DamageBits */
    if ((esPDU->appearance & DISAppearanceDamageDestroyed)) {
	/* deactivated or destroyed entity. if we know about it, exit it */
	if (eid >= 0)
	    entityExit(eid);

	return 0;
    }
    else {
	/* normal entity state PDU. if we know about it, update data,
	   otherwise enter it */
	if (eid >= 0) {
	    getEntityStateData(eid, esPDU);

	    return 0;
	}
	else {
	    eid = entityEnter(esPDU);
	    if (eid >= 0)
		return 0;
	    else
		return -1;
	}
    }
}


/*
 *  f i r e P D U
 *
 *  Process an incoming fire PDU.
 *
 *  Zero is returned on success.
 */


 /*
  *  These munition types are renderable with ACM's cannon simulation
  *  code.
  */

static dis_entity_type cannon_types[] =
{
    {2, 2, 225, 2, 1, 0, 0},
    {2, 2, 225, 2, 2, 0, 0},
    {2, 2, 225, 2, 3, 0, 0},
    {2, 2, 225, 2, 4, 0, 0},
    {2, 2, 222, 2, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}
};

static int
firePDU(dis_fire_pdu * fPDU)
{
    int i, eid;
    craft *m;
    dis_entity_type *dp;
    extern craftType *lookupCraft();

    for (dp = cannon_types; dp->kind != 0; ++dp) {
	if (fPDU->burst.munition.kind == dp->kind &&
	    fPDU->burst.munition.domain == dp->domain &&
	    fPDU->burst.munition.country == dp->country &&
	    fPDU->burst.munition.category == dp->category &&
	    fPDU->burst.munition.subcategory == dp->subcategory) {
	    break;
	}
    }

/*
 *  Not one of the ones that we model?  Then do nothing.
 */

    if (dp->kind == 0) {
	return 0;
    }
/*
 *  Allocate a projectile record
 */

    for ((i = 0, m = mtbl); i < MAXPROJECTILES; (++i, ++m)) {
	if (m->type == CT_FREE) {
	    m->type = CT_DIS_CANNON;
	    break;
	}
    }
    if (i == MAXPROJECTILES) {
	return -1;
    }
/*
 *  Fill out the projectile record
 */

    eid = findEntity(&fPDU->firing_id);
    if (eid >= 0) {
	m->owner = entities[eid].c->pIndex;
    }
    else {
	m->owner = -1;
    }
    m->createTime = curTime;
    m->curRoll = 0.0;
    m->curPitch = 0.0;
    m->curHeading = 0.0;

/*
 *  Determine the initial position.
 */

    DIStoACMVelocity(&fPDU->vel, &m->Cg);
    DIStoACMWorld(&fPDU->pos, &m->Sg);

    m->prevSg = m->Sg;
    m->rounds = fPDU->burst.quantity;
    m->tracerMod = 10 /*TRACER_MOD */ ;
    m->tracerVal = 0;
    m->cinfo = lookupCraft("m61a1 cannon");

    return 0;
}


/*
 *  d e t o n a t i o n P D U
 *
 *  Process an incoming detonation PDU.
 *
 *  Zero is returned on success.
 */

static int
detonationPDU(dis_detonation_pdu * dPDU)
{
    int firingEid, targetEid, eid, ftype;
    double time, worldLocation[3], entityLocation[3];
    craft *c;

    if (detonationCb == NULL) {
	eid = findEntity(&dPDU->munition_id);
	if (eid >= 0) {
	    entities[eid].local = -1;
	    if (eid == entity_top) {
		entity_top--;
	    }
	}
	return 0;
    }
    firingEid = findEntity(&dPDU->firing_id);
    if (firingEid < 0)
	return -1;
    targetEid = findEntity(&dPDU->target_id);
    if (targetEid < 0)
	return -2;

    /* HACK */
    if (dPDU->burst.munition.category == 2)	/* Ballistic */
	ftype = DIS_FIRE_M61A1;
    else
	ftype = DIS_FIRE_AIM9M;

    time = timeDISToDouble(dPDU->hdr.time_stamp);

    worldLocation[0] = dPDU->pos.x;
    worldLocation[1] = dPDU->pos.y;
    worldLocation[2] = dPDU->pos.z;

    entityLocation[0] = dPDU->loc.x;
    entityLocation[1] = dPDU->loc.y;
    entityLocation[2] = dPDU->loc.z;

    eid = findEntity(&dPDU->munition_id);
    c = (eid < 0) ? NULL : entities[eid].c;

    (detonationCb) (ftype, firingEid, targetEid, time, worldLocation,
		    entityLocation, c);

    if (eid >= 0) {
	entities[eid].local = -1;
	if (eid == entity_top) {
	    entity_top--;
	}
    }
    return 0;
}

/*
 *  e m i s s i o n P D U
 *
 *  Process an incoming EM emission PDU.
 *
 *  Zero is returned on success.
 */

static int
emissionPDU(dis_em_emission_pdu * pdu)
{
    Entity *e;
    int emitterEid;

    emitterEid = findEntity(&pdu->emitter_id);
    if (emitterEid < 0) {
	return -1;
    }
    else {
	e = &entities[emitterEid];
    }

/*
 *  First emission received?
 */

    if (e->em == (EntityEM *) NULL) {

/*
 *  Allocate  EM emission information structure and initialize it to reflect
 *  that this is an external entity (mode == -1).
 */

	e->em = (EntityEM *) Vmalloc(sizeof(EntityEM));
	e->em->mode = -1;
	e->em->cur_target = -1;

    }

/*
 *  Not the first emission.  Free the old PDU variable fields and insert
 *  the new one.
 */

    else {
	DISFreePDUComponents ((dis_pdu *) &e->em->em);
    }
    e->em->em = *pdu;
    e->em->lastTime = theTime;
    return 0;
}


/*
 *  d i s _ r e c e i v e
 *
 *  Process all available incoming PDU's from the network.
 *  User callbacks will be called for entering entities,
 *  exiting entities, firing entities and detonations.
 *
 *  Zero is returned on success.
 */

int
dis_receive(void)
{
    int status, err, free_needed;
    dis_pdu pdu;
    int i;

    err = 0;
    while ((status = DISxReadPDU(app, &pdu)) == 0) {

	free_needed = 1;

	/* ignore other exercises */
	if (pdu.hdr.exercise_id != exercise)
	    continue;

	switch (pdu.hdr.pdu_type) {
	case PDUTypeEntityState:

	    /* don't read our own broadcasts */
	    if (pdu.entity_state.id.sim_id.site_id == site
	     && pdu.entity_state.id.sim_id.application_id == application)
		continue;

	    err = entityStatePDU(&pdu.entity_state);
	    break;

	case PDUTypeFire:

	    /* don't read our own broadcasts */
	    if (pdu.fire.firing_id.sim_id.site_id == site
	      && pdu.fire.firing_id.sim_id.application_id == application)
		continue;

	    err = firePDU(&pdu.fire);
	    break;

	case PDUTypeDetonation:

	    /* don't read our own broadcasts */
	    if (pdu.detonation.firing_id.sim_id.site_id == site
		&& pdu.detonation.firing_id.sim_id.application_id == application)
		continue;

	    err = detonationPDU(&pdu.detonation);
	    break;

	case PDUTypeEmission:

	    /* don't read our own broadcasts */
	    if (pdu.em_emission.emitter_id.sim_id.site_id == site
		&& pdu.em_emission.emitter_id.sim_id.application_id == application)
		continue;
	    err = emissionPDU(&pdu.em_emission);
	    free_needed = 0;
	    break;

	default:
	    fprintf(stderr, "dis_receive: Ignoring PDU type %d\n", pdu.hdr.pdu_type);
	    err = 0;
	    break;
	}

/*
 *  Free any dynamically allocated variable components that are part of this
 *  PDU.
 */

	if (free_needed) {
	    DISFreePDUComponents(&pdu);
        }

    } /* while (... DISxReadPDU ...) */

/*
 *  check for timeouts on remote entities
 */

    for (i = 0; i <= entity_top; i++) {
	if (entities[i].local == 0
	    && theTime - entities[i].lastTime > RECV_TIMEOUT_SECONDS) {
	    entityExit(i);
	}
    }

    if (err != 0) {
	return -2;
    }
    else {
	return 0;
    }
}


/*
 *  s e n d E n t i t y S t a t e
 *
 *  Send an entity state PDU for the local entity with id (index) eid.
 *
 *  Zero is returned on success.
 */

static int
sendEntityState(int eid)
{
    dis_entity_state_pdu pdu, *esPDU = &pdu;
    int i;

    esPDU->hdr.pdu_type = PDUTypeEntityState;
    esPDU->id = entities[eid].entityId;
    esPDU->force_id = entities[eid].forceId;
    esPDU->art_parm_count = 0;
    esPDU->type = entities[eid].entityType;
    esPDU->alt_type = entities[eid].altEntityType;
    esPDU->pos.x = entities[eid].location[0];
    esPDU->pos.y = entities[eid].location[1];
    esPDU->pos.z = entities[eid].location[2];
    esPDU->vel.x = entities[eid].velocity[0];
    esPDU->vel.y = entities[eid].velocity[1];
    esPDU->vel.z = entities[eid].velocity[2];
    esPDU->orientation.psi = entities[eid].orientation[0];
    esPDU->orientation.theta = entities[eid].orientation[1];
    esPDU->orientation.phi = entities[eid].orientation[2];
    esPDU->appearance = entities[eid].appearance;
    esPDU->dr_parm.algorithm = DISDRMethodRVW;
    esPDU->dr_parm.linear_acc.x = entities[eid].linearAcceleration[0];
    esPDU->dr_parm.linear_acc.y = entities[eid].linearAcceleration[1];
    esPDU->dr_parm.linear_acc.z = entities[eid].linearAcceleration[2];
    esPDU->dr_parm.angular_vel.psi = entities[eid].angularVelocity[0];
    esPDU->dr_parm.angular_vel.theta = entities[eid].angularVelocity[1];
    esPDU->dr_parm.angular_vel.phi = entities[eid].angularVelocity[2];
    esPDU->marking.charset = DISCharSetASCII;
    memset(esPDU->marking.marking, 0, MARKINGS_LEN);
    strcpy(esPDU->marking.marking, entities[eid].markings);
    esPDU->capabilities = 0;
    esPDU->art_parm = NULL;

    if (DISxWritePDU(app, (dis_pdu *) esPDU) != 0) {
	return -2;
    }
    else {
	entities[eid].lastTime = theTime;
	for (i = 0; i < 3; i++) {
	    entities[eid].lastLocation[i] = entities[eid].location[i];
	    entities[eid].lastVelocity[i] = entities[eid].velocity[i];
	    entities[eid].lastLinearAcc[i] = entities[eid].linearAcceleration[i];
	    entities[eid].lastOrientation[i] = entities[eid].orientation[i];
	    entities[eid].lastAngularVel[i] = entities[eid].angularVelocity[i];
	}
	eulerToQuaternion(entities[eid].lastOrientation,
			  &entities[eid].lastOrientationQ);
	angularVelocityToQuaternion(entities[eid].lastAngularVel,
				    &entities[eid].lastAngularVelQ);

	return 0;
    }
}


/*
 *  s e t P o s D a t a
 *
 *  Set the position data of the entity with id (index) eid to the given
 *  values.
 */

static void
setPosData(int eid, double loc[3], double vel[3], double linAcc[3],
	   double ori[3], double angVel[3])
{
    int i;

    for (i = 0; i < 3; i++) {
	entities[eid].location[i] = loc[i];
	entities[eid].velocity[i] = vel[i];
	entities[eid].linearAcceleration[i] = linAcc[i];
	entities[eid].orientation[i] = ori[i];
	entities[eid].angularVelocity[i] = angVel[i];
    }
}


/*
 *  d i s _ e n t i t y E n t e r
 *
 *  Enter an entity of type etype (DIS_ENTITY_XXX).
 *  The initial location, velocity, linear acceleration, orientation
 *  and angular velocity will be set from the corresponding arguments.
 *  Velocity and acceleration shall be given in world coordinates.
 *  All parameter units are based on meters, radians and seconds.
 *
 *  The world coordinate system used in DIS is GCC (geocentric cartesian
 *  coordinates), an earth-centered right-handed Cartesian system with
 *  the positive X-axis passing through the Prime Meridian at the Equator,
 *  with the positive Y-axis passing through 90 degrees East longitude
 *  at the Equator and with the positive Z-axis passing through the
 *  North Pole.
 *
 *  The body coordinate system used in DIS is centered at the center of
 *  the entity's bounding volume (excluding articulated parts) and have
 *  the positive x-axis pointing to the front of the entity, the positive
 *  y-axis pointing to the right side of the entity and the positive z-axis
 *  pointing out of the bottom of the entity.
 *
 *  Orientation is given as [psi, theta, phi]. Angular velocity is given
 *  as [angular velocity around body x-axis, ditto y, ditto z].
 *
 *  The id to be used for further reference is returned in eid.
 *
 *  Zero is returned on success.
 */

int
dis_entityEnter(int team, craft * c, dis_entity_type * e1, dis_entity_type * e2,
		double loc[3], double vel[3],
		double linAcc[3], double ori[3], double angVel[3],
		int *neid)
{
    static int eid;

    for (eid = 0; eid < MAX_ENTITIES; eid++) {
	if (entities[eid].local < 0) {
	    break;
	}
    }
    if (eid >= MAX_ENTITIES) {
	return -1;
    }
    if (eid > entity_top) {
	entity_top = eid;
    }
#ifdef DIS_DEBUG
    printf("Allocated eid %d; entity_top %d\n", eid, entity_top);
#endif

    entities[eid].local = 1;
    entities[eid].c = c;
    DISxIssueEntityID(app, &entities[eid].entityId);

    entities[eid].forceId = (team == 1) ?
	DISForceFriendly :
	DISForceOpposing;
    entities[eid].entityType = *e1;
    entities[eid].altEntityType = *e2;

    memset(entities[eid].markings, 0, MARKINGS_LEN);
    entities[eid].markings[0] = '\0';
    entities[eid].appearance = 0;
    setPosData(eid, loc, vel, linAcc, ori, angVel);

    *neid = eid;

/*
 *  Allocate  EM emission fields (optimized for the fact that we have
 *  only one system, with one beam, and one potentially tracked target.
 */

    entities[eid].em =
	(EntityEM *) Vmalloc(sizeof(EntityEM));
    entities[eid].em->mode = 0;
    entities[eid].em->cur_target = -1;
    entities[eid].em->em.hdr.pdu_type = PDUTypeEmission;
    entities[eid].em->em.emitter_id = entities[eid].entityId;

    entities[eid].em->s =
	(dis_em_system_info *) Vmalloc(sizeof(dis_em_system_info));
    entities[eid].em->b =
	(dis_beam_info *) Vmalloc(sizeof(dis_beam_info));
    entities[eid].em->target =
	(dis_track_info *) Vmalloc(sizeof(dis_track_info));
    entities[eid].em->em.system = entities[eid].em->s;
    entities[eid].em->em.num_systems = 1;
    entities[eid].em->em.system[0].num_beams = 0;

    if (sendEntityState(eid) < 0)
	return -1;
    else
	return 0;
}


/*
 *  d i s _ e n t i t y E x i t
 *
 *  Remove the local entity with id eid from the simulation.
 *
 *  Zero is returned on success.
 */

int
dis_entityExit(int eid)
{

#ifdef DIS_DEBUG
    printf("Entity exit called: eid %d\n", eid);
#endif

    if (entities[eid].local != 1)
	return -1;
    else {
	entities[eid].appearance = DISAppearanceDamageDestroyed;
	sendEntityState(eid);

	if (entities[eid].em) {
	    free(entities[eid].em->s);
	    free(entities[eid].em->b);
	    free(entities[eid].em->target);
	    free(entities[eid].em);
	}
	entities[eid].local = -1;
	if (eid == entity_top) {
	    entity_top--;
	}
	return 0;
    }
}


/*
 *  d i s _ e n t i t y S t a t e
 *
 *  Update the state information for a local entity.
 *  The information will be broadcasted on the network
 *  only if it is necessary to keep the other hosts dead
 *  reckoning from exceeding the thresholds.
 *  See dis_entityEnter for information about the arguments.
 *
 *  This procedure also handles the transmission of emission PDU's.
 *
 *  Zero is returned on success.
 */

int
dis_entityState(int eid, double loc[3], double vel[3],
		double linAcc[3],
		double ori[3], double angVel[3])
{
    double drLoc[3], drVel[3], drOri[3];
    double delta;
    Quaternion drOriQ;
    int needToSend = 0, needEMPDU = 0, status, i, j;

    if (entities[eid].local != 1)
	return -1;

    setPosData(eid, loc, vel, linAcc, ori, angVel);

/*
 *  EM emission PDU possibly needed ?
 */

    if (entities[eid].em && entities[eid].em->mode > 0) {
	delta = theTime - entities[eid].em->lastTime;
	if (delta > SEND_TIMEOUT_SECONDS) {
	    needEMPDU = 1;
	    for (i=0; i<entities[eid].em->em.num_systems; ++i) {
		for (j=0; j<entities[eid].em->em.system[0].num_beams; ++j) {
		    entities[eid].em->em.system[i].beam[j].fundamental.beam_sweep_sync += delta;
		}
	    }
	    entities[eid].em->lastTime = theTime;
	    entities[eid].em->em.state_update = 0;
	}
    }
    if ((theTime - entities[eid].lastTime) > SEND_TIMEOUT_SECONDS) {
	needToSend = 4;
    }
    else {
	deadReckonRVW(theTime - entities[eid].lastTime,
		      entities[eid].lastLocation,
		      entities[eid].lastVelocity,
		      entities[eid].lastLinearAcc,
		      &entities[eid].lastOrientationQ,
		      &entities[eid].lastAngularVelQ,
		      drLoc, drVel, &drOriQ);
	quaternionToEuler(&drOriQ, drOri);

	needToSend = checkLocationThreshold(locationThreshold, drLoc,
					    entities[eid].location);
	needToSend += checkOrientationThreshold(orientationThreshold, drOri,
					      entities[eid].orientation);
#ifdef DIS_DEBUG
       if (needToSend) {
         static double base = -1;
         double lastOri[3];


#define R2D(x) ((x)*180.0/3.141593)

  	  if (base == -1)
	    base = theTime;

         quaternionToEuler(&entities[eid].lastOrientationQ, lastOri);

         printf("%6.2f: old %5.0f %5.0f %5.0f   %5.0f %5.0f %5.0f,\n"
                "%6.2f: new %5.0f %5.0f %5.0f   %5.0f %5.0f %5.0f,\n"
                "%+6.2f: dr  %5.0f %5.0f %5.0f   %5.0f %5.0f %5.0f\n",
                entities[eid].lastTime - base,
                entities[eid].lastLocation[0], entities[eid].lastLocation[1],
                entities[eid].lastLocation[2],
                R2D(lastOri[0]), R2D(lastOri[1]), R2D(lastOri[2]),
                theTime - base,
                loc[0], loc[1], loc[2],
                R2D(ori[0]), R2D(ori[1]), R2D(ori[2]),
                theTime - entities[eid].lastTime,
                drLoc[0], drLoc[1], drLoc[2],
                R2D(drOri[0]), R2D(drOri[1]), R2D(drOri[2]));
       }
#endif
    }

    if (needToSend > 0) {
	sendEntityState(eid);
    }
    else {
	/* no need to send ES PDU */
    }

    if (needEMPDU) {
	status = DISxWritePDU(app, (dis_pdu *) & entities[eid].em->em);
    }
    return (needToSend > 0) ? 1 : 0;
}


/*
 *  d i s _ g e t E n t i t y S t a t e
 *
 *  Return state information for a remote entity.
 *  The state information is dead reckoned from the last
 *  received data on the entity.
 *
 *  Zero is returned on success.
 */

int
dis_getEntityState(int eid, double loc[3], double vel[3], double ori[3])
{
    int i;
    Quaternion drOriQ;

    if (entities[eid].local < 0) {
	return -1;
    }
    else if (entities[eid].local == 1) {
	for (i = 0; i < 3; i++) {
	    loc[i] = entities[eid].location[i];
	    ori[i] = entities[eid].orientation[i];
	}

	return 0;
    }
    else {
	deadReckonRVW(theTime - entities[eid].lastTime,
		      entities[eid].lastLocation,
		      entities[eid].lastVelocity,
		      entities[eid].lastLinearAcc,
		      &entities[eid].lastOrientationQ,
		      &entities[eid].lastAngularVelQ,
		      entities[eid].location,
		      entities[eid].velocity,
		      &drOriQ);
	quaternionToEuler(&drOriQ, entities[eid].orientation);

	for (i = 0; i < 3; i++) {
	    loc[i] = entities[eid].location[i];
	    vel[i] = entities[eid].velocity[i];
	    ori[i] = entities[eid].orientation[i];
	}

	return 0;
    }
}


/*
 *  d i s _ f i r e
 *
 *  Broadcast information about an entity firing a weapon.
 *  The type of fire is given by ftype as one of the DIS_FIRE_XXX types.
 *  The id's of the firing entity and the target entity are given with
 *  firingEid and targetEid or as DIS_ID_NONE if not known.
 *  The number of rounds, location of the source of fire, the velocity
 *  vector of the rounds and the range of the rounds are given with
 *  the corresponding arguments.
 *  The id of the event generated is returned in eventId.
 *  If the fire type is a missile, a missile entity is created and its
 *  id is returned in missileEid. The user program should generate
 *  position data for the missile during its lifetime by calling
 *  dis_entityState().
 *
 *  Zero is returned on success.
 *
 *  Not yet implemented.
 */

int
dis_fire(int ftype, int firingEid, int targetEid, int rounds,
	 double location[3], double velocity[3], double range,
	 int *eventId, int *missileEid)
{
    /* TODO */
    *eventId = 0;
    *missileEid = 0;

    return 0;
}

static dis_entity_id null_id =
{0, 0, 0};

int
dis_fire_cannon(craft * c, VPoint * pos, VPoint * vel, int quantity, int rate)
{
    dis_fire_pdu fire;
    int status;

    fire.hdr.pdu_type = PDUTypeFire;

    fire.firing_id = entities[c->disId].entityId;
    fire.target_id = null_id;
    fire.munition_id = null_id;

/*    DISxIssueEventID(app, &fire.event); */

    ACMtoDISWorld(pos, &fire.pos);
    ACMtoDISVelocity(vel, &fire.vel);
    fire.burst.munition = cannon_types[0];
    fire.burst.warhead = 0;
    fire.burst.fuze = 0;
    fire.burst.quantity = quantity;
    fire.burst.rate = rate;
    fire.range = 0.0;

    status = DISxWritePDU(app, (dis_pdu *) & fire);
    return (status == 0) ? 0 : -1;
}


/*
 *  d i s _ d e t o n a t i o n
 *
 *  Broadcast information about a detonation.
 *  The type of fire is given by ftype as one of the DIS_FIRE_XXX types.
 *  The id's of the firing entity and the target entity are given with
 *  firingEid and targetEid or as DIS_ID_NONE if not known.
 *  The id of the corresponding fire event is given as eventId or as
 *  DIS_ID_NONE if not known.
 *  If the detonation is from a missile, the id of the missile is given
 *  as missileEid  or as DIS_ID_NONE. The library will exit the
 *  missile entity.
 *  The location of the detonation in world coordinates and in target
 *  body coordinates are given as worldLocation and entityLocation.
 *
 *  Zero is returned on success.
 */

int
dis_detonation(dis_entity_type * etype,
	       int firingEid, int targetEid, int missileEid,
	       double worldLocation[3], double entityLocation[3])
{
    dis_detonation_pdu pdu, *dPDU = &pdu;
    int status;
    dPDU->hdr.pdu_type = PDUTypeDetonation;

    if (entities[firingEid].local != 1) {
	return -1;
    }
    if (targetEid != DIS_ID_NONE && entities[targetEid].local == -1) {
	return -2;
    }
    dPDU->firing_id = entities[firingEid].entityId;
    if (targetEid != DIS_ID_NONE) {
	dPDU->target_id = entities[targetEid].entityId;
    }
    else {
	dPDU->target_id.sim_id.site_id = 0;
	dPDU->target_id.sim_id.application_id = 0;
	dPDU->target_id.entity_id = 0;
    }

    if (missileEid != DIS_ID_NONE) {
	dPDU->munition_id = entities[missileEid].entityId;
    }
    else {
	dPDU->munition_id.sim_id.site_id = 0;
	dPDU->munition_id.sim_id.application_id = 0;
	dPDU->munition_id.entity_id = 0;
    }

    DISxIssueEventID(app, &dPDU->event);

    /* HACK */
    dPDU->vel.x = 0;
    dPDU->vel.y = 0;
    dPDU->vel.z = 0;

    dPDU->pos.x = worldLocation[0];
    dPDU->pos.y = worldLocation[1];
    dPDU->pos.z = worldLocation[2];

    dPDU->burst.munition = *etype;

/*
 *  This code will need some extra work ...
 */

    if (dPDU->burst.munition.category == 2) {
	dPDU->burst.warhead = DISWarheadKinetic;
	dPDU->burst.fuze = DISFuzeContact;
	dPDU->burst.quantity = 1;
	dPDU->burst.rate = 0;
	dPDU->result = DISDetonationResultEntityImpact;
    }
    else {
	dPDU->burst.warhead = DISWarheadHEFragment;
	dPDU->burst.fuze = DISFuzeProximity;
	dPDU->burst.quantity = 1;
	dPDU->burst.rate = 0;
	dPDU->result = DISDetonationResultDetonation;
    }

    dPDU->loc.x = entityLocation[0];
    dPDU->loc.y = entityLocation[1];
    dPDU->loc.z = entityLocation[2];

    dPDU->num_art_parms = 0;
    dPDU->art_parm = NULL;

    status = DISxWritePDU(app, (dis_pdu *) dPDU);
    return (status == 0) ? 0 : -1;
}

/*
 *  Construct an EM emission PDU that reflects the current state of our
 *  radar set.
 */

int
constructEmissionPDU(craft * c, int mode, int update)
{
    dis_em_emission_pdu *em;
    dis_em_system_info *s;
    dis_beam_info *b;
    dis_track_info *target;

    entities[c->disId].em->mode = mode;
    em = &entities[c->disId].em->em;
    s = entities[c->disId].em->s;
    b = entities[c->disId].em->b;
    target = entities[c->disId].em->target;

    DISxIssueEventID(app, &em->event);

    em->system = s;
    em->state_update = update;
    em->num_systems = 1;

    s->location.x = 0.0;
    s->location.y = 0.0;
    s->location.z = 0.0;
    s->emitter_system.name = 0;
    s->emitter_system.function = DISEmitterFuncAirborneFireControl;
    s->emitter_system.id = 1;

    switch (mode) {
    case 0:
	s->num_beams = 0;
	s->beam = NULL;
	break;

/*
 *  Three-bar track while scan mode
 */

    case 1:
	s->num_beams = 1;
	s->beam = b;
	b->beam_id = 1;
	b->beam_parm_index = 0;
	b->beam_function = DISBeamFuncAcquisitionAndTracking;
	b->fundamental.freq = 14000.0;
	b->fundamental.erp = 100.0;
	b->fundamental.prf = 1.0;
	b->fundamental.pulse_width = 1.0;
	b->fundamental.beam_azimuth_center = 0.0;
	b->fundamental.beam_azimuth_sweep = 120.0;
	b->fundamental.beam_elev_center = 0.0;
	b->fundamental.beam_elev_sweep = 120.0;
	b->fundamental.beam_sweep_sync = 0.0;
	b->pad = 0;
	b->jamming_mode = 0;
	break;

/*
 *  Four bar 20 x 30 ACM mode
 */

    case 2:
	s->num_beams = 1;
	s->beam = b;
	b->beam_id = 2;
	b->beam_parm_index = 0;
	b->beam_function = DISBeamFuncAcquisitionAndTracking;
	b->fundamental.freq = 14000.0;
	b->fundamental.erp = 100.0;
	b->fundamental.prf = 1.0;
	b->fundamental.pulse_width = 1.0;
	b->fundamental.beam_azimuth_center = 0.0;
	b->fundamental.beam_azimuth_sweep = 30.0;
	b->fundamental.beam_elev_center = 0.0;
	b->fundamental.beam_elev_sweep = 20.0;
	b->fundamental.beam_sweep_sync = 0.0;
	b->pad = 0;
	b->jamming_mode = 0;
	break;
    }

    entities[c->disId].em->cur_target = c->curRadarTarget;

    if (c->curRadarTarget == -1) {
	b->tracked_target = NULL;
	b->num_targets = 0;
    }
    else {
	b->tracked_target = target;
	b->num_targets = 1;

	target->target =
	    entities[ptbl[c->curRadarTarget].disId].entityId;
	target->emitter_id = 1;
	target->beam_id = 1;
    }
    return 0;
}

/*
 *  Set local entities current radar mode (modes are 0=off, 1=wide scan)
 */

int
dis_setRadarMode(craft * c, int mode, int update)
{
    int status;

    if (entities[c->disId].em->mode == mode) {
	return 0;
    }
    constructEmissionPDU(c, mode, 1);
    status = DISxWritePDU(app, (dis_pdu *) &entities[c->disId].em->em);
    entities[c->disId].em->lastTime = theTime;
    return (status == 0) ? 0 : -1;
}

/*
 *  Notify the world that our current radar target changed
 */

int
dis_radarTargetChanged(craft * c)
{
    int status;

    if (entities[c->disId].em->cur_target == c->curRadarTarget) {
	return 0;
    }
    constructEmissionPDU(c, entities[c->disId].em->mode, 1);
    status = DISxWritePDU(app, (dis_pdu *) &entities[c->disId].em->em);
    entities[c->disId].em->lastTime = theTime;
    return (status == 0) ? 0 : -1;
}

/*
 *  Get the number of beams emitted from this aircraft
 */

int
dis_getBeamCount (craft *c)
{
    if (entities[c->disId].em && entities[c->disId].em->em.num_systems > 0) {
	return entities[c->disId].em->em.system[0].num_beams;
    }
    return 0;
}

/*
 *  Get parameters describing the specified beam
 */

void
dis_getRadarParameters(craft *c, int j, double *az_center, double *az_width,
    		double *el_center, double *el_width, double *e)
{
    dis_beam_info *b = &entities[c->disId].em->em.system[0].beam[j];

    *az_center = b->fundamental.beam_azimuth_center;
    *az_width = b->fundamental.beam_azimuth_sweep;
    *el_center = b->fundamental.beam_elev_center;
    *el_width = b->fundamental.beam_elev_sweep;
    *e = b->fundamental.erp;
}
