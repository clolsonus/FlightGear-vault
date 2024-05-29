/* fgkbd.c -- This is a prototype FlightGear keyboard driver             
 *  It will pause the simulation when the main program reports that
 *  control-p has been pressed                                      
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
 * $Id: fgkbd.c,v 2.1 1996/10/31 18:12:04 korpela Exp korpela $ 
 */

#include <stdio.h>
#include "fgdefs.h"
#include "fgtypes.h"
#include "fgdriver.h"
#include "fgevent.h"
#include "fgkbd.h"
#include "fgtimer.h"
#include "fgscreen.h"
#include "fgmain.h"
#include "fgkeydef.h"

#ifdef USE_ALLEGRO
#include "allegro.h"
#endif

#if defined(USE_PM) || defined(USE_WIN32)
volatile FG_INT kbd_buffer[128];
volatile FG_INT kb_read_ptr=0,kb_write_ptr=0;
#endif

#if defined(USE_X11)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

XEvent E;

#endif

int fg_define_key(char *event_name);
int fg_keymap_changed=0;

FG_INT nkeys;

static FGEVENT_CALL key_event;

typedef struct _FGKEY {
  FG_CHAR event_name[MAX_FGEVENT_NAME];
  FG_INT  key_code;
  FG_INT  event_code;
} FGKEY;

FGKEY keys[NUM_KEY_EVENTS];

#if defined(USE_FB)
#include <curses.h>
#include <fcntl.h>
WINDOW *win;
#ifndef nodelay
int stioflags;
#define nodelay(x,y) ((y) ? (fcntl(0,F_GETFL,&stioflags), \
                             fcntl(0, F_SETFL, FNDELAY )) \
	                  :  fcntl(0, F_SETFL,stioflags))
#endif
#endif


FG_VOID *fgkbd_init(FGDRIVER_INIT_CALL *callstruct)
{
  int nfields;
  FILE *keymap;
  char buffer[255];

  static FGDRIVER_INIT_CALL fgkbd_init_struct=
                                    {sizeof(FGDRIVER_INIT), /* version id */
                                     0};         /* nothing to tell       */ 
  if (callstruct->msg == FG_INIT) {
  #ifdef USE_ALLEGRO
    if (install_keyboard()) {
       fprintf(stdout,"Error Installing Keyboard Handler\n");
    }
  #elif defined(USE_FB)
    { 
      if (win=initscr()) {
        raw();
        noecho();
        nodelay(win,TRUE);
      }
    }
  #endif
    handle_fgevent("master",fgkbd_event);
    if ((keymap=fopen("fgkeymap.ini","r"))) {
      while (fgets(buffer,255,keymap) && (nkeys<NUM_KEY_EVENTS)) {
        nfields=sscanf(buffer,"%s 0x%x",(keys[nkeys].event_name),
                         &(keys[nkeys].key_code));
        keys[nkeys].event_code=get_fgevent(keys[nkeys].event_name);
        nkeys++;
      }
      fclose(keymap);
    } 
    handle_fgkey_event("exit",fgkbd_exit);
  }
  return (&fgkbd_init_struct);
}


