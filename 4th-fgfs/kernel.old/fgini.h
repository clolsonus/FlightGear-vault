/* fgini.h -- ini file manager definitions 
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
 * $Id: fgini.h,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */

#ifndef _FGINI_H
#define _FGINI_H

#include "fgevent.h"
#include "fgdriver.h"

FG_INT next_entry;

typedef struct _FG_INI_ENTRY {
  FG_INT  struct_size;           /* size of this structure       */
  FG_CHAR filename[32];          /* file that driver is in       */
  FG_CHAR driver_name[32];       /* name of the driver           */
  FG_INT  autoload;              /* automatically load driver    */
  FG_INT  has_bitmap;            /* driver has a bitmap          */
  FG_INT  window_number;         /* which window is bitmap in    */
  FG_INT  x,y,width,height;      /* position and size of bitmap  */
  FG_INT  flags;                 /* flags used by driver         */
} FG_INI_ENTRY;

extern char *ini_read_format;
extern char *ini_write_format;

#define FG_INI_NUMFIELDS 10

#define FG_WINDOW_MEMORY 0  /* bitmap is in memory only */

int read_ini_file();
int write_ini_file();
FG_INI_ENTRY *find_ini_entry(char *driver_name);
FG_INI_ENTRY *get_ini_entry(int number);
FG_INI_ENTRY *new_ini_entry();

void *exit_ini_write(FGEVENT_CALL *callstruct);
void *ini_manager_init(FGDRIVER_INIT_CALL *callstruct);
void fgini_export();

#endif

/* $Log: fgini.h,v $
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
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
