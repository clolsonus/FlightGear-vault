/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991,1992  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */
typedef struct {
	int	xorg;		/* x loc of bottom of scale */
	int	yorg;		/* y loc of bottom of scale */
	int	length;		/* length of scale (pixels) */
	int	orientation;	/* orientation flags */
	double	scale;		/* units per pixel */
	int	minorInterval;	/* units per minor tick */
	int	minorSize;	/* width of minor ticks (pixels) */
	int	majorInterval;	/* units per major tick */
	int	majorSize;	/* width of major ticks (pixels) */
	int	indexSize;	/* width of index (pixels) */
	double	divisor;	/* divisor on digit scale */
	char	*format;	/* output format */
	int	pixel;		/* pixel value to use when drawing */
	int	fontSize;	/* font size (pixels) */
	}	scaleControl;

#define orientRight		1
#define orientHorizontal	2
