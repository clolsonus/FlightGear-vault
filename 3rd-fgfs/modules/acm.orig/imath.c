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
#include <Vlib.h>
#include "imath.h"

void VMatrixToShortMatrix (v, s)
VMatrix		*v;
short_matrix_t	*s;
{
	register int	i, j;

	for (i=0; i < 4; ++i) {
		for (j=0; j < 4; ++j) {
			s->m[i][j] = (short) ((double) UNITY * v->m[i][j]);
		}
	}
}

void ShortTransform (a, m, b)
short_point_t	*a;
short_matrix_t	*m;
short_point_t	*b;
{
	b->x = (a->x * m->m[0][0] + a->y * m->m[0][1] +
		a->z * m->m[0][2]) / UNITY + m->m[0][3];
	b->y = (a->x * m->m[1][0] + a->y * m->m[1][1] +
		a->z * m->m[1][2]) / UNITY + m->m[1][3];
	b->z = (a->x * m->m[2][0] + a->y * m->m[2][1] +
		a->z * m->m[2][2]) / UNITY + m->m[2][3];
}
