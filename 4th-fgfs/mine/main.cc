// main.cc
//
// This is the main file.
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file)
//


#include "Display.h"
#include "Trans.h"


// Ok, let's get started ...
int main( int argc, char **argv ) {
    Display D;
    Trans T;

    int xsize, ysize;

    // front
    double p1[3]  = { 0.0,  0.0, 54.0};
    double p2[3]  = {16.0,  0.0, 54.0};
    double p3[3]  = {16.0, 10.0, 54.0};
    double p4[3]  = { 8.0, 16.0, 54.0};
    double p5[3]  = { 0.0, 10.0, 54.0};

    // back
    double p6[3]  = { 0.0,  0.0, 30.0};
    double p7[3]  = {16.0,  0.0, 30.0};
    double p8[3]  = {16.0, 10.0, 30.0};
    double p9[3]  = { 8.0, 16.0, 30.0};
    double p10[3] = { 0.0, 10.0, 30.0};

    double vp[2];

    int sp1[2], sp2[2], sp3[2], sp4[2], sp5[2], sp6[2], sp7[2],
      sp8[2], sp9[2], sp10[2];

    char s[256];

    // Set up and initialize graphics system
    D.Init();
    xsize = D.Get_XSize();
    ysize = D.Get_YSize();
    printf("xsize = %d  ysize = %d\n", xsize, ysize);

    // Set up and initialize the transformation matrix
    T.CalcTransMatrix();

    // Initialize screen mapping parameters
    T.Set_x_and_y(xsize, ysize);

    // Map points to screen
    T.CalcProjection(p1, vp); T.MapToScreen(vp, sp1);
    printf("(%.0f, %0f)\n", vp[0], vp[1]);
    T.CalcProjection(p2, vp); T.MapToScreen(vp, sp2);
    T.CalcProjection(p3, vp); T.MapToScreen(vp, sp3);
    T.CalcProjection(p4, vp); T.MapToScreen(vp, sp4);
    T.CalcProjection(p5, vp); T.MapToScreen(vp, sp5);
    T.CalcProjection(p6, vp); T.MapToScreen(vp, sp6);
    T.CalcProjection(p7, vp); T.MapToScreen(vp, sp7);
    T.CalcProjection(p8, vp); T.MapToScreen(vp, sp8);
    T.CalcProjection(p9, vp); T.MapToScreen(vp, sp9);
    T.CalcProjection(p10,vp); T.MapToScreen(vp, sp10);

    // draw front
    gl_line(sp1[0], sp1[1], sp2[0], sp2[1], 1);
    gl_line(sp2[0], sp2[1], sp3[0], sp3[1], 1);
    gl_line(sp3[0], sp3[1], sp4[0], sp4[1], 1);
    gl_line(sp4[0], sp4[1], sp5[0], sp5[1], 1);
    gl_line(sp5[0], sp5[1], sp1[0], sp1[1], 1);

    // draw back
    gl_line(sp6[0], sp6[1], sp7[0], sp7[1], 1);
    gl_line(sp7[0], sp7[1], sp8[0], sp8[1], 1);
    gl_line(sp8[0], sp8[1], sp9[0], sp9[1], 1);
    gl_line(sp9[0], sp9[1], sp10[0],sp10[1],1);
    gl_line(sp10[0],sp10[1],sp6[0], sp6[1], 1);

    gets(s);

    printf("(%d, %d)\n", sp1[0], sp1[1]);
    printf("(%d, %d)\n", sp2[0], sp2[1]);
    printf("(%d, %d)\n", sp3[0], sp3[1]);
    printf("(%d, %d)\n", sp4[0], sp4[1]);

    return( 0 );
}


//
// $Log: graph_main.c,v $
//

