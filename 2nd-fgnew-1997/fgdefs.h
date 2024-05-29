/* fgdefs.h -- flight gear definitions
 *
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
 * $Id: fgdefs.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGDEFS_H
#define _FGDEFS_H
#include <stdlib.h>
#include <unistd.h>

#if defined(__BCC__) && !defined(RC_INVOKED)
#if  (!defined(__WIN32__) && !defined(_WIN32))
#include <windows.h>
#include "fgwin16.h"
#define memset(x,y,z) _fmemset(x,y,z)
#define malloc(x) WinAlloc(x)
#define calloc(x,y) WinAlloc((x)*(y))
#define free(x) WinFree(x)
#endif
#define inline
#include <mem.h>
#include <string.h>
#define random() rand()
#define srandom(x) srand(x)
#endif

#if defined(USE_ANSI_C)
#define inline
#endif

#if defined(__DJGPP__)
  #if (!defined(USE_SVGALIB) && !defined(RSX))
	 #define USE_ALLEGRO
  #endif
  #if (!defined(RSX))
	 #define USE_DLL101
  #endif
#endif

#if defined(RC_INVOKED) && (defined(OPEN32) || defined(OS2) || (defined(__EMX__) && !defined(RSX)))
  #define USE_OS2_RC
#endif


#if defined(__EMX__) && !(defined(RSX) || defined(WIN32) || defined(USE_WIN32) || defined(OPEN32))
  #if !defined(USE_SVGALIB) && !defined(USE_X11)
	 #define USE_PM
	 #define INCL_WIN
	 #define INCL_GPI
  #endif
  #define USE_OS2_DLL
  #define INCL_DOS
  #include <os2.h>
  #define usleep(x) DosSleep(x/1000);
#endif

#if defined(WIN32) || defined(USE_WIN32) || defined(OPEN32) || defined(RSX) && !defined(RC_INVOKED)
  #define WIN32
  #define USE_WIN32
  #define USE_WIN32_DLL
  #if !defined(OPEN32)
	 #include <windows.h>
  #else
         #define _System
	 #include <os2win.h>
  #endif
  #include <stdio.h>
  #ifdef stdout
	 #undef stdout
  #endif
  #ifdef stderr
	 #undef stderr
  #endif
  #define stdout output
  #define stderr output
  #if !(defined(RSX) || defined(__BCC__))
	 #define usleep(x) Sleep(x/1000)
  #else
	 #define usleep(x) (x)
  #endif
#endif

#ifndef RC_INVOKED
#if (defined(__linux__) || defined(__sparc__) || defined(__sun__)) && !defined(USE_WIN32)
  #define USE_UNIX_LDSO
  #if !defined(USE_SVGALIB) && !defined(USE_FB)
	 #define USE_X11
  #endif
#endif

#if defined(USE_X11)
  #define COLOR_INDEX(color) (Xpal[color])
#else
  #define COLOR_INDEX(color) (color)
#endif

#define MAX_CMDLINE_PARAMS 32
#define MAX_FRAME_RATE 30
#define NUM_TIMERS 32
#define MAX_FGEVENT_NAME 33
#define FGEVENT_TABLE_SIZE 1024
#define NUM_KEY_EVENTS 256
#define NUM_COLORS 256

#define PUBLIC_VARS_SIZE sizeof(FG_PUBVARS)

#define FG_ONE 32768L
#define FG_SQRT_ONE 5931641L

/* Max number of drivers that can be run                               */
#define MAXDRIVERS 255
#define MAX_DYN_DRIVERS MAXDRIVERS
/* Max number of key codes to be watched                               */
#define MAXKEYS 1024

#define FG_MAX(x,y)  ( ((x)<(y)) ? (y) : (x)  )
#define FG_MIN(x,y)  ( ((x)>(y)) ? (y) : (x)  )
#define FG_ABS(x)    ( ((x)>0) ? (x) : (-1*(x)) )

/* Add memmove for sparc SunOS4                                        */
#if (defined(__sparc__) || defined(__sun__)) && !defined(__svr4__)
  #define memmove(x,y,z) bcopy((y),(x),(z))
#endif

/* Add 64 bit macros */
#define MUL64(a,b)  ((FG_INT64)(a)*(b))
#define DIV64(a,b)  ((FG_INT64)(a)/(b))
#define SHL64(a,b)  (((FG_INT64)a)<<(b))
#define SHR64(a,b)  (((FG_INT64)a)>>(b))

#endif
#endif

/* $Log: fgdefs.h,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.9  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.8  1996/05/17  18:42:37  korpela
 * minor changes
 *
 * Revision 1.7  1996/05/17  18:40:56  korpela
 * minor changes
 *
 * Revision 1.6  1996/05/05  22:01:26  korpela
 * minor changes
 * */
