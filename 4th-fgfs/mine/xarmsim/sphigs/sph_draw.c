#include "HEADERS.h"
#include "sphigslocal.h"



/** DRAW ROUTINES
The display-traverser has set the two variables "currentViewIndex" and 
    "currentCompositeModxform".
**/



srgp__point srgp_pdc_points[20], srgp_polygon_points[20];



/*!*/
static void ChangeMCpointsToPDC3d_points 
   (MAT3hvec *MCvlist, pdc_point *PDCvlist, int vertex_count)
{
   register i;
   MAT3hvec temp_outpt;

   for (i=(vertex_count-1); i>=0; i--) {
      MAT3mult_hvec (temp_outpt, MCvlist[i], currentTOTALxform, 1);
      PDCvlist[i][0] = (int)temp_outpt[0];
      PDCvlist[i][1] = (int)temp_outpt[1];
      PDCvlist[i][2] = (int)temp_outpt[2];
   }
}


/*!*/
static void ChangeMCpointsToSRGP_points 
   (MAT3hvec *MCvlist, srgp__point *SRGP_vlist, int vertex_count)
{
   register i;
   MAT3hvec temp_outpt;

   for (i=(vertex_count-1); i>=0; i--) {
      MAT3mult_hvec (temp_outpt, MCvlist[i], currentTOTALxform, 1);
      SRGP_vlist[i].x = (int)temp_outpt[0];
      SRGP_vlist[i].y = (int)temp_outpt[1];
   }
}



/*!*/
void
SPH__draw_polyhedron (poly, attrs)
POLYHEDRON *poly;
attribute_group *attrs;
{
   register int v, f, viindex;
   pdc_point *vertptr;
   vertex_index *viptr;
   facet *facetptr;
   obj *newobj;

   switch (currentRendermode) {

    case WIREFRAME_RAW:

      SRGP_setColor (attrs->edge_color);
      SRGP_setLineWidth (attrs->edge_width);
      SRGP_setLineStyle (attrs->edge_style);

      ChangeMCpointsToSRGP_points (poly->vertex_list, srgp_pdc_points,
				   poly->vertex_count);

      for (f = 0, facetptr = poly->facet_list;
	   f < poly->facet_count;      
	   f++, facetptr++) {
	 viindex = facetptr->vertex_count;
	 viptr = facetptr->vertex_indices + viindex;
	 while (viindex--)
	    srgp_polygon_points[viindex] = srgp_pdc_points[*(--viptr)];
	 SRGP_polygon (facetptr->vertex_count, srgp_polygon_points);
      }
      break;

    default:
      OBJECT__addPoly (currentViewSpec, poly, currentMCtoUVNxform, attrs);
      break;
   }
}


/*!*/
void
SPH__draw_fill_area (element *elptr, attribute_group *attrs)
{
   switch (currentRendermode) {

    case WIREFRAME_RAW:
      SRGP_setColor (attrs->edge_color);
      SRGP_setLineWidth (attrs->edge_width);
      SRGP_setLineStyle (attrs->edge_style);
      ChangeMCpointsToSRGP_points 
	 (elptr->data.points, srgp_pdc_points, elptr->info.count);
      SRGP_polygon (elptr->info.count, srgp_pdc_points);
      break;

    default:
      OBJECT__addFillArea (currentViewSpec, 
			   elptr->data.points, elptr->info.count,
			   currentMCtoUVNxform, attrs);
      break;
   }
}




/*!*/
void
SPH__draw_lines (element *elptr, attribute_group *attrs)
{
   ChangeMCpointsToSRGP_points 
      (elptr->data.points, srgp_pdc_points, elptr->info.count);

   SRGP_setColor (attrs->line_color);
   SRGP_setLineStyle (attrs->line_style);
   SRGP_setLineWidth (attrs->line_width);
   SRGP_polyLine (elptr->info.count, srgp_pdc_points);
}



/*!*/
void
SPH__draw_markers (element *elptr, attribute_group *attrs)
{
   ChangeMCpointsToSRGP_points 
      (elptr->data.points, srgp_pdc_points, elptr->info.count);

   SRGP_setColor (attrs->marker_color);
   SRGP_setMarkerStyle (attrs->marker_style);
   SRGP_setMarkerSize (attrs->marker_size);
   SRGP_polyMarker (elptr->info.count, srgp_pdc_points);
}
		 
      

/*!*/
void
SPH__draw_text (mc_origin, text, attrs)
MAT3hvec mc_origin;
char *text;
attribute_group *attrs;
{
   ChangeMCpointsToSRGP_points ((MAT3hvec*)mc_origin, srgp_pdc_points, 1);
   SRGP_setColor (attrs->text_color);
   SRGP_setFont (attrs->font);
   SRGP_text (srgp_pdc_points[0], text);
}
