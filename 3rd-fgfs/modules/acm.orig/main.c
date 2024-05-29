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

#if defined(SVR4)
#include <sys/filio.h>
#endif

#include "manifest.h"
#include "patchlevel.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <signal.h>
#include <sys/socket.h>
#include "pm.h"

#undef a

#ifdef SVR4
/*
 * This forces the inclusion of filio.h (and other things) in ioctl.h 
 * on NCR SVR4.
 */
#define BSD_COMP 1
#endif				/* SVR4 */

#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#include <setjmp.h>

#ifdef SVR4
static sigset_t sigset_mask, sigset_omask;
#endif				/* SVR4 */

#if !defined(lint) && !defined(SABER)
static char sccsid[] = "@(#) acm by Riley Rainey; Revision 4.8";
#endif

extern struct servent *getservent();
int sdebug = 1;
int listen_socket;
char *sceneFile = (char *) NULL;
extern double atof();

extern void SetJoystickPort PARAMS((char *));


#ifdef HAVE_DIS

#include "dis.h"

int dis_site = DIS_SITE;
int dis_application = DIS_APPLICATION;
int dis_exercise = DIS_EXERCISE;

extern craftType *lookupCraft(), *lookupCraftByEntityType();

void
disEntityEnterCb(eid, etype, force, cptr)
int eid;
dis_entity_type *etype;
int force;
craft **cptr;
{
    int i, j, top, mtype;
    craft *c;
    char *type;
    int team;
    craft *tbl;

#ifdef DIS_DEBUG
    printf("%8.2f: Network Entity enter: %d\n", curTime, eid);
#endif

    if (etype->kind == DISKindPlatform) {
	tbl = ptbl;
	top = MAXPLAYERS;
	mtype = CT_DIS_PLANE;
    }
    else if (etype->kind == DISKindMunition) {
	tbl = mtbl;
	top = MAXPROJECTILES;
	mtype = CT_DIS_MUNITION;
    }
    else {
	return;
    }

/*
 *  most of this is a copy of the newDrone code in newPlane.c
 */

    for (i = 0; i < top; ++i) {

	if (tbl[i].type == CT_FREE) {

	    if (force == DISForceFriendly) {
		team = 1;
	    }
	    else {
		team = 2;
	    }

	    c = &tbl[i];
	    *cptr = c;

	    c->createTime = curTime;
	    c->vl = NULL;
	    c->disId = eid;
	    c->pIndex = i;
	    c->team = team;
	    c->curOpponent = -1;
	    c->holdCount = 0;

	    c->vl = NULL;
	    c->type = mtype;
	    c->cinfo = lookupCraftByEntityType(etype);

/*
 *  If we don't know about a given entity type, make it a MiG-29
 */

	    if (!c->cinfo) {
		c->cinfo = lookupCraft("MiG-29");
	    }
	    strncpy(c->name, "DIS", sizeof(c->name));

	    c->curThrust = calcThrust(c);

	    c->curNWDef = 0.0;
	    c->flags = 0;
	    c->radarMode = RM_NORMAL;
	    c->curRadarTarget = -1;
	    for (j = 0; j < 3; ++j) {
		c->leftHUD[j] = NULL;
		c->rightHUD[j] = NULL;
	    }

	    break;
	}
    }

    if (i == top)
	fprintf(stderr, "Out of players (increase MAXPLAYERS)\n");
}

#define M_TO_FT(x)     ((x)/0.3048)

void
disDetonationCb(ftype, firingEid, targetEid, time,
		worldLocation, entityLocation, m)
int ftype;
int firingEid;
int targetEid;
double time;
double worldLocation[3];
double entityLocation[3];
craft *m;
{
    craft *c;
    int j;
    VPoint Sg;

    if (m) {
	killMissile(m, (craft *) NULL);
    }

#ifdef DIS_DEBUG
    printf("DIS detonation seen; target %d\n", targetEid);
#endif

    Sg.x = M_TO_FT(worldLocation[0]);
    Sg.y = M_TO_FT(worldLocation[1]);
    Sg.z = M_TO_FT(worldLocation[2]);
    if (ftype == DIS_FIRE_M61A1)
	newExplosion(&Sg, 3.0, 2.0, 0.5);
    else
	newExplosion(&Sg, 15.0, 10.0, 3.0);

    for (c = ptbl, j = 0; j < MAXPLAYERS; ++j, ++c)
	if (c->type != CT_FREE && c->disId == targetEid) {
	    /* found the target */
	    if (c->type != CT_DIS_PLANE) {
		/* the target is a local player, damage him */
		if (absorbDamage(c, ftype == DIS_FIRE_M61A1 ? 3 : 20) == 0)
		    killPlayer(c);
	    }
	    break;
	}
}

