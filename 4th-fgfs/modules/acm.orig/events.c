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

#include "pm.h"
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>

#define MAX_MAPPED_STRING_LEN	20
#define MAX_POPUP_STRING_LEN	40

extern int cur;

extern void resizePlayerWindow(), resizeChaserWindow(), redrawPanel();
extern int debug;
extern int newDrone();

extern void startBlackBoxRecording(), endBlackBoxRecording();
extern void startBlackBoxPlayback();
extern void radioFrequencyChanged();
extern void CalibrateJoystick();

void 
doEvents(c)
craft *c;
{

    XEvent ev;
    viewer *u;
    int player;

/*
 *  Process events for each player and any "back seaters"
 */

    player = 1;
    for (u = c->vl; u != (viewer *) NULL; u = u->next) {


#define mask 	(-1L)

#ifndef REAL_DELTA_T
	if (cur % REDRAW_EVERY == 0)
#endif
	    getStick(c, u);

	while (XEventsQueued(u->dpy, QueuedAlready)) {

	    if (XCheckWindowEvent(u->dpy, u->win, mask, &ev)) {

		switch (ev.type) {

		case KeyPress:
		    if ((c->type == CT_CHASER) ?
			doKeyEventChaser(c, u, &ev, player) :
			doKeyEvent(c, u, &ev, player) < 0)
			return;
		    break;

		case ButtonPress:
		    if (doButtonEvent(c, u, &ev, player) < 0)
			return;
		    break;

		case ButtonRelease:
		    if (doButtonReleaseEvent(c, u, &ev, player) < 0)
			return;
		    break;

		case ConfigureNotify:
		    if (c->type == CT_CHASER) {
			resizeChaserWindow(c, u,
					   ev.xconfigure.width,
					   ev.xconfigure.height, 0);
		    } else {
			resizePlayerWindow(c, u,
					   ev.xconfigure.width,
					   ev.xconfigure.height, 0);
		    }
		    VResizeViewport(u->v, 12.0 * 25.4 / 1000.0,
				    u->scaleFactor, 0.5,
				    u->width,
				    u->height);
		    break;

		case Expose:
		    redrawPanel(c, u);
		    ForceWindowRedraw(u->v->w);
		    break;

		default:
		    break;
		}

	    } else if (XCheckTypedEvent(u->dpy, ClientMessage, &ev)) {
		if (ev.xclient.message_type == u->protocolsAtom
		    && (ev.xclient.data.l[0] == u->deleteWindowAtom ||
			ev.xclient.data.l[0] == u->closedownAtom)) {
		    if (c->type == CT_CHASER) {
			killChaser(c);
		    } else {
			killPlayer(c);
		    }
		}
	    }
	}

	player = 0;

    }
}

/*ARGSUSED */
int 
doButtonEvent(c, u, ev, player)
craft *c;
viewer *u;
XEvent *ev;
int player;
{

    if (ev->xbutton.button == Button2)
	fireWeapon(c);

    else if (ev->xbutton.button == Button3)
	selectWeapon(c);

    return 0;
}

static int last_switches = 0;

/*ARGSUSED */
int 
doJoystickEvent(c, u, throttle, switches)
craft *c;
viewer *u;
double throttle;
int switches;
{

    int switch_xor = switches ^ last_switches;

/*
 *  Change in state of any buttons ?
 */

    if (switch_xor != 0) {

	if (switch_xor & 1) {

/*
 *  Button 1 press
 */
	    if (switches & 1) {
		fireWeapon(c);
	    }

/*
 *  Button 1 release
 */

	    else {
	    	ceaseFireWeapon(c);
	    }
	}

/*
 * Button 2 press
 */

	if (switch_xor & 2) {
	    if (switches & 2) {
		selectWeapon(c);
	    }
	}

    }

    last_switches = switches;
#ifdef notdef
    c->throttle = 6553 + (32768 - 6553) * throttle;
#endif

    return 0;
}

/*ARGSUSED */
int 
doButtonReleaseEvent(c, u, ev, player)
craft *c;
viewer *u;
XEvent *ev;
int player;
{

    if (ev->xbutton.button == Button2)
	ceaseFireWeapon(c);

    return 0;
}

