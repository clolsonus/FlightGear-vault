/* fgmain.h -- flight gear main program
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
 * $Id: fgmain.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGMAIN_H
#define _FGMAIN_H

extern char *driver_names[MAXDRIVERS];
extern FGDRIVER_INIT *driver_data[MAXDRIVERS];
extern FG_INT32 next_driver;
extern FG_BITMAP *display_bitmaps[20];
extern FG_BITMAP *driver_bitmap[MAXDRIVERS];

extern FG_INT32 master_event, /* called every time through the main loop           */
				  idle_event,   /* called at end of main loop is there's time before */
									 /* next clock tick                                   */
				  exit_event;   /* called before exit                                */

extern FG_PUBVARS *public_vars; /* pointer to public variable space                  */
extern FG_INT32 exit_now;

#if defined(USE_WIN32)
#if defined(__BCC__)
#define INT int
#endif
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine,
						 INT nCmdShow);
extern HINSTANCE my_hPrevInstance,my_hInstance;
extern FG_INT32 my_nCmdShow;
extern FILE *output;
#else
int main(int argc, char *argv[]);
#endif
int fg_main(int argc, char *argv[]);
FG_INT32 create_kernel_events(void);
FG_INT32 export_services(void);
FG_INT32 allocate_public_vars(void);
FG_INT32 load_static_drivers(void);
FG_INT32 load_external_drivers(void);
FG_INT32 main_loop(void);

#endif

/* $Log: fgmain.h,v $
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
 * Revision 1.1  1996/09/12  18:38:58  korpela
 * Initial revision
 *
 * */
