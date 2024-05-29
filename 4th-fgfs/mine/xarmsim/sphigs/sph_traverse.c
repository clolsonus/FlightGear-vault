#include "HEADERS.h"
#include "sphigslocal.h"
#include "sph_draw.h"
#include <string.h>

/** Transforming the normal from MV to UVN
Here, we compute a matrix that can be used to transform a normal 
from MV to UVN.  Based on John Hughes' magic!
**/

static void ComputeNormalTransformer (void)
{
#  define a currentMCtoUVNxform[0][0]
#  define b currentMCtoUVNxform[0][1]
#  define c currentMCtoUVNxform[0][2]
#  define d currentMCtoUVNxform[1][0]
#  define e currentMCtoUVNxform[1][1]
#  define f currentMCtoUVNxform[1][2]
#  define p currentMCtoUVNxform[2][0]
#  define q currentMCtoUVNxform[2][1]
#  define r currentMCtoUVNxform[2][2]

   bzero (currentNormalMCtoUVNxform, sizeof(matrix));
   currentNormalMCtoUVNxform[0][0] = e*r-q*f;
   currentNormalMCtoUVNxform[0][1] = p*f-d*r;
   currentNormalMCtoUVNxform[0][2] = d*q-p*e;
   currentNormalMCtoUVNxform[1][0] = q*c-b*r;
   currentNormalMCtoUVNxform[1][1] = a*r-p*c;
   currentNormalMCtoUVNxform[1][2] = p*b-q*a;
   currentNormalMCtoUVNxform[2][0] = b*f-c*e;
   currentNormalMCtoUVNxform[2][1] = d*c-a*f;
   currentNormalMCtoUVNxform[2][2] = a*e-d*b;
}
      


