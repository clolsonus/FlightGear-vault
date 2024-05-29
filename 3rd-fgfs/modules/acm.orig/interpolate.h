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

/*
 *  Some data structures to perform fast 2-D interpolation
 *
 *  Alot of aircraft information is in tabular form.  We perform linear
 *  interpolation to determine values.  In order to speed up that process,
 *  we'll create preprocessed tables.  Each entry in the table contains the
 *  upper bound of the domain value (x) and the equation of the line that
 *  defines the data for that interval (y = m * x + b).
 *
 *  The program ibuild preprocesses the tabular information into a
 *  corresponding C structure that the interpolate() function can use.
 */

#include <math.h>

typedef float float_t;

typedef struct {
	float_t	x;		/* upper x value for this interval */
	float_t	m;		/* slope of line in this interval */
	float_t	b;		/* y-intercept of line in this interval */
	} IEntry;

typedef struct {
	int	count;		/* entry count in the interpolation table */
	float_t	minX;		/* minimum x value that can be interpolated */
	IEntry	*entry;		/* vector of intterpolation entries */
	} ITable;

/*
 *  I'm doing this to catch bad interpolation requests.  If an invalid
 *  domain value is submitted, (and we're on an IEEE FP system) we'll
 *  get back the IEEE NaN value as the interpolated result.
 */

#define I_NaN	(sqrt(-1.0))

extern float_t interpolate();