FG_VOID *fgkbd_event(FGEVENT_CALL *event)
{
  FG_INT code,i=0;
#ifdef USE_ALLEGRO
  if (keypressed()) {
    code=readkey() & 0xff;
    if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
      code^=FG_SHIFT_FLAG;
    if (key[KEY_CONTROL])
      code^=FG_CTRL_FLAG;
    if (key[KEY_ALT])
      code^=FG_ALT_FLAG;
#elif defined(USE_FB)
  if ((code=wgetch(win)) && (code != -1)) {
#elif defined(USE_PM) || defined(USE_WIN32)
  if (kb_read_ptr!=kb_write_ptr) {
    code=kbd_buffer[kb_read_ptr];
    kb_read_ptr=(kb_read_ptr+1) % 128;
#elif defined(USE_X11)
  if(XCheckWindowEvent(Dsp,Wnd[0],KeyPressMask|KeyReleaseMask,&E)) {
    code=fg_GetXKey();
#endif
    if (code) {
      i=0;
      while ((keys[i].key_code != code) && (++i<nkeys)); 
      if (i<nkeys) {
        key_event.event_id=keys[i].event_code;
        key_event.event_data=NULL;
        send_fgevent(&key_event);
      }
    }
  }
  return (NULL);
}

FG_VOID *fgkbd_exit(FGEVENT_CALL *event)
{
  int i;
  FILE *keymap;

#ifdef USE_FB
  nodelay(win,FALSE);
  noraw();
  echo();
#endif
  if (fg_keymap_changed && (keymap=fopen("fgkeymap.ini","w"))) {
    for (i=0;i<nkeys;i++) {
      fprintf(keymap,"%s 0x%x\n",keys[i].event_name,keys[i].key_code);
    }
    fclose(keymap);
  }
  exit_now=1;
  return(NULL);
}

#if defined(USE_X11) 
int fg_GetXKey() 
{
  unsigned int J,state;
  int mod=0;
  
  J=XLookupKeysym((XKeyEvent *)&E,0);
  state=((XKeyEvent *)&E)->state;
  if (E.type==KeyPress && ((J<XK_Shift_L) || (J>XK_Hyper_R)))  
  {
    if (state & ShiftMask) 
      J^= FG_SHIFT_FLAG;
    if (state & LockMask) 
      J^= FG_SHIFT_FLAG; 
    if (state & ControlMask) 
      J^= FG_CTRL_FLAG;
    if (state & (Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)) 
      J^= FG_ALT_FLAG;
    return (J);
  } else return(0);
}
#endif

#if defined(USE_PM) 
int fg_GetPMKey(MPARAM mp1, MPARAM mp2)
{
  short flags=SHORT1FROMMP(mp1);
  int key=SHORT1FROMMP(mp2);
  int v_key=SHORT2FROMMP(mp2);

  if (!(flags & KC_KEYUP)) {
    if (!(flags & KC_VIRTUALKEY)) {
      if (flags & KC_SHIFT)
        key^=FG_SHIFT_FLAG;
      if (flags & KC_CTRL)
        key^=FG_CTRL_FLAG;
      if (flags & KC_ALT)
        key^=FG_ALT_FLAG;
    } else {
      switch(v_key) {
        case VK_BUTTON1:
        case VK_BUTTON2:
        case VK_BUTTON3:
        case VK_SHIFT:
        case VK_CTRL:
        case VK_ALT:
        case VK_ALTGRAF:
        case VK_CAPSLOCK:
        case VK_ENDDRAG:
                          key=0;
                          break;
        default:	  key=0xffff-v_key;
                          if (flags & KC_SHIFT)
                            key^=FG_SHIFT_FLAG;
                          if (flags & KC_CTRL)
                            key^=FG_CTRL_FLAG;
                          if (flags & KC_ALT)
                            key^=FG_ALT_FLAG;

      }
    }
  } else {
    key=0;
  }
  return(key);
}
#endif

#if defined(USE_WIN32) 
int fg_GetWinKey(WPARAM key, int flags)
{

  switch(key) {
    case VK_LBUTTON:
    case VK_RBUTTON:
    case VK_MBUTTON:
    #ifdef VK_LSHIFT
    case VK_LSHIFT:
    case VK_RSHIFT:
    #endif
    case VK_SHIFT:
    case VK_CONTROL:
    #ifdef VK_LCONTROL
    case VK_LCONTROL:
    case VK_RCONTROL:
    #endif
    case VK_MENU:
    #ifdef VL_LMENU
    case VK_LMENU:
    case VK_RMENU:
    #endif
    key=0;
    break;
    default:	  if (flags & KC_SHIFT)
                    key^=FG_SHIFT_FLAG;
                  if (flags & KC_CTRL)
                    key^=FG_CTRL_FLAG;
                  if (flags & KC_ALT)
                    key^=FG_ALT_FLAG;
  }
  return(key);
}
#endif

int fg_define_key( char *event_name )
{
  FG_INT code;

  printf("Press a key for the %s event\n",event_name);
#ifdef USE_ALLEGRO
  while  (!keypressed() && !(code=readkey() & 0xff)) ;
  if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
    code^=FG_SHIFT_FLAG;
  if (key[KEY_CONTROL])
    code^=FG_CTRL_FLAG;
  if (key[KEY_ALT])
    code^=FG_ALT_FLAG;
#elif defined(USE_FB)
  while (!((code=wgetch(win)) && (code != -1))) ;
#elif defined(USE_PM) || defined(USE_WIN32)
  code=0;
  while (code==0) {
    #ifdef USE_WIN32
      FG_message_loop(NULL);
    #endif
    if (kb_read_ptr!=kb_write_ptr) { 
      code=kbd_buffer[kb_read_ptr];
      kb_read_ptr=(kb_read_ptr+1)%128;
    }
  }
#elif defined(USE_X11)
  while (!XCheckWindowEvent(Dsp,Wnd[0],KeyPressMask|KeyReleaseMask,&E) || 
    !(code=fg_GetXKey()))  ;
#endif
  printf("%s 0x%x\n",event_name,code);
  return (code);
}

int handle_fgkey_event(char *event_name,FG_EVENT_FN function)
{
  int i=0,code=0;
  
  while (i<nkeys) {
     if (strcmp(keys[i].event_name,event_name) == 0) {
        return (handle_fgevent(event_name,function));
     }
     i++;
  } 

  while (!code) {
    printf("Event (%s) has no mapped key\n",event_name);
    code=fg_define_key(event_name);
    for (i=0;i<nkeys;i++) {
      if (code==keys[i].key_code) {
        printf("Key 0x%x is already mapped to event %s.\n",
                code,keys[i].event_name);
        code=0;
      }
    }
  }
  if (nkeys<NUM_KEY_EVENTS) {
    keys[nkeys].event_code=handle_fgevent(event_name,function);
    keys[nkeys].key_code=code;
    strncpy(keys[nkeys].event_name,event_name,MAX_FGEVENT_NAME);
    fg_keymap_changed=1;
    return(keys[nkeys++].event_code);
  } else {
    printf("Out of keymap slots!\n");
    return(0);
  }
}


      
/* $Log $
 * */
