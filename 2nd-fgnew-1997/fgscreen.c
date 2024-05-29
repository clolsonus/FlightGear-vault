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
 * $Id: fgscreen.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */
#include "fgscreen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef __BCC__
#include <sys/time.h>
#include <unistd.h>
#else
#include <time.h>
#endif
#include <limits.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fggrx.h"
#include "fgkbd.h"
#include "fgtimer.h"
#include "fgmain.h"
#include "fgpalet.h"
#include "fgfont.h"
#include "fgini.h"
#include "fgpause.h"

char *Title="FlightGear version 0.0beta";
FG_INT32 display_window_number=0;
FG_INI_ENTRY *screen_ini_entry,*tmp_ini_entry;
FG_INT32 scn_width,scn_height;


#if defined(USE_X11)
/* #define MITSHM */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
XShmSegmentInfo SHMInfo;
FG_INT32 UseSHM=1;
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
#include <sys/fcntl.h>
#include <sun/fbio.h>
#include <sys/mman.h>
struct fbgattr fb;
FG_INT32 fb_fd;
off_t fb_offset[FBTYPE_LASTPLUSONE] = {
   [6] 0x04000000,
  [12] 0x70000000 
};
size_t fb_im_offset[FBTYPE_LASTPLUSONE] = {
  [12] 16000
};
#endif


#ifdef USE_ALLEGRO
#include "allegro.h"
#include <sys/segments.h>
void _stub_bank_switch();
BITMAP *display_alias;
#endif

#ifdef USE_WIN32
WNDCLASS fg_WndClass;
HWND fg_hwnd;
FG_INT32 thread_id;
LPSTR ClassName="FG_CLASS";

#endif

#ifdef USE_PM
#include <mmioos2.h>
#include <dive.h>
#include <fourcc.h>
HAB fg_hab;
HMQ fg_hmq;
QMSG fg_qmsg;
HDIVE fg_hdive;
DIVE_CAPS DiveCaps;
ULONG DiveBufferNumber;
FOURCC fourcc=FOURCC_LUT8;
HPS fg_hps;
HWND fg_hwndFrame,fg_hwndClient;
ULONG flCreateOpts=FCF_STANDARD;
FG_INT32 thread_id,wait_sem;
#endif

FG_INI_ENTRY *new_ini_entry(void);
FG_BITMAP *display_bitmaps[20];

