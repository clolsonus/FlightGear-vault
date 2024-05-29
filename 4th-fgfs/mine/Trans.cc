// trans.cc
//
// This file contains all transformation & projection specific code.  
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file)
//

#include <math.h>

#include "Trans.h"


// Constructor
Trans::Trans(void) {
    printf("Creating a new instance of class Trans\n");
    // vrp[0] = 0.0; vrp[1] = 0.0; vrp[2] = 0.0;  // view ref point
    vrp[0] = 4.0; vrp[1] = 0.0; vrp[2] = 0.0; vrp[3] = 1.0; // view ref point

    // vpn[0] = 0.0; vpn[1] = -1.0; vpn[2] = 0.0;  // view plane normal
    vpn[0] = 0.0; vpn[1] = -3.0; vpn[2] = 0.0; vpn[3] = 1.0; //view plane normal

    // This is "up"
    // vup[0] = 0.0; vup[1] = 0.0; vup[2] = 1.0;  // view up vector
    vup[0] = 0.0; vup[1] = 1.0; vup[2] = 0.0; vup[3] = 1.0;  // view up vector

    // This is where your "eye" is located
    // prp[0] = 0.0; prp[1] = -10.0; prp[2] = 0.5; // per ref point
    prp[0] = 5.0; prp[1] = 8.0; prp[2] = 6.0; prp[3] = 1.0; // per ref point

    // view plane bounds
    // umin = -5.0; umax = 5.0; vmin = -5.0; vmax = 5.0;
    umin = -2.0; umax = 2.0; vmin = -1.0; vmax = 3.0;
    du = umax - umin; dv = vmax - vmin;

    // front and back clipping planes
    F = 1.0; B = -9.0;
}


// set the view reference point
void Trans::Set_vrp(MAT3hvec point) {
    vrp[0] = point[0];
    vrp[1] = point[1];
    vrp[2] = point[2];
    vrp[3] = 1.0;
}


// set the view plane normal
void Trans::Set_vpn(MAT3hvec vector) {
    vpn[0] = vector[0];
    vpn[1] = vector[1];
    vpn[2] = vector[2];
    vpn[3] = 1.0;
}


// set the "up" vector
void Trans::Set_vup(MAT3hvec vector) {
    vup[0] = vector[0];
    vup[1] = vector[1];
    vup[2] = vector[2];
    vup[3] = 1.0;
}


// set the perspective reference point
void Trans::Set_prp(MAT3hvec point) {
    prp[0] = point[0];
    prp[1] = point[1];
    prp[2] = point[2];
    prp[3] = 1.0;
}


// set the view plane bounds
void Trans::Set_u_and_v(double umin, double umax, double vmin, double vmax) {
    Trans::umin = umin; Trans::umax = umax;
    Trans::vmin = vmin; Trans::vmax = vmax;
    du = umax - umin; dv = vmax - vmin;
}


// set front and back clipping planes
void Trans::Set_f_and_b(double front, double back) {
    F = front; B = back;
}


