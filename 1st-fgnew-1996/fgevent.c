/* fgevent.c -- flight gear event manager
 *
 * Written by Curtis Olson
 * Started 4/23/96
 *
 * Copyright (C) 1996  Curtis L. Olson  - curt@sledge.mn.org
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
 *
 * $Id: fgevent.c,v 2.1 1996/10/31 18:05:22 korpela Exp korpela $
 * (Log is kept at end of this file)
 */


#include <stdlib.h>    /* for malloc() */
#include <string.h>    /* for strcpy(), strncpy() */
#include "fgdefs.h"
#include "fgevent.h"

#ifdef USE_DLL101
#include "dll.h"
#endif


/***********************************************************************
 * Private definitions and variables
 **********************************************************************/

typedef struct _FGEVENT {
    char event_name[MAX_FGEVENT_NAME];  /* ascii name of event */
    FG_EVENT_FN function;     /* pointer to function (called when event
                                       is generated */
    struct _FGEVENT *next;     /* pointer to next record (in case mulitple
                                       fuctions need to be called on a single 
                                       event */
} FGEVENT;

FGEVENT fgevent_table[FGEVENT_TABLE_SIZE];

int max_fgevent;


/***********************************************************************
 * Public functions for use by modules 
 **********************************************************************/


/* Create a new fgevent with the fgevent manager
 *
 * In:  character string fgevent name
 * Out:  a unique fgevent id (-1 on error)
 */
int get_fgevent(char *event_name) {
    int i, len;

    /* first make sure we haven't already created this event */
    i = 0;
    while ( i < max_fgevent ) {
	if (strcmp(fgevent_table[i].event_name, event_name) == 0) {
	    return(i);
	}

	i++;
    }

    if ( max_fgevent < FGEVENT_TABLE_SIZE ) {
	len = strlen(event_name);
	if ( len < MAX_FGEVENT_NAME ) {
	    strcpy(fgevent_table[max_fgevent].event_name, event_name);
	} else {
	    strncpy(fgevent_table[max_fgevent].event_name, event_name, 
		    MAX_FGEVENT_NAME);
	    fgevent_table[max_fgevent].event_name[MAX_FGEVENT_NAME - 1] = '\0';
	}
	fgevent_table[max_fgevent].function = NULL;
	fgevent_table[max_fgevent].next = NULL;
	
	max_fgevent++;

	return(max_fgevent - 1);
    } else {
	return(-1);
    }
}


/* Send out a fgevent
 *
 * In:  a unique fgevent id
 *      a pointer to some data structure
 * Out:  number of functions notified
 */
int send_fgevent(FGEVENT_CALL *event) {
    FGEVENT *event_ptr;
    int count = 0;

    event_ptr = fgevent_table[event->event_id].next;
    while ( event_ptr != NULL ) {
	event_ptr->function(event);
	event_ptr = event_ptr->next;
	count++;
    }

    return count;
}


/* Register as a handler of the specified event
 *
 * In:  event name, a pointer to the requesting function
 * Out:  success/failure
 */
int handle_fgevent(char *event_name, FG_EVENT_FN function) {
    FGEVENT *event_ptr, *prev_event_ptr;
    int event_id;
    
    /* find the requested event */
    event_id = get_fgevent(event_name);

    prev_event_ptr = &fgevent_table[event_id];
    event_ptr = fgevent_table[event_id].next;

    while ( (event_ptr != NULL) && (event_ptr->function != null_fgevent ) ) {
	prev_event_ptr = event_ptr;
	event_ptr = event_ptr->next;
    }

    if ( (event_ptr == NULL) ) {
      prev_event_ptr->next = malloc(sizeof(FGEVENT));
      event_ptr = prev_event_ptr->next;
      event_ptr->function = function;
      event_ptr->next = NULL;
    } else {
      event_ptr->function = function;
    }

    return(event_id);
}

/* Unregister as a handler of the specified event
 *
 * In:  event name, a pointer to the function to deregister;
 * Out: success/failure
 */
int unhandle_fgevent(char *event_name, FG_EVENT_FN function) {
  FGEVENT *event_ptr, *prev_event_ptr;
  int event_id;

  /* find the requested event */
  event_id = get_fgevent(event_name);

  prev_event_ptr = &fgevent_table[event_id];
  event_ptr = fgevent_table[event_id].next;

  while ( (event_ptr != NULL) && (event_ptr->function != function ) ) {
    prev_event_ptr = event_ptr;
    event_ptr = event_ptr->next;
  }

  if (event_ptr->function == function) {
    event_ptr->function=null_fgevent;
    return(1);
  } else {
    return(0);
  } 
}

/***********************************************************************
 * Private functions for use by kernel
 **********************************************************************/


/* Initialize the fgevent table
 *
 * In:  none
 * Out:  success/failure
 */
int init_fgevent_table() {
    max_fgevent = 0;

    return(0);
}

/* A null function to use when deregistering event handlers */
FG_VOID *null_fgevent(FGEVENT_CALL *event) {
  return (NULL);
}

void fgevent_export()
{
#ifdef USE_DLL101
  static char *symbols[]={"_get_fgevent","_send_fgevent","_handle_fgevent",
                          "_unhandle_fgevent",NULL};
  void *functions[]={get_fgevent,send_fgevent,handle_fgevent,
                           unhandle_fgevent,NULL};
  int i=0;

  while (symbols[i]) {
    DLL_AddSymbol(symbols[i],functions[i]);
    i++;
  }
#endif
}


/* $Log: fgevent.c,v $
 * Revision 2.1  1996/10/31  18:05:22  korpela
 * Added Scalable Fonts, a clock driver, fixed some bugs, started
 * Win32 support.
 *
 * Revision 2.0  1996/10/09  00:54:17  korpela
 * Screen works now.
 *
 * Revision 1.4  1996/09/12  18:38:58  korpela
 * *** empty log message ***
 *
 * Revision 1.3  1996/05/16  23:14:51  korpela
 * Added DJGPP DLL support
 *
 * Revision 1.1  1996/05/05  22:01:26  korpela
 * Initial revision
 *
 * Revision 1.1  1996/04/30 21:55:29  clolson
 * Initial revision.
 *
 */