FG_INT32 allocate_display_bitmaps()
{
  if (!(screen_ini_entry=find_ini_entry("fgscreen")))
  {
	 screen_ini_entry=new_ini_entry();
	 strcpy(screen_ini_entry->filename,"static");
	 strcpy(screen_ini_entry->driver_name,"fgscreen");
    screen_ini_entry->width=SCN_WIDTH;
    screen_ini_entry->height=SCN_HEIGHT;
    screen_ini_entry->autoload=1;
    tmp_ini_entry=find_ini_entry("fgini");
    tmp_ini_entry->flags=2;
  }
  scn_width=screen_ini_entry->width;
  scn_height=screen_ini_entry->height;

#ifdef USE_FB
  {
    FG_INT32 i;
    if (strcmp(ttyname(0),"/dev/console")) {
		fprintf(stderr,"You must be logged into the console to use /dev/fb\n");
      exit(1);
	 }
    if ((fb_fd=open("/dev/fb", O_RDWR))==-1) {
		fprintf(stderr,"Unable to open /dev/fb\n");
      exit(1);
    }
    if (ioctl(fb_fd,FBIOGATTR,&fb)) {
      if (ioctl(fb_fd,FBIOGTYPE,&fb.fbtype)) {
		  fprintf(stderr,"Unable to determine fb type.\n");
        exit(1);
      }
    }
    if ((fb.fbtype.fb_width != scn_width) || (fb.fbtype.fb_height != scn_height)) 
    {
      tmp_ini_entry=find_ini_entry("fgini");
      tmp_ini_entry->flags=2;
      scn_width=fb.fbtype.fb_width;
      scn_height=fb.fbtype.fb_height;
      screen_ini_entry->width=scn_width;
      screen_ini_entry->height=scn_height;
    } 

    display_bitmaps[0]=(FG_BITMAP *)calloc(1,
	     sizeof(FG_BITMAP)+fb.fbtype.fb_height*sizeof(FG_VOID **));
    switch (fb.fbtype.fb_depth) {
      case 8:  display_bitmaps[0]->color_format=FG_CF_LUT8;
               display_bitmaps[0]->pix_size=pix_size[FG_CF_LUT8];
               break;
      case 24: display_bitmaps[0]->color_format=FG_CF_RGB888;
               display_bitmaps[0]->pix_size=pix_size[FG_CF_RGB888];
               break;
      default: fprintf(stderr,"Unsupported color dept\n");
               exit(1);
    }
    display_bitmaps[0]->struct_size=sizeof(FG_BITMAP);
    display_bitmaps[0]->w=fb.fbtype.fb_width;
    display_bitmaps[0]->h=fb.fbtype.fb_height;
    display_bitmaps[0]->clip=1; 
    display_bitmaps[0]->cl=0;
    display_bitmaps[0]->cr=fb.fbtype.fb_width-1;
    display_bitmaps[0]->ct=0;
    display_bitmaps[0]->cb=fb.fbtype.fb_height-1;
    display_bitmaps[0]->lines=(FG_UINT8 **)(((char *)display_bitmaps[0])
				+sizeof(FG_BITMAP));
    display_bitmaps[0]->plot=plot_fns(display_bitmaps[0]->color_format);
    display_bitmaps[0]->hline=hline_fns(display_bitmaps[0]->color_format);
    display_bitmaps[0]->getpixel=getp_fns(display_bitmaps[0]->color_format);
    
    if (!(display_bitmaps[0]->lines[0]=
      mmap(0,
      fb.fbtype.fb_width*fb.fbtype.fb_height+fb_im_offset[fb.fbtype.fb_type],
      PROT_READ|PROT_WRITE,MAP_SHARED,
      fb_fd,fb_offset[fb.fbtype.fb_type])
	+fb_im_offset[fb.fbtype.fb_type]))
    {
		fprintf(stderr,"Unable to map frame buffer to process memory.\n");
      exit(1);
    }
    for (i=1;i<fb.fbtype.fb_height;i++) {
      display_bitmaps[0]->lines[i]=(display_bitmaps[0]->lines[i-1])
                                    +fb.fbtype.fb_width*display_bitmaps[0]->pix_size;
    }
    fg_set_startup_palette();
  }
#endif

#ifdef USE_ALLEGRO
  {
    FG_INT32 modes_x[]={ 320, 640, 640, 800, 1024, 1280, 0 };
    FG_INT32 modes_y[]={ 200, 400, 480, 600,  768, 1024, 0 };
    FG_INT32 i=0,closest_mode,diff=INT_MAX,mode_diff;

    while (modes_x[i])  {
      mode_diff=FG_ABS(scn_width-modes_x[i]);
      mode_diff+=FG_ABS(scn_height-modes_y[i]);
      if (mode_diff<diff) {
         closest_mode=i;
         diff=mode_diff;
      }
      i++;
    }

    if ((modes_x[closest_mode] != scn_width) || 
        (modes_y[closest_mode] != scn_height)) 
    {
      tmp_ini_entry=find_ini_entry("fgini");
      tmp_ini_entry->flags=2;
      scn_width=modes_x[closest_mode];
      scn_height=modes_y[closest_mode];
      screen_ini_entry->width=scn_width;
      screen_ini_entry->height=scn_height;
    }
    
    /* only one display window in DOS, for now at least... */
    if (allegro_init() ||
       !(display_alias=create_bitmap(scn_width,scn_height)) ||
       set_gfx_mode(GFX_AUTODETECT,scn_width,scn_height, scn_width, scn_height))
    {
		fprintf(stderr,"Error setting up display!\n");
      exit(1);
    }
    fg_set_startup_palette();
    display_bitmaps[0]=calloc(1,sizeof(FG_BITMAP)+
                                 sizeof(FG_UINT8 **)*scn_height);
    display_bitmaps[0]->w=display_alias->w;
    display_bitmaps[0]->h=display_alias->h;
    display_bitmaps[0]->clip=display_alias->clip;
    display_bitmaps[0]->cl=display_alias->cl;
    display_bitmaps[0]->cr=display_alias->cr;
    display_bitmaps[0]->ct=display_alias->ct;
    display_bitmaps[0]->cb=display_alias->cb;
    display_bitmaps[0]->color_format=FG_CF_LUT8;
    display_bitmaps[0]->pix_size=pix_size[FG_CF_LUT8];
    display_bitmaps[0]->plot=plot_fns[FG_CF_LUT8];
    display_bitmaps[0]->hline=hline_fns[FG_CF_LUT8];
    display_bitmaps[0]->getpixel=getp_fns[FG_CF_LUT8];
    display_bitmaps[0]->lines=(FG_UINT8 **)(((char *)display_bitmaps[0])
				+sizeof(FG_BITMAP));
    for (i=0;i<display_alias->h;i++) display_bitmaps[0]->lines[i]
                                             =display_alias->line[i];
  }
#endif

#if defined(USE_PM) /* || defined(USE_WIN32) */
  wait_sem=1;
  if ((thread_id=_beginthread(FG_message_thread, NULL, 32768, NULL)) == -1) 
  {
	 fprintf(stderr,"Unable to start thread\n");
    exit(-1);
  }
#endif

#if defined(USE_WIN32)
  FG_message_thread(NULL);
#endif 

#if defined(USE_X11)
  { 
    FG_INT32 i;
    Screen *Scr;
    XSizeHints Hints;
    XWMHints WMHints;

    if (!(Dsp=XOpenDisplay(NULL))) 
    {
		fprintf(stderr,"Error setting up display!\n");
      exit(1);
    }
    Scr=DefaultScreenOfDisplay(Dsp);
    White=WhitePixelOfScreen(Scr);
    Black=BlackPixelOfScreen(Scr);
    DefaultGC=DefaultGCOfScreen(Scr);
    DefaultCMap=DefaultColormapOfScreen(Scr);
    XSynchronize(Dsp,True);
    if (!(Wnd[0]=XCreateSimpleWindow(Dsp,RootWindowOfScreen(Scr),0,0,scn_width,scn_height,0,White,Black)))
    {
		fprintf(stderr,"Error setting up display!\n");
      exit(1);
    }
  
    Hints.flags=PSize|PMinSize|PMaxSize;
    Hints.min_width=Hints.max_width=Hints.base_width=scn_width;
    Hints.min_height=Hints.max_height=Hints.base_height=scn_height;
    WMHints.input=True; WMHints.flags=InputHint;
    XSetWMHints(Dsp,Wnd[0],&WMHints);
    XSetWMNormalHints(Dsp,Wnd[0],&Hints);
    XStoreName(Dsp,Wnd[0],Title);
    XSelectInput(Dsp,Wnd[0],FocusChangeMask|ExposureMask|KeyPressMask|KeyReleaseMask);
    XMapRaised(Dsp,Wnd[0]);
    XClearWindow(Dsp,Wnd[0]);
    XAutoRepeatOn(Dsp);
  
  #ifdef MITSHM
    if(UseSHM)
    {
		fprintf(stderr,"Using Shared Memory\n");
      if (!(Img=XShmCreateImage(Dsp,DefaultVisualOfScreen(Scr),8,ZPixmap,NULL,
                  &SHMInfo,scn_width,scn_height)) ||
          ((SHMInfo.shmid=shmget(IPC_PRIVATE,Img->bytes_per_line*Img->height,
                           IPC_CREAT|0777))<0) ||
          !(XBuf=(FG_UINT8 *)(Img->data=SHMInfo.shmaddr=shmat(SHMInfo.shmid,0,0))))
      {
		  fprintf(stderr,"Error setting up display!\n");
        exit(1);
      } 
      SHMInfo.readOnly=False;
      if (!XShmAttach(Dsp,&SHMInfo)) 
      {
		  fprintf(stderr,"Error setting up display!\n");
        exit(1);
      } 
    } else
  #endif
    {
      if (!(XBuf=(FG_UINT8 *)malloc(sizeof(FG_UINT8)*scn_width*scn_height)) ||
          !(Img=XCreateImage(Dsp,DefaultVisualOfScreen(Scr),8,ZPixmap,0,
                  XBuf,scn_width,scn_height,8,0)))
      {
		  fprintf(stderr,"Error setting up display!\n");
        exit(1);
      } 
    }
  
    fg_set_startup_palette();
    
    display_bitmaps[0]=(FG_BITMAP *)malloc(sizeof(FG_BITMAP)+scn_height*sizeof(FG_UINT8 *));
    display_bitmaps[0]->struct_size=sizeof(FG_BITMAP);
    display_bitmaps[0]->w=Img->width; display_bitmaps[0]->h=Img->height;
    display_bitmaps[0]->clip=1; 
    display_bitmaps[0]->cl=0; display_bitmaps[0]->cr=Img->width; 
    display_bitmaps[0]->ct=0; display_bitmaps[0]->cb=Img->height;
    display_bitmaps[0]->type=FG_NORMALBITMAP;
    display_bitmaps[0]->lines=(FG_UINT8 **)(((char *)display_bitmaps[0])
                                             +sizeof(FG_BITMAP));
    display_bitmaps[0]->color_format=FG_CF_LUT8;
    display_bitmaps[0]->pix_size=pix_size[FG_CF_LUT8];
    display_bitmaps[0]->plot=plot_fns[FG_CF_LUT8];
    display_bitmaps[0]->hline=hline_fns[FG_CF_LUT8];
    display_bitmaps[0]->getpixel=getp_fns[FG_CF_LUT8];

    for (i=0;i<Img->height;i++) 
      display_bitmaps[0]->lines[i]=Img->data+i*Img->bytes_per_line;
  }   
#endif  
  if (display_bitmaps[0]) {
    fg_clearbmp(0,display_bitmaps[0]);
  }
  fgfont_init();
  return (0);
}
   