// Calculate the transformation matrix.  This must be called anytime
// the viewing parameters have been changed.
void Trans::CalcTransMatrix() {
    MAT3mat T1, R, T2, SH, S1, S2, S, TEMP;
    MAT3hvec minus_vrp, minus_prp, n, u, v, cw, dop, s1vec, s2vec;
    double xshear, yshear, temp;
    int i, j;

    // Calculate First Translation Matrix
    MAT3_SCALE_VEC(minus_vrp, vrp, -1.0);
    MAT3translate(T1, minus_vrp); // makes an identity matrix internally

    // Calculate Rotation Matrix
      // Calculate n
      MAT3_COPY_VEC(n, vpn);
      MAT3_NORMALIZE_VEC(n, temp);
      printf("n = (%.2f, %.2f, %.2f)\n", n[0], n[1], n[2]);

      // Calculate u
      MAT3cross_product(u, vup, n);
      MAT3_NORMALIZE_VEC(u, temp);
      printf("u = (%.2f, %.2f, %.2f)\n", u[0], u[1], u[2]);

      // Calculate v
      MAT3cross_product(v, n, u);
      printf("v = (%.2f, %.2f, %.2f)\n", v[0], v[1], v[2]);

    MAT3identity(R);
    MAT3_COPY_VEC(R[0], u);
    MAT3_COPY_VEC(R[1], v);
    MAT3_COPY_VEC(R[2], n);
    MAT3transpose(R, R);  // uses a temporary matrix internally 

    // Calculate Second Translation Matrix
    MAT3_SCALE_VEC(minus_prp, prp, -1.0);
    MAT3translate(T2, minus_prp); // makes an identity matrix internally

    // Calculate Shear Matrix
      // Calculate CW
      cw[0] = (umin + umax) / 2.0;
      cw[1] = (vmin + vmax) / 2.0;
      cw[2] = 0.0;

      printf ("CW = (%.2f, %.2f)\n", cw[0], cw[1]);
      printf ("prp = (%.2f, %.2f, %.2f)\n", prp[0], prp[1], prp[2]);

      // Calculate DOP
      MAT3_SUB_VEC(dop, cw, prp);

      printf ("DOP = (%.2f, %.2f, %.2f)\n", dop[0], dop[1], dop[2]);

      // Calculate x and y shear components
      xshear = -dop[0]/dop[2];
      yshear = -dop[1]/dop[2];
      printf("xshear = %.2f  yshear = %.2f\n", xshear, yshear);

    MAT3shear(SH, xshear, yshear); // makes an identity matrix internally

    // Calculate Scaling Matrix
      // Calculate S1
      s1vec[0] = 2 * prp[2] / (umax - umin);
      s1vec[1] = 2 * prp[2] / (vmax - vmin);
      s1vec[2] = 1.0;
      MAT3scale(S1, s1vec); // makes an identity matrix internally
      // Calculate S2
      s2vec[0] = -1.0 / (B - prp[2]);
      s2vec[1] = -1.0 / (B - prp[2]);
      s2vec[2] = -1.0 / (B - prp[2]);
      MAT3scale(S2, s2vec); // makes an identity matrix internally
    MAT3mult(S, S2, S1);
    for ( i = 0; i < 4; i++) {
	for ( j = 0; j < 4; j++) {
	    printf("%.3f ", S[j][i]);
	}
	printf("\n");
    }

    // Calculate Combined Transformation Matrix
    MAT3mult(N, T1, R);
    MAT3copy(TEMP, N);
    MAT3mult(N, TEMP, T2);
    MAT3copy(TEMP, N);
    MAT3mult(N, TEMP, SH);
    MAT3copy(TEMP, N);
    MAT3mult(N, TEMP, S);

    for ( i = 0; i < 4; i++) {
	for ( j = 0; j < 4; j++) {
	    printf("%.3f ", N[j][i]);
	}
	printf("\n");
    }
}


// Project a point in the world coordinate system onto the viewing plane.
// returns a the point on the viewing plane
void Trans::CalcProjection(MAT3hvec wp, point vp) {
    MAT3mult_vec(vp, wp, N);
}


// Set the screen dimensions
void Trans::Set_x_and_y(int xmax, int ymax) {
    Trans::xmax = xmax; Trans::ymax = ymax;
}


// Map a point to its screen coords
// returns a point in screen coordinates
void Trans::MapToScreen(point vp, int sp[]) {
    sp[0] = (xmax - 1) - (int)( ( (vp[0] - umin) / du ) * (xmax - 1) );
    sp[1] = (ymax - 1) - (int)( ( (vp[1] - vmin) / dv ) * (ymax - 1) );
}


// Destructor
Trans::~Trans(void) {
    printf("Destroying an instance of class Trans\n");
}


//
// $Log: graph_main.c,v $
//