/** SPH_traverse_struct_for_display
**/
void
SPH__traverse_struct_for_display (int structID, 
				 attribute_group *inherited_attrs)
{
   element *curel;
   matrix_4by4 global_modxform, local_modxform;
   matrix_4by4 save_compositeModxform, save_MCtoUVNxform, 
               save_NormalMCtoUVNxform, save_TOTALxform;
   attribute_group my_attrs;

   my_attrs = *inherited_attrs;

   /* Since a subordinate structure shouldn't affect its parent's 
    *   composite modeling xform, each subordinate saves and restores it.
    */
   MAT3copy (save_compositeModxform, currentCompositeModxform);
   MAT3copy (save_MCtoUVNxform, currentMCtoUVNxform);
   MAT3copy (save_NormalMCtoUVNxform, currentNormalMCtoUVNxform);
   MAT3copy (save_TOTALxform, currentTOTALxform);

   /* The local matrix for any structure is initially the identity matrix,
    *   so its initial composite matrix is equal to the global matrix.
    */
   MAT3identity (local_modxform);
   MAT3copy (global_modxform, currentCompositeModxform);


/*!*/
   for /* FOR EACH ELEMENT IN THE STRUCTURE, IN ORDER: */
   (curel = SPH__structureTable[structID].first_element;
    curel;
    curel = curel->next)

      switch (curel->type) {

       case ELTYP__POLYHEDRON:
	 SPH__draw_polyhedron (curel->data.poly, &my_attrs);
	 break;
       case ELTYP__FILL_AREA:
	 SPH__draw_fill_area (curel, &my_attrs);
	 break;
       case ELTYP__POLYLINE:
	 SPH__draw_lines (curel, &my_attrs);
	 break;
       case ELTYP__POLYMARKER:
	 SPH__draw_markers (curel, &my_attrs);
	 break;
       case ELTYP__TEXT:
	 SPH__draw_text
	    (curel->data.point, curel->info.textstring, &my_attrs);
	 break;

       case ELTYP__EXECUTE_STRUCTURE:
	 SPH__traverse_struct_for_display (curel->data.value, &my_attrs);
	 break;

       case ELTYP__SET_INTERIOR_COLOR:
	 my_attrs.interior_color = curel->data.value;
	 break;

       case ELTYP__SET_EDGE_COLOR:
	 my_attrs.edge_color = curel->data.value;
	 break;
       case ELTYP__SET_EDGE_WIDTH:
	 my_attrs.edge_width = curel->data.value;
	 break;
       case ELTYP__SET_EDGE_STYLE:
	 my_attrs.edge_style = curel->data.value;
	 break;
       case ELTYP__SET_EDGE_FLAG:
	 my_attrs.edge_flag = curel->data.value;
	 break;

       case ELTYP__SET_MARKER_COLOR:
	 my_attrs.marker_color = curel->data.value;
	 break;
       case ELTYP__SET_MARKER_SIZE:
	 my_attrs.marker_size = curel->data.value;
	 break;
       case ELTYP__SET_MARKER_STYLE:
	 my_attrs.marker_style = curel->data.value;
	 break;

       case ELTYP__SET_TEXT_COLOR:
	 my_attrs.text_color = curel->data.value;
	 break;
       case ELTYP__SET_TEXT_FONT:
	 my_attrs.font = curel->data.value;
	 break;

       case ELTYP__SET_LINE_COLOR:
	 my_attrs.line_color = curel->data.value;
	 break;
       case ELTYP__SET_LINE_WIDTH:
	 my_attrs.line_width = curel->data.value;
	 break;
       case ELTYP__SET_LINE_STYLE:
	 my_attrs.line_style = curel->data.value;
	 break;

       case ELTYP__CLEAR_MODXFORM:
       	  MAT3identity (local_modxform);
	  MAT3copy (currentCompositeModxform, save_compositeModxform);
	  MAT3copy (currentTOTALxform, save_TOTALxform);
	  MAT3copy (currentMCtoUVNxform, save_MCtoUVNxform);
	  MAT3copy (currentNormalMCtoUVNxform, save_NormalMCtoUVNxform);
	  break;
       	 
	 
       case ELTYP__SET_MODXFORM:
	 switch (curel->info.update_type) {
	     case ASSIGN:
	       MAT3copy (local_modxform, curel->data.matrix);
               break;
	     case POSTCONCATENATE:
	       MAT3mult (local_modxform, curel->data.matrix, local_modxform);
	       break;
	     case PRECONCATENATE:
	       MAT3mult (local_modxform, local_modxform, curel->data.matrix);
	       break;
	    }
	 MAT3mult (currentCompositeModxform, local_modxform, global_modxform);
	 MAT3mult (currentMCtoUVNxform, currentCompositeModxform,
		   SPH_viewTable[currentViewIndex].vo_matrix);
         MAT3mult (currentTOTALxform, currentCompositeModxform, 
	           SPH_viewTable[currentViewIndex].cammat);
	 if (SPH_viewTable[currentViewIndex].rendermode > WIREFRAME)
	    ComputeNormalTransformer();
      }

   /* The restoration referred to earlier. */
   MAT3copy (currentCompositeModxform, save_compositeModxform);
   MAT3copy (currentTOTALxform, save_TOTALxform);
   MAT3copy (currentMCtoUVNxform, save_MCtoUVNxform);
   MAT3copy (currentNormalMCtoUVNxform, save_NormalMCtoUVNxform);
}
      



#define INITIALIZED_LATER  0

static attribute_group default_attribute_group = {
   INITIALIZED_LATER /* black line color */,
   LINE_WIDTH_UNIT_IN_PIXELS /* line width */, CONTINUOUS /* line style */,

   INITIALIZED_LATER /* black marker color */,
   MARKER_SIZE_UNIT_IN_PIXELS /* marker size */, 
   MARKER_CIRCLE /* marker style */,

   INITIALIZED_LATER /* white interior color */,

   1 /* edge width */, CONTINUOUS /* edge style */,
   EDGE_VISIBLE /* edge_flag */, INITIALIZED_LATER /* black edge color */,

   INITIALIZED_LATER /* black text color */, 0 /* default SRGP font */
};


void SPH__initDefaultAttributeGroup (void)
{
   default_attribute_group.line_color = SRGP_BLACK;
   default_attribute_group.marker_color = SRGP_BLACK;
   default_attribute_group.text_color = SRGP_BLACK;
   default_attribute_group.edge_color = SRGP_BLACK;
   default_attribute_group.interior_color = SRGP_WHITE;
}



/** SPH_traverse_network_for_display 
**/
void
SPH__traverse_network_for_display (view_spec *viewSpec, root_header *network)
{
   MAT3identity (currentCompositeModxform);
   MAT3copy (currentTOTALxform, viewSpec->cammat);
   MAT3copy (currentMCtoUVNxform, viewSpec->vo_matrix);
   if (viewSpec->rendermode > WIREFRAME)
      ComputeNormalTransformer();

   /* NOW LET'S TRAVERSE THE NETWORK!!!! */
   SPH__traverse_struct_for_display
      (network->root_structID, &default_attribute_group);
}