void copy_display_bitmap_to_screen(FG_INT32 display_bmp_number)
{
#ifdef USE_ALLEGRO
  /* only one display window in DOS, for now at least... */
  blit(display_alias,screen,0,0,0,0,scn_width,scn_height);   
#endif
#ifdef USE_PM 
  WinInvalidateRect(fg_hwndClient,NULL,FALSE);
#endif
#ifdef USE_WIN32
  InvalidateRect(fg_hwnd,NULL,FALSE);
#endif
#if defined(USE_X11)
#ifdef MITSHM
  if(UseSHM) 
    XShmPutImage(Dsp,Wnd[0],DefaultGC,Img,0,0,0,0,scn_width,scn_height,False);
  else
#endif
    XPutImage(Dsp,Wnd[0],DefaultGC,Img,0,0,0,0,scn_width,scn_height);
    XFlush(Dsp);
#endif
}

#ifdef USE_WIN32
FG_INT32 appState=0;

void FG_message_thread( void *nothing )
{
  MSG msg;
  if (!my_hPrevInstance)
  {
	 fg_WndClass.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
	 fg_WndClass.lpfnWndProc=FG_WndProc;
	 fg_WndClass.cbClsExtra=0;
	 fg_WndClass.cbWndExtra=0;
	 fg_WndClass.hInstance=my_hInstance;
	 fg_WndClass.hIcon=LoadIcon(NULL,MAKEINTRESOURCE(FG_PROGRAM_ID));
	 fg_WndClass.hCursor=LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW));
	 fg_WndClass.hbrBackground=GetStockObject(BLACK_BRUSH);
	 fg_WndClass.lpszMenuName=MAKEINTRESOURCE(FG_PROGRAM_ID);
	 fg_WndClass.lpszClassName=ClassName;
	 appState=!RegisterClass(&fg_WndClass);
  }
  fg_hwnd= CreateWindow(ClassName,Title,WS_OVERLAPPEDWINDOW,
                        0,0, scn_width, scn_height,
                        NULL,NULL,my_hInstance,NULL);
  appState=(appState || !fg_hwnd);
  if (!appState) {
	 ShowWindow(fg_hwnd, my_nCmdShow);
	 UpdateWindow(fg_hwnd);
  }
  handle_fgevent("master",FG_message_loop);
}