int
disInit()
{
    int err, count;
    char name[32];
    struct sockaddr_in sin;
    extern double getCurrentTime();

    curTime = getCurrentTime();

    err = dis_init(dis_exercise,
		   dis_site, dis_application,
		   disEntityEnterCb, disDetonationCb);
    dis_setDRThresholds(DIS_LOCATION_THRESHOLD, DIS_ORIENTATION_THRESHOLD);

    return err;
}

#endif


void
parseinfo(s, a, b, c)
char *s, *a, *b, *c;
{

    char *p;

    for (p = a; *s; ++s, ++p)
	if ((*p = *s) == ' ') {
	    *p = '\0';
	    break;
	}
    ++s;

    for (p = b; *s; ++s, ++p)
	if ((*p = *s) == ' ') {
	    *p = '\0';
	    break;
	}
    ++s;

    strcpy(c, s);

    return;
}

main(argc, argv)
int argc;
char *argv[];
{

    struct sockaddr_in sin;
    int on = 1;
    int i, news;
    char name[64];

#ifdef MALLOC_DEBUG
    mcheck((char *) NULL);
#endif

    curTime = 0.0;

    droneAggressiveness = DEFAULT_DRONE_FACTOR;

    ptblCount = ctblCount = 0;
    chaseOthers = 0;
    visibility = 50.0 * NM;

/*
 *  parse arguments
 */

    for (i = 1; i < argc; ++i) {

	if (*argv[i] == '-')
	    if (strcmp(argv[i], "-simx") == 0) {
		dis_site = 0;
		dis_application = 0;
	    }
	    else if (strcmp(argv[i], "-dis-site") == 0) {
		dis_site = strtol(argv[++i], (char **) NULL, 0);
	    }
	    else if (strcmp(argv[i], "-dis-appl") == 0) {
		dis_application = strtol(argv[++i], (char **) NULL, 0);
	    }
	    else if (strcmp(argv[i], "-dis-exercise") == 0) {
		dis_exercise = strtol(argv[++i], (char **) NULL, 0);
	    }
	    else
		switch (*(argv[i] + 1)) {

		case 'j':
		    if (strcmp(argv[i], "-js") == 0) {
			if (argv[i + 1] && *(argv[i + 1]) != '-') {
			    SetJoystickPort(argv[++i]);
			}
			else {
			    SetJoystickPort("/dev/cua0");
			}
		    }
		    break;

		case 's':
		    sceneFile = argv[++i];
		    break;

		case 'n':
		    strcpy(name, argv[++i]);
		    break;

		case 'a':
		case 'g':
		    arcadeMode = 1;
		    break;

		case 'c':
		    chaseOthers = 1;
		    break;

		case 'd':
		    if (strcmp(argv[i], "-da") == 0 && argv[++i]) {
			droneAggressiveness = atof(argv[i]) * NM;
			if (droneAggressiveness <= MIN_DRONE_FACTOR) {
			    droneAggressiveness = MIN_DRONE_FACTOR;
			}
			else if (droneAggressiveness > 1.0) {
			    droneAggressiveness = 1.0;
			}
			break;
		    }
		case 'v':
		    if (strcmp(argv[i], "-visibility") == 0 &&
			argv[++i]) {
			visibility = atof(argv[i]);
			if (visibility < 1.0) {
			    visibility = 1.0;
			}
			else if (visibility > 500.0) {
			    visibility = 500.0;
			}
			visibility *= NM;
		    }
		    break;

		default:
		    fprintf(stderr, "Invalid switch \"%s\"\n", argv[i]);
		    break;
		}
    }

#ifdef HAVE_DIS
    {

/*
 *  If the user has not specified a DIS application ID and has not referred
 *  us to a SIM/x server, automatically generate an application id based
 *  on the low-order 16-bits of this host's IP address.  This is a bit of a
 *  hack, but it will assure that we get a unique application id so long as we
 *  are not using any sort of DIS bridge software.
 */

	char name[256], *p;
	struct hostent *h;

	gethostname(name, sizeof(name));

	if (dis_application == DIS_APPLICATION) {
	    if ((h = gethostbyname(name)) != (struct hostent *) NULL) {
		p = h->h_addr;
		dis_application = (p[2] << 8) | p[3];
	    }
	}
    }

    if (disInit() == 0)
	disInUse = 1;
    else
	disInUse = 0;
#endif

    printf("\
ACM version %s,  Copyright (C) 1991-1995   Riley Rainey (rainey@netcom.com)\n\n\
ACM comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome to distribute it under the\n\
conditions described in the COPYING file.\n\n", REVISION_STRING);

#ifdef HAVE_DIS
    if (disInUse)
	printf("Distributed Interactive Simulation protocols in use.\n\n");
#endif


#if defined(NETAUDIO)
    printf("This server supports netaudio.\n\n");
#else
#if defined(_HPUX_SOURCE)
    printf("This server supports the HP AAPI.\n\n");
#endif
#endif

    init();

    news = 2;
    if (newPlayer(news, ":0.0", name, "") == 0) {
	write(news, "Ready.\n", 7);
    }
    input();
#ifdef LINT
    return 0;
#endif

}

