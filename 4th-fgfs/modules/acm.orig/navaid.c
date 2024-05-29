/*
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
#include "pm.h"
#include "alarm.h"
#include <math.h>

struct _navaid_t *vhf_navaid[VOR_CHANNEL_COUNT];

static char *bad_freq = "*****";

static navaid_t *test_navaid;

double mag_var = DEGtoRAD (8.0);  /* eight degrees east */

void InitNavaid();
char * PrintableVHFFreq();

extern alarm_id_t addAlarm ();
extern double heading();

double
magHeading (c)
craft	*c;
{
	register double h = c->curHeading - mag_var;
	return (h < 0.0) ? h + 2.0 * pi : h;
}

void
InitNavaidSystem()
{
	navaid_t	*n;

	test_navaid = (navaid_t *) malloc (sizeof (navaid_t));
	test_navaid->next = (navaid_t *) NULL;
	strcpy (test_navaid->id, "ONE");
	test_navaid->frequency = 20;		/* 109.0 */
	test_navaid->flags = NAVAID_VOR | NAVAID_DME;
	test_navaid->bearing = DEGtoRAD (8.0);	/* mag variation */
	test_navaid->slope = 0.0;
	test_navaid->Sg.x = 0.0;
	test_navaid->Sg.y = 2000.0;
	test_navaid->Sg.z = 0.0;
	InitNavaid (test_navaid);

	n = (navaid_t *) malloc (sizeof (navaid_t));
	strcpy (n->id, "IONE");
	n->frequency = 40;		/* 110.0 */
	n->flags = NAVAID_LOC | NAVAID_DME | NAVAID_GS;
	n->bearing = DEGtoRAD (0.0);
	n->slope = DEGtoRAD (2.75);
	n->Sg.x = -500.0;
	n->Sg.y = 0.0;
	n->Sg.z = 0.0;
	n->gs_Sg.x = 11000.0;
	n->gs_Sg.y = 200.0;
	n->gs_Sg.z = 0.0;
	n->beam_width = RADtoDEG(atan (350.0/13000.0)) / 2.0;
	InitNavaid (n);
	n->next = test_navaid;
	test_navaid = n;

	n = (navaid_t *) malloc (sizeof (navaid_t));
	strcpy (n->id, "ITWO");
	n->frequency = 70;		/* 111.5 */
	n->flags = NAVAID_LOC | NAVAID_DME | NAVAID_GS;
	n->bearing = DEGtoRAD (180.0);
	n->slope = DEGtoRAD (2.75);
	n->Sg.x = -0.6 * NM - 500.0;
	n->Sg.y = 49.0 * NM;
	n->Sg.z = 0.0;
	n->gs_Sg.x = -0.6 * NM + 1000;
	n->gs_Sg.y = 49.0 * NM - 200.0;
	n->gs_Sg.z = 0.0;
	n->beam_width = RADtoDEG (atan (350.0/13000.0)) / 2.0;
	InitNavaid (n);
	n->next = test_navaid;
	test_navaid = n;

	n = (navaid_t *) malloc (sizeof (navaid_t));
	strcpy (n->id, "TWO");
	n->frequency = 90;		/* 112.5 */
	n->flags = NAVAID_VOR | NAVAID_DME;
	n->bearing = DEGtoRAD (8.0);	/* mag variation */
	n->slope = DEGtoRAD (0.0);
	n->Sg.x = 1 * NM;
	n->Sg.y = 49.0 * NM + 1000;
	n->Sg.z = 0.0;
	InitNavaid (n);
	n->next = test_navaid;
	test_navaid = n;
	
}



int
radioReceptionCheck (r)
radio_t *r;
{
	navaid_t	*n;

	for (n = test_navaid; n; n=n->next) {
		if (r->frequency == n->frequency) {
			r->station = n;
			return 1;		
		}
	}

	return 0;
}

void
radioFrequencyChanged (r)
radio_t *r;
{
	r->station = NULL;
	radioReceptionCheck (r);
	PrintableVHFFreq(r->frequency, r->freq_readout);
	strcpy (r->dme_readout, "DME ----");
}

void
initRadio (r)
radio_t	*r;
{
	r->frequency = r->standby_frequency = 0;
	r->obs_setting = 0;
	radioFrequencyChanged(r);
}

void
InitNavaid (n)
navaid_t	*n;
{
	VMatrix	m;
	VPoint  p;

	VIdentMatrix (&m);
	VSetPoint (p, 1.0, 0.0, 0.0);

	switch (n->flags & (NAVAID_VOR | NAVAID_LOC | NAVAID_GS)) {

	case NAVAID_VOR:
	case NAVAID_LOC:
		break;

	case (NAVAID_LOC | NAVAID_GS):
		VRotate (&m, ZRotation, n->bearing);
		VRotate (&m, YRotation, n->slope);
		n->gst = m;
		break;
	}
}

double
glideSlopeOffset (n, c)
navaid_t	*n;
craft		*c;
{
	VPoint	p, p1;

	p.x = c->Sg.x - n->gs_Sg.x;
	p.y = c->Sg.y - n->gs_Sg.y;
	p.z = c->Sg.z - n->gs_Sg.z;

	VTransform (&p, &n->gst, &p1);
	return atan2 (p1.z, p1.x);

}

double
radial (n, c)
navaid_t	*n;
craft		*c;
{
	VPoint	p;
	double	true_radial, mag_radial;

	p.x = c->Sg.x - n->Sg.x;
	p.y = c->Sg.y - n->Sg.y;
	p.z = c->Sg.z - n->Sg.z;

	true_radial = heading (&p);
	mag_radial = true_radial - n->bearing;
	if (mag_radial > 2.0 * M_PI)
		mag_radial -= 2.0 * M_PI;
	else if (mag_radial < 0.0)
		mag_radial += 2.0 * M_PI;

	return mag_radial;
}

char *
PrintableVHFFreq (f, s)
freq_t f;
char *s;
{
	if (f > 180) {
	    fprintf (stderr,
		"invalid frequency passed to PrintableVHFFreq\n");
	    return bad_freq;
	}

	sprintf (s, "%5d", 10800 + 5 * f);
	s[6] = '\0';
	s[5] = s[4];
	s[4] = s[3];
	s[3] = '.';

	return s;
}

char *
PrintableTACANChannel (f, s)
freq_t f;
char *s;
{
	if (f > 180) {
	    fprintf (stderr,
		"invalid frequency passed to PrintableTACANChannel\n");
	    return bad_freq;
	}

	sprintf (s, "%d%c", f / 2 + 17, (f % 2) ? 'Y' : 'X');
	return s;
}

/* ARGSUSED */
void
DMECheckAlarm (arg1, arg2)
char	*arg1, *arg2;
{

	int	i;
	craft	*c;
	VPoint	p;
	navaid_t *n;
	alarm_id_t id;

	for (i=0, c = ptbl; i < MAXPLAYERS; ++i, ++c) {
		if ((c->type == CT_PLANE) && c->hsiSelect->station) {
	
			n = c->hsiSelect->station;
			p.x = c->Sg.x - n->Sg.x;
			p.y = c->Sg.y - n->Sg.y;
			p.z = c->Sg.z - n->Sg.z;

			sprintf (c->hsiSelect->dme_readout, "DME %.1f",
				mag(p) / 6076.115);
		}
	}

	id = addAlarm (5.0, DMECheckAlarm, (char *) NULL, (char *) NULL);
}
