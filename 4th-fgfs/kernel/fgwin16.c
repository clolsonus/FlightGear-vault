/* fgwin16.c -- Win 16 (UGH) support routines for memory allocation, etc.
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
 * $Id: fgwin16.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#include <windows.h>
#include <stdlib.h>
#include <dos.h>
#include "fgwin16.h"

typedef struct _WIN_ALLOCS {
	HANDLE handle;
	void _huge *ptr;
	DWORD remaining;
} WIN_ALLOCS;

typedef struct _FG_ALLOCS {
	void _huge *ptr;
	DWORD size;
	int flags;
}  FG_ALLOCS;

#define NUM_WIN_ALLOCS 16
#define NUM_FG_ALLOCS 128
#define WIN_ALLOC_SIZE 524288L
static WIN_ALLOCS win_allocs[NUM_WIN_ALLOCS];
static FG_ALLOCS fg_allocs[NUM_WIN_ALLOCS][NUM_FG_ALLOCS];
static int last_alloc=-1;

void WinFree(void *ptr)
{
  int i,j;
  for (i=0;i<NUM_WIN_ALLOCS;i++) {
	 for (j=0;j<NUM_FG_ALLOCS;j++) {
		if (fg_allocs[i][j].ptr==ptr) {
		  fg_allocs[i][j].flags=0;
		}
	 }
  }
}

void WinAllocExit(void)
{
  int i;
  for (i=0;i<NUM_WIN_ALLOCS;i++) {
	 if (win_allocs[i].handle) {
		GlobalUnlock(win_allocs[i].handle);
		GlobalFree(win_allocs[i].handle);
	 }
  }
}

void *WinAlloc(DWORD size)
{
  int i=0,j=0,found=0;
  void _huge *retval;
  size=(size+32);
  size=(size/32)*32;
  if (last_alloc==-1) {
	 if (atexit(WinAllocExit))
		exit(1);
  }
  while ((i<NUM_WIN_ALLOCS) &&
			(win_allocs[i].remaining < size) &&
			(win_allocs[i].ptr))
	 i++;
  if (i<NUM_WIN_ALLOCS)
  {
	 if (!win_allocs[i].ptr) {
		if ((win_allocs[i].handle=
					GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,WIN_ALLOC_SIZE)) &&
			 (win_allocs[i].ptr=GlobalLock(win_allocs[i].handle)))
		{
		  win_allocs[i].remaining=WIN_ALLOC_SIZE;
		}
	 }
	 do {
		while ((j<NUM_FG_ALLOCS) && (fg_allocs[i][j].flags)) j++;
		if (j<NUM_FG_ALLOCS) {
		  if (!fg_allocs[i][j].size) {
			 fg_allocs[i][j].size=size;
			 fg_allocs[i][j].ptr=(((char _huge *)win_allocs[i].ptr)+
											  (WIN_ALLOC_SIZE-win_allocs[i].remaining));
			 if (FP_OFF((char _huge *)fg_allocs[i][j].ptr+size)<
					FP_OFF(fg_allocs[i][j].ptr)) {
				fg_allocs[i][j].ptr=MK_FP(
											 FP_SEG((char _huge *)fg_allocs[i][j].ptr
															 +0xffffL),
											 0);
				win_allocs[i].remaining=(WIN_ALLOC_SIZE-
										((char _huge *)fg_allocs[i][j].ptr-
										 (char _huge *)win_allocs[i].ptr));
			 }
			 win_allocs[i].remaining-=size;
			 fg_allocs[i][j].flags=1;
			 retval=fg_allocs[i][j].ptr;
			 found=1;
		  } else {
			 if (fg_allocs[i][j].size<size) {
				found=1;
				fg_allocs[i][j].flags=1;
				retval=fg_allocs[i][j].ptr;
			 } else {
				j++;
			 }
		  }
		} else {
		  found=1;
		  retval=NULL;
		}
	 } while (!found);
  } else {
	 retval=NULL;
  }
  last_alloc=0;
  return (retval);
}







/* $Log: fgwin16.c,v $
 * Revision 2.2  1996/11/24  23:47:15  korpela
 * Added WIN16 support.  Changed public_vars to a struct.
 * Many minor changes.
 *
 */
