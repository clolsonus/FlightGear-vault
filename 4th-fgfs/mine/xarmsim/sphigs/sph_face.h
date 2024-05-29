/**********************
                      *
 File                 * face.h
                      *
                      ********************************************************/

#ifndef _H_face_
#define _H_face_


/**********************
                      *
 Types		      *
                      *
                      ********************************************************/


/*****************************************************************************/
/*
/*   objType
/*
/*   Types of objects which our routines handle
/*
/*****************************************************************************/

typedef enum { objFace, objLine, objText, objPoint } objType;

/*****************************************************************************/
/*
/*   typeFace
/*
/*   Face specific fields
/*
/*****************************************************************************/

typedef struct _typeFace {
     int		numPoints;   	/* passed (need minimum 3 points) */
     vertex_index *	points;		/* passed (no sentinel) */
} typeFace;

/*****************************************************************************/
/*
/*   typeLine
/*
/*   Line specific fields
/*
/*****************************************************************************/

typedef struct _typeLine {
     MAT3hvec			end1, end2;			/* passed */
} typeLine;

/*****************************************************************************
/*
/*   typeText
/*
/*   Text specific fields
/*
/*****************************************************************************/

typedef struct _typeText {
     char *			text;
} typeText;

/*****************************************************************************/
/*
/*   typePoint
/*
/*   Point specific fields
/*
/*****************************************************************************/

typedef struct _typePoint {
   char dummy;
} typePoint;


/*****************************************************************************/
/*
/*   obj
/*
/*   Data structure used for each object to be rendered
/*   Object can be a face, line, point, or text item
/*
/*****************************************************************************/

typedef struct _obj {
     
     /* object type */
     objType			type;		/* face, line, or text */
     
     /* pointer to the next object */
     struct _obj * 		next;

     /* normal (for face, text) */
     MAT3hvec 			normal;		/* passed for face */
     
     /* bounding cube */
     /* for face: we calculate */
     /* for point: pass coordinates in min[x], min[y], min[z] */
     /* for line: we calculate */
     /* for text: pass z in min[z], pass min and max [x] and [y] */
     MAT3hvec			min;
     MAT3hvec			max;
     
     /* color intensity */
     double	intensity;  	/* should actually be a field in "typeFace" structure */
     
     /* pointer to specifics */
     attribute_group 		attributes;

     /* plane equation */
     double			directed_distance;
     
     /* object specifics */
     union {
	  typeFace		face;
	  typeLine		line;
	  typeText		text;
	  typePoint		point;
     } data;

     /* stored for all objects */
     MAT3hvec			p_min;
     MAT3hvec			p_max;
     boolean			already_moved;
     MAT3hvec			p_normal;
     unsigned short		traversal_index;  /* used for pick correlation */
} obj;




/**********************
                      *
 Prototypes           *
                      *
                      ********************************************************/

/*

extern void cull(view_spec *vs);
extern void clip(view_spec *vs);
extern void map_to_canon(view_spec *vs);
extern void generate_pdc_vertices(view_spec *vs);
extern void zsort(view_spec *vs);

extern void debug_object(obj * head_of_list);

*/

extern void convexify();
extern void cull( );
extern void clip( );
extern void calc_intensity( );
extern void map_to_canon( );
extern void generate_pdc_vertices( );
extern void zsort( );

#endif




