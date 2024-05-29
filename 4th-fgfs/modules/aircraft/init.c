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

#include <X11/Xos.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "modelacm.h"
#include "alarm.h"

extern craftType *newCraft();
extern craftType *lookupCraft();
extern int  compileAircraftInventory();
extern void initaim9(), initm61a1();
extern void buildExplosion();
extern void placeObject();
extern void InitNavaidSystem();
extern void DMECheckAlarm();
extern void resupplyCheck();
extern void acm_srand PARAMS((int seed));

/*
 *  Defined in doViews.c :
 */

extern VPolygon **poly;
extern long polyCount;

void undersampleObject();

static char *errmsg = "Sorry, I cannot find the file %s in ../objects/ or %s\n";

/*
 *  acm_fopen :  Open an important acm data file (or die trying).
 */

FILE *
acm_fopen (name, access)
char *name, *access;
{

	FILE	*f;
	char	libname[256];

	if ((f = fopen (name, access)) == (FILE *) NULL) {
		strcpy (libname, "../objects/");
		strcat (libname, name);
		if ((f = fopen (libname, access)) == (FILE *) NULL) {
			strcpy (libname, ACM_LIBRARY);
			strcat (libname, name);
			if ((f = fopen (libname, access)) == (FILE *) NULL) {
				fprintf (stderr, errmsg, name, ACM_LIBRARY);
				exit (1);
			}
		}
	}

	return f;
}

/*
 *  acm_find_file :  Find an important acm data file (or die trying).
 */

char *
acm_find_file (name)
char *name;
{
	static char	pname[1024];
	struct stat	statbuf;

	strcpy(pname, name);
	if (stat (name, &statbuf) != 0) {
		strcpy (pname, "../objects/");
		strcat (pname, name);
		if (stat (pname, &statbuf) != 0) {
			strcpy (pname, ACM_LIBRARY);
			strcat (pname, name);
			if (stat (pname, &statbuf) != 0) {
				fprintf (stderr, errmsg, name, ACM_LIBRARY);
				exit (1);
			}
		}
	}

	return pname;
}


/*
 *  readScene :  Process the scene description file, returns zero if successful
 *		 or -1 otherwise.
 */

int
readScene(ground, len)
char	*ground;
int	len;
{
	FILE	*f, *f1;
	char	file[256], *name, *filep;
	double	x, y, z, heading;
	int	s, i = 0, undersample, depthcue;
	craftType * c;
	extern	char *sceneFile;	/* from server.c */

	name = (sceneFile) ? sceneFile : "default-scene";

	f = acm_fopen (name, "r");

	fscanf (f, "%lf %lf %lf %lf", &x, &y, &z, &heading);
	teamLoc[1].x = x * NM;
	teamLoc[1].y = y * NM;
	teamLoc[1].z = z * NM;
	teamHeading[1] = DEGtoRAD(heading);

	fscanf (f, "%lf %lf %lf %lf", &x, &y, &z, &heading);
	teamLoc[2].x = x * NM;
	teamLoc[2].y = y * NM;
	teamLoc[2].z = z * NM;
	teamHeading[2] = DEGtoRAD(heading);

/*
 *  Get the ground color (the first call to fgets() gets us to a new line).
 */
 	
	ground[0] = '\0';
	if (fgets (ground, len, f) == (char *) NULL) {
		return -1;
	}
	if (fgets (ground, len, f) == (char *) NULL) {
		return -1;
	}
	len = strlen(ground);
	if (len > 0 && ground[len-1] == '\n') {
		ground[len-1] = '\0';
	}

	while (s = fscanf(f, "%s %lf %lf %lf %lf",
		file, &x, &y, &z, &heading) == 5) {

		depthcue = 1;

		if (file[0] == '@') {
			filep = &file[1];
			undersample = 1;
		}
		else if (file[0] == '+') {
			filep = &file[1];
			depthcue = 0;
		}
		else {
			filep = file;
			undersample = 0;
		}
		stbl[i].type = CT_SURFACE;
		if ( c = lookupCraft(filep) ) {
			stbl[i].cinfo = c;
		}
		else {
			f1 = acm_fopen (filep, "r");
			stbl[i].cinfo = newCraft();
			stbl[i].cinfo->name = strdup(filep);
			stbl[i].cinfo->object =
				VReadDepthCueuedObject(f1, depthcue);
			if (!stbl[i].cinfo->object) {
				fprintf (stderr,
					"Error reading object %s\n", filep);
				exit (1);
			}
			if (undersample) {
				undersampleObject (stbl[i].cinfo->object, 3);
			}
			fclose (f1);
		}
		stbl[i].Sg.x = x * NM;
		stbl[i].Sg.y = y * NM;
		stbl[i].Sg.z = z * NM;
		stbl[i].curHeading = DEGtoRAD(heading);
		stbl[i].curPitch = stbl[i].curRoll = 0.0;
		if (++i == MAXSURFACE) {
			fprintf (stderr,
		"Only the first %d surface objects will be displayed.\n",
			MAXSURFACE);
			return 0;
		}
	}
	return (s == EOF || s == 0) ? 0 : -1;
}

