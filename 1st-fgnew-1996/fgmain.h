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
 * $Id: fgmain.h,v 2.1 1996/10/31 18:20:21 korpela Exp korpela $
 */

char *driver_names[MAXDRIVERS];
FGDRIVER_INIT *driver_data[MAXDRIVERS];
FG_INT next_driver;
FG_BITMAP *display_bitmaps[20];
FG_BITMAP *driver_bitmap[MAXDRIVERS];

FG_INT master_event, /* called every time through the main loop           */
       idle_event,   /* called at end of main loop is there's time before */
                     /* next clock tick                                   */
       exit_event;   /* called before exit                                */

FG_VOID *public_vars; /* pointer to public variable space                  */
extern int exit_now;

#if defined(USE_WIN32)
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine,
                   INT nCmdShow);
extern HINSTANCE my_hPrevInstance,my_hInstance;
extern int my_nCmdShow;
#else
int main(int argc, char *argv[]);
#endif
int fg_main(int argc, char *argv[]);
int create_kernel_events();
int export_services();
int allocate_public_vars();
int load_static_drivers();
int load_external_drivers();
int main_loop();
    
/* $Log: fgmain.h,v $
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
