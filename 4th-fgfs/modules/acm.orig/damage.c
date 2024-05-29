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
#include "damage.h"

#if defined (HAVE_RAND)
int
acm_rand ()
{
	return rand() & 0xffff;
}

void
acm_srand(seed)
int seed;
{
	srand (seed);
}
#else
#if defined (HAVE_RANDOM)
int
acm_rand ()
{
	return random() & 0xffff;
}

void
acm_srand (seed)
int seed;
{
	srandom (seed);
}
#else

int
acm_rand ()
{
	return 1;
}

void
acm_srand (seed)
int seed;
{
    fprintf(stderr,"Hmm. Your system seems to lack both rand() and random()\n");
    fprintf(stderr,"What kind of system is this, anyway?\n\n");
    exit (1);
}

#endif
#endif

double
frandom ()
{

	return (double) (acm_rand() & 0xffff) / 65536.0;
}

/*
 *  Select a subsystem to receive damage.
 */

long
selectSystem ()
{

	double	r;
	long	i;

	if ((r = frandom()) < 0.25)
		i = SYS_ENGINE1;
	else if (r < 0.35)
		i = SYS_RADAR;
	else if (r < 0.40)
		i = SYS_TEWS;
	else if (r < 0.45)
		i = SYS_HYD1;
	else if (r < 0.50)
		i = SYS_HYD2;
	else if (r < 0.52)
		i = SYS_GEN1;
	else if (r < 0.54)
		i = SYS_GEN2;
	else if (r < 0.59)
		i = SYS_FLAPS;
	else if (r < 0.63)
		i = SYS_SPEEDBRAKE;
	else if (r < 0.75)
		i = SYS_FUEL;
	else if (r < 0.80)
		i = SYS_HUD;
	else if (r < 0.80)
		i = SYS_LEFTMAIN;
	else if (r < 0.85)
		i = SYS_RIGHTMAIN;
	else if (r < 0.95)
		i = SYS_NOSEGEAR;
	return i;
}

void
damageSystem (c, sys)
craft	*c;
long	sys;
{

	if ((c->damageBits & sys) || (sys == SYS_FUEL)) {

		c->damageBits &= ~ sys;

		switch (sys) {

		case SYS_ENGINE1:
			c->throttle = 0;
			break;

		case SYS_RADAR:
			c->curRadarTarget = -1;
			break;

/*
 *  Fuel leaks can be up to 40 pounds per second here.
 */

		case SYS_FUEL:
			c->leakRate += (frandom() + frandom()) * 20.0;
			break;
		
		case SYS_HYD1:
		case SYS_HYD2:
			if ((c->damageBits & (SYS_HYD1 | SYS_HYD2)) == 0) {
				c->damageBits &= ~ SYS_SPEEDBRAKE;
				c->damageBits &= ~ SYS_FLAPS;
			}
			break;

		case SYS_GEN1:
		case SYS_GEN2:
			if ((c->damageBits & (SYS_GEN1 | SYS_GEN2)) == 0) {
				c->damageBits &= ~
				    (SYS_HUD | SYS_RADAR | SYS_TEWS);
				break;
			}
		}

	}
}

/*
 * absorbDamage :  craft c is hit with d points of damage.
 */

int
absorbDamage (c, d)
craft	*c;
int	d;
{

	double	n, x;
	register long	sys;

/*
 *  Actual damage sustained is adjusted by a damage factor that forms a
 *  bell curve centered around 0.75 * d.
 */
 
	x = (frandom() + frandom()) / 2.0 + 0.25;

	d = d * x + 0.5;

	if (d > 0) {
		playSound (c, SoundExplosion);
	}

	for (; d > 0; --d) {

/*
 *  For each damage point absorbed, there is a 15 percent chance that
 *  it will be absorbed by some subsystem other than the actual
 *  airframe.
 */

		if (frandom() <= 0.15) {
			sys = selectSystem();
			damageSystem (c, sys);
		}

/*
 *  For each point absorbed by the airframe, there is a 20% chance that
 *  it'll be absorbed by the wing and induce a rolling moment or a 10 
 *  percent chance that it will hit a horizontal stabilizer and induce
 *  a pitching and rolling moment.
 */

		else {

		if ((n = frandom()) <= 0.20) {
			c->damageCL += (frandom() - 0.5) * 0.20;
		}
		else if (n <= 0.30) {
			c->damageCL += (frandom() - 0.5) * 0.10;
			c->damageCM += (frandom() - 0.5) * 0.20;
		}

		if (--(c->structurePts) <= 0)
			return 0;
		}

	}

	return 1;

}

void
initDamage (c)
craft	*c;
{

	c->damageBits = c->cinfo->damageBits;
	c->structurePts = c->cinfo->structurePts;
	c->leakRate = 0.0;
	c->damageCL = 0.0;
	c->damageCM = 0.0;
}