int peerdied = 0;

acm_sig_t
deadpeer()
{
    fprintf(stderr, "SIGPIPE\n");
    peerdied = 1;
}

static struct sigvec alrm, spipe;
int doUpdate = 0;

acm_sig_t
myalarm()
{
    doUpdate++;
    sigvec(SIGALRM, &alrm, (struct sigvec *) 0);
}

acm_sig_t
killed()
{
    printf("\ninterrupt\n");
    shutdown(listen_socket, 2);
    close(listen_socket);
#ifdef HAVE_DIS
    dis_close();
#endif
    exit(0);
}

input()
{

    fd_set fdset, ofdset;
    int news = -1, playerchange = 0, n, pno, addrlen;
    int on = 1;
    struct sockaddr addr;
#ifdef USE_ALARM
    struct itimerval update;
#endif
    char *bp, buf[128], name[64], display[64], args[256];
    struct timeval zero_timeout, update_timeout;

    signal(SIGINT, killed);
    signal(SIGQUIT, killed);

    zero_timeout.tv_sec = 0;
    zero_timeout.tv_usec = 0;
    update_timeout.tv_sec = 0;
    update_timeout.tv_usec = UPDATE_INTERVAL;

#ifdef USE_ALARM
    alrm.sv_handler = myalarm;
    alrm.sv_mask = 0;
#ifdef __hpux
    alrm.sv_flags = SV_BSDSIG;
#else
    alrm.sv_flags = SV_INTERRUPT;
#endif
    sigvec(SIGALRM, &alrm, (struct sigvec *) 0);
#endif

/*
 *  Set real time clock to interrupt us every UPDATE_INTERVAL usecs.
 */

#ifdef USE_ALARM
    update.it_interval.tv_sec = 0;
    update.it_interval.tv_usec = UPDATE_INTERVAL;
    update.it_value.tv_sec = 0;
    update.it_value.tv_usec = UPDATE_INTERVAL;
    setitimer(ITIMER_REAL, &update, 0);
#endif

    FD_ZERO(&ofdset);
    FD_ZERO(&fdset);
    FD_SET(listen_socket, &ofdset);

    for (;;) {

#ifdef SVR4
	(void) sigprocmask(SIG_SETMASK, &sigset_omask, NULL);
#else
	sigsetmask(0);
#endif				/* SVR4 */

	fdset = ofdset;
#ifdef USE_ALARM
	pno = select(32, &fdset, (fd_set *) NULL, (fd_set *) NULL,
		     (struct timeval *) NULL);
#else
	if (ptblCount == 0) {
	    pno = select(32, &fdset, (fd_set *) NULL, (fd_set *) NULL,
			 &update_timeout);
	}
	else {
	    pno = select(32, &fdset, (fd_set *) NULL, (fd_set *) NULL,
			 &zero_timeout);
	}
	doUpdate++;
#endif

#ifdef SVR4
	(void) sigemptyset(&sigset_mask);
	(void) sigaddset(&sigset_mask, SIGALRM);
	(void) sigprocmask(SIG_BLOCK, &sigset_mask, &sigset_omask);
#else
	sigblock(sigmask(SIGALRM));
#endif				/* SVR4 */

	if (pno < 0) {
	    FD_CLR(listen_socket, &fdset);
	    if (news > 0)
		FD_CLR(news, &fdset);
	}
	if (doUpdate) {
	    doUpdate = 0;
	    redraw();
	}
    }
}
