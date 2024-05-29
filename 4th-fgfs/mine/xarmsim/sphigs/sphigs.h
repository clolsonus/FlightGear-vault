#ifndef SPHIGS_H_INCLUDED_ALREADY

#define SPHIGS_H_INCLUDED_ALREADY

#include "mat3.h"
#include "srgp_sphigs.h"
#include "srgppublic.h"


#ifdef SPHIGS_BOSS
#define DECLARE
#else
#define DECLARE extern
#endif


/** LIMITATION CONSTANTS **/
#define DEFAULT_MAX_STRUCTURE_ID	15
#define DEFAULT_MAX_VIEW_INDEX		 5

#define MAX_HIERARCHY_LEVEL		14
#define MAX_NAME		  	31


DECLARE short MAX_STRUCTURE_ID
#ifdef SPHIGS_BOSS
   = DEFAULT_MAX_STRUCTURE_ID
#endif
   ;
DECLARE short MAX_VIEW_INDEX
#ifdef SPHIGS_BOSS
   = DEFAULT_MAX_VIEW_INDEX
#endif
   ;




/** TYPE DEFINITIONS
Geometric 2D floating-point.
**/
typedef struct {double x, y;}				NDC_point;
typedef struct {NDC_point bottom_left, top_right;}	NDC_rectangle;





/*!*/
/* RENDERING MODE CONSTANTS */
#define WIREFRAME_RAW		1
#define WIREFRAME		2  /* default */
#define FLAT			3
#define LIT_FLAT		4
#define GOURAUD			5

/*!*/
/* GEOMETRIC DATA TYPES */
typedef MAT3vec			point;
typedef MAT3vec 		vector;
typedef MAT3mat			matrix;

typedef short vertex_index;


/*!*/
/* NETWORK DELETION TYPES */
#define UNCONDITIONAL	1
#define CONDITIONAL	2

/* MATRIX CONCATENATION TYPES */
#define ASSIGN			1
#define PRECONCATENATE		2
#define POSTCONCATENATE		3

/* PROJECTION TYPES */
#define ORTHOGRAPHIC 0
#define PERSPECTIVE  1

/* EDGE FLAGS */
#define EDGE_VISIBLE 0
#define EDGE_INVISIBLE 1

/* IMPLICIT REGENERATION MODES */
#define SUPPRESSED 1
#define ALLOWED    2


#include "elementType.h"

#define SPH_setModXform(A,B)   \
     A,SPH_setModelingTransformation (temp_matrix, B)


/** TYPE DEFINITIONS 
**/

typedef struct {
    point position;
    int   view_index;
    int   button_chord[3];
    int   button_of_last_transition;
} locator_measure;
#define viewIndex view_index
#define locatorMeasure locator_measure

typedef
   struct {
     int structureID;
     int elementIndex;
     int elementType;    /* symbolic constants in elementType.h */
     int pickID;
   } pickPathItem;

typedef pickPathItem pickPath[MAX_HIERARCHY_LEVEL];

typedef struct {
     int pickLevel;
     pickPath path;
   } pickInformation;

typedef long   nameset;   /* bit vector supporting 32 names */
typedef char   name;      /* a number from 1 to 32 */


/** PROTOTYPES FOR ALL PUBLIC SPHIGS ROUTINES **/

/****************************** state.c */
void		SPH_begin (int width, int height, int color_planes_desired, 
			   int shades_per_flexicolor);
void		SPH_setRenderingMode (int viewindex, int value);
void 		SPH_setDoubleBufferingFlag (boolean);
void		SPH_end (void);
void		SPH_setMaxStructureID (int);
void		SPH_setMaxViewIndex (int);


/****************************** utility.c */
double 		*SPH_defPoint (double *pt, double x, double y, double z);
NDC_rectangle	SPH_defNDCrectangle 
                   (double lx, double by, double rx, double ty);


/****************************** filter.c */
void 		SPH_addToHiliteFilter (nameset);
void 		SPH_removeFromHiliteFilter (nameset);
void 		SPH_addToInvisFilter (nameset);
void		SPH_removeFromInvisFilter (nameset);


/****************************** element.c */
void 		SPH_addToNameSet (name);
void 		SPH_removeFromNameSet (name);

void 		SPH_polyhedron (int numverts, int numfacets, 
				point *verts, vertex_index *facets);
void 		SPH_fillArea (int vCount, point *vertices);
void 		SPH_polyMarker(int vCount, point *vertices);
void 		SPH_polyLine (int vCount, point *vertices);
void 		SPH_text (point origin, char *string);

