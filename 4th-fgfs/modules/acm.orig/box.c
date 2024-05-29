/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
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
#include "box.h"
#include <stdio.h>
#include <string.h>

static FILE *bbin = 0, *bbout = 0;

extern craftType *lookupCraft();
extern int newPlane();
extern void buildEulerMatrix();
extern void transpose();

short rp_map[MAXPLAYERS], rm_map[MAXPROJECTILES];
short pp_map[MAXPLAYERS], pm_map[MAXPROJECTILES];

static char *no_room =
	"No room in player table to add another black box object.\n";

void
newBlackBoxCraft(id, type, name)
int	id;
int	type;
char	*name;
{
	register craft	*c;
	register int	i, max;
	register short	*p;

	switch (type) {

	case CT_PLANE:
	case CT_DRONE:
		if ((i = newPlane (name)) >= 0) {
			c = &ptbl[i];
			c->type = type;
			c->flags = FL_BLACK_BOX;
			strcpy (c->name, "black-box");
			strcpy (c->display, "*none*");
			pp_map[id] = i;
		}
		else {
			fprintf (stderr, no_room);
		}
		return;
/*NOTREACHED*/	break;

	case CT_MISSILE:
	case CT_CANNON:
		c = mtbl;
		max = MAXPROJECTILES;
		p = rm_map;
		break;

	default:
		break;
	}

	for (i=0; i<max; ++i, ++c) {
		if (c->type == CT_FREE) {
			c->type = type;
			c->flags = FL_BLACK_BOX;
			c->cinfo = lookupCraft (name);
			p[id] = i;
		}
	}
}

void
startBlackBoxRecording()
{
	int	i;

	if ((bbout = fopen ("./black_box_output", "w")) == (FILE *) NULL) {
		fprintf (stderr, "unable to open black box recording file\n");
	}
	for (i=0; i<MAXPLAYERS; ++i) {
		rp_map[i] = -1;
	}
	for (i=0; i<MAXPROJECTILES; ++i) {
		rm_map[i] = -1;
	}
}

void
endBlackBoxRecording()
{
	fclose (bbout);
	bbout = (FILE *) NULL;
}

void
startBlackBoxPlayback()
{
	int	i;

	if ((bbin = fopen ("./black_box_input", "r")) == (FILE *) NULL) {
		fprintf (stderr, "unable to open black box playback file\n");
	}
	for (i=0; i<MAXPLAYERS; ++i) {
		pp_map[i] = -1;
	}
	for (i=0; i<MAXPROJECTILES; ++i) {
		pm_map[i] = -1;
	}
}

/*
 *  Update items under the control of black box playback
 */

void
blackBoxInput()
{
	register int	i;
	BBRecord	rec;
	craft		*c;

	if (bbin) {
		while (fread((char *) &rec, 2, 1, bbin) == 1) {

			c = (rec.table == 0) ? &ptbl[pp_map[rec.id]] :
				&mtbl[pm_map[- rec.id]];

			switch (rec.rectype) {

			case BB_TYPE_SHORT_STATE:
				fread((char *) &rec.u.short_state,
					sizeof(rec.u.short_state), 1, bbin);
				if (pp_map[rec.id] == -1)
					break;
				c->prevSg = c->Sg;
				c->Sg = rec.u.short_state.Sg;
				c->Cg = rec.u.short_state.Cg;
				c->curRoll = rec.u.short_state.roll;
				c->curPitch = rec.u.short_state.pitch;
				c->curHeading = rec.u.short_state.heading;
				buildEulerMatrix (c->curRoll, c->curPitch,
					c->curHeading, &c->trihedral);
				transpose (&c->trihedral, &c->Itrihedral);
				break;

			case BB_TYPE_ADD_OBJECT:
				fread((char *) &rec.u.object,
					sizeof(rec.u.object), 1, bbin);
				newBlackBoxCraft(rec.id, rec.u.object.type,
					rec.u.object.name);
				break;

			case BB_TYPE_DELETE_OBJECT:
				pp_map[rec.id] = -1;
				killPlayer (c);
				break;

			case BB_TYPE_END_OF_FRAME:
				return;

			default:
				fprintf (stderr, "unknown rectype in\
 black box recording: %d\n", rec.rectype);
				fclose (bbin);
				bbin = (FILE *) NULL;
				break;
			}
		}
		fclose (bbin);
		bbin = (FILE *) NULL;
		for (i=0; i<MAXPLAYERS; ++i) {
			if (pp_map[i] != -1)
				killPlayer (&ptbl[pp_map[i]]);
		}
	}
}

/*
 *  Write out black box records
 */

void
blackBoxOutput()
{
	register int	i;
	register craft	*c;
	BBRecord	rec;

	if (bbout) {
		for (i=0, c=ptbl; i<MAXPLAYERS; ++i, ++c) {

			if (c->type != CT_FREE) {

				if (rp_map[i] == -1) {
					rp_map[i] = i;
					rec.rectype = BB_TYPE_ADD_OBJECT;
					rec.table = 0;
					rec.id = i;
					rec.u.object.type = c->type;
					strcpy( rec.u.object.name,
						c->cinfo->name);
					fwrite((char *) &rec, 2, 1, bbout);
					fwrite((char *) &rec.u.object,
						sizeof(rec.u.object), 1, bbout);
				}
				rec.rectype = BB_TYPE_SHORT_STATE;
				rec.id = i;
				rec.u.short_state.Sg = c->Sg;
				rec.u.short_state.Cg = c->Cg;
				rec.u.short_state.roll = c->curRoll;
				rec.u.short_state.pitch = c->curPitch;
				rec.u.short_state.heading = c->curHeading;
				fwrite((char *) &rec, 2, 1, bbout);
				fwrite((char *) &rec.u.object,
					sizeof(rec.u.short_state), 1, bbout);
			}
		}
		for (i=0; i<MAXPROJECTILES; ++i) {
			rm_map[i] = -1;
		}

		rec.rectype = BB_TYPE_END_OF_FRAME;
		rec.id = 0;
		fwrite((char *) &rec, 2, 1, bbout);
	}
}

void
blackBoxKillPlayer (id)
int	id;
{
	BBRecord	rec;

	if (bbout) {
		rec.rectype = BB_TYPE_DELETE_OBJECT;
		rec.id = id;
                fwrite((char *) &rec, 2, 1, bbout);
	}
}
