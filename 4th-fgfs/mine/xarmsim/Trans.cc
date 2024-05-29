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
    vrp[0] = 0.0; vrp[1] = 0.0; vrp[2] = 0.0;  // view ref point
    vpn[0] = 0.0; vpn[1] = 1.0; vpn[2] = 0.0;  // view plane normal

    // This is "up"
    vup[0] = 0.0; vup[1] = 1.0; vup[2] = 0.0;  // view up vector

    // This is where your "eye" is located
    prp[0] = 0.0; prp[1] = 0.0; prp[2] = 10.0; // per ref point

    // view plane bounds
    umin = -5.0; umax = 5.0; vmin = -5.0; vmax = 5.0;
    du = umax - umin; dv = vmax - vmin;

    // front and back clipping planes
    F = 0.0; B = -10.0;
}


// set the view reference point
void Trans::Set_vrp(MAT3vec point) {
    vrp = point;
}


// set the view plane normal
void Trans::Set_vpn(MAT3vec vector) {
    vpn = vector;
}


// set the "up" vector
void Trans::Set_vup(MAT3vec vector) {
    vup = vector;
}


// set the perspective reference point
void Trans::Set_prp(MAT3vec point) {
    prp = point;
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
    MAT3vec minus_vrp, minus_prp, n, u, v, cw, dop, s1vec, s2vec;
    double xshear, yshear, temp;

    // Calculate First Translation Matrix
    MAT3_SCALE_VEC(minus_vrp, vrp, -1.0);
    MAT3translate(T1, minus_vrp);

    // Calculate Rotation Matrix
      // Calculate n
      MAT3_COPY_VEC(n, vpn);
      MAT3_NORMALIZE_VEC(n, temp);
      // Calculate u
      MAT3cross_product(u, vup, n);
      MAT3_NORMALIZE_VEC(u, temp);
      // Calculate v
      MAT3cross_product(v, n, u);
    MAT3identity(R);
    MAT3_COPY_VEC(R[0], u);
    MAT3_COPY_VEC(R[1], v);
    MAT3_COPY_VEC(R[2], n);
    MAT3transpose(R, R);

    // Calculate Second Translation Matrix
    MAT3_SCALE_VEC(minus_prp, prp, -1.0);
    MAT3translate(T2, minus_prp);

    // Calculate Shear Matrix
      // Calculate CW
      cw[0] = (umin + umax) / 2.0;
      cw[1] = (vmin + vmax) / 2.0;
      cw[2] = 0.0;
      // Calculate DOP
      MAT3_SUB_VEC(dop, cw, prp);
      // Calculate x and y shear components
      xshear = -dop[0]/dop[2];
      yshear = -dop[1]/dop[2];
    MAT3shear(SH, xshear, yshear);

    // Calculate Scaling Matrix
      // Calculate S1
      s1vec[0] = 2 * prp[2] / (umax - umin);
      s1vec[1] = 2 * prp[2] / (vmax - vmin);
      s1vec[2] = 1.0;
      MAT3identity(S1);
      MAT3scale(S1, s1vec);
      // Calculate S2
      s2vec[0] = -1.0 / (B - prp[2]);
      s2vec[1] = -1.0 / (B - prp[2]);
      s2vec[2] = -1.0 / (B - prp[2]);
      MAT3identity(S2);
      MAT3scale(S2, s2vec);
    MAT3mult(S, S2, S1);

    // Calculate Combined Transformation Matrix
    MAT3mult(N, T1, R);
    MAT3copy(TEMP, N);
    MAT3mult(N, TEMP, T2);
    MAT3copy(TEMP, N);
    MAT3mult(N, TEMP, SH);
    MAT3copy(TEMP, N);
    MAT3mult(N, TEMP, S);
}


// Project a point in the world coordinate system onto the viewing plane.
// returns a the point on the viewing plane
void Trans::CalcProjection(MAT3vec wp, point vp) {
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
