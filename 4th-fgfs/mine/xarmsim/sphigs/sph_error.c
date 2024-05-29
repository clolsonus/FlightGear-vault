#include "HEADERS.h"
#include "sphigslocal.h"
#include <stdio.h>


#ifndef THINK_C
static
char *(sphigs_errmsgs[]) =
  {
   "UNUSED",
   "SPHIGS is not enabled yet!  Have you heard about SPH_begin()?\n",
   "You sent a bad rectangle (either your l>r, or your b>t).\n",
   "A structure is already open.  You attempted to open another one.\n",
   "No structure is currently open.  You can't do editing operations.\n",
   "You sent a bad structure-id.\n",
   "You sent a bad name.\n", 
   "You sent a bad view index.\n",
   "You attempted to change the window or viewport for view #0.\n",
   "You tried to move the element ptr outside the bounds of the structure.\n",
   "You referred to a label not found during a forward scan.\n",
   "You tried to delete an element where none exists.\n",
   "You tried to unpost a structure that is not known to be posted there.\n",
   "SPHIGS is already enabled!  You tried to enable it yet again.\n",
   "You sent a bad 'update-method' (second parameter).\n",
   "You sent a bad vertex count.\n",
   "You sent a bad edge count.\n",
   "You referred to a device other than KEYBOARD.\n",
   "Pick corr. is illegal when the display is out-of-synch with the CSS!\n",
   "Pick correlation is illegal for a viewport using wireframe-raw mode.\n", 
   "You tried to add one edge/vertex too many!\n", 
   "The vertex-index for the first vertex in this edge is illegal!\n",
   "The vertex-index for the second vertex in this edge is illegal!\n",
   "You're closing the polyhedron before specifying all the vertices!\n",
   "You're closing the polyhedron before specifying all the edges!\n",
   "Viewing parameter error: %s\n",
   "You sent an element-index-range where the first index was not <= the second one!\n",
   "The MAT3 matrix package encountered an error.  See SRGPlogfile.\n",
   "UNUSED",
   "Sorry!  I ran out of dynamic memory!\n",
   "You sent a color index that is beyond the available range.\n",
   "You can't set entries 0, 1, or any out-of-range entry of the color table.\n",
   
   "UNUSED"
};
#endif


extern char *srgp__identifierOfPackage;

#ifdef THINK_C
extern int   srgp__identifierOfMessages;
#else
extern char **srgp__identifierOfMessages;
#endif

#ifdef THINK_C
/* We hide this from gnu's compiler, which doesn't understand it. */
void SRGP__error (int errtype, ...);
#endif


void
SPH__error (errtype, arg1, arg2, arg3, arg4, arg5)
int errtype, arg1, arg2, arg3, arg4, arg5;
{
   srgp__identifierOfPackage = "SPHIGS";
#ifdef THINK_C
   srgp__identifierOfMessages = 129;
#else
   srgp__identifierOfMessages = sphigs_errmsgs;
#endif
   SRGP__error (errtype, arg1, arg2, arg3, arg4, arg5);
}
