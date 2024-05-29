/*                                      
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
 * $Id: fgscreen.h,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#ifndef _FGSCREEN_H_
#define _FGSCREEN_H_

#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"

#ifdef USE_PM
#include "fgres.h"
extern HWND fg_hwndClient,fg_hwndFrame;
extern HAB  fg_hab;

void FG_message_thread(void *nothing);
MRESULT EXPENTRY FG_ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#endif

#ifdef USE_WIN32
#include "fgres.h"
extern HWND fg_hwnd;
extern HBITMAP fg_hbitmap;
extern HDC fg_memdc;
void FG_message_thread(void *nothing);
void *FG_message_loop(FGEVENT_CALL *nothing);
LRESULT CALLBACK FG_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif


FG_INT32 allocate_display_bitmaps(void);
void copy_display_bitmap_to_screen(FG_INT32 display_bmp_number);

#ifdef USE_PM
void FG_message_thread( void *nothing );
MRESULT EXPENTRY FG_ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
#endif

#if defined(USE_X11)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <X11/extensions/XShm.h>
XShmSegmentInfo SHMInfo;
FG_INT32 UseSHM=1;
void fg_update_process();
#endif


Display *Dsp;
Window Wnd[20];
Colormap DefaultCMap;
XImage *Img;
FG_UINT8 *XBuf;
GC DefaultGC;
unsigned long White,Black;

#endif

#ifdef USE_FB
#include <sys/ioctl.h>
#include <sun/fbio.h>
#include <sys/mman.h>
extern struct fbgattr fb;
extern FG_INT32 fb_fd;
#endif


#ifdef USE_ALLEGRO
#include "allegro.h"
#include <sys/segments.h>
void _stub_bank_switch();
BITMAP *display_alias;
#endif

#ifdef USE_PM
#include <mmioos2.h>
#include <dive.h>
#include <fourcc.h>
HAB hab;
PULONG pPaletteArray;
HMQ hmq;
QMSG qmsg;
HPAL hpal;
HDIVE hdive;
DIVE_CAPS DiveCaps;
ULONG DiveBufferNumber;
extern FOURCC fourcc;
extern SETUP_BLITTER SetupBlitter;
extern SWP swpos;
extern POINTL pointl;
extern RECTL rectl[50];
extern HPS hps;
extern HWND hwndFrame,hwndClient;
extern ULONG flCreateOpts;
extern FG_INT32 thread_id,wait_sem;

#endif


extern FG_BITMAP *display_bitmaps[20];
extern FG_INT32 scn_width,scn_height;

#define SCN_WIDTH 640
#define SCN_HEIGHT 480

extern char *Title;
extern FG_INT32 display_window_number;

FG_VOID *fgscreen_init(FGDRIVER_INIT_CALL *callstruct);
FG_VOID *fgscreen_event(FGEVENT_CALL *event);
FG_VOID *fgscreen_exit(FGEVENT_CALL *event);

#endif
