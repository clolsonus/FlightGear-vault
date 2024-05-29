/*
 *  Pre-defined sounds that can be played for the user
 */

#ifndef SOUND_H
#define SOUND_H


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

#if defined(NETAUDIO)
#  include <audio/audiolib.h>
#else
#  if defined(HPAUDIO)
#    include <audio/Alib.h>
#  else
     typedef	char	*AuServer;
     typedef char	AuBucketID;
     typedef char	AuFlowID;
     typedef char	AuDeviceID;
#  endif
#endif

extern int initializeAudio();
extern void shutdownAudio(), playSound(), playContinuousSound(), stopSound();


#endif SOUND_H