/*ARGSUSED */
int 
doKeyEvent(c, u, ev, player)
craft *c;
viewer *u;
XEvent *ev;
int player;
{

    KeySym keysym;
    XComposeStatus compose;
    char buffer[MAX_MAPPED_STRING_LEN];
    int buflen = MAX_MAPPED_STRING_LEN;
#ifdef SPECIAL_KEYS
    FILE *fp;
    craft pentry;
#endif

    (void) XLookupString((XKeyEvent *) ev, buffer, buflen,
			 &keysym, &compose);

    if (player) {

	switch (keysym) {

#ifdef sun
	case XK_R4:
#else
	case XK_Left:
#endif
	    if (c->flags & FL_NWS) {
		c->curNWDef = c->curNWDef - c->cinfo->NWIncr;
		if (c->curNWDef < -c->cinfo->maxNWDef)
		    c->curNWDef = -c->cinfo->maxNWDef;
	    } else {
		c->Sa = c->Sa - 0.05;
	    }
	    break;

#ifdef sun
	case XK_R6:
#else
	case XK_Right:
#endif
	    if (c->flags & FL_NWS) {
		c->curNWDef = c->curNWDef + c->cinfo->NWIncr;
		if (c->curNWDef > c->cinfo->maxNWDef)
		    c->curNWDef = c->cinfo->maxNWDef;
	    } else {
		c->Sa = c->Sa + 0.05;
	    }
	    break;

#ifdef sun
	case XK_R2:
#else
	case XK_Up:
#endif
	    c->Se = c->Se - 0.01;
	    break;

#ifdef sun
	case XK_R5:
#else
	case XK_Down:
#endif
	    c->Se = c->Se + 0.01;
	    break;

	case XK_z:
	case XK_Z:
	    c->Sr = c->Sr + 0.01;
	    break;

	case XK_c:
	case XK_C:
	    c->Sr = c->Sr - 0.01;
	    break;

	case XK_x:
	case XK_X:
	    c->Sr = 0.0;
	    break;

#ifdef sun
	case XK_R7:
#else
	case XK_Home:
#endif
	    c->Se = 0.0;
	    c->Sr = 0.0;
	    c->Sa = 0.0;
	    break;

/*
 *  J  :  Adjust Elevator Trim
 */
	case XK_J:
	case XK_j:
	    c->SeTrim += c->Se;
	    break;

/*
 *  U  :  Set Elevator trim to the Takeoff setting
 */

	case XK_U:
	case XK_u:
	    c->SeTrim = c->cinfo->SeTrimTakeoff;
	    break;

	case XK_y:
	case XK_Y:
	    flapsUp(c);
	    break;

	case XK_h:
	case XK_H:
	    flapsDown(c);
	    break;

	case XK_w:
	case XK_W:
	    speedBrakeRetract(c);
	    break;

	case XK_s:
	case XK_S:
	    speedBrakeExtend(c);
	    break;

	case XK_2:
	case XK_KP_Subtract:
	    c->throttle = (c->throttle <= 6553) ? 6553 :
		c->throttle - 512;
	    break;

	case XK_3:
	case XK_KP_Add:
	    c->throttle = (c->throttle >= 32768) ? 32768 :
		c->throttle + 512;
	    break;

	case XK_4:
	case XK_Prior:
	    c->throttle = 32768;
	    break;

	case XK_1:
	case XK_Next:
	    c->throttle = 8192;
	    break;

	case XK_A:
	case XK_a:
	    c->flags ^= FL_AFTERBURNER;
	    break;

	case XK_7:
	    c->hsiSelect->obs_setting =
		(c->hsiSelect->obs_setting + 20) % 360;
	    break;

	case XK_8:
	    c->hsiSelect->obs_setting =
		(c->hsiSelect->obs_setting + 1) % 360;
	    break;

	case XK_9:
	    c->hsiSelect->frequency =
		(c->hsiSelect->frequency + 20) %
		VOR_CHANNEL_COUNT;
	    radioFrequencyChanged(c->hsiSelect);
	    break;

	case XK_0:
	    c->hsiSelect->frequency =
		(c->hsiSelect->frequency + 1) %
		VOR_CHANNEL_COUNT;
	    radioFrequencyChanged(c->hsiSelect);
	    break;

	case XK_B:
	case XK_b:
	    c->flags ^= FL_BRAKES;
	    break;

	case XK_G:
	case XK_g:
	    playSound(c, (c->flags & FL_GHANDLE_DN) ?
		      SoundGearUp : SoundGearDown);
	    c->flags ^= FL_GHANDLE_DN;
	    break;

	case XK_L:
	case XK_l:
	    newDrone(c);
	    break;

#ifdef sun
	case XK_Up:
#else
	case XK_KP_8:
#endif
	    c->viewDirection.x = 1.0;
	    c->viewDirection.y = 0.0;
	    c->viewDirection.z = 0.0;
	    c->viewUp.x = 0.0;
	    c->viewUp.y = 0.0;
	    c->viewUp.z = -1.0;
	    break;

/* look right */

#ifdef sun
	case XK_Right:
#else
	case XK_KP_6:
#endif
	    c->viewDirection.x = 0.0;
	    c->viewDirection.y = 1.0;
	    c->viewDirection.z = 0.0;
	    c->viewUp.x = 0.0;
	    c->viewUp.y = 0.0;
	    c->viewUp.z = -1.0;
	    break;

/* look left */

#ifdef sun
	case XK_Left:
#else
	case XK_KP_4:
#endif
	    c->viewDirection.x = 0.0;
	    c->viewDirection.y = -1.0;
	    c->viewDirection.z = 0.0;
	    c->viewUp.x = 0.0;
	    c->viewUp.y = 0.0;
	    c->viewUp.z = -1.0;
	    break;

/* look back */

#ifdef sun
	case XK_Down:
#else
	case XK_KP_2:
#endif
	    c->viewDirection.x = -1.0;
	    c->viewDirection.y = 0.0;
	    c->viewDirection.z = 0.0;
	    c->viewUp.x = 0.0;
	    c->viewUp.y = 0.0;
	    c->viewUp.z = -1.0;
	    break;

/* look up */

#ifdef sun
	case XK_R11:
#else
	case XK_KP_5:
#endif
	    c->viewDirection.x = 0.0;
	    c->viewDirection.y = 0.0;
	    c->viewDirection.z = -1.0;
	    c->viewUp.x = -1.0;
	    c->viewUp.y = 0.0;
	    c->viewUp.z = 0.0;
	    break;

	case XK_Q:
	case XK_q:
	    c->curRadarTarget = newRadarTarget(c);
#ifdef HAVE_DIS
	    dis_radarTargetChanged (c);
#endif
	    break;

	case XK_R:
	case XK_r:
	    switch (c->radarMode) {

	    case RM_STANDBY:
		c->radarMode = RM_NORMAL;
		c->curRadarTarget = -1;
		break;

	    case RM_NORMAL:
		c->radarMode = RM_ILS;
		c->curRadarTarget = -1;
		break;

	    case RM_ILS:
		c->radarMode = RM_STANDBY;
		c->curRadarTarget = -1;
		break;
	    }
#ifdef HAVE_DIS
	    switch (c->radarMode) {
	    case RM_OFF:
	    case RM_STANDBY:
	    case RM_ILS:
		dis_setRadarMode (c, 0);
		break;
	    case RM_NORMAL:
		dis_setRadarMode (c, 1);
		break;
	    }
#endif
	    break;

	case XK_N:
	case XK_n:
	    c->flags ^= FL_CHASE_VIEW;
	    break;

#ifdef SPECIAL_KEYS

	case XK_o:
	    if (absorbDamage(c, 3) == 0) {
		killPlayer(c);
		return -1;
	    }
	    break;

	case XK_I:
	    fp = fopen("./flight-state", "w");
	    fwrite((char *) &ptbl[c->pIndex],
		   sizeof(craft), 1, fp);
	    fclose(fp);
	    break;

	case XK_i:
	    if (fp = fopen("./flight-state", "r")) {
		fread((char *) &pentry, sizeof(craft), 1, fp);
		fclose(fp);

		c->flags = pentry.flags;
		c->Sg = pentry.Sg;
		c->prevSg = pentry.prevSg;
		c->Cg = pentry.Cg;
		c->trihedral = pentry.trihedral;
		c->Itrihedral = pentry.Itrihedral;
		c->curHeading = pentry.curHeading;
		c->curPitch = pentry.curPitch;
		c->curRoll = pentry.curRoll;
		c->p = pentry.p;
		c->q = pentry.q;
		c->r = pentry.r;
		c->damageBits = pentry.damageBits;
		c->structurePts = pentry.structurePts;
		c->damageCL = pentry.damageCL;
		c->damageCM = pentry.damageCM;
		c->curThrust = pentry.curThrust;
		c->throttle = pentry.throttle;
	    } else {
		fprintf(stderr,
			"unable to open ./flight-state\n");
	    }
	    break;

	case XK_semicolon:
	    debug ^= 1;
	    break;

#endif

	case XK_P:
	case XK_p:
	    killPlayer(c);
	    return -1;
/*NOTREACHED */ break;

	case XK_braceleft:
	    startBlackBoxRecording();
	    break;

	case XK_braceright:
	    endBlackBoxRecording();
	    break;

	case XK_bracketleft:
	    startBlackBoxPlayback();
	    break;

	case XK_k:
	case XK_K:
	    CalibrateJoystick();
	    break;

	}

    }
    return 0;
}