void *FG_message_loop(FGEVENT_CALL *nothing)
{
  MSG msg;

  while (PeekMessage(&msg, fg_hwnd, 0, 0,PM_REMOVE))
  {
    if (!appState)
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return (NULL);
}
#endif

#ifdef USE_PM

void FG_message_thread( void *nothing )
{
  static FG_INT32 appState=0;
  static PSZ ClassName="FG_CLASS";
  FG_INT32 reg_res;

  if (!(fg_hab=WinInitialize(0)) ||
      !(fg_hmq=WinCreateMsgQueue(fg_hab,0)) ||
      !(reg_res=WinRegisterClass(fg_hab, ClassName, FG_ClientWndProc, CS_SIZEREDRAW,0)) ||
      !(fg_hwndFrame=WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &flCreateOpts,
         ClassName, Title, 0L, 0, FG_PROGRAM_ID, &fg_hwndClient)))
  {
	 fprintf(stderr,"Unable to create Window\n");
	 fprintf(stderr,"error #%x\n",WinGetLastError(fg_hab));
	 fprintf(stderr,"fg_hab=%x\n",fg_hab);
	 fprintf(stderr,"fg_hmq=%x\n",fg_hmq);
	 fprintf(stderr,"reg_res=%d\n",reg_res);
	 fprintf(stderr,"fg_hwndFrame=%x\n",fg_hwndFrame);
	 fprintf(stderr,"fg_hwndClient=%x\n",fg_hwndClient);
  } else {
/*    DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,-1,thread_id); */
    while (!appState) {
      while (WinGetMsg(fg_hab, &fg_qmsg, 0, 0, 0))
        WinDispatchMsg(fg_hab, &fg_qmsg);

      if (WinSendMsg(fg_hwndClient, WM_COMMAND, MPFROM2SHORT(CMD_VERIFY, 0), 0)
           != MBID_NO) appState=USERQUIT;

      if (!fg_qmsg.hwnd) /* system shutdown */
        if (appState != USERQUIT) WinCancelShutdown(fg_hmq, FALSE);
      
    }
    handle_WM_CLOSE(fg_hwndClient,fg_hpal);
    exit_now=1;
    WinSetVisibleRegionNotify(fg_hwndClient,FALSE);
  }

  if (DiveBufferNumber) DestroyDiveBuffer(fg_hdive, DiveBufferNumber, display_bitmaps[0]->lines);
  if (fg_hdive) EndDive(fg_hdive,&DiveCaps);
  WinDestroyWindow(fg_hwndFrame);
  WinDestroyMsgQueue(fg_hmq);
  WinTerminate(fg_hab);
  _endthread();
}

#endif