void 		SPH_setInteriorColor (int value);
void 		SPH_setLineStyle (int LINE_STYLE);
void 		SPH_setLineColor (int value);
void 		SPH_setLineWidthScaleFactor (double value);
void 		SPH_setEdgeFlag (int value);
void 		SPH_setEdgeStyle (int LINE_STYLE);
void 		SPH_setEdgeColor (int value);
void 		SPH_setEdgeWidthScaleFactor (double value);
void 		SPH_setMarkerStyle (int MARKER_STYLE);
void 		SPH_setMarkerColor (int value);
void 		SPH_setMarkerSizeScaleFactor (double value);
void 		SPH_setTextFont (int value);
void 		SPH_setTextColor (int value);

void		SPH_setModelingTransformation (matrix, int method);
void		SPH_clearModelingTransformation (void);
void 		SPH_label (int);
void 		SPH_setPickIdentifier (int);
void 		SPH_executeStructure (int structID);

/********************************* post.c */
void 		SPH_postRoot (int structID, int viewID);
void 		SPH_unpostRoot (int structID, int viewID);
void 		SPH_unpostAllRoots (int viewID);


/********************************* edit.c */
void 		SPH_openStructure (int structID);
void 		SPH_closeStructure (void);
int		SPH_inquireElementPointer (void);

void 		SPH_setElementPointer (int elptr);
void 		SPH_offsetElementPointer (int offset);
void 		SPH_moveElementPointerToLabel (int label);

void 		SPH_deleteElement (void);
void 		SPH_deleteElementsInRange (int index1, int index2);
void 		SPH_deleteElementsBetweenLabels (int lab1, int lab2);


/***************************** refresh.c */
void 		SPH_setImplicitRegenerationMode (int mode);
void 		SPH_regenerateScreen (void);


/***************************** view.c */
void 		SPH_evaluateViewOrientationMatrix 
                   (point view_ref_point,
		    vector view_plane_normal, vector view_up_vector,
		    matrix vo_matrix /*varparm*/ );

void		SPH_evaluateViewMappingMatrix
                   (double umin, double umax, double vmin, double vmax,
		    int proj_type,  /* ORTHOGRAPHIC or PERSPECTIVE */
		    point proj_ref_point,
		    double front_plane_dist, double back_plane_dist,
		    double viewport_minx, 
		    double viewport_maxx, 
		    double viewport_miny, 
		    double viewport_maxy,
		    double viewport_minz, 
		    double viewport_maxz,
		    matrix vm_matrix /*varparm*/ );

void 		SPH_setViewRepresentation 
                   (int viewIndex,
		    matrix vo_matrix, matrix vm_matrix,
		    double vp_minx, double vp_maxx, 
		    double vp_miny, double vp_maxy,
		    double vp_minz, double vp_maxz);
		    
void		SPH_enableView (int viewIndex);    /* undocumented */
void		SPH_disableView (int viewIndex);   /* undocumented */

void		SPH_setViewBackgroundColor (int viewIndex, int color);

void		SPH_setViewPointLightSource 
                   (int viewIndex, double u, double v, double n);

/******************** input.c */
#define SPH_setInputMode SRGP_setInputMode
#define SPH_waitEvent SRGP_waitEvent
#define SPH_getKeyboard SRGP_getKeyboard
#define SPH_sampleKeyboard SRGP_sampleKeyboard
#define SPH_setKeyboardProcessingMode SRGP_setKeyboardProcessingMode
#define SPH_setKeyboardEchoColor SRGP_setKeyboardEchoColor
#define SPH_setKeyboardEchoFont SRGP_setKeyboardEchoFont
#define SPH_setKeyboardMeasure SRGP_setKeyboardMeasure
#define SPH_setLocatorEchoCursorShape SRGP_setLocatorEchoCursorShape
#define SPH_setLocatorButtonMask SRGP_setLocatorButtonMask

void	    SPH_getLocator (locator_measure*);
void	    SPH_sampleLocator (locator_measure*);
void	    SPH_setLocatorMeasure (point position);
void	    SPH_setKeyboardEchoOrigin (point position);

void SPH_pickCorrelate (point npc_position, int viewindex, 
			pickInformation *pickinfo);

void SPH_setPickIdentifier (int id);



/******************** modxform.c */
void	SPH_scale (double x, double y, double z, matrix result);
void	SPH_rotateX (double angle, matrix result);
void	SPH_rotateY (double angle, matrix result);
void	SPH_rotateZ (double angle, matrix result);
void 	SPH_translate (double x, double y, double z, matrix result);
void	SPH_compose (matrix m1, matrix m2, matrix result);


/******************** attribute.c */
void SPH_loadCommonColor (int microcolorindex, char *name);


#define SPH_beep      SRGP_beep
#endif


#undef DECLARE
