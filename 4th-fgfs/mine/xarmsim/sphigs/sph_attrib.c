#include "HEADERS.h"
#include "sphigslocal.h"



/** LOADING A MICROCOLOR INTO THE SPHIGS VIRTUAL COLORTABLE 
Only for loading into the very small microcolor table, excluding the
first two microcolors (0 and 1).
**/

#ifdef __MSDOS__
extern palData srgp__common_color_pal[1];
#endif

void SPH_loadCommonColor (int colorindex, char *name)
{
   register i;
   unsigned short  r, g, b;     /* A SHADE OF THE ACTUAL COLOR */
   unsigned short rr,gg,bb;	/* ACTUAL COLOR */
   int basethiscolor;
   double bias;

   if ( ! IS_CHANGEABLE_COLOR_INDEX(colorindex))
      return;

   SRGP_loadCommonColor (colorindex, name);
   
   if ( ! IS_A_FLEXICOLORINDEX(colorindex))
      return;
      
   /* THE FOLLOWING EXECUTED ONLY IF ACTUAL FLEXICOLOR */
#ifdef __MSDOS__
   rr = srgp__common_color_pal[0].palRed;
   gg = srgp__common_color_pal[0].palGreen;
   bb = srgp__common_color_pal[0].palBlue;
#else
   SRGP_inquireColorTable (colorindex, 1, &rr, &gg, &bb);
#endif

   basethiscolor = BASE_OF_SHADE_LUT_ENTRIES + 
      (colorindex-2)*NUM_OF_SHADES_PER_FLEXICOLOR;

   for (i=0; i<NUM_OF_SHADES_PER_FLEXICOLOR; i++) {
      bias = 
	 0.35 + 
	    (0.65 * ((double)(i+1) / (double)NUM_OF_SHADES_PER_FLEXICOLOR));
      r = rr * bias;
      g = gg * bias;
      b = bb * bias;
      SRGP_loadColorTable ((basethiscolor+i), 1, &r, &g, &b);
   }
}



extern int srgp__available_depth;

void SPH__initColorTable (int shades_per_flexicolor)
{
   register i;
   int num_of_srgp_colors = (1 << SRGP_inquireCanvasDepth());
   
#ifdef THINK_C
   /* On Mac, very highest LUT entry must be reserved and stay black! */
   if (num_of_srgp_colors == (1 << srgp__available_depth))
      num_of_srgp_colors--; 
#endif
   
   /* Compute the number of flexicolors that will be possible */
   /* I assume that C truncates when division is performed. */
   NUM_OF_FLEXICOLORS = 
      (num_of_srgp_colors - 2) / (1 + shades_per_flexicolor);
   			
   if (NUM_OF_FLEXICOLORS < 1) {
      NUM_OF_FLEXICOLORS = 0;
      BASE_OF_SHADE_LUT_ENTRIES = num_of_srgp_colors;
   }
   else {
      /* Calculate dependent statistics */
      NUM_OF_SHADES_PER_FLEXICOLOR = shades_per_flexicolor;
      BASE_OF_SHADE_LUT_ENTRIES = 
	 num_of_srgp_colors - (NUM_OF_FLEXICOLORS * shades_per_flexicolor);
      /* Load black into all the changeable color-table entries */
      for (i=2; i<BASE_OF_SHADE_LUT_ENTRIES; i++)
          SPH_loadCommonColor (i, "black");
   }
}