#ifdef USE_WIN32
LRESULT CALLBACK FG_WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static HWND hwnd_desktop;
  static FG_INT32 key_flags;
  static BITMAP bitmap;
  HMENU hMenu;
  HDC hdc;
  LRESULT rv=0;
  PAINTSTRUCT ps;
  FG_INT32 cmd;
  static LONG TotalWidth,TotalHeight;


  switch (msg) {
	 case WM_CREATE:
		{
		  int color_format;
		  LONG BorderWidth= (LONG)GetSystemMetrics(SM_CXFRAME);
		  LONG BorderHeight= (LONG)GetSystemMetrics(SM_CYFRAME);
		  LONG TitleBarHeight= (LONG)GetSystemMetrics(SM_CYCAPTION);
		  LONG MenuHeight= (LONG)GetSystemMetrics(SM_CYMENU);
		  TotalWidth = scn_width+2*BorderWidth;
		  TotalHeight = scn_height+MenuHeight+TitleBarHeight+2*BorderHeight;
		  hwnd_desktop=GetDesktopWindow();
		  SetWindowPos(hwnd,HWND_TOP,
		      (GetSystemMetrics(SM_CXSCREEN)-TotalWidth)/2,
		      (GetSystemMetrics(SM_CYSCREEN)-TotalHeight)/2,
		      TotalWidth, TotalHeight,SWP_SHOWWINDOW );
		  SetFocus(hwnd);
		  hdc=GetDC(hwnd);
		  fg_hbitmap=CreateCompatibleBitmap(hdc,scn_width,scn_height);
		  GetObject(fg_hbitmap,sizeof(BITMAP),&bitmap);
		  switch (bitmap.bmBitsPixel) {
		    case 8:   color_format=FG_CF_LUT8;
                              break;
		    case 16:  color_format=FG_CF_RGB555;
		              break;
		    case 24:  color_format=FG_CF_RGB888;
		              break;
		    case 32:  color_format=FG_CF_RGBX8888;
		              break;
		    default:  fprintf(stderr,"Unable to determine color format\n");
		              exit(1);
		  }
                  display_bitmaps[0]=fg_create_bitmap(scn_width,scn_height,color_format);
		  fg_set_startup_palette(hwnd);
		  SetBitmapBits(fg_hbitmap,scn_width*scn_height*(bitmap.bmBitsPixel/8),display_bitmaps[0]->lines[0]);
		  SelectObject(fg_memdc,fg_hbitmap);
		  SetMapMode(fg_memdc,GetMapMode(hdc));
		  ReleaseDC(hwnd,hdc);
		  SendMessage(hwnd,WM_COMMAND,CMD_PALETTE_CHANGE,0);
		}
		break;
	 case WM_SIZE:
		if ((LOWORD(lparam) != TotalWidth) || (HIWORD(lparam) != TotalHeight)) {
		  SetWindowPos(hwnd,HWND_TOP,0,0,TotalWidth,TotalHeight,
			 SWP_SHOWWINDOW | SWP_NOMOVE );
		}
		break;
	 case WM_PAINT:
		{ int i;
		  POINT ptSize,ptOrg;
		  hdc=BeginPaint(hwnd,&ps);
		  fg_memdc=CreateCompatibleDC(hdc);
		  SetMapMode(fg_memdc,GetMapMode(hdc));
		  SetBitmapBits(fg_hbitmap,
			 display_bitmaps[0]->w*display_bitmaps[0]->h*display_bitmaps[0]->pix_size,
			 display_bitmaps[0]->lines[0]);
		  SelectObject(fg_memdc,fg_hbitmap);
		  GetObject(fg_hbitmap,sizeof(BITMAP),&bitmap);
		  ptSize.x=bitmap.bmWidth;
		  ptSize.y=bitmap.bmHeight;
		  DPtoLP(hdc,&ptSize,1);
		  ptOrg.x=0;
		  ptOrg.y=0;
		  DPtoLP(fg_memdc,&ptOrg,1);
		  BitBlt(hdc,0,0,ptSize.x,ptSize.y,fg_memdc,ptOrg.x,ptOrg.y,SRCCOPY);
		  DeleteDC(fg_memdc);
		  EndPaint(hwnd,&ps);
		}
		return (0);
		break;
	 case WM_COMMAND:
		switch (wparam)
		{
		  case CMD_EXIT:
		         {
		           FG_INT32 state=(public_vars->fg_is_paused);
		           if (!(public_vars->fg_is_paused)) fgpause_event(NULL);
			   if (SendMessage(fg_hwnd, WM_COMMAND,CMD_VERIFY, 0) != IDNO) {
				appState=1;
				exit_now=1;
				SendMessage(fg_hwnd, WM_CLOSE, 0, 0);
			   }
			   if (!state) fgpause_event(NULL);
			   break;
			 }
		  case CMD_PALETTE_CHANGE:
			 if (fg_hpalette) {
				hdc=GetDC(hwnd);
				SelectPalette(hdc,fg_hpalette,FALSE);
				RealizePalette(hdc);
				ReleaseDC(hwnd,hdc);
			 }
			 break;
		  case CMD_VERIFY:
			 cmd=IDNO;
			 cmd=MessageBox( hwnd,
					"Are you sure you want to exit FlightGear?",
					"Exit",
					MB_YESNO | MB_ICONQUESTION);
			 rv=(LRESULT)cmd;
			 return(rv);
		/* add other commands later */
		}
		break;
	 case WM_PALETTECHANGED:
		if (wparam == hwnd) break;
	 case WM_ACTIVATE:
		SendMessage(hwnd,WM_COMMAND,CMD_PALETTE_CHANGE,0);
		break;
	 case WM_KEYUP:
		switch (wparam) {
		#ifdef VK_LSHIFT
		  case VK_LSHIFT:
		  case VK_RSHIFT:
		#endif
		  case VK_SHIFT: key_flags&=~KC_SHIFT;
							  break;
		#ifdef VL_LCONTROL
		  case VK_LCONTROL:
		  case VK_RCONTROL:
		#endif
		  case VK_CONTROL: key_flags&=~KC_CTRL;
								  break;
		#ifdef VK_LMENU
		  case VK_LMENU:
		  case VK_RMENU:
		#endif
		  case VK_MENU: key_flags&=~KC_ALT;
							  break;
		}
		break;
	 case WM_KEYDOWN:
		switch (wparam) {
		#ifdef VL_LSHIFT
		  case VK_LSHIFT:
		  case VK_RSHIFT:
		#endif
		  case VK_SHIFT: key_flags|=KC_SHIFT;
							  break;
		#ifdef VL_LCONTROL
		  case VK_LCONTROL:
		  case VK_RCONTROL:
		#endif
		  case VK_CONTROL: key_flags|=KC_CTRL;
								  break;
		#ifdef VL_LMENU
		  case VK_LMENU:
		  case VK_RMENU:
		#endif
		  case VK_MENU: key_flags|=KC_ALT;
							  break;
		}
		break;
	 case WM_CHAR:
		{
		  #ifdef KF_ALTDOWN
		    #define ALTDOWN (((KF_ALTDOWN & HIWORD(lparam))!=0)*KC_ALT)
		  #else
		    #define ALTDOWN 0
		  #endif
		  FG_INT32 code=fg_GetWinKey(wparam,key_flags | ALTDOWN);
		  if (code) {
			 kbd_buffer[kb_write_ptr]=code;
			 kb_write_ptr=(kb_write_ptr+1) % 128;
		  }
		  return (0);
		}
	 case WM_DESTROY:
		exit_now=1;
		appState=1;
		PostQuitMessage(0);
		return(0);
  }
  rv=DefWindowProc(hwnd, msg, wparam, lparam);
  return (rv);
}
#endif

