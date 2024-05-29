/* fgpause.c -- This is a prototype FlightGear driver             
 *  It will pause the simulation when the main program reports that
 *  control-p has been pressed                                      
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
 * $Id: fgpause.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $ 
 */

#include <stdio.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgkbd.h"
#include "fgpause.h"

/* This is a kludge.  Change it to something better later.                */

static int pause_key_event;
static int pause_notify;
static int unpause_notify;
static FG_PUBVARS *fgpause_public;

FG_VOID *fgpause_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgpause_init_struct=
                                    {sizeof(FGDRIVER_INIT_CALL),/* version id */
                                     0};         /* nothing to tell       */ 
  if (callstruct->msg == FG_INIT) {
    pause_key_event=handle_fgkey_event("pause_toggle_key",fgpause_event);
    pause_notify=get_fgevent("pause_notify");
    unpause_notify=get_fgevent("unpause_notify");
    fgpause_public=callstruct->public_vars;
  }
  return (&fgpause_init_struct);
}

FG_VOID *fgpause_event(FGEVENT_CALL *event)
{
  static state=0;
  static FGEVENT_CALL pause;

  if (!state) {
    pause.event_id=pause_notify;
    send_fgevent(&pause);
  } else {
    pause.event_id=unpause_notify;
    send_fgevent(&pause);
  }
  state^=1;
  fgpause_public->fg_is_paused=state;
  return (NULL);
}

void fgpause_export()
{
}

/* $Log: fgpause.c,v $
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
 * Revision 1.3  1996/05/05  22:01:26  korpela
 * Started over
 * */
