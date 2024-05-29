#include "Vlib.h"

int _VProcessColor PARAMS((VWorkContext *cxt, Viewport *v, VColor *vc));

static char *errmsg = "Not enough pixel space for all colors\n";
static char *errmsg2= "Unable to parse color \"%s\".\n";

/*
 *  We'll create the notion of a viewport-specific color tweaking
 *  procedure.  This procedure allows some user code to get control just
 *  before we hand the color name to XParseColor() so that we might
 *  alter the name a bit.  Why use it?  It allows us to shrink down the
 *  color space a bit on color-poor screens.
 */

void
VSetColorTweakProc (v, proc)
Viewport *v;
void (*proc)();
{
	v->colorTweakProc = proc;
}

static int
pmap (vec, n)
unsigned long	*vec;
int		n; {

    static int itbl[] = {1, 2, 4, 8};
    register int i, r=0;

    for (i=0; i<4; ++i)
	if (itbl[i] & n)
	   r += vec[i];
    return r;
}

int
VBindColors (v, background)
Viewport *v;
char * background; {

    register int i, j, k, n, c;
    static   int parseComplete = 0;
    unsigned int pixel;
    VColor 	*p, *bg;
    XColor	*colorSet;
    Display	*dpy;
    unsigned long planemask[PLANES*2];
    unsigned long pixels[1];
    int		maxpcolors;
    char	realColor[64];

    maxpcolors = (v->flags & VPMono) ? 256 : v->visual->map_entries;

    dpy = v->dpy;
	
    _VDefaultWorkContext->nextPixel = 0;

/*
 *  If this is a color or greyscale pixmap, allocate color cells for each
 *  color, and then set up the Viewport for drawing.
 *
 *  If this is a monochrome monitor, then the aPixel value for this color is
 *  an index into our standard pixmap table rather than being a Pixel value.
 */

    if (v->flags & (VPPixmap | VPFastAnimation | VPMono)) {
    
    	bg = VAllocColor (background);

	if (_VProcessColor (_VDefaultWorkContext, v, bg) != 0) {
		return -1;
	}

	for ((i=0, p=_VDefaultWorkContext->VColorList); p ; i++) {

	    if (i > maxpcolors) {
	        fprintf (stderr, "Too many colors selected.\n");
	        return -1;
	    }

	    if (_VProcessColor (_VDefaultWorkContext, v, p) != 0) {
		return -1;
	    }

	    p = p->next;
	}

	v->colors = _VDefaultWorkContext->nextPixel;
	v->set = 0;
	v->pixel = v->aPixel;
	v->mask = AllPlanes;

	return 0;
    }

/*
 *  If we fall through to here, we are doing double buffering.  This is
 *  basically obsolete code, but I'll leave it around for now.
 */

    colorSet = (XColor *) Vmalloc (sizeof(XColor) * maxpcolors);

    n = PLANES;
    c = 1 << n;

    if (XAllocColorCells (dpy, v->cmap, False, planemask, n*2, pixels, 1) == 0) {
        fprintf (stderr, "Cannot allocate color cells\n");
	free ((char *) colorSet);
        return -1;
    }

/*
 *  Parse background color
 */

    if (/*parseComplete == 0*/ 1) {

	if (v->colorTweakProc) {
		(*v->colorTweakProc)(v, background, realColor);
	}
	else {
		strcpy (realColor, background);
	}

	if (XParseColor(dpy, v->cmap, realColor, &colorSet[0]) == 0) {
	    fprintf (stderr, errmsg2, background);
	    free(colorSet);
            return -1;
        }

/*
 *  Parse each color defined in the V Color List
 */

	for ((i=0, p=_VDefaultWorkContext->VColorList); p ; i++) {
	    if (i > c) {
	        fprintf (stderr, "Too many colors selected.\n");
		free(colorSet);
	        return -1;
	    }

	    if (v->colorTweakProc) {
		(*v->colorTweakProc)(v, p->color_name, realColor);
	    }
	    else {
		strcpy (realColor, p->color_name);
	    }

	    if (XParseColor (dpy, v->cmap, realColor, &colorSet[i+1]) == 0) {
	        fprintf (stderr, errmsg2, p->color_name);
		free(colorSet);
	        return -1;
	    }
 	    p->cIndex = i+1;
	    p = p->next;
	}

	parseComplete = 1;
    }

    v->colors = i+1;

#ifdef DEBUG
    fprintf (stderr, "%d colors defined in the V color list.\n", i);
#endif

/*
 *  PAY ATTENTION!
 *
 *  We will now create a two lists of XColors. Each will expose a particular
 *  drawing buffer (there are two drawing buffers created here).
 *  A drawing is exposed by passing one of these lists to the XSetColors
 *  procedure.
 *  We create a list by iterating through each possible combination of
 *  pixel values, based on the values returned in pixel and planemask.
 *  The pixel value is determined using a function called pmap.  Each pixel
 *  value is assigned the appropriate XColor.
 */

    k = 0;
    for (i=0; i<v->colors; ++i) {
	pixel = v->aPixel[i] = pmap(&planemask[0], i) | pixels[0];
	for (j=0; j<v->colors; ++j) {
	    v->aColor[k] = colorSet[i];
	    v->aColor[k++].pixel = pixel | pmap (&planemask[n], j);
	}
    }

    v->aMask = pmap(&planemask[0], (c-1)) | pixels[0];

    k = 0;
    for (i=0; i<v->colors; ++i) {
	pixel = v->bPixel[i] = pmap(&planemask[n], i) | pixels[0];
	for (j=0; j<v->colors; ++j) {
	    v->bColor[k] = colorSet[i];
	    v->bColor[k++].pixel = pixel | pmap(&planemask[0], j);
	}
    }

    v->bMask = pmap(&planemask[n], (c-1)) | pixels[0];

    free(colorSet);
    return 0;
}

