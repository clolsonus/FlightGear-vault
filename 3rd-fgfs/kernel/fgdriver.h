/* fgdriver.h -- definitions of driver formats and parameters
 *
 * Copyright (C) 1996  Eric J. Korpela -- korpela@ssl.berkeley.edu
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * $Id: fgdriver.h,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */         

#ifndef _FGDRIVER_H
#define _FGDRIVER_H

#ifndef _FGTYPES_H
#include "fgtypes.h"
#endif



typedef struct _FGDRIVER_INIT_CALL {
   FG_INT     struct_size;            /* set to sizeof(FGDRIVER_INIT_CALL) for    */
                                      /* version identification              */
   FG_INT     msg;                    /* type of call                        */
   FG_VOID   *public_vars;            /* pointer to shared variables         */
   FG_BITMAP *bitmap;                 /* pointer to driver's graphics bitmap */
                                      /* addresses a pixel                   */
                  /* This buffer is not necessarily part                     */
                  /* of the frame buffer, it could be                        */
                  /* separately allocated for color space                    */
                  /* or size transformations by the main program             */
   FG_INT     event_id;               /* event id                            */
} FGDRIVER_INIT_CALL;

/* INIT message types                                                        */
#define FG_INIT 0
#define FG_REINIT 1
#define FG_BITMAP_CHANGE 2

typedef FG_VOID *(*FG_DRIVER_FN)(FGDRIVER_INIT_CALL *);

/* A pointer to an FGDRIVER_INIT structure is returned by the driver when    */
/* it initializes                                                            */

typedef struct _FGDRIVER_INIT {
   FG_INT         struct_size;        /* set to sizeof(FGDRIVER_INIT) for    */
                                      /* version identification              */
   FG_UINT        driver_flags;       /* flags (see below)                   */
} FGDRIVER_INIT;

/* Definition of flags available for drivers                                 */
    /*notify driver of mouse events in its window                            */
#define FGDRIVER_MOUSE             0x00000001  
    /*notify driver if a key in its keylist is pressed                       */
#define FGDRIVER_KEY               0x00000002  
    /*notify driver of any keypress                                          */
#define FGDRIVER_ANYKEY            0x00000004  
    /*driver needs to be called for screen updates (otherwise only called    */
    /*for events)                                                            */
#define FGDRIVER_SCREEN            0x00000008
    /*driver is flight model (there can only be one of these at a time?)     */
#define FGDRIVER_FLIGHT            0x00000010
    /*driver is to be called after a certain time has passed                 */
#define FGDRIVER_TIMED             0x00000020
    /*driver wants to be allocated a screen region                           */
#define FGDRIVER_NEEDSCREEN        0x00000040

#endif

/* $Log: fgdriver.h,v $
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * minor changes
 * */
