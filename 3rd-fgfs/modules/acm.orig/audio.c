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

#include <pm.h>
#include <X11/Xos.h>

static char	*files[] = {
	"crash.au",
	"gear_up.au",
	"gear_dn.au",
	"missile.au",
	"cannon.au",
	"crash.au",
	"screetch.au",
	"stall.au",
	"rwr.au"
	};
	
static char	*paths[NUM_SOUNDS];

#if defined(NETAUDIO)

static void	done_callback();

int
initializeAudio(c, v, display)
craft *c;
viewer *v;
char *display;
{
	int		i, n;
	AuBucketAttributes *ret_attr;
	AuDeviceAttributes *dev;
	AuStatus	status;
	char		auserver[256], *p;

/*
 *  If the X display specification has a screen field, remove it so that we
 *  can use it as an audio server name.
 */
 
	strcpy (auserver, display);
	n = strlen(auserver);	
	if (auserver[n] == '.') {
		auserver[n] = '\0';
	}

/*
 *  Lazy initialization of sound paths array
 */

	if (!paths[0]) {
		for (i=0; i<NUM_SOUNDS; ++i) {
			paths[i] = strdup(acm_find_file(files[i]));
		}
	}

/*
 *  Open the audio server and send it our sounds
 */
	
	if ((v->aserver = AuOpenServer(auserver, 0, (char *) NULL,
		0, (char *) NULL, (char **) NULL)) == NULL) {
		v->aserver = (AuServer *) NULL;
		return -1;
	}

	for (i=0; i<NUM_SOUNDS; ++i) {
		if ((v->sound[i] = AuSoundCreateBucketFromFile (v->aserver,
			paths[i], AuAccessListMask | AuAccessImportMask,
			&ret_attr, &status)) == AuNone) {
			printf ("Unable to create sound bucket: %s\n",
				paths[i]);
			AuCloseServer (v->aserver);
			v->aserver = (AuServer *) NULL;
			return -1;
		}
		v->flow[i] = 0;
	}

/*
 *  Select an output device
 */

	v->audioOutput[0] = AuNone;
	for (i=0; i<AuServerNumDevices(v->aserver); ++i) {
		dev = AuServerDevice(v->aserver, i);
		if (AuDeviceKind(dev) == AuComponentKindPhysicalOutput &&
		    AuDeviceNumTracks(dev) == 1) {
			v->audioOutput[0] = AuDeviceIdentifier(dev);
			break;
		}
	}

	return 0;
}

void
shutdownAudio(c, v)
craft *c;
viewer *v;
{
	int i;
	AuStatus status;

	if (v->aserver) {
		for (i=0; i<NUM_SOUNDS; ++i) {
			AuDestroyBucket(v->aserver, v->sound[i], &status);
		}
		AuCloseServer (v->aserver);
		v->aserver = (AuServer *) NULL;
	}
}

void
playSound (c, id)
craft *c;
int id;
{
	viewer *v;
	AuStatus	status;

	for (v=c->vl; v; v=v->next) {
		if (v->aserver) {
			AuSoundPlayFromBucket (v->aserver, v->sound[id],
				v->audioOutput[0], AuFixedPointFromSum(1,0),
				NULL, NULL, 0, &v->flow[id], (int *) NULL,
				(int *) NULL, &status);
		}
	}
}

void
playContinuousSound (c, id)
craft *c;
int id;
{
	viewer *v;
	AuStatus	status;

	for (v=c->vl; v; v=v->next) {
		if (v->aserver) {
			AuSoundPlayFromBucket (v->aserver, v->sound[id],
				v->audioOutput[0], AuFixedPointFromSum(1,0),
				done_callback, (AuPointer) v,
				10, &v->flow[id], (int *) NULL, (int *) NULL,
				&status);
		}
	}
}

void
stopSound(c, id)
craft *c;
int id;
{
	viewer *v;
	AuStatus	status;

	for (v=c->vl; v; v=v->next) {
		if (v->aserver != (AuServer *) NULL && v->flow[id] != 0) {
			AuStopFlow (v->aserver, v->flow[id], &status);
			AuDestroyFlow (v->aserver, v->flow[id], &status);
			v->flow[id] = 0;
		}
	}
}

static void
done_callback (audio, which, event, data)
AuServer	*audio;
AuEventHandlerRec *which;
AuEvent		*event;
AuPointer	data;
{
	viewer *v = (viewer *) data;
	int		mult, mon;
	AuStatus	status;
	
	printf ("event type = %d\n", event->type);
}


#else /* defined(NETAUDIO) */
#if defined (HPAUDIO)

#undef	duration /* defined by pm.h as "radarMode" */

