#include "HEADERS.h"
#include "sphigslocal.h"
#include <string.h> 

static element *element_ptr;
static int      element_ptr_index;

static substruct_bitstring possible_no_longer_child = NULL;

int ID_of_open_struct;
structure *OPENSTRUCT;



element *SPH__currentElementDirectAccess ()
{
   return element_ptr;
}




/** SPH_cleanup_and_kill_element
Frees up any malloc'd areas associated with an element.
**/

static void SPH_cleanup_and_kill_element (element *elptr)
{
   switch (elptr->type) {
      
    case ELTYP__POLYHEDRON:
      SPH__freePolyhedron (elptr->data.poly);
      break;

    case ELTYP__POLYMARKER:
    case ELTYP__POLYLINE:
    case ELTYP__FILL_AREA:
      free (elptr->data.points);
      break;

    case ELTYP__EXECUTE_STRUCTURE:
      SPH__structureTable[elptr->data.value].refcount--;
      SetBit (possible_no_longer_child, elptr->data.value);
      break;
      
   }

   free (elptr);
}




/** SPH_inquireElptr
**/
int
SPH_inquireElementPointer (void)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   return element_ptr_index;
}


/** SPH_init_structure_table
All structures exist initially; they're simply empty.
**/
void
SPH__init_structure_table (void)
{
   register i;

   bzero (SPH__structureTable, sizeof(structure)*(MAX_STRUCTURE_ID+1));
   for (i=0; i<=MAX_STRUCTURE_ID; i++) {
      ClearBitstring (&(SPH__structureTable[i].child_list));
   }
}




/** INTERNAL USE ONLY: SPH__insertElement
Inserts given element in the currently open structure so that it
   follows the currently active element.
Because internal-use only, ASSUMES THERE IS AN OPEN STRUCTURE.
**/

void
SPH__insertElement (element *baby)
{
   if (element_ptr == NULL) {
      /* BEING INSERTED AT VERY BEGINNING OF STRUCTURE */
      baby->next = OPENSTRUCT->first_element;
      OPENSTRUCT->first_element = baby;
      baby->previous = NULL;
   }
   else {
      /* BEING INSERTED AFTER AT LEAST ONE EXTANT ELEMENT */
      baby->next = element_ptr->next;
      element_ptr->next = baby;
      baby->previous = element_ptr;
   }

   OPENSTRUCT->element_count++;
   if (baby->next == NULL)
      OPENSTRUCT->last_element = baby;
   else
      baby->next->previous = baby;
   element_ptr = baby;
   element_ptr_index++;
}


/** SPH_openStructure
**/
void
SPH_openStructure (structID)
int structID;
{
   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_structure_id;

   ID_of_open_struct = structID;
   OPENSTRUCT = &(SPH__structureTable[structID]);
   element_ptr_index = OPENSTRUCT->element_count;
   element_ptr = OPENSTRUCT->last_element;
   SPH__structureCurrentlyOpen = TRUE;

   if (NULL == possible_no_longer_child)
      possible_no_longer_child = (unsigned char*) malloc (BYTES_PER_BITSTRING);

   ClearBitstring (&possible_no_longer_child);
}
   

static substruct_bitstring BB = NULL;


/** SPH_closeStructure
**/
void
SPH_closeStructure ()
{
   SPH_check_system_state;
   SPH_check_open_structure;

   /* IF AN INVOCATION WAS DESTROYED BY THE EDITING.... */
   if ( ! BitstringIsClear(possible_no_longer_child)) {
      register element *elptr;


      /* RECALC CHILD_LIST */
      ClearBitstring (&(OPENSTRUCT->child_list));
      elptr = OPENSTRUCT->first_element;
      while (elptr) {
	      if (elptr->type == ELTYP__EXECUTE_STRUCTURE)
	         SetBit (OPENSTRUCT->child_list, elptr->data.value);
         elptr = elptr->next;
      }
      
      /* TEST FOR OBSOLESCENCE OF DEPENDENCIES. */
      ClearBitstring (&BB);  /* forces a malloc the 1st time around */
      AndBitstrings (BB, possible_no_longer_child, OPENSTRUCT->child_list);
      /* NOW: BB is a list of all structures that are definitely
       *   children AND that were suspected of having been lost by
       *   a deletion of an invocation.
       * IF BB is equal to the list of all structures suspected of
       *   having been lost, then it must be true that NO
       *   children were lost due to editing in this session.
       * IF BB is NOT equal to...,  then it must be true that
       *   at least one child was lost, and some views' descendent-lists
       *   may now be obsolete!
       */
      if ( ! BitstringsAreEqual (BB, possible_no_longer_child)) 
         /* All views that own S now have obsolete descendent lists! */
         VIEWOPT__afterChildLoss (ID_of_open_struct);
   }

   SPH__structureCurrentlyOpen = FALSE;
   SPH__refresh_structure_close (ID_of_open_struct);
}