#ifdef USE_PM

MRESULT EXPENTRY FG_ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static FG_INT32 ncolors=NUM_COLORS;
  RECTL rcl;
  ULONG cmd;
  FILEDLG fd;
  HWND menu;
  int color_format;
  static HWND frame=0;
  MRESULT rv=0;
 
  switch (msg) {
    case WM_CREATE:
      if ((fg_hdive=StartDive(&DiveCaps)))
      { 
        fourcc=DiveCaps.fccColorEncoding;
        switch (fourcc) {
          case FOURCC_MONO:
          case FOURCC_GREY:  color_format=FG_CF_GREY8;
                             break; 
          case FOURCC_VGA:
          case FOURCC_LUT8:  color_format=FG_CF_LUT8;
                             break;
          case FOURCC_R565:  color_format=FG_CF_RGB565;
                             break;
          case FOURCC_R555:  color_format=FG_CF_RGB555;
                             break;
          case FOURCC_R664:  color_format=FG_CF_RGB664;
                             break;
          case FOURCC_RGB3:  color_format=FG_CF_RGB888;
                             break;
          case FOURCC_BGR3:  color_format=FG_CF_BGR888;
                             break;
          case FOURCC_RGB4:  color_format=FG_CF_RGBX8888;
                             break;
          case FOURCC_BGR4:  color_format=FG_CF_XBGR8888;
                             break;
          default:      fprintf(stderr,"Unsupported color format\n");
                        exit_now=1;
        }
        display_bitmaps[0]=fg_create_bitmap(scn_width,scn_height,color_format);
        DiveAllocImageBuffer(fg_hdive,&DiveBufferNumber,fourcc,scn_width,
           scn_height,scn_width,display_bitmaps[0]->lines[0]);
        fg_set_startup_palette(hwnd);
        if (!frame) frame=WinQueryWindow(hwnd,QW_PARENT);
        SetWindowSize(frame,scn_width,scn_height);
        WinSetVisibleRegionNotify(hwnd,TRUE);
        WinSendMsg(hwnd,WM_VRNENABLED,0,0);
        wait_sem=0;
      } else {
        WinSendMsg(hwnd,WM_CLOSE,0,0);
      }
      break;
    case WM_VRNENABLED:
      handle_WM_VRNENABLED(hwnd,fg_hdive,scn_width,scn_height);
      WinInvalidateRect(hwnd,NULL,FALSE);
      break;
    case WM_PAINT:
      handle_WM_PAINT(hwnd,fg_hdive,DiveBufferNumber);
      DosSleep(250/MAX_FRAME_RATE);
      return (rv);
      break;
    case WM_VRNDISABLED:
      handle_WM_VRNDISABLED(fg_hdive);
      break;
    case WM_COMMAND:
      menu=WinWindowFromID(fg_hwndFrame,FID_MENU);
      switch (cmd=SHORT1FROMMP(mp1)) 
      {
        case CMD_EXIT:
          WinSendMsg(hwnd,WM_CLOSE,0,0);
          break;
        case CMD_PALETTE_CHANGE:
          WinRealizePalette(hwnd, fg_hps, &ncolors);
          DiveSetSourcePalette(fg_hdive,0,NUM_COLORS,PaletteArray);
          DiveSetDestinationPalette(fg_hdive,0,NUM_COLORS,0);
          break;
        case CMD_VERIFY:
          cmd=MBID_NO;
          cmd=WinMessageBox(HWND_DESKTOP, hwnd,
               "Are you sure you want to exit FlightGear?",
               "Exit",0,
               MB_YESNO | MB_QUERY);
          rv=(MRESULT)cmd;
          break;
      /* add other commands later */
      }
      break;
    case WM_REALIZEPALETTE:
      DiveSetDestinationPalette(fg_hdive,0,NUM_COLORS,0);
      break;
    case WM_ACTIVATE:
      handle_WM_ACTIVATE(hwnd,SHORT1FROMMP(mp1),fg_hpal);
      return(rv);
      break;
    case WM_CHAR:
      {
        FG_INT32 code=fg_GetPMKey(mp1,mp2);
        if (code) {
          kbd_buffer[kb_write_ptr]=code;
          kb_write_ptr=(kb_write_ptr+1) % 128;
        }
        
      }
      break;
  }
  rv=WinDefWindowProc(hwnd,msg,mp1,mp2);
  return rv;
}



