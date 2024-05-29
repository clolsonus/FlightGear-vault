// mat3.cc
//
// Copyright 1988, Brown Computer Graphics Group.  All Rights Reserved.
//
// mat3.cc & mat3.h contains all vector & matrix specific code.  
// These routines were taken directly from the srgp/sphigs libraries.
// The entire srgp & sphigs libraries can be found at:
//
//     ftp.cs.brown.edu:/pub/{srgp,sphigs}.tar.Z
//
// $Id: graph_main.c,v 1.19 1994/03/15 03:43:02 curt Exp $
// (Log is kept at end of this file)
//


#include <string.h>
#include "mat3.h"


//
// Sets the given matrix to be a scale matrix for the given vector of
// scale values.
//

void MAT3scale(MAT3mat result_mat, MAT3vec scale) {
   MAT3identity(result_mat);

   result_mat[0][0] = scale[0];
   result_mat[1][1] = scale[1];
   result_mat[2][2] = scale[2];
}


//
// Sets the given matrix to be a translation matrix for the given vector of
// translation values.
//

void MAT3translate(MAT3mat result_mat, MAT3vec trans) {
   MAT3identity(result_mat);

   result_mat[3][0] = trans[0];
   result_mat[3][1] = trans[1];
   result_mat[3][2] = trans[2];
}


//
// Sets the given matrix to be a shear matrix for the given x and y shear
// values.
//

void MAT3shear(MAT3mat result_mat, double xshear, double yshear) {
   MAT3identity(result_mat);

   result_mat[2][0] = xshear;
   result_mat[2][1] = yshear;
}


//
// This multiplies two matrices, producing a third, which may the same as
// either of the first two.
//

void MAT3mult (MAT3mat result_mat, register MAT3mat mat1, 
	       register MAT3mat mat2) {
   register int i, j;
   MAT3mat      tmp_mat;

   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         tmp_mat[i][j] = (mat1[i][0] * mat2[0][j] +
                       mat1[i][1] * mat2[1][j] +
                       mat1[i][2] * mat2[2][j] +
                       mat1[i][3] * mat2[3][j]);
   MAT3copy (result_mat, tmp_mat);
}


//
// This returns the transpose of a matrix.  The result matrix may be
// the same as the one to transpose.
//

void MAT3transpose (MAT3mat result_mat, register MAT3mat mat) {
   register int i, j;
   MAT3mat      tmp_mat;

   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         tmp_mat[i][j] = mat[j][i];

   MAT3copy (result_mat, tmp_mat);
}


//
// Sets a matrix to identity.
//

void MAT3identity (register MAT3mat mat) {
   register int i;

   bzero (mat, sizeof(MAT3mat));
   for (i = 0; i < 4; i++)
      mat[i][i] = 1.0;
}


//
// Copies one matrix to another.
//

void MAT3copy(MAT3mat to, MAT3mat from) {
   bcopy (from, to, sizeof(MAT3mat));
}


//
// Sets the first vector to be the cross-product of the last two vectors.
//

void MAT3cross_product(MAT3vec result_vec, register MAT3vec vec1, 
		       register MAT3vec vec2) {
   MAT3vec              tempvec;
   register double      *temp = tempvec;

   temp[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
   temp[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
   temp[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

   MAT3_COPY_VEC(result_vec, temp);
}


//
// Multiplies a vector by a matrix, setting the result vector.
// It assumes all homogeneous coordinates are 1.
// The two vectors involved may be the same.
//

void MAT3mult_vec(MAT3vec result_vec, register MAT3vec vec, 
		  register MAT3mat mat) {
   MAT3vec              tempvec;
   register double      *temp = tempvec;

   temp[0] =    vec[0] * mat[0][0] + vec[1] * mat[1][0] +
                vec[2] * mat[2][0] +          mat[3][0];
   temp[1] =    vec[0] * mat[0][1] + vec[1] * mat[1][1] +
                vec[2] * mat[2][1] +          mat[3][1];
   temp[2] =    vec[0] * mat[0][2] + vec[1] * mat[1][2] +
                vec[2] * mat[2][2] +          mat[3][2];

   MAT3_COPY_VEC(result_vec, temp);
}


//
// $Log: graph_main.c,v $
//

