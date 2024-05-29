/* fgload.c -- flight gear driver loader 
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
 * $Id: fgload.c,v 2.2 1996/11/24 23:47:15 korpela Exp korpela $
 */

#include <stdio.h>
#include <string.h>
#include <memory.h> 

#include "fgdefs.h"
#include "fgdriver.h"
#include "fgload.h"
/************************ GLOBAL VARIABLES **************************/

FG_INT32 n_dyn_drivers;

/************************ SPARC & LINUX DEFINES**********************/
#if defined(USE_UNIX_LDSO)
#include <dlfcn.h>

struct _DYN_DRIVER_FILES {
   void *handle;
   FG_INT32  reference;
   char name[32];
} dyn_driver_files[MAX_DYN_DRIVERS]; 

   
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY
#endif


#endif

/************************ OS/2 DEFINES ******************************/
#ifdef USE_OS2_DLL

struct _DYN_DRIVER_FILES {
   HMODULE handle;
   FG_INT32     reference;
   char    name[32];
} dyn_driver_files[MAX_DYN_DRIVERS]; 

#endif
/************************ WIN32 DEFINES *****************************/
#ifdef USE_WIN32_DLL

struct _DYN_DRIVER_FILES {
   HINSTANCE handle;
   FG_INT32       reference;
   char      name[32];
} dyn_driver_files[MAX_DYN_DRIVERS];

#endif
/************************ DJGPP DEFINES *****************************/

#ifdef USE_DLL101
#include "dll.h"

struct _DYN_DRIVER_FILES {
   FG_INT32     reference;
   char    name[32];  
} dyn_driver_files[MAX_DYN_DRIVERS];

#endif

/************************* END DEFINES ******************************/

/* return a pointer to the function <driver>_init    *
 * in the file <filename>[.dll,.so,...]              */