HDIVE StartDive(DIVE_CAPS *pDiveCaps)
{
  HDIVE DiveHandle=1;

  pDiveCaps->ulStructLen=sizeof(DIVE_CAPS);

  if (!(pDiveCaps->pFormatData=malloc(100*sizeof(FOURCC)))) {
    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,"Unable to allocate memory!",
      "Fatal DIVE Error",0,MB_OK | MB_INFORMATION);
    DiveHandle=0;
  }

  pDiveCaps->ulFormatLength=100;

  if ( DiveQueryCaps(pDiveCaps,DIVE_BUFFER_SCREEN) ) {
    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
      "Unable to query DIVE capabilities","Fatal DIVE Error",0,
      MB_OK | MB_INFORMATION);
    DiveHandle=0;
  }

  if ( pDiveCaps->ulDepth < 8 ) {
    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
      "This program requires at least 256 colors to run!","Fatal DIVE Error",
      0,MB_OK | MB_INFORMATION);
    DiveHandle=0;
  }

  if (DiveHandle) {
    DiveHandle=0;
    if (DiveOpen(&DiveHandle,FALSE,NULL)) {
      WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
        "Unable to open a DIVE Instance","Fatal DIVE Error",0,
        MB_OK | MB_INFORMATION);
        DiveHandle=0;
    }
  }

  return (DiveHandle);
}


ULONG EndDive(HDIVE DiveHandle, DIVE_CAPS *pDiveCaps)
{
  if (DiveHandle) {
    if (pDiveCaps->pFormatData) {
      free(pDiveCaps->pFormatData);
      pDiveCaps->pFormatData=NULL;
    }
    DiveClose(DiveHandle);
  }
  return (0);
}

ULONG DestroyDiveBuffer(HDIVE DiveHandle, ULONG BufferNumber, PBYTE *Lines)
{
  DiveFreeImageBuffer(DiveHandle,BufferNumber);
  free(*Lines);
  free(Lines);
  return (0);
}

