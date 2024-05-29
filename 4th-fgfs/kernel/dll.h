/*    DLL.H -> DLL definitions and declarations.
         Copyright (c) 1995 Malcolm Taylor (empty@sans.vuw.ac.nz)
         This software is copyrighted freeware and may only be 
         distributed with this copyright message intact. 
         This software comes with no warranty.
*/


#ifndef _dll_h
#define _dll_h

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VOIDFN)(void);

typedef struct __dll_structure {
   char *name;
   unsigned char *base;
   int dll_id;
   int load_count;
   VOIDFN init;
   VOIDFN deinit;
   VOIDFN DLL_Exit_Point;
   unsigned char *destructors;
   int num_destructors;
   struct __dll_structure *next;
} DLL;

typedef struct __old_symbol_entry {
   struct __old_symbol_entry *next;
   void *address;
   int dll_owner;
   char symbol[1];
} DLL_SYMBOL_ENTRY;

typedef struct __dll_address_entry {
   struct __dll_address_entry *next;
   void *address;
   int symnum;
} OLD_ADDRESS_ENTRY;

int DLL_Load(char *filename);          /* Loads the DLL filename */
void DLL_AddSymbol(char *name,void *address);
                                       /* Adds a symbol to the table */
void *DLL_LookupSymbol(char *symbol);  /* Returns the address of symbol */
void DLL_UnLoad(char *filename);       /* Unloads the DLL filename */

#ifdef __cplusplus
}
#endif


#endif
