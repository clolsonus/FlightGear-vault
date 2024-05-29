#include "mat3defs.h"
#include "sphigs.h"
#include "geom.h"
#include "sph_errtypes.h"
#include "falloc.h"
#include "assert.h"

#define REPORT_ERROR  SPH__error
#include "macros.h"

#define X 0
#define Y 1
#define Z 2


#ifdef SPHIGS_BOSS
#define DECLARE
boolean SPH__enabled=FALSE;
boolean	SPH__structureCurrentlyOpen=FALSE;
#else
#define DECLARE extern
extern boolean SPH__enabled;
extern boolean SPH__structureCurrentlyOpen;
#endif


#define ASSUMED_NUM_OF_OBJECTS	 200	/* per view; automatically realloc'd if nec. */



/** DEVICE-DEPENDENT MEASUREMENTS
Add one for your own screen if not already listed here.
**/
#ifdef sparc
#define MARKER_SIZE_UNIT_IN_PIXELS  10  /* unit size geared towards Sparcstn */
#define LINE_WIDTH_UNIT_IN_PIXELS    2
#endif
#ifdef THINK_C
#define MARKER_SIZE_UNIT_IN_PIXELS   6  /* unit size geared towards MacII */
#define LINE_WIDTH_UNIT_IN_PIXELS    1
#endif

/* a default in case you don't provide on for your machine */
#ifndef MARKER_SIZE_UNIT_IN_PIXELS
#define MARKER_SIZE_UNIT_IN_PIXELS   6  /* I assume big pixels */
#define LINE_WIDTH_UNIT_IN_PIXELS    1
#endif


typedef matrix  matrix_4by4;


/** ALLOCATION MACRO USED BY MATRIX PACKAGE
**/
#define ALLOC(PTR, TYPE, NUM, ERR) {                              \
   PTR = (TYPE *) calloc((unsigned)(NUM), (unsigned)sizeof(TYPE)); \
   if (PTR == NULL)                                                 \
      SPH__error (ERR_MALLOC);					     \
}
/* LAST ARGUMENT OF ALLOC() exists only for backward compatibility */


/** SCREEN DETAILS 
The width (or height) in pixels of one NPC unit.
**/
DECLARE int SPH_ndcSpaceSizeInPixels;



/** SUBSTRUCT_BITSTRING
Each structure uses one of these to keep track of which structures
are subordinates of it.

It allows a string of N bits to be cleared, set, and tested.
   BYTE [0] stores bits 0 through 7
   BYTE [1] stores bits 8 through 15

   Each byte stores its 8 bits in right-to-left order, so the bit with
   the lowest number is stored in the one's place, the bit with the
   highest number is stored in the 2^7 place. 
   Below is a picture of the first byte of a bitstring:

		7   6   5   4   3   2   1   0
		-----------------------------

   The bitstring (shown in hex)   03 82 00 00 00 00

   thus says that bits 0, 1, 9, and 15 are ON.

**/

DECLARE int BYTES_PER_BITSTRING;

typedef unsigned char *substruct_bitstring;

void    ClearBitstring (substruct_bitstring*);

#define CopyBitstring(Bd,Bs)          (bcopy(Bs,Bd,BYTES_PER_BITSTRING))
#define SetBit(B,N)		      (B[N>>3] |= ((char)1)<<(N&7))
#define ClearBit(B,N)		      (B[N>>3] &= ( ! ( ((char)1)<<(N&7))))
boolean BitstringIsClear (substruct_bitstring B);
#define TestBit(B,N)		      (B[N>>3] &  ((char)1)<<(N&7))
#define BitstringsAreEqual(B1,B2)     ( ! (bcmp(B1,B2,BYTES_PER_BITSTRING)))

#define MergeBitstring(Bdest,Bsrc) \
   {register int i; for (i=0; i<BYTES_PER_BITSTRING; i++) Bdest[i] |= Bsrc[i];}
#define AndBitstrings(Bdest,B1,B2) \
   {register int i; \
    for (i=0; i<BYTES_PER_BITSTRING; i++) Bdest[i] = B1[i] & B2[i];}



/** INTELLIGENT RECTANGLE
A rectangle which may be either empty (meaning non-existent) or nonempty.
An empty rectangle is used to, for example, initialize a data item like
the extent of a group of prims: before any primitives are drawn, the
extent is of course "non-existent".
**/
typedef struct {
   boolean nonempty;
   srgp__rectangle rect;
} intelligent_rectangle;       




/** ATTRIBUTE GROUP
Only exists during traversal.
Each structure inherits its parent's attribute group as a value-parameter.
Some of the attributes stored here are already transformed into SRGP
   "pixel-units" versions (like linewidth, markersize).
**/
typedef unsigned char Bbyte;  /* to avoid collision with THINK C's "Byte" */

typedef struct {
   /*.... line */
   Bbyte line_color;
   Bbyte line_width, line_style;

   /*.... marker */
   Bbyte marker_color;
   Bbyte marker_size, marker_style;

   /*.... polyhedra and fill areas */
   Bbyte interior_color;
   Bbyte edge_width, edge_style;
   Bbyte edge_flag, edge_color;

   /*.... text */
   Bbyte text_color, font;
} attribute_group;

