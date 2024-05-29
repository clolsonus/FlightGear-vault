/* fgmain.c -- flight gear main program 
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
 * $Id: fgmain.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include "fgdefs.h"
#include "fgvars.h"
#include "fgtypes.h"
#include "fgini.h"
#include "fggrx.h"
#include "fgevent.h"
#include "fgstatdv.h"
#include "fgscreen.h"
#include "fgkbd.h"
#include "fggeom.h"
#include "fgload.h"
#include "fgfont.h"
#include "fgmain.h"

int exit_now=0;
char *driver_names[MAXDRIVERS];
FGDRIVER_INIT *driver_data[MAXDRIVERS];
FG_INT next_driver=0;
FG_BITMAP *display_bitmaps[20];
FG_BITMAP *driver_bitmap[MAXDRIVERS];

FG_INT master_event, /* called every time through the main loop           */
       idle_event,   /* called at end of main loop is there's time before */
                     /* next clock tick                                   */
       exit_event;   /* called before exit                                */

FG_VOID *public_vars; /* pointer to public variable space                  */

#if defined(WIN32)

#include <windows.h>
HINSTANCE my_Instance, my_PrevInstance;
int my_CmdShow;

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst, LPSTR lpszCmdLine,
                    int nCmdShow)
{
  char *argv[MAX_CMDLINE_PARAMS];
  int argc=0;

  my_Instance=hInst;
  my_PrevInstance=hPrevInst;
  my_CmdShow=nCmdShow;
  argv[argc++]=strtok(lpszCmdLine," ");
  while ((argv[argc++]=strtok(NULL," ")));
  return (fg_main(argc,argv));
}
#else
int main(int argc, char *argv[])
{
  return (fg_main(argc,argv));
}
#endif


int fg_main(int argc, char *argv[]) 
{
  int i=0;
  char *start_names[]={"init_fgevent_table()","create_kernel_events()",
                       "allocate_public_vars()","read_ini_file()",
                       "allocate_display_bitmaps()","load_static_drivers()",
                       "load_external_drivers()","load_startup_mode()"};

  if ( !init_fgevent_table() && ++i 
      && !export_services() && ++i
      && !create_kernel_events() && ++i
      && !allocate_public_vars() && ++i
      && !read_ini_file() && ++i
      && !allocate_display_bitmaps() && ++i
      && !load_static_drivers() && ++i
      && !load_external_drivers()  && ++i
/*      && !load_startup_mode() && ++i */ )
  { 
    main_loop();
  } else {
    fprintf(stderr,"Something failed in %s.\n",start_names[i]);
  }
  return (0);
}

int create_kernel_events() 
{
  if ( ((master_event = get_fgevent("master")) != -1) &&
       ((idle_event = get_fgevent("idle")) != -1) &&
       ((exit_event = get_fgevent("exit")) != -1) ) 
  {
     return (0);
  } else {
     return (1);
  }
}

int export_services()
{
  fgevent_export();
  fggeom_export();
  fggrx_export();
  fgini_export();
  fgload_export();
  fgpause_export();
  flight_export();
  return (0);
}

int allocate_public_vars()
{
  return (!(public_vars=(FG_VOID *)calloc(PUBLIC_VARS_SIZE,1)));
}

int load_static_drivers() 
{
  FGDRIVER_INIT_CALL callstruct;
  int i=0;
  FG_INI_ENTRY *ini_entry;

  /* set up callstruct */
  callstruct.struct_size=sizeof(callstruct);
  callstruct.msg=FG_INIT;
  callstruct.public_vars=public_vars;

  do {
    if ((ini_entry=find_ini_entry(static_names[i]))) {
      if (ini_entry->autoload) {
        if (ini_entry->has_bitmap) {
          if (ini_entry->window_number) {
            callstruct.bitmap=
              fg_create_subbitmap(display_bitmaps[ini_entry->window_number-1],
                FG_MIN(ini_entry->x*scn_width/1000,scn_width-1),
                FG_MIN(ini_entry->y*scn_height/1000,scn_height-1),
                FG_MIN(ini_entry->width*scn_width/1000,scn_width-1), 
                FG_MIN(ini_entry->height*scn_height/1000,scn_height-1));
          } else {
            callstruct.bitmap=
              fg_create_bitmap(ini_entry->width, ini_entry->height);
          }
          driver_bitmap[next_driver]=callstruct.bitmap;
        }
        printf("loading static driver %s\n",driver_names[next_driver]=static_names[i]);
        driver_data[next_driver++]=static_drivers[i](&callstruct);
      }
    }
    i++;
  } while (static_drivers[i]);
  return (0);
}
    
int load_external_drivers() 
{
  FGDRIVER_INIT_CALL callstruct;
  int i=0;
  FG_INI_ENTRY *ini_entry;
  FG_DRIVER_FN init_fn;

  do {
    if ((ini_entry=get_ini_entry(i))) {
      if (ini_entry->autoload) {
        if (strcmp(ini_entry->filename,"static")) {
          if (ini_entry->has_bitmap) {
            if (ini_entry->window_number) {
              callstruct.bitmap=
                fg_create_subbitmap(display_bitmaps[ini_entry->window_number-1],
                  ini_entry->x, ini_entry->y, 
                  ini_entry->width, ini_entry->height);
            } else {
              callstruct.bitmap=
                fg_create_bitmap(ini_entry->width, ini_entry->height);
            }
            driver_bitmap[next_driver]=callstruct.bitmap;
          }
          printf("loading external driver %s\n",driver_names[next_driver]=ini_entry->driver_name);
          init_fn=fg_load_driver(ini_entry->filename,ini_entry->driver_name); 
          driver_data[next_driver++]=init_fn(&callstruct);
        }
      }
    }
    i++;
  } while (i<next_entry);
  return (0);
}

int main_loop()
{
  time_t clock1;
  FGEVENT_CALL master_call={master_event,NULL};
  FGEVENT_CALL idle_call={idle_event,NULL};
  FGEVENT_CALL exit_call={exit_event,NULL};
  fprintf(stderr,"In main loop.\n");
  do {
   clock1=time(0); 
   send_fgevent(&master_call);
   while (clock1 == clock()) {
     send_fgevent(&idle_call);
     usleep(250000/MAX_FRAME_RATE);
   }
  } while (!exit_now);
  send_fgevent(&exit_call);
  return(0);
}



/* $Log: fgmain.c,v $
 * Revision 2.1  1996/10/31  18:12:04  korpela
 *  Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.6  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.5  1996/05/05  22:01:26  korpela
 * Started over
 * */
