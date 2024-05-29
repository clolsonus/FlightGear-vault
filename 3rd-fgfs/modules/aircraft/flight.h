/* flight.c -- This is the ACM flight model
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
 * $Id: fgslew.h,v 2.1 1996/10/31 18:20:21 korpela Exp korpela $ 
 */

#ifndef _FLIGHT_H
#define _FLIGHT_H

FG_VOID *flight_event(FGEVENT_CALL *event);
FG_VOID *flight_init(FGDRIVER_INIT_CALL *callstruct);

#endif _FLIGHT_H

/* $Log: fgslew.h,v $
 */
