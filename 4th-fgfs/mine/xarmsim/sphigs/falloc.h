/** FALLOC : falloc.h
FALLOC public header file.
**/

/* $Id: falloc.h,v 1.3 90/09/05 18:05:35 dfs Exp Locker: dfs $ */


#ifndef FALLOC_HEADER
#define FALLOC_HEADER

/** Constants
**/

#define FALLOC__ZERO      1
#define FALLOC__DONT_ZERO 0
#define FALLOC_BLOCK_SIZE (4096-12)


/** Types
**/

typedef struct  FALLOCchunk     FALLOCchunk;

struct FALLOCchunk {
   int  free_bytes;
   char *cur_ptr;
   int  cur_block, num_blocks;
   char **blocks;
   char **over_blocks;          /* Blocks larger than blocksize */
   int  num_over_blocks;
   int  magic;
};


/** Prototypes
**/

FALLOCchunk     *FALLOCnew_chunk (void);
char            *FALLOCalloc(FALLOCchunk*, int nbytes, int zero);
void            FALLOCfree (FALLOCchunk*);
void            FALLOCclear_chunk (FALLOCchunk*);

#endif FALLOC_HEADER

