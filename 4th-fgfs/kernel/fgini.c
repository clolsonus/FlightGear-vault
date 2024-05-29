/* fgini.c -- manager for ini files... 
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
 * $Id: fgini.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */
#include <stdio.h>
#include <stdlib.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgvars.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgini.h"
#include "fgmain.h"

char *ini_read_format="%s %s %d %d %d %d %d %d %d %d";
char *ini_write_format="%s %s %d %d %d %d %d %d %d %d\n";

FG_INI_ENTRY *ini_data=NULL;
FG_INT32 next_entry=0;

FG_INT32 read_ini_file() 
{
  char buffer[255];
  FG_INT32 nfields;
  FILE *ini_file=fopen("fltgear.ini","r");

  if (!ini_data) ini_data=(FG_INI_ENTRY *)calloc(MAXDRIVERS,sizeof(FG_INI_ENTRY));

  if (ini_file && ini_data) {
    while (fgets(buffer,255,ini_file) && (next_entry<MAXDRIVERS)) {
      ini_data[next_entry].struct_size=sizeof(FG_INI_ENTRY);
      nfields=sscanf(buffer,ini_read_format,
                      &(ini_data[next_entry].filename),
                      &(ini_data[next_entry].driver_name),
                      &(ini_data[next_entry].autoload),
                      &(ini_data[next_entry].has_bitmap),
                      &(ini_data[next_entry].window_number),
                      &(ini_data[next_entry].x),
                      &(ini_data[next_entry].y),
                      &(ini_data[next_entry].width),
                      &(ini_data[next_entry].height),
                      &(ini_data[next_entry].flags));
      if (nfields != FG_INI_NUMFIELDS) {
        fprintf(stderr,"Possible INI file corruption for driver %s\n",
                  ini_data[next_entry].driver_name);
      }
      next_entry++;
    }
    fclose(ini_file);

    return (0);
  } else {
    if (ini_file) fclose(ini_file);
    return (1);
  }
}


FG_INT32 write_ini_file() 
{
  FILE *ini_file=fopen("fltgear.ini","w");
  FG_INT32 entry;
  
  if (ini_file && ini_data) {
    for (entry=0; entry<next_entry; entry++) {
      fprintf(ini_file,ini_write_format,
              &(ini_data[entry].filename),
              &(ini_data[entry].driver_name),
              ini_data[entry].autoload,
              ini_data[entry].has_bitmap,
              ini_data[entry].window_number,
              ini_data[entry].x,
              ini_data[entry].y,
              ini_data[entry].width,
              ini_data[entry].height,
              ini_data[entry].flags);
    }
    fclose(ini_file);
    return (0);
  } else {
    if (ini_file) fclose(ini_file);
    return (1);
  }
}

FG_INI_ENTRY *find_ini_entry(char *driver_name) 
{
  FG_INT32 i=0;

  while ((i<next_entry) && strcmp(ini_data[i].driver_name,driver_name)) {
     i++;
  }
  if (i<next_entry) {
    return(&(ini_data[i]));
  } else {
    return(NULL);
  }
}

FG_INI_ENTRY *get_ini_entry(FG_INT32 number)
{
  if (number<next_entry) {
    return(&(ini_data[number]));
  } else {
    return (NULL);
  }
}

FG_INI_ENTRY *new_ini_entry()
{
  if (next_entry<MAXDRIVERS) {
    ini_data[next_entry].struct_size=sizeof(FG_INI_ENTRY);
	 return (&(ini_data[next_entry++]));
  } else {
	 return (NULL);
  }
}

FG_INT32 ini_exit_event;

FG_VOID *ini_manager_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT ini_init={ sizeof(FGDRIVER_INIT), 0 };
  ini_exit_event=handle_fgevent("exit",exit_ini_write);
  if (!ini_data) read_ini_file();
  return (&ini_init);
}

FG_VOID *exit_ini_write(FGEVENT_CALL *event)
{
  if (event->event_id == ini_exit_event) {
    FG_INI_ENTRY *entry=find_ini_entry("fgini");
    FG_INT32 flags=entry->flags;
  
    switch (flags) {
      case 2:  entry->flags=0;         /* don't write in the future   */
                                       /* but update the ini file now */

      case 1:  write_ini_file();       /* save changes to ini file    */

      default: break;                  /* do nothing                  */
    }
  }
  return (NULL);
}

void fgini_export()
{
}

/* $Log: fgini.c,v $
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
 * Revision 1.2  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.1  1996/05/05  22:01:26  korpela
 * Initial revision
 * */
