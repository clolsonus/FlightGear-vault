/* fgtimer.c -- set up timer events                                  
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
 * $Id: fgtimer.h,v 2.1 1996/10/31 18:20:21 korpela Exp korpela $ 
 */

#ifndef _FG_TIMER_H
#define _FG_TIMER_H

FG_VOID *fgtimer_master(FGEVENT_CALL *event);
FG_VOID *fgtimer_pause(FGEVENT_CALL *event);
FG_VOID *fgtimer_unpause(FGEVENT_CALL *event);
FG_VOID *fgtimer_init(FGDRIVER_INIT_CALL *callstruct);
int handle_fgtimer_event(int time_ms,FG_EVENT_FN function);
int unhandle_fgtimer_event(int time_ms,FG_EVENT_FN function);

#endif

/* $Log: fgtimer.h,v $
 * Revision 2.1  1996/10/31  18:20:21  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.1  1996/09/12  18:38:58  korpela
 * Initial revision
 *
 */
