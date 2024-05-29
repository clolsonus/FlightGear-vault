/* fgstatdv.h -- flight gear static driver definitions 
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
 * $Id $
 */

#ifndef _FGSTATDV_H
#define _FGSTATDV_H

#ifndef _FGDRIVER_H
#include "fgdriver.h"
#endif

/* prototypes for default drivers */
#include "fgslew.h"
#include "fgtimer.h"
#include "fgpause.h"
#include "fgini.h"
#include "fgscreen.h"
#include "fgkbd.h"
#include "fgrand.h"
#include "fgbars.h"
#include "fgclock.h"

FG_DRIVER_FN static_drivers[]={ini_manager_init,fgtimer_init,fgslew_init,
                               fgkbd_init,fgscreen_init, fgrand_init,
                               fgpause_init, fgbars_init, fgclock_init,
                               (FG_DRIVER_FN)0}; 
FG_CHAR *static_names[]={"fgini","fgtimer","fgslew","fgkbd",
                         "fgscreen","fgrand","fgpause","fgbars","fgclock",
                          NULL};

#endif

/* $Log: fgstatdv.h,v $
 * Revision 2.1  1996/10/31  18:20:21  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.2  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.1  1996/05/05  22:01:26  korpela
 * Initial revision
 * */