#include "sph_face.h"


   

/** FACET
**/
typedef struct {
   vertex_index *vertex_indices;   /* vertex_index defined in sphigs.h */
   Bbyte    	vertex_count;
   MAT3hvec 	normal;
   boolean 	do_draw;
} facet;


/** PDC POINT
**/
typedef int pdc_point[3];


/** SPHIGS INFO STRUCT
Each face-set given to the painter-alg must carry with it its attributes.
**/
typedef struct {
   attribute_group attrs;
} sphigs_info_struct;




/** SPHIGS' COLOR TABLE
**/

DECLARE int NUM_OF_FLEXICOLORS,
	    NUM_OF_SHADES_PER_FLEXICOLOR,
	    BASE_OF_SHADE_LUT_ENTRIES;

#define NUM_OF_APPL_SETTABLE_COLORS  (BASE_OF_SHADE_LUT_ENTRIES-2)

#define IS_A_FLEXICOLORINDEX(i)    \
   ( (i > 1) && (i < (2 + NUM_OF_FLEXICOLORS)) )

#define IS_LEGAL_COLOR_INDEX(C)         \
   ( (C>=0) && (C<BASE_OF_SHADE_LUT_ENTRIES) )

#define IS_CHANGEABLE_COLOR_INDEX(C)         \
   ( (C>=2) && (C<BASE_OF_SHADE_LUT_ENTRIES) )


/** POLYHEDRON
**/
typedef struct {
   int vertex_count;
   MAT3hvec *vertex_list;  /* POINTS TO ARRAY OF 4-element vectors */
   int facet_count;
   facet *facet_list;   /* POINTS TO ARRAY OF FACET STRUCTURES */
} POLYHEDRON;







/** ELEMENT
An element's type field determines which subfields of its two union fields 
are active.
**/
typedef struct element {
   short type;   /* enumerated values begin with ELTYP__ */
   union {
      int count;
      char *textstring;  /* used only by text element */
      int update_type;   /* used only by setModXform element */
   } info;
   union {
      int value;
      MAT3hvec point;
      MAT3hvec *points;    /* ptr to dynamically allocated space */
      POLYHEDRON *poly;    /* ptr to dynamically allocated space */
      matrix_4by4 matrix;
   } data;
   struct element *next, *previous;
} element;

/* UNDOCUMENTED and DANGEROUS... aren't the most useful things like that? */
element *SPH__currentElementDirectAccess (void);



/** STORAGE OF A STRUCTURE
A child of structure S is a structure invoked DIRECTLY by structure S.
A descendent is a structure invoked indirectly by structure S.
The refcount tells us how many times the structure is DIRECTLY INVOKED
   anywhere in the Universe.  Each posting of structure S counts for
   one "ref"; and each execute-structure-S counts for one "ref".
**/
typedef struct {
   short refcount;
   short element_count;
   element *first_element, *last_element;
   substruct_bitstring child_list;
} structure;

DECLARE structure *SPH__structureTable;

void SPH__init_structure_table (void);
void SPH__insertElement (element *baby);
void SPH__add_polyhedron_element (POLYHEDRON *newpoly);

/* THE child_list IS ALWAYS UP-TO-DATE.
 * THE descendent_list CAN BECOME OUT-OF-FASHION, AND MUST BE COMPLETELY 
 * RECALCULATED WHENEVER THE FOLLOWING BOOLEAN IS TRUE.
 */
DECLARE boolean SPH__descendentListObsolete;




/** REPRESENTATION OF A NETWORK
The ID of the structure which is the root of this network is stored.
**/
typedef struct root_header {
   int root_structID;
   struct root_header *nextHigherOverlapRoot, *nextLowerOverlapRoot;
} root_header;



/** VIEW TRANSFORMATION TABLE
The list of objects stored in the view table is updated each time the view's
roots are traversed for display.  So it does not necessarily represent the
*current* state of the networks posted to the view, especially if implicit
regeneration has been disabled.

The stored list of objects is used for pick correlation, and for optimized
screen regeneration.
**/
typedef struct view_spec {
   NDC_rectangle viewport;
   srgp__rectangle pdc_viewport;   /* computed from viewport */
   matrix vo_matrix, vm_matrix, cammat;
   double frontPlaneDistance, backPlaneDistance;
   obj *objects, *objectTail;
   FALLOCchunk *objectChunk;
   int vertexCount, vertexArraySize;
   MAT3hvec *uvnVertices, *npcVertices;
   srgp__point *pdcVertices;
   root_header *highestOverlapNetwork, *lowestOverlapNetwork;
   char rendermode;
   MAT3vec uvn_point_light_source;
   substruct_bitstring descendent_list;
   boolean obsolete_descendent_list;
   boolean obsolete_object_list, obsolete_pdc_vertices;
   boolean currently_disabled;
   short background_color;  /* default is WHITE */
   unsigned short curTraversalIndex;
} view_spec;

