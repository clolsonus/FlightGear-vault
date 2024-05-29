#include "HEADERS.h"
#include "sphigslocal.h"


void ClearBitstring (substruct_bitstring *B)
{
   if (*B == NULL)
      *B = (unsigned char*) malloc(BYTES_PER_BITSTRING);
   bzero (*B,BYTES_PER_BITSTRING);
}


/** BitstringIsClear
I wish I could have made this a macro, but it wasn't possible.  Checks
each byte in a bitstring to verify all bits are off.
**/

boolean BitstringIsClear (substruct_bitstring B)
{
   register int i; 

   for (i=0; i<BYTES_PER_BITSTRING; i++)
      if (B[i])
	 return FALSE;
   return TRUE;
}