FG_DRIVER_FN fg_load_driver(char *filename,char *driver)
{

/* local variables shared by all OS version */
  FG_DRIVER_FN init=NULL;
  char path[255]={FG_DRIVER_PATH};

/************************ SPARC & LINUX ROUTINE**********************/
#if defined(USE_UNIX_LDSO)
  {
    void *handle=NULL;
    FG_INT32 i=0;

    /* is this file loaded yet?   */
    while ((i<n_dyn_drivers) && (!handle)) {
      if (!strncmp(filename,dyn_driver_files[i].name,32)) { 
        handle=dyn_driver_files[i].handle;
	dyn_driver_files[i].reference++;
      }
      i++;
    }
    
    /* if not, load it */
     
    if (!handle) {
      /* set the path to the driver */
      strncat(path,filename,254-strlen(FG_DRIVER_PATH));
      strncat(path,".so",254-strlen(path));

      /* find the first free slot */
      i=0;
      while ((i<n_dyn_drivers) && (!dyn_driver_files[i].reference)) i++;

      /* load the driver */
      if ((i==n_dyn_drivers) && (n_dyn_drivers<MAX_DYN_DRIVERS)) {
        n_dyn_drivers++;
      }

      if (i!=MAX_DYN_DRIVERS) {
        if ((handle=dlopen(path, RTLD_NOW))) {
          dyn_driver_files[i].handle=handle;
          strncpy(dyn_driver_files[i].name,filename,32);
          dyn_driver_files[i].reference++;
        }
      }
    }

    /* if we've got a handle return a pointer to the driver_init function */

    if (handle) {
      strncpy(path,driver,254);
      strncat(path,"_init",254-strlen(path));
      init=(FG_DRIVER_FN)dlsym(handle,path);
      return (init);
    } else {
      return (NULL);
    }
  }
#endif

/************************ OS/2 ROUTINE ******************************/
#ifdef USE_OS2_DLL
  {
    HMODULE handle=NULLHANDLE;
    char achFailName[128]={""};
    FG_INT32 i=0;

    /* is this file loaded yet?   */
    while ((i<n_dyn_drivers) && (!handle)) {
      if (!strncmp(filename,dyn_driver_files[i].name,32)) { 
        handle=dyn_driver_files[i].handle;
        dyn_driver_files[i].reference++;
      }
      i++;
    }

    /* if not, load it */
    if (!handle) {
      strncat(path,filename,254-strlen(FG_DRIVER_PATH));
 
      /* find the first free slot */
      i=0;
      while ((i<n_dyn_drivers) && (!dyn_driver_files[i].reference)) i++;


      /* load the driver */ 
      if ((i==n_dyn_drivers) && (n_dyn_drivers<MAX_DYN_DRIVERS)) {
        n_dyn_drivers++;
      }

      if (i!=MAX_DYN_DRIVERS) {
        if (!(DosLoadModule(achFailName,128,path,&handle))) {
          dyn_driver_files[i].handle=handle;
          strncpy(dyn_driver_files[i].name,filename,32);
          dyn_driver_files[i].reference++;
        }
      }
    }

    if (handle) {
      strncpy(path,driver,254);
      strncat(path,"_init",254-strlen(path));
      DosQueryProcAddr(handle,0L,path,(PFN *)&init);
      return (init);
    } else {
      return (NULL);
    }
  }
#endif

/************************ WIN32 ROUTINE ******************************/
#ifdef USE_WIN32_DLL
  {
    HINSTANCE handle=NULL;
    FG_INT32 i=0;

    /* is this file loaded yet?   */
    while ((i<n_dyn_drivers) && (!handle)) {
      if (!strncmp(filename,dyn_driver_files[i].name,32)) { 
        handle=dyn_driver_files[i].handle;
        dyn_driver_files[i].reference++;
      }
      i++;
    }

    /* if not, load it */
    if (!handle) {
      strncat(path,filename,254-strlen(FG_DRIVER_PATH));
 
      /* find the first free slot */
      i=0;
      while ((i<n_dyn_drivers) && (!dyn_driver_files[i].reference)) i++;


      /* load the driver */ 
      if ((i==n_dyn_drivers) && (n_dyn_drivers<MAX_DYN_DRIVERS)) {
        n_dyn_drivers++;
      }

      if (i!=MAX_DYN_DRIVERS) {
        if (!(handle=LoadLibrary(path))) {
          dyn_driver_files[i].handle=handle;
          strncpy(dyn_driver_files[i].name,filename,32);
          dyn_driver_files[i].reference++;
        }
      }
    }

    if (handle) {
      strncpy(path,driver,254);
      strncat(path,"_init",254-strlen(path));
      init=GetProcAddress(handle,path);
      return (init);
    } else {
      return (NULL);
    }
  }
#endif

/************************ DJGPP ROUTINE *****************************/

#ifdef USE_DLL101
  {
    FG_INT32 success=0,i=0;
    /* set the path to the driver */
    strncat(path,filename,254-strlen(FG_DRIVER_PATH));
    strncat(path,".dll",254-strlen(path));

    /* is this file loaded yet?   */
    while ((i<n_dyn_drivers) && (!success)) {
      if (!strncmp(filename,dyn_driver_files[i].name,32)) { 
        dyn_driver_files[i].reference++;
        success=1;
      } 
      i++;
    }

    /* if not, load it */
    if (!success) {

      /* find the first free slot */
      i=0;
      while ((i<n_dyn_drivers) && (!dyn_driver_files[i].reference)) i++;
    
      /* load the driver */
      if ((i==n_dyn_drivers) && (n_dyn_drivers<MAX_DYN_DRIVERS)) {
        n_dyn_drivers++;
      }

      if (i!=MAX_DYN_DRIVERS) {
        if ((success=DLL_Load(path))) {
          strncpy(dyn_driver_files[i].name,filename,32);
          dyn_driver_files[i].reference++;
        }
      }
    }

    /* if it loaded return a pointer to the driver_init function */
    if (success) {
      strcpy(path,"_");
      strncat(path,driver,254-strlen(path));
      strncat(path,"_init",254-strlen(path));
      init=(FG_DRIVER_FN)DLL_LookupSymbol(path);
      return (init);
    } else {
      return (NULL);
    }
  }
#endif

}

FG_INT32 fg_unload_driver(char *filename)
/* unload the module pointed to by filename                     */
/* returns 0 if successful.  non-zero if module is still in use */
{
  FG_INT32 i=0,success=0;

  /* find the driver entry for this module */
  while ((i<n_dyn_drivers) && (!success)) {
    if (!strncmp(filename,dyn_driver_files[i].name,32)) { 
      dyn_driver_files[i].reference--;
      success=1;
    } else i++;
  }

  /* Is the module unused? */
  if (i<n_dyn_drivers) {
    if (!dyn_driver_files[i].reference) {

#if  defined(USE_UNIX_LDSO)
      dlclose(dyn_driver_files[i].handle);
#elif defined(USE_OS2_DLL)
      DosFreeModule(dyn_driver_files[i].handle);
#elif defined(USE_DLL101)
      DLL_UnLoad(dyn_driver_files[i].name);
#endif
      memset(&(dyn_driver_files[i]),0,sizeof(struct _DYN_DRIVER_FILES));
    }  
    return(dyn_driver_files[i].reference);
  } else {
    return(0);
  }
}

void fgload_export() 
{
}

  
/* $Log: fgload.c,v $
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
 * Revision 1.6  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.5  1996/05/05  22:01:26  korpela
 * minor changes
 * */