/*
 *  undersampleObject
 *
 *  Remove some of the detail in an object to reduce the CPU overhead
 *  of transforming and clipping it.
 */

void
undersampleObject (obj, rate)
VObject	*obj;
int	rate;
{
	int	i, j, k;

	k = 0;

	for (i=1; i<obj->numPolys; ++i) {
		for (j=0; j<obj->polygon[i]->numVtces; j+=rate) {
			obj->polygon[i]->vertex[k++] =
				obj->polygon[i]->vertex[j];
		}
		obj->polygon[i]->numVtces = k;
	}
}

/*
 *  init :  Set up the acm server
 */

int
init ()
{
	int	i;
	craft	*p;
	alarm_id_t id;
	char	ground[256];

	polyCount = 32768;
	poly = (VPolygon **) Vmalloc (sizeof(VPolygon *) * polyCount);

	cbase = - 2800.0;
	ctop  = - 9000.0;

/*
 *  VSetDepthCue() must be called before calling VSetVisibility().
 */

	VSetDepthCue (VAllocColor("#749395"), 8);
	VSetVisibility (visibility);


        for ((i=0, p=stbl); i<MAXSURFACE; (++i, ++p))
		p->type = CT_FREE;

        for ((i=0, p=ptbl); i<MAXPLAYERS; (++i, ++p)) {
		p->pIndex = i;
		p->type = CT_FREE;
	}

        for ((i=0, p=mtbl); i<MAXPROJECTILES; (++i, ++p))
		p->type = CT_FREE;

	acm_srand ( (int) time(0) );

	HUDColor = VAllocColor ("white");
	whiteColor = VAllocColor ("white");
	blackColor = VAllocColor ("black");
	radarColor = VAllocColor ("#0c0");
	radarBackgroundColor = VAllocColor ("#094200");
	cloudColor = VAllocDepthCueuedColor ("#ccc", 1);

	buildExplosion();

/*
 *  Set up the scene
 */

	if (readScene (ground, sizeof(ground)) != 0) {
		fprintf (stderr, "Fatal error\n");
		exit (1);
	}

	groundColor = VAllocDepthCueuedColor (ground, 1);

/*
 *  Compile the aircraft inventory
 */

	if (compileAircraftInventory() != 0) {
		fprintf (stderr, "Fatal error\n");
		exit (1);
	}

/*
 *  Initialize weapons
 */

	initaim9();
	initm61a1();

/*
 *  Set up the navaids.
 */

	InitNavaidSystem ();
	id = addAlarm (5.0, DMECheckAlarm, NULL, NULL);

/*
 *  Add the periodic resupply check procedure
 */

	id = addAlarm (RESUPPLY_INTERVAL, resupplyCheck, NULL, NULL);

/*
 *  Set time intervals.
 */

	deltaT = (double) UPDATE_INTERVAL / 1000000.0;

	halfDeltaTSquared = 0.5 * deltaT * deltaT;

	return 0;
}