VOID SetWindowSize(HWND hwndFrame,ULONG Width,ULONG Height)
{
  LONG BorderWidth    = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
  LONG BorderHeight   = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);
  LONG TitleBarHeight = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
  LONG MenuHeight     = (LONG)WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);
  LONG TotalWidth = Width+2*BorderWidth;
  LONG TotalHeight = Height+MenuHeight+TitleBarHeight+2*BorderHeight;
  WinSetWindowPos(hwndFrame,HWND_TOP, 
     (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - TotalWidth ) / 2,
     (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - TotalHeight ) / 2,
     TotalWidth,TotalHeight, SWP_MOVE|SWP_SIZE|SWP_SHOW|SWP_ACTIVATE);
}


VOID handle_WM_ACTIVATE(HWND hwndClient,SHORT WindowActive,HPAL hpal) 
{
  static FG_INT32 ncolors=NUM_COLORS;

  if (fg_hps) {
    if (WindowActive && hpal)  {
      GpiSelectPalette(fg_hps,hpal);
      WinRealizePalette(hwndClient, fg_hps, &ncolors);
      WinInvalidateRect(hwndClient, NULL, FALSE);
    } else {
      GpiSelectPalette(fg_hps,NULLHANDLE);
      WinRealizePalette(hwndClient, fg_hps, &ncolors);
    }
  }
}

VOID handle_WM_CLOSE(HWND hwndClient, HPAL hpal)
{
  handle_WM_ACTIVATE(hwndClient, 0, NULLHANDLE);
  if (hpal) GpiDeletePalette(hpal);
  if (fg_hps) GpiDestroyPS(fg_hps);
}

VOID handle_WM_VRNENABLED(HWND hwnd, HDIVE hDive, UINT xsize, UINT ysize)
{
  static SETUP_BLITTER SetupBlitter;
  RGNRECT            rgnCtl;
  SWP swp;
  POINTL pointl;
  HRGN hrgn;
  RECTL rcls[50];
  HWND hwndFrame;
  
  hwndFrame=WinQueryWindow(hwnd,QW_PARENT);
  if ( fg_hps && (hrgn=GpiCreateRegion(fg_hps,0L,NULL)) ) {
    WinQueryVisibleRegion(hwnd,hrgn);
    rgnCtl.ircStart=0;
    rgnCtl.crc=50;
    rgnCtl.ulDirection  = 1;
    if ( GpiQueryRegionRects ( fg_hps, hrgn, NULL, &rgnCtl, rcls) ) {
      WinQueryWindowPos (hwnd, &swp );
      pointl.x=swp.x;
      pointl.y=swp.y;
      WinMapWindowPoints ( hwndFrame, HWND_DESKTOP, &pointl, 1 );
      
      SetupBlitter.ulStructLen=sizeof(SetupBlitter);
      SetupBlitter.fInvert=FALSE;
      SetupBlitter.fccSrcColorFormat=fourcc;
      SetupBlitter.ulSrcWidth=xsize;
      SetupBlitter.ulSrcHeight=ysize;
      SetupBlitter.ulSrcPosX=0;
      SetupBlitter.ulSrcPosY=0;
      SetupBlitter.ulDitherType=1;

      SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
      SetupBlitter.ulDstWidth        = swp.cx;
      SetupBlitter.ulDstHeight       = swp.cy;
      SetupBlitter.lDstPosX          = 0;
      SetupBlitter.lDstPosY          = 0;
      SetupBlitter.lScreenPosX       = pointl.x;
      SetupBlitter.lScreenPosY       = pointl.y;
      SetupBlitter.ulNumDstRects     = rgnCtl.crcReturned;
      SetupBlitter.pVisDstRects      = rcls;
      DiveSetupBlitter (hDive, &SetupBlitter);
    } else {
      DiveSetupBlitter (hDive, NULL);
    } 
    GpiDestroyRegion(fg_hps,hrgn);
  }
}

VOID handle_WM_PAINT(HWND hwnd, HDIVE hDive, ULONG DiveBufferNumber)
{
  HPS hps;
  hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);
  DiveBlitImage(hDive, DiveBufferNumber, DIVE_BUFFER_SCREEN);
  WinEndPaint(hps);
}


VOID handle_WM_VRNDISABLED(HDIVE hDive)
{
  DiveSetupBlitter (hDive, 0);
}

#endif

FG_VOID *fgscreen_init(FGDRIVER_INIT_CALL *callstruct)
{
  static FGDRIVER_INIT_CALL fgscreen_init_struct=
                                    {sizeof(FGDRIVER_INIT), /* version id */
                                     0};         /* nothing to tell       */ 

  handle_fgtimer_event(1000/MAX_FRAME_RATE,fgscreen_event);
  handle_fgevent("exit",fgscreen_exit);
  return (&fgscreen_init_struct);
}

FG_VOID *fgscreen_event(FGEVENT_CALL *event)
{
  copy_display_bitmap_to_screen(0); 
  return (NULL);
}

FG_VOID *fgscreen_exit(FGEVENT_CALL *event)
{
  return (NULL);
}

/* $Log: fgscreen.c,v $
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
 */
