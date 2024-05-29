// trans.h
//
// This file contains all declarations for the transformation &
// projection specific code.
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file)

#ifndef TRANS_H
#define TRANS_H


#include "mat3.h"
#include "xarmsim.h"


class Trans {
    // Default Viewing Parameters Section

    // The view reference point & the view plane normal define the view plane
    MAT3vec vrp;  // { 0.0, 0.0,  0.0} view ref point
    MAT3vec vpn;  // { 0.0, 1.0,  0.0} view plane normal

    // This is "up"
    MAT3vec vup;  // { 0.0, 1.0,  0.0} view up vector

    // This is where your "eye" is located
    MAT3vec prp;  // { 0.0, 0.0, 10.0} per ref point

    // view plane bounds
    double umin, umax, vmin, vmax;
    // du = umax - umin; dv = vmax - vmin;
    double du, dv;

    // front and back clipping planes
    double F, B;

    // Viewing Transformation Matrix
    MAT3mat N;

    // Screen dimensions
    int xmax, ymax;

  public:

    Trans(void);

    // set the view reference point
    void Set_vrp(MAT3vec point);

    // set the view plane normal
    void Set_vpn(MAT3vec vector);

    // set the "up" vector
    void Set_vup(MAT3vec vector);

    // set the perspective reference point
    void Set_prp(MAT3vec point);

    // set the view plane bounds
    void Set_u_and_v(double umin, double umax, double vmin, double vmax);

    // set front and back clipping planes
    void Set_f_and_b(double front, double back);

    // Calculate the transformation matrix.  This must be called anytime
    // the viewing parameters have been changed.
    void CalcTransMatrix();

    // Project a point in the world coordinate system onto the viewing plane.
    // returns a the point on the viewing plane
    void CalcProjection(MAT3vec wp, point vp);

    // Set the screen dimensions
    void Set_x_and_y(int xmax, int ymax);

    // Map a point to its screen coords
    // returns a point in screen coordinates
    void MapToScreen(point vp, int sp[]);

    ~Trans(void);
};



#endif TRANS_H

//
// $Log: graph_main.c,v $
//
