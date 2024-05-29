#include "HEADERS.h"
#include "sphigslocal.h"

/**
One simple form of optimization is to record, for each view V, a list of the structures
that are subordinates of view V.  This data must be updated in the following ways:


WHEN A STRUCTURE S IS POSTED TO VIEW V:
	Add S to the subord-list for view V.
	Traverse the network rooted at S:
		Add each encountered structure to the subord-list for view V.
		
WHEN A STRUCTURE S IS UNPOSTED FROM VIEW V:
	Completely recalculate view V's subord-list.
		
WHEN A STRUCTURE P BECOMES A NEW CHILD OF STRUCTURE S:
	Create a temporary subord-list, initially clear.
	Add P to the temp list.
	Traverse the network rooted at P:
		Add each encountered structure to the temp subord-list.
	For each view V:
		If structure S is a subord of view V:
		   "OR" the temp subord-list with view V's current subord-list.
			
WHEN A STRUCTURE S IS EDITED IN SUCH A WAY THAT IT POSSIBLY LOST A CHILD:
	For each view V:
		If structure S is posted to view V:
			Completely recalculate view V's subord-list.
			
Here's the algorithm for completely recalculating view V's subord list:
	Clear the subord-list for view V.
	For each structure P still posted to view V:
	    Traverse the network rooted at P:
	        Add each encountered structure to the subord-list for view V.

**/
			

static substruct_bitstring temp_descendent_list = NULL;


/** MERGE DESCENDENT LIST
Uses an optimization algorithm to avoid unnecessary re-traversal.
Warning: assumes bit #sid already set!
**/
static void MergeDescendentList (int sid)
{
   register int s;
   
   for (s=0; s<=MAX_STRUCTURE_ID; s++)
      /* IF STRUCTURE s IS AN IMMEDIATE CHILD OF sid THEN... */
      if (TestBit(SPH__structureTable[sid].child_list, s))
         /* RECURSE ONLY IF STRUCTURE s IS NOT YET RECORDED */
         if ( ! TestBit(temp_descendent_list, s)) {
	    SetBit (temp_descendent_list, s);
            MergeDescendentList (s);
         }
}


static void CalculateDescendentListForOneStructure (int struct_ID)
{
   
   ClearBitstring (&temp_descendent_list);
   SetBit (temp_descendent_list, struct_ID);
   MergeDescendentList (struct_ID);
}


static void CalculateDescendentListForView (view_spec *vs)
{
   root_header *rptr;
   
   ClearBitstring (&temp_descendent_list);
   
   for (rptr=vs->highestOverlapNetwork; rptr; 
	rptr=rptr->nextLowerOverlapRoot) {	
      SetBit (temp_descendent_list, rptr->root_structID);
      MergeDescendentList (rptr->root_structID);
   }
}



void VIEWOPT__afterNewPosting (view_spec *v, int struct_ID)
{
   CalculateDescendentListForOneStructure (struct_ID);
   MergeBitstring (v->descendent_list, temp_descendent_list);
}


void VIEWOPT__afterUnposting (view_spec *v, int struct_ID)
{
   CalculateDescendentListForOneStructure (struct_ID);
   MergeBitstring (v->descendent_list, temp_descendent_list);
}


void VIEWOPT__newExecuteStructure (int ID_of_new_parent, int ID_of_new_child)
{
   register int v;
   
   CalculateDescendentListForOneStructure (ID_of_new_child);
   for (v=0; v<=MAX_VIEW_INDEX; v++)
      if (TestBit(SPH_viewTable[v].descendent_list, ID_of_new_parent))
	 MergeBitstring 
	    (SPH_viewTable[v].descendent_list, temp_descendent_list);
}


void VIEWOPT__afterChildLoss (int ID_of_changed_parent)
{
   register int v;
   
   for (v=0; v<=MAX_VIEW_INDEX; v++)
      if (TestBit(SPH_viewTable[v].descendent_list, ID_of_changed_parent))
	 CalculateDescendentListForView (&(SPH_viewTable[v]));
}
