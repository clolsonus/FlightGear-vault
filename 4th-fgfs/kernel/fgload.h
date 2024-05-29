/* fgload.h -- flight gear driver loader 
 *
 * sparc and OS/2 versions by Eric Korpela 
 * Started 5/1/96
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
 * $Id: fgload.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGLOAD_H
#define _FGLOAD_H

/* Define defrault path for drivers */
#if defined(USE_UNIX_LDSO)
#define FG_DRIVER_PATH "./"
#elif defined(USE_OS2_DLL) || defined(USE_DLL101) || defined(USE_WIN32_DLL)
#define FG_DRIVER_PATH ""
#endif

FG_DRIVER_FN fg_load_driver(char *filename,char *driver);
void fgload_export(void);

#endif


/* $Log: fgload.h,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 * Revision 2.1  1996/10/31  18:20:21  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.5  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.4  1996/05/05  22:01:26  korpela
 * minor changes
 * */