int
_VProcessColor (cxt, v, vc)
VWorkContext	*cxt;
Viewport	*v;
VColor		*vc;
{
	char	realColor[256];
	Display *dpy = v->dpy;
	XColor	xcolor, xcolor2, hcolor;
	double	d;
	int	i, swap = 0;
	unsigned long temp;

/*
 *  First, if we are doing any depth cueing, insure that the haze color has
 *  been converted to RGB values.
 */

	if (cxt->depthCueSteps > 1 && (v->flags & VPDepthCueParsed) == 0) {

		if (v->colorTweakProc) {
			(*v->colorTweakProc)(v, cxt->depthCueColor->color_name,
				realColor);
		}
		else {
			strcpy (realColor, cxt->depthCueColor->color_name);
		}

		if (XParseColor(dpy, v->cmap,
			realColor, &hcolor) == 0) {
				fprintf (stderr, errmsg2,
					cxt->depthCueColor->color_name);
				return -1;
        	}

        	if (XAllocColor (dpy, v->cmap, &hcolor) == 0) {
		    fprintf (stderr, errmsg);
		    return -1;
		}

		cxt->depthCueColor->cIndex = cxt->nextPixel;
		v->aPixel[cxt->nextPixel++] = hcolor.pixel;
		v->xdepthCueColor = hcolor;
		v->flags |= VPDepthCueParsed;
		swap = 1;
	}

/*
 *  Now parse this color.
 */
	
	if (v->colorTweakProc) {
		(*v->colorTweakProc)(v, vc->color_name, realColor);
	}
	else {
		strcpy (realColor, vc->color_name);
	}

	if (XParseColor(dpy, v->cmap, realColor, &xcolor) == 0) {
		fprintf (stderr, errmsg2, vc->color_name);
		return -1;
        }

	if (v->flags & VPMono) {
		vc->cIndex = cxt->nextPixel;
		v->aPixel[cxt->nextPixel++] = (
			xcolor.red   * 299L +
			xcolor.green * 587L +
			xcolor.blue  * 114L) / (1000 * 8192);
	}
	else if (v->flags & VPDepthCueing && cxt->depthCueSteps > 1 &&
		vc->flags & ColorEnableDepthCueing) {
	    vc->cIndex = cxt->nextPixel;
	    hcolor = v->xdepthCueColor;
	    for (i=0; i<cxt->depthCueSteps-1; ++i) {
	    	d = (double) i / (double) cxt->depthCueSteps;
	    	xcolor2.red   = xcolor.red   * (1.0 - d) + hcolor.red   * d;
	    	xcolor2.green = xcolor.green * (1.0 - d) + hcolor.green * d;
	    	xcolor2.blue  = xcolor.blue  * (1.0 - d) + hcolor.blue  * d;
	    	xcolor2.flags = xcolor.flags;
	    	if (XAllocColor (dpy, v->cmap, &xcolor2) == 0) {
		    fprintf (stderr, errmsg);
		    return -1;
	    	}
	    	v->aPixel[cxt->nextPixel++] = xcolor2.pixel;
	    }
	}
	else {
	    if (XAllocColor (dpy, v->cmap, &xcolor) == 0) {
		fprintf (stderr, errmsg);
		return -1;
	    }
	    vc->cIndex = cxt->nextPixel;
	    v->aPixel[cxt->nextPixel++] = xcolor.pixel;
	}

/*
 *  Sorry for the hack, but here goes ...
 *
 *  The background color in the V library is a bit of an orphan.  It is
 *  defined when VBindColors is called.  There are parts of the V library
 *  that assume that v->aPixel[0] is the pixel value of the background color,
 *  but that is not true at this point in time if we just got through parsing
 *  the depth cue color.  By swapping the aPixel[0] and aPixel[1] values
 *  (and updating the corresponding VColor entries that point to them), we
 *  an hack around the problem.
 */
 
	if (swap) {
		temp = v->aPixel[0];
		v->aPixel[0] = v->aPixel[1];
		v->aPixel[1] = temp;
		temp = cxt->depthCueColor->cIndex;
		cxt->depthCueColor->cIndex = vc->cIndex;
		vc->cIndex = temp;
	}
	
	return 0;
}
