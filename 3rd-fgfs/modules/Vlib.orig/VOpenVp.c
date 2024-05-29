#include "Vlib.h"

#include "gray0.xbm"
#include "gray1.xbm"
#include "gray2.xbm"
#include "gray3.xbm"
#include "gray4.xbm"
#include "gray5.xbm"
#include "gray6.xbm"
#include "gray7.xbm"

extern void VComputeClipNormals();

int
ViewportSetDepthCueing (v, flag)
Viewport	*v;
int		flag;
{
	int status = 0;

	if (_VDefaultWorkContext->depthCueSteps > 1 &&
		v->flags & VPFastAnimation) {
		if (flag)
			v->flags |= VPDepthCueing;
		else
			v->flags &= ~VPDepthCueing;
	}
	else {
		return -1;
	}
	return status;
}

Viewport *
VOpenViewport (dpy, screen, win, cmap, visual, unit, dist, scale,
	width, height)
Display	*dpy;
int	screen;
Window	win;
Colormap cmap;
Visual	*visual;
double  unit;
double  dist;
double  scale;
int	width;
int	height; {

	Viewport *v;
	VPoint	 clip[4];
	int	 planes, i;
	int      maxpcolors;

	maxpcolors = (visual->map_entries == 2) ? 256 : visual->map_entries;

/*
 *  Allocate space for the Viewport structure
 */

	v = (Viewport *) Vmalloc (sizeof(Viewport));
	v->zsize = 32768;
	v->ztop = 0;
	v->zpool = (ZInfo *) Vmalloc (sizeof(ZInfo) * v->zsize);
	v->depth = MaxDepth;

/*
 * Calculate screen resolution in pixels per unit.
 */

	v->dist = dist;
	v->units = unit;

        v->xres = ((((double) DisplayWidth(dpy,screen)) * unit * 1000.0) /
            ((double) DisplayWidthMM(dpy,screen)));
        v->yres = ((((double) DisplayHeight(dpy,screen)) * unit * 1000.0) /
            ((double) DisplayHeightMM(dpy,screen)));
/*
 *  Use that info to set scaling factors.
 */

	v->Scale.x = v->xres * dist * scale * 4;
	v->Scale.y = v->yres * dist * scale * 4;
	v->Scale.z = 1.0;

	v->Middl.x = width * 2;
	v->Middl.y = height * 2;

/*
 *  Build the clipping planes for our view into the eye space.
 */

	clip[0].x = - width / v->xres / 2.0 / scale;
	clip[0].y = - height / v->yres / 2.0 / scale;
        clip[0].z = dist;
        clip[1].x = - width / v->xres / 2.0 / scale;
        clip[1].y = height / v->yres / 2.0 / scale;
        clip[1].z = dist;
        clip[2].x = width / v->xres / 2.0 / scale;
        clip[2].y = height / v->yres / 2.0 / scale;
        clip[2].z = dist;
        clip[3].x = width / v->xres / 2.0 / scale;
        clip[3].y = - height / v->yres / 2.0 / scale;
        clip[3].z = dist;

        v->clipPoly = VCreatePolygon (4, clip, (VColor *) 0);
        VGetPlanes (v->clipPoly);
	VComputeClipNormals (v);

/*
 *  Fill out the rest of the structure.
 */

	v->flags = VPPerspective | VPClip | VPFastAnimation;
	v->dpy = dpy;
	v->cmap = cmap;
	v->visual = visual;
	v->screen = screen;
	v->win = win;
	VIdentMatrix (&v->eyeSpace);
	v->set = 0;
	v->width = width;
	v->height = height;
	v->aPixel = (unsigned long *) Vmalloc(sizeof(*v->aPixel) * maxpcolors);
	memset(v->aPixel, 0, sizeof(*v->aPixel) * maxpcolors);
	for (i=0; i<MAXCOLORS; ++i) {
		v->bPixel[i] = 0;
	}

	v->colorTweakProc = (void (*)()) NULL;

/*
 *  Is this a monochrome situation?  We also resort to monochrome on
 *  color systems that probably won't have enough color cell space
 *  to support double buffering.  We'll also use pixmaps if the
 *  global variable "usePixmaps" is non-zero.
 */

	planes = DisplayPlanes (dpy, screen);
	if (planes < PLANES*2+2 || _VDefaultWorkContext->usePixmaps != 0) {
		if ((v->flags & VPFastAnimation) == 0) {
			v->monoPixmap = XCreatePixmap (dpy,
			    RootWindow(dpy, screen), width, height, planes);
			v->flags |= VPPixmap;
		}
		if (planes == 1 || _VDefaultWorkContext->usePixmaps != 0)
			v->flags |= VPMono;
		v->mask = AllPlanes;
	}

	v->w = InitializeX11AWindow (dpy, screen, win, 
		(v->flags & VPMono) ? AMonoMode : 0);

/*
 *  Create the halftone bitmaps
 */

	v->halftone[0] = XCreateBitmapFromData (dpy, win, gray0_bits,
		gray0_width, gray0_height);
	v->halftone[1] = XCreateBitmapFromData (dpy, win, gray1_bits,
		gray1_width, gray1_height);
	v->halftone[2] = XCreateBitmapFromData (dpy, win, gray2_bits,
		gray2_width, gray2_height);
	v->halftone[3] = XCreateBitmapFromData (dpy, win, gray3_bits,
		gray3_width, gray3_height);
	v->halftone[4] = XCreateBitmapFromData (dpy, win, gray4_bits,
		gray4_width, gray4_height);
	v->halftone[5] = XCreateBitmapFromData (dpy, win, gray5_bits,
		gray5_width, gray5_height);
	v->halftone[6] = XCreateBitmapFromData (dpy, win, gray6_bits,
		gray6_width, gray6_height);
	v->halftone[7] = XCreateBitmapFromData (dpy, win, gray7_bits,
		gray7_width, gray7_height);

	v->w->stipple = v->halftone;

	return v;
}
