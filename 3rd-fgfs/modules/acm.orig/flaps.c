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

int	flapsDown (c)
craft	*c; {

	c->flapSetting += 10.0 * pi / 180.0;
	if (c->flapSetting > c->cinfo->maxFlap)
		c->flapSetting = c->cinfo->maxFlap;
	return 0;
}

int	flapsUp (c)
craft	*c; {

	c->flapSetting -= 10.0 * pi / 180.0;
	if (c->flapSetting < 0.0)
		c->flapSetting = 0.0;
	return 0;
}

void	flapControl (c)
craft	*c; {

	double geardown;

	if (isFunctioning(c, SYS_FLAPS)) {

	    if (c->flapSetting > c->curFlap) {
		c->curFlap += c->cinfo->flapRate * deltaT;
		if (c->curFlap > c->flapSetting)
			c->curFlap = c->flapSetting;
	    }
	    else if (c->flapSetting < c->curFlap) {
		c->curFlap -= c->cinfo->flapRate * deltaT;
		if (c->curFlap < c->flapSetting)
			c->curFlap = c->flapSetting;
	    }
	}

	if (isFunctioning(c, SYS_SPEEDBRAKE)) {

	    if (c->speedBrakeSetting > c->curSpeedBrake) {
		c->curSpeedBrake += c->cinfo->speedBrakeRate * deltaT;
		if (c->curSpeedBrake > c->speedBrakeSetting)
			c->curSpeedBrake = c->speedBrakeSetting;
	    }
	    else if (c->speedBrakeSetting < c->curSpeedBrake) {
		c->curSpeedBrake -= c->cinfo->speedBrakeRate * deltaT;
		if (c->curSpeedBrake < c->speedBrakeSetting)
			c->curSpeedBrake = c->speedBrakeSetting;
	    }
	}

	geardown = pi / 2.0;

	if (isFunctioning(c, SYS_NOSEGEAR)) {
	    if (c->flags & FL_GHANDLE_DN) {
		if (c->curGear[0] != geardown) {
		    c->curGear[0] += c->cinfo->gearRate * deltaT;
		    if (c->curGear[0] > geardown)
			c->curGear[0] = geardown;
		    c->flags &= ~FL_GEAR0_UP;
		}
	    }
	    else {
		if (c->curGear[0] != 0.0) {
		    c->curGear[0] -= c->cinfo->gearRate * deltaT;
		    if (c->curGear[0] < 0.0) {
			c->flags |= FL_GEAR0_UP;
			c->curGear[0] = 0.0;
		    }
		}
	    }
	}

	if (isFunctioning(c, SYS_RIGHTMAIN)) {
	    if (c->flags & FL_GHANDLE_DN) {
		if (c->curGear[1] != geardown) {
		    c->curGear[1] += c->cinfo->gearRate * deltaT;
		    if (c->curGear[1] > geardown)
			c->curGear[1] = geardown;
		    c->flags &= ~FL_GEAR1_UP;
		}
	    }
	    else {
		if (c->curGear[1] != 0.0) {
		    c->curGear[1] -= c->cinfo->gearRate * deltaT;
		    if (c->curGear[1] < 0.0) {
			c->flags |= FL_GEAR1_UP;
			c->curGear[1] = 0.0;
		    }
		}
	    }
	}

	if (isFunctioning(c, SYS_LEFTMAIN)) {
	    if (c->flags & FL_GHANDLE_DN) {
		if (c->curGear[2] != geardown) {
		    c->curGear[2] += c->cinfo->gearRate * deltaT;
		    if (c->curGear[2] > geardown)
			c->curGear[2] = geardown;
		    c->flags &= ~FL_GEAR2_UP;
		}
	    }
	    else {
		if (c->curGear[2] != 0.0) {
		    c->curGear[2] -= c->cinfo->gearRate * deltaT;
		    if (c->curGear[2] < 0.0) {
			c->flags |= FL_GEAR2_UP;
			c->curGear[2] = 0.0;
		    }
		}
	    }
	}

/*
 *  Set some status flags
 */

	if (c->fuel < 1000.0)
		c->damageBits &= ~FLAG_LOWFUEL;
	else
		c->damageBits |= FLAG_LOWFUEL;

	if (c->flags & FL_BRAKES)
		c->damageBits &= ~FLAG_WHEELBRAKE;
	else
		c->damageBits |= FLAG_WHEELBRAKE;

	if (c->speedBrakeSetting > 0.0)
		c->damageBits &= ~FLAG_SPEEDBRAKE;
	else
		c->damageBits |= FLAG_SPEEDBRAKE;
}

int	speedBrakeExtend (c)
craft	*c; {

	c->speedBrakeSetting += c->cinfo->speedBrakeIncr;
	if (c->speedBrakeSetting > c->cinfo->maxSpeedBrake)
		c->speedBrakeSetting = c->cinfo->maxSpeedBrake;
	return 0;
}

int	speedBrakeRetract (c)
craft	*c; {

	c->speedBrakeSetting -= c->cinfo->speedBrakeIncr;
	if (c->speedBrakeSetting < 0.0)
		c->speedBrakeSetting = 0.0;
	return 0;
}
