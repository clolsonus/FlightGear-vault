
/* fgevent.h -- flight gear event manager
 *
 * Written by Curtis Olson
 * Started 4/23/96
 *
 * Copyright (C) 1996  Curtis L. Olson  - curt@sledge.mn.org
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
 *
 * $Id: fgevent.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 * (Log is kept at end of this file)
 */

#ifndef _FG_EVENT_H
#define _FG_EVENT_H

#include "fgtypes.h"

/***********************************************************************
 * Public defines for use by modules 
 **********************************************************************/

#define INIT 0
#define EVENT 1

typedef struct _FGEVENT_CALL {
  FG_INT32   event_id;
  void *event_data;
} FGEVENT_CALL;

typedef FG_VOID *(*FG_EVENT_FN)(FGEVENT_CALL *);

/***********************************************************************
 * Public functions for use by modules 
 **********************************************************************/


/* Create a new fgevent with the fgevent manager
 *
 * In:  character string fgevent name
 * Out:  a unique fgevent id (-1 on error)
 */
FG_INT32 get_fgevent(char *event_name);


/* Send out a fgevent
 *
 * In:  a unique fgevent id
 *      a pointer to some data structure
 * Out:  number of functions notified
 */
FG_INT32 send_fgevent(FGEVENT_CALL *event);


/* Register as a handler of the specified event
 *
 * In:  event name, a pointer to the requesting function
 * Out:  event id
 */
FG_INT32 handle_fgevent(char *event_name,FG_EVENT_FN function);

/* Unregister as a handler of the specified event
 *
 * In:  event name, a pointer to the function to deregister;
 * Out: success/failure
 */
FG_INT32 unhandle_fgevent(char *event_name,FG_EVENT_FN function);

/***********************************************************************
 * Private functions for use by kernel
 **********************************************************************/


/* Initialize the fgevent table
 *
 * In:  none
 * Out:  success/failure
 */
FG_INT32 init_fgevent_table(void);

/* A null function to use when deregistering event handlers */
FG_VOID *null_fgevent(FGEVENT_CALL *event);

void fgevent_export(void);

#endif


/* $Log: fgevent.h,v $
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
 * Revision 1.4  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.3  1996/05/16  23:14:51  korpela
 * Added DJGPP DLL support
 *
 * Revision 1.1  1996/05/05  22:01:26  korpela
 * Initial revision
 *
 * Revision 1.1  1996/04/30 21:55:30  clolson
 * Initial revision.
 *
 */