DECLARE view_spec *SPH_viewTable;

DECLARE int SPH_implicit_regeneration_mode;

void SPH__init_view_table (void);
void SPH__updateViewInfo (int viewIndex);

void SPH__translateWCtoPDC (int viewIndex, MAT3hvec inpt, MAT3hvec outpt);
void SPH__translateMCtoPDC (matrix xformmat, MAT3hvec inpt, MAT3hvec outpt);
void SPH__map_to_canon (view_spec *vs);
void SPH__calc_intensity (view_spec *vs);
void SPH__zsort (view_spec *vs);

void SPH__refresh_post (int viewIndex);
void SPH__refresh_unpost (int viewIndex);
void SPH__refresh_structure_close (int structID);
void SPH__repaintScreen (void);
void SPH__refresh_viewport_change (int viewIndex, srgp__rectangle old_viewport_rect);

void VIEWOPT__newExecuteStructure (int ID_of_open_struct, int structID);
void VIEWOPT__afterChildLoss (int ID_of_open_struct);
void VIEWOPT__afterNewPosting (view_spec *v, int struct_ID);
void VIEWOPT__afterUnposting (view_spec *v, int struct_ID);






/** GLOBALS USED DURING DISPLAY TRAVERSAL 
**/
DECLARE int currentViewIndex;
DECLARE view_spec *currentViewSpec;
DECLARE matrix
   currentCompositeModxform, 
   currentMCtoUVNxform, 
   currentNormalMCtoUVNxform;
DECLARE int currentRendermode;

DECLARE matrix currentTOTALxform;
/* FROM MC TO PDC IN ONE SWELL FOOP! */
/* ONLY POSSIBLE BECAUSE NO CLIPPING IS SUPPORTED YET. */




/** ERRORS **/
#ifdef THINK_C
/* We hide this from gnu's compiler, which doesn't understand it. */
void SPH__error (int errtype, ...);
#endif


void OBJECT__addFillArea
   (view_spec *vs, MAT3hvec *points, int count,
    matrix xform, attribute_group *attrs);
void OBJECT__addPoly 
   (view_spec *vp, POLYHEDRON *poly, matrix xform, attribute_group *attrs);
void OBJECT__drawAll (view_spec *vs);
void OBJECT__process (view_spec *vs);
void OBJECT__init (view_spec *vs);

void SPH__freePolyhedron (POLYHEDRON *poly);
   

/** VIEWING TRANSFORMATIONS **/
void SPH__generate_pdc_vertices (view_spec *vs);
void SPH__clip (view_spec *vs);
void SPH__cull (view_spec *vs);

void SPH__traverse_network_for_display (view_spec *viewSpec, root_header *network);
void SPH__traverse_struct_for_display (int structID, 
				       attribute_group *inherited_attrs);

void SPH__draw_polyhedron (POLYHEDRON*, attribute_group*);
void SPH__draw_lines (element*, attribute_group*);
void SPH__draw_markers (element*, attribute_group*);
void SPH__draw_text (point, char*, attribute_group*);

void SPH__initDefaultAttributeGroup (void);

void SPH__initColorTable (int num_of_planes);





DECLARE FALLOCchunk	*objectChunk;
DECLARE obj		*objects;
DECLARE int		vertexCount;
DECLARE int		vertexArraySize;
DECLARE MAT3hvec	*uvnVertices;
DECLARE srgp__point	*pdcVertices;



/*!*/
/* in ERROR.C */


#define SPH_check_system_state	\
   if (!SPH__enabled) SPH__error(ERR_NOT_ENABLED)

#define SPH_check_rectangle(LX,BY,RX,TY)		\
   if(!((LX<=RX)&&(BY<=TY)))  SPH__error(ERR_BAD_RECT)

#define SPH_check_no_open_structure	\
   if (SPH__structureCurrentlyOpen) SPH__error(ERR_STRUCTURE_IS_OPEN)

#define SPH_check_open_structure	\
   if ( ! SPH__structureCurrentlyOpen) SPH__error(ERR_NO_STRUCTURE_IS_OPEN)

#define SPH_check_structure_id	\
   if ((structID<0) || (structID>MAX_STRUCTURE_ID)) SPH__error(ERR_STRUCTURE_ID)

#define SPH_check_name	\
   if ((name<1) || (name>MAX_NAME)) SPH__error(ERR_NAME)

#define SPH_check_view_index	\
   if ((viewIndex<0) || (viewIndex>MAX_VIEW_INDEX)) SPH__error(ERR_VIEW_INDEX)

#define SPH_check_vertex_count(C)	\
   if (C<2) SPH__error(ERR_VERTEX_COUNT)

#define SPH_check_edge_count(C)	\
   if (C<2) SPH__error(ERR_EDGE_COUNT)

#define SPH_check_modxform_method	\
   if ((method<ASSIGN) || (method>POSTCONCATENATE)) SPH__error(ERR_METHOD)

#define SPH_check_elindex_range         \
   if ( ! (first_index<=second_index)) SPH__error(ERR_BAD_ELEMENT_INDEX_RANGE)