int
initializeAudio(c, v, display)
craft *c;
viewer *v;
char *display;
{
	int		i, n;
	Audio		*conn;
	AudioAttributes	sb_attr;
	long		status;

/*
 *  Lazy initialization of sound paths array
 */

	if (!paths[0]) {
		for (i=0; i<NUM_SOUNDS; ++i) {
			paths[i] = strdup(acm_find_file(files[i]));
		}
	}

/*
 *  Open the audio server and send it our sounds
 */
	
	if ((v->aserver = AOpenAudio(display, &status)) == NULL) {
printf("AOpenAudio: failed, status = %d\n", status);
		v->aserver = (Audio *) NULL;
		return -1;
	}

	for (i=0; i<NUM_SOUNDS; ++i) {
		if ((v->sound[i] = ALoadAFile(v->aserver,
					      paths[i], AFFUnknown, 0, &sb_attr,
					      &status)) == NULL) {
			printf ("Unable to create sound bucket: %s (status %d)\n",
				paths[i], status);
			ACloseAudio(v->aserver, &status);
			v->aserver = (Audio *) NULL;
			return -1;
		}
		v->flow[i] = 0;
	}

	return 0;
}

void
shutdownAudio(c, v)
craft *c;
viewer *v;
{
	int i;
	long status;

	if (v->aserver) {
		for (i=0; i<NUM_SOUNDS; ++i) {
			ADestroySBucket(v->aserver, v->sound[i], &status);
		}
		ACloseAudio(v->aserver, &status);
		v->aserver = (Audio *) NULL;
	}
}

void
playSound (c, id)
craft *c;
int id;
{
	viewer *v;
	SBPlayParams	pparams;
	AGainEntry	gainEntry;
	long		status;

	gainEntry.u.o.out_ch = AOCTMono;
	gainEntry.gain = AUnityGain;
	gainEntry.u.o.out_dst = AODTMonoIntSpeaker;

	bzero(&pparams, sizeof(pparams));
	pparams.priority = APriorityNormal;
	pparams.gain_matrix.type = AGMTOutput;
	pparams.gain_matrix.num_entries = 1;
	pparams.gain_matrix.gain_entries = &gainEntry;
	pparams.play_volume = AUnityGain;
	pparams.pause_first = False;
	pparams.start_offset.type = ATTSamples;
	pparams.start_offset.u.samples = 0;
	pparams.duration.type = ATTFullLength;
	pparams.loop_count = 0;
	pparams.previous_transaction = 0;
	pparams.event_mask = 0;

	for (v=c->vl; v; v=v->next) {
		if (v->aserver) {
			v->flow[id] = APlaySBucket(v->aserver, v->sound[id],
						   &pparams, &status);
if (v->flow[id] == NULL) printf("APlaySBucket: failed, status %d\n", status);
		}
	}
}

void
playContinuousSound (c, id)
craft *c;
int id;
{
	viewer *v;
	AGainEntry	gainEntry;
	SBPlayParams	pparams;
	long		status;

	gainEntry.u.o.out_dst = AODTMonoIntSpeaker /* AODTMonoJack */;
	gainEntry.u.o.out_ch = AOCTMono;
	gainEntry.gain = AUnityGain;

	bzero(&pparams, sizeof(pparams));
	pparams.gain_matrix.type = AGMTOutput;
	pparams.gain_matrix.num_entries = 1;
	pparams.gain_matrix.gain_entries = &gainEntry;
	pparams.play_volume = AUnityGain;
	pparams.pause_first = False;
	pparams.start_offset.type = ATTSamples;
	pparams.start_offset.u.samples = 0;
	pparams.duration.type = ATTFullLength;
	pparams.loop_count = -1;		/* repeating... */
	pparams.priority = APriorityNormal;
	pparams.previous_transaction = 0;
	pparams.event_mask = 0;

	for (v=c->vl; v; v=v->next) {
		if (v->aserver) {
			v->flow[id] = APlaySBucket(v->aserver, v->sound[id],
						   &pparams, &status);
if (v->flow[id] == NULL) printf("APlaySBucket: failed, status %d\n", status);
		}
	}
}

void
stopSound(c, id)
craft *c;
int id;
{
	viewer *v;
	long		status;

	for (v=c->vl; v; v=v->next) {
		if (v->aserver != (Audio *) NULL && v->flow[id] != 0) {
			AStopAudio(v->aserver, v->flow[id], ASMEndLoop, NULL, &status);
if (status) printf("AStopAudio: status %d\n", status);
			v->flow[id] = 0;
		}
	}
}

#else /* defined(HPAUDIO) */

int
initializeAudio(c, v)
craft *c;
viewer *v;
{
	int i;
	for (i=0; i<NUM_SOUNDS; ++i) {
		v->flow[i] = 0;
	}
	return -2;
}

void
shutdownAudio(c, v)
craft *c;
viewer *v;
{
}

void
playSound (c, id)
craft *c;
int id;
{
}

void
playContinuousSound (c, id)
craft *c;
int id;
{
}

void
stopSound(c, id)
craft *c;
int id;
{
}

#endif /* defined(HPAUDIO) */
#endif /* defined(NETAUDIO) */
