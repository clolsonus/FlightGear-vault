#include "Vlib.h"
#define EPSILON	0.00000000000001

/*
 *  Quaternion code based on code from "Advanced Animation and Rendering
 *  Techniques: Theory and Practice" by Alan Watt and Mark Watt.
 */

void
VQuaternionToMatrix (q, m)
VQuaternion	*q;
VMatrix		*m;
{
	double	s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

	s = 2.0 / (q->v.x * q->v.x + q->v.y * q->v.y +
		q->v.z * q->v.z + q->s * q->s);

	xs = q->v.x * s;
	ys = q->v.y * s;
	zs = q->v.z * s;
	wx = q->s * xs;
	wy = q->s * ys;
	wz = q->s * zs;
	xx = q->v.x * xs;
	xy = q->v.x * ys;
	xz = q->v.x * zs;
	yy = q->v.y * ys;
	yz - q->v.y * zs;
	zz = q->v.z * zs;

	VIdentMatrix (m);
	
	m->m[0][0] = 1.0 - (yy + zz);
	m->m[0][1] = xy + wz;
	m->m[0][2] = xz - wy;

	m->m[1][0] = xy - wz;
	m->m[1][1] = 1.0 - (xx + zz);
	m->m[1][2] = yz + wx;

	m->m[2][0] = xz + wy;
	m->m[2][1] = yz - wx;
	m->m[2][2] = 1.0 - (xx + yy);
}

void
VMatrixToQuaternion (m, q)
VMatrix	*m;
VQuaternion *q;
{
	double	tr, s, *q1 = (double *) &q->v;
	int	i, j, k;

	tr = m->m[0][0] + m->m[1][1] + m->m[2][2];
	if (tr > 0.0) {
		s = sqrt (tr + 1.0);
		q->s = s * 0.5;
		s = 0.5 / s;
		q->v.x = (m->m[1][2] - m->m[2][1]) * s;
		q->v.y = (m->m[2][0] - m->m[0][2]) * s;
		q->v.z = (m->m[0][1] - m->m[1][0]) * s;
	}
	else {
		i = 0;
		if (m->m[1][1] > m->m[0][0]) {
			i = 1;
		}
		if (m->m[2][2] > m->m[i][i]) {
			i = 2;
		}
		j = (i + 1) % 3;
		k = (j + 1) % 3;
	
		s = sqrt ((m->m[i][i] - (m->m[j][j] + m->m[k][k])) + 1.0);

		q1[i] = s * 0.5;
		s = 0.5 / s;
		q->s = (m->m[j][k] + m->m[k][j]) * s;
		q1[j] = (m->m[i][j] + m->m[j][i]) * s;
		q1[k] = (m->m[i][k] + m->m[k][i]) * s;
	}
}

void
VInterpolateQuaternion (p, q, t, qt)
VQuaternion	*p;
VQuaternion	*q;
double		t;
VQuaternion	*qt;
{
	double	omega, cosom, sinom, sclp, sclq;

	cosom = p->v.x * p->v.x + p->v.y * p->v.y +
		p->v.z * p->v.z + p->s * p->s;

	if ((1.0 + cosom) > EPSILON) {

		if ((1.0 - cosom) > EPSILON) {
			omega = acos (cosom);
			sinom = sin (omega);
			sclp = sin ((1.0 - t) * omega) / sinom;
			sclq = sin (t * omega) / sinom;
		}
		else {
			sclp = 1.0 - t;
			sclq = t;
		}

		qt->v.x = p->v.x * sclp + q->v.x * sclq;
		qt->v.y = p->v.y * sclp + q->v.y * sclq;
		qt->v.z = p->v.z * sclp + q->v.z * sclq;
		qt->s = p->s * sclp + q->s * sclq;

	}
	else {

		qt->v.x = - p->v.y;
		qt->v.y =   p->v.x;
		qt->v.z = - p->s;
		qt->s   =   p->v.z;

		sclp = sin ((1.0 - t) * M_PI_2);
		sclq = sin (t * M_PI_2);

		qt->v.x = p->v.x * sclp + qt->v.x * sclq;
		qt->v.y = p->v.y * sclp + qt->v.y * sclq;
		qt->v.z = p->v.z * sclp + qt->v.z * sclq;
	}
			
}

VQuaternion *
VQuaternionMult (a, b, r)
VQuaternion *a, *b, *r;
{
	VPoint	aa, bb, c;

	r->s = a->s * b->s - VDotProd (&a->v, &b->v);
	VCrossProd (&a->v, &b->v, &c);
	c.x += a->s * b->v.x + b->s * a->v.x;
	c.y += a->s * b->v.y + b->s * a->v.y;
	c.z += a->s * b->v.z + b->s * a->v.z;
	r->v = c;
	return r;
}

VQuaternion *
VQuaternionComplement (a, r)
VQuaternion *a, *r;
{
	r->s   =   a->s;
	r->v.x = - a->v.x;
	r->v.y = - a->v.y;
	r->v.z = - a->v.z;
	return r;
}
	

