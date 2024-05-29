/* fgkbd.h -- This is a prototype FlightGear keyboard driver             
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
 * $Id: fgkbd.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGKBD_H
#define _FGKBD_H

#include "fgdefs.h"
#include "fgtypes.h"
#include "fgevent.h"
#include "fgdriver.h"

#if defined(USE_PM) || defined(USE_WIN32)
  extern volatile FG_INT32 kbd_buffer[128];
  extern volatile FG_INT32 kb_read_ptr, kb_write_ptr;
  #ifdef USE_PM
	 FG_INT32 fg_GetPMKey(MPARAM mp1,MPARAM mp2);
  #else
	 FG_INT32 fg_GetWinKey(WPARAM key, FG_INT32 flags);
    #define KC_SHIFT 1
    #define KC_CTRL  2
    #define KC_ALT   4
  #endif
#endif

#ifdef USE_X11
  FG_INT32 fg_GetXKey();
#endif

FG_VOID *fgkbd_event(FGEVENT_CALL *event);
FG_VOID *fgkbd_init(FGDRIVER_INIT_CALL *callstruct);
FG_VOID *fgkbd_exit(FGEVENT_CALL *event);


FG_INT32 handle_fgkey_event(char *event_name,FG_EVENT_FN function);


#endif
/* $Log: fgkbd.h,v $
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
 * Revision 1.1  1996/09/12  18:38:58  korpela
 * Initial revision
 *
 * */