/** SPH_setElptr
Special case: 0 means set elptr to even before the very first element.
              1 means set elptr to very first element.
**/
void
SPH_setElementPointer (index)
int index;
{
   register i;

   SPH_check_system_state;
   SPH_check_open_structure;

   if (index == 0)
      element_ptr = NULL;
   else
      if ((index > OPENSTRUCT->element_count) || (index < 0))
	 SPH__error (ERR_BAD_SET_ELPTR);
      else {
	 element_ptr = OPENSTRUCT->first_element;
	 for (i=1; i<index; i++)
	    element_ptr = element_ptr->next;
      }

   element_ptr_index = index;
}
      


/** SPH_offsetElptr
Its internal call to SPH_setElptr does the verification work on the index.
**/
void
SPH_offsetElementPointer (delta)
int delta;
{
   SPH_check_system_state;
   SPH_check_open_structure;

   element_ptr_index += delta;
   SPH_setElementPointer (element_ptr_index);
}


/** SPH_moveElptrToLabel
Question: should I include the current element in the search?  Or does
the search begin with the first element after the current one?

This version does NOT include the current element in the search.
This version gives fatal error if label not found.
**/
void
SPH_moveElementPointerToLabel (lab)
int lab;
{
   SPH_check_system_state;
   SPH_check_open_structure;

   while (1) {
      /* ADVANCE ELEMENT POINTER */
      element_ptr_index++;
      if (element_ptr_index == 1)
	 element_ptr = OPENSTRUCT->first_element;
      else
	 element_ptr = element_ptr->next;

      if ( ! element_ptr)
	 /* REACHED END: NOT FOUND: FATAL ERROR */
	 SPH__error (ERR_LABEL_NOT_FOUND);
      
      if (  (element_ptr->type == ELTYP__LABEL)
	 && (element_ptr->data.value == lab) )
	    /* FOUND */
	    break;
   }
}




/** SPH_deleteEl
Element pointer is left pointing to the element (if any) just before
   the one to be killed.
**/
void
SPH_deleteElement (void)
{
   element *el_to_die;

   SPH_check_system_state;
   SPH_check_open_structure;
   if (element_ptr == NULL)
      SPH__error (ERR_NO_ELEMENT_TO_DELETE);

   /* FIRST, HANDLE ITS RELATIONSHIP TO ITS PREVIOUS ELEMENTS */
   if (element_ptr->previous == NULL)
      OPENSTRUCT->first_element = element_ptr->next;
   else
      element_ptr->previous->next = element_ptr->next;
   /* THEN, HANDLE ITS RELATIONSHIP TO ITS NEXT ELEMENTS */
   if (element_ptr->next == NULL)
      OPENSTRUCT->last_element = element_ptr->previous;
   else
      element_ptr->next->previous = element_ptr->previous;

   /* UPDATE ELEMENT-COUNT AND ELEMENT-POINTER */
   OPENSTRUCT->element_count--;
   el_to_die = element_ptr;
   element_ptr = el_to_die->previous;
   element_ptr_index--;


   SPH_cleanup_and_kill_element (el_to_die);
}






/** SPH_deleteElsInRange
**/
void
SPH_deleteElementsInRange (first_index, second_index)
int first_index, second_index;
{
   element *first_el_to_die, *last_el_to_die;
#define number_to_be_killed    (second_index - first_index + 1)

   SPH_check_system_state;
   SPH_check_open_structure;
   
   SPH_setElementPointer (first_index);
   first_el_to_die = element_ptr;
   SPH_setElementPointer (second_index);
   last_el_to_die = element_ptr;

   SPH_check_elindex_range;

   /* FIRST, HANDLE ITS RELATIONSHIP TO ITS PREVIOUS ELEMENTS */
   if (first_el_to_die->previous == NULL)
      OPENSTRUCT->first_element = last_el_to_die->next;
   else
      first_el_to_die->previous->next = last_el_to_die->next;

   /* THEN, HANDLE ITS RELATIONSHIP TO ITS NEXT ELEMENTS */
   if (last_el_to_die->next == NULL)
      OPENSTRUCT->last_element = first_el_to_die->previous;
   else
      last_el_to_die->next->previous = first_el_to_die->previous;

   /* UPDATE ELEMENT-COUNT AND ELEMENT-POINTER */
   OPENSTRUCT->element_count -= number_to_be_killed;
   element_ptr = first_el_to_die->previous;
   element_ptr_index = first_index - 1;

   /* KILL AND CLEANUP THE ELEMENTS */
   do {
      SPH_cleanup_and_kill_element (first_el_to_die);
      if (first_el_to_die == last_el_to_die)
	 break;
      else
	 first_el_to_die = first_el_to_die->next;
   } while (1);
}



/** SPH_deleteElsBetweenLabels
**/
void
SPH_deleteElementsBetweenLabels (lab1, lab2)
int lab1, lab2;
{
   int first_index, second_index;

   SPH_check_system_state;
   SPH_check_open_structure;

   SPH_moveElementPointerToLabel (lab1);
   first_index = element_ptr_index;
   SPH_moveElementPointerToLabel (lab2);
   second_index = element_ptr_index;
   if (first_index == (second_index-1))
      /* do nothing: nothing lies between the label elements */;
   else
      SPH_deleteElementsInRange (first_index+1, second_index-1);
}

