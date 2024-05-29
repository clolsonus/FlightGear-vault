/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1993 Riley Rainey, Tetsuji Rai
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
#include <malloc.h>

#define MAX_MAPPED_STRING_LEN	20
#define MAX_POPUP_STRING_LEN	40

extern int debug;
extern void startBlackBoxRecording();
extern void endBlackBoxRecording();
extern void startBlackBoxPlayback();

int
newChaser (who, me)
int who;
char *me;
{
  register craft   *c;
  int i;

    if (ptbl[who].type == CT_PLANE || ptbl[who].type == CT_DRONE) {
      if (!chaseOthers && ptbl[who].type != CT_DRONE &&
	strcmp(ptbl[who].name, me))
	return NC_CANNOT_CHASE;
      for ( i=0; i<MAXCHASERS; i++) {
	if (ctbl[i].type == CT_CHASER)
	  continue;
	ctbl[i].cinfo = (craftType *) malloc( sizeof( craftType) );
	ctbl[i].viewDirection.x = 1.;
	ctbl[i].viewDirection.y = 0.;
	ctbl[i].viewDirection.z = 0.;
	ctbl[i].viewUp.x = 0.;
	ctbl[i].viewUp.y = 0.;
	ctbl[i].viewUp.z = -1.0;
	ctbl[i].Sg.x = ptbl[who].Sg.x;
	ctbl[i].Sg.y = ptbl[who].Sg.y;
	ctbl[i].Sg.z = ptbl[who].Sg.z;
	ctbl[i].p = 100.;  /* distance from the chased plane */
	return i;
      }
    }

  return NC_NOT_PLANE;
}

/*ARGSUSED*/
int
doKeyEventChaser (c, u, ev, player)
craft	*c;
viewer	*u;
XKeyEvent	*ev;
int	player; 
{

	KeySym		keysym;
	XComposeStatus	compose;
	char		buffer[MAX_MAPPED_STRING_LEN];
	int		buflen = MAX_MAPPED_STRING_LEN;

	(void) XLookupString (ev, buffer, buflen, &keysym, &compose);

	if (player) {

		switch (keysym) {

		case XK_j:
		case XK_J:
			c->p -= 20; if (c->p < 20) c->p = 20;
			break;

		case XK_k:
		case XK_K:
			c->p += 20;
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
/*
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

*/
#ifdef SPECIAL_KEYS

		case XK_o:
			if (absorbDamage(c, 3) == 0) {
				killChaser (c);
				return -1;
			}
			break;

		case XK_semicolon:
			debug ^= 1;
			break;

#endif

		case XK_P:
		case XK_p:
			killChaser (c);
			return -1;
/*NOTREACHED*/		break;

		case XK_braceleft:
			startBlackBoxRecording();
			break;

		case XK_braceright:
			endBlackBoxRecording();
			break;

		case XK_bracketleft:
			startBlackBoxPlayback();
			break;

		}

	}

	return 0;
}


int
killChaser (c)
craft *c;
{

  viewer *v, *vn;
  int i;

  /*
   *  Decrement the player count, iff this is a real person that just got
   *  killed.
   */

  if (c->type == CT_CHASER)
    {
      --ctblCount;
    }

  /*
   *  Free HUD string storage
   */

  if (c->leftHUD[0] != (char *) NULL)
    for (i = 0; i < 3; ++i)
      {
	free (c->leftHUD[i]);
	free (c->rightHUD[i]);
      }

  /*
   *  Close viewers' display
   */

  for (v = c->vl; v != (viewer *) NULL;)
    {
      VCloseViewport (v->v);
      XCloseDisplay (v->dpy);
      vn = (viewer *) v->next;
      free ((char *) v);
      v = vn;
    }

  c->type = CT_FREE;
  return 0;
}

void /*ARGSUSED*/
resizeChaserWindow (c, u, width, height, initial_flag)
craft	*c;
viewer	*u;
int	width;
int	height;
int	initial_flag;
{

	u->width = width;
	u->height = height;
	u->xCenter = (u->width + 1) / 2;
	u->yCenter = (u->height + 1) / 2;
	u->scaleFactor = (double) width / (double) VIEW_WINDOW_WIDTH;;

}
