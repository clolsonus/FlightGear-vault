#include "HEADERS.h"
#include "sphigslocal.h"
#include <string.h>


element *baby;

#define MAKE_BABY_ELEMENT   baby = (element*) malloc ((size_t)sizeof(element))
#define INSERT_BABY	    SPH__insertElement (baby)







/*!*/
static
char *MAKE_COPY_OF_STRING (char *str)
{
   char *copy;

   copy = (char*) malloc (strlen(str)+1);
   strcpy(copy,str);
   return copy;
}




/*!*/
void
SPH_setModelingTransformation (matrix mat, int method)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_modxform_method;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__SET_MODXFORM;
   baby->info.update_type = method;
   MAT3copy (baby->data.matrix, mat);
   INSERT_BABY;
}


/*!*/
void
SPH_clearModelingTransformation (void)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__CLEAR_MODXFORM;
   INSERT_BABY;
}




/*!*/
/* not called directly by the application, called by SPH_polyhedron */
void
SPH__add_polyhedron_element (POLYHEDRON *newpoly)
{
   register i;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__POLYHEDRON;
   baby->data.poly = newpoly;

   INSERT_BABY;
}




/*!*/
void
SPH_text (origin, string)
point origin;
char *string;
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__TEXT;
   baby->info.textstring = MAKE_COPY_OF_STRING (string);
   bcopy (origin, baby->data.point, sizeof(point));
   baby->data.point[3] = 1.0;   /* h-coordinate */
   INSERT_BABY;
}



static void
StorePointList (element *baby, int vCount, point *verts)
{
   register i;

   baby->info.count = vCount;
   ALLOC (baby->data.points, MAT3hvec, vCount, 0);

   /* COPY VERTICES, TRANSFORMING TO HVERTS. */
   for (i=0; i<vCount; i++)
      MAT3_SET_HVEC (baby->data.points[i] ,
		     verts[i][0], verts[i][1], verts[i][2], 1.0);
}



/*!*/
void
SPH_polyLine (int vCount, point *verts)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__POLYLINE;
   StorePointList (baby, vCount, verts);
   INSERT_BABY;
}



/*!*/
void
SPH_polyMarker (int vCount, point *verts)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__POLYMARKER;
   StorePointList (baby, vCount, verts);
   INSERT_BABY;
}



/*!*/
void
SPH_fillArea (int vCount, point *verts)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__FILL_AREA;
   StorePointList (baby, vCount, verts);
   INSERT_BABY;
}




static void SimpleElement (int type, int value)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = type;
   baby->data.value = value;
   INSERT_BABY;
}


/*!*/
void
SPH_label (lab)
int lab;
{
   SimpleElement (ELTYP__LABEL,lab);
}


/*!*/
void
SPH_setPickIdentifier (lab)
int lab;
{
   SimpleElement (ELTYP__PICK_ID,lab);
}


/*!*/
void
SPH_setInteriorColor (value)
int value;
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 0;  /* white is default color index for interiors */
   SimpleElement (ELTYP__SET_INTERIOR_COLOR,value);
}

/*!*/
void
SPH_setLineColor (value)
int value;
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_LINE_COLOR,value);
}

/*!*/
void
SPH_setLineWidthScaleFactor (double value)
{
   register i;
   
   SimpleElement 
      (ELTYP__SET_LINE_WIDTH,
       ((i=LINE_WIDTH_UNIT_IN_PIXELS*value) < 1) ? 1 : i);
}

/*!*/
void
SPH_setLineStyle (value)
int value;
{
   SimpleElement (ELTYP__SET_LINE_STYLE,value);
}

/*!*/
void
SPH_setMarkerColor (value)
int value;
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_MARKER_COLOR,value);
}

/*!*/
void
SPH_setMarkerSizeScaleFactor (double value)
{
   register i;
   
   SimpleElement 
      (ELTYP__SET_MARKER_SIZE,
       ((i=MARKER_SIZE_UNIT_IN_PIXELS*value) < 1) ? 1 : i);
}

/*!*/
void
SPH_setMarkerStyle (value)
int value;
{
   SimpleElement (ELTYP__SET_MARKER_STYLE,value);
}

/*!*/
void
SPH_setEdgeColor (value)
int value;
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_EDGE_COLOR,value);
}

/*!*/
void
SPH_setEdgeStyle (value)
int value;
{
   SimpleElement (ELTYP__SET_EDGE_STYLE,value);
}

/*!*/
void
SPH_setEdgeFlag (value)
int value;
{
   SimpleElement (ELTYP__SET_EDGE_FLAG,value);
}

/*!*/
void
SPH_setEdgeWidthScaleFactor (double value)
{
   register i;
   
   SimpleElement 
      (ELTYP__SET_EDGE_WIDTH,
       ((i=LINE_WIDTH_UNIT_IN_PIXELS*value) < 1) ? 1 : i);
}




/*!*/
void
SPH_setTextFont (value)
int value;
{
   SimpleElement (ELTYP__SET_TEXT_FONT, value);
}

/*!*/
void
SPH_setTextColor (value)
int value;
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_TEXT_COLOR, value);
}





/** Borrows data from edit.c **/
extern structure *OPENSTRUCT;
extern int ID_of_open_struct;

void
SPH_executeStructure (int structID)
{
   register int sid;


   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_structure_id;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__EXECUTE_STRUCTURE;
   baby->data.value = structID;
   INSERT_BABY;

   SPH__structureTable[structID].refcount++;

   /* UPDATE THE OPEN STRUCTURE'S CHILD_LIST. */
   SetBit (OPENSTRUCT->child_list, structID);

   /* UPDATE VIEWS' DESCENDENT LISTS! */
   VIEWOPT__newExecuteStructure (ID_of_open_struct, structID);
}
