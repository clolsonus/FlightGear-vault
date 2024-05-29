#include <pm.h>

#define WGS84_A	6378137.0
#define WGS84_B	6356752.3142

void
UpdateWorldCoordinates (WorldCoordinates *p, double course, double d)
{
	double n1, n2, m1, ecc, ecc_sqr;
	double sin_lat, sin_lat_sqr, tan_lat, sin_course, sin_course_sqr;
	double delta_latitude, delta_longitude, d_sqr, cos_lat;
	double B, C, D, E, h, sin_newlat;
	double wgs84_a = WGS84_A - p->z;

	sin_lat = sin (p->latitude);
	sin_lat_sqr = sin_lat_sqr;
	tan_lat = tan (p->latitude);
	sin_course = sin (course);
	sin_course_sqr = sin_course * sin_course;
	ecc = 0.082;
	ecc_sqr = ecc * ecc;
	d_sqr = d * d;

	n1 = wgs84_a / sqrt (1.0 - ecc_sqr * sin_lat_sqr);
	m1 = ( wgs84_a * (1.0 - ecc_sqr) ) / 
		pow (1.0 - ecc_sqr * sin_lat_sqr, 1.5);
	
	B = 1.0 / m1;

	h = d * B * cos (course);

	C = tan_lat / (2.0 * m1 * n1);

	D = ( 3.0 * ecc_sqr * sin_lat * cos_lat ) /
		( 2.0 * ( 1.0 - ecc_sqr * sin_lat_sqr ) );

	E = ( 1.0 + 3.0 * tan_lat * tan_lat ) *
		( 1.0 - ecc_sqr * sin_lat_sqr ) / ( 6.0 * WGS84_A * WGS84_A );

	delta_latitude = d * B * cos (course) -
		d_sqr * C * sin_course_sqr -
		h * d_sqr * E * sin_course_sqr;

	p->latitude += delta_latitude;	
	sin_newlat = sin (p->latitude);

	n2 = wgs84_a / sqrt ( 1.0 - ecc_sqr * sin_newlat * sin_newlat );

	delta_longitude = d * sin_lat / ( n2 * cos (p->longitude) );

	p->longitude += delta_longitude;
}
