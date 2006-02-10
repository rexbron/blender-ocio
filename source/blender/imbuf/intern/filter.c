/**
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 * filter.c
 *
 * $Id$
 */

#include "BLI_blenlib.h"

#include "imbuf.h"
#include "imbuf_patch.h"
#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"
#include "IMB_filter.h"


/************************************************************************/
/*				FILTERS					*/
/************************************************************************/

static void filtrow(unsigned char *point, int x)
{
	unsigned int c1,c2,c3,error;

	if (x>1){
		c1 = c2 = *point;
		error = 2;
		for(x--;x>0;x--){
			c3 = point[4];
			c1 += (c2<<1) + c3 + error;
			error = c1 & 3;
			*point = c1 >> 2;
			point += 4;
			c1=c2;
			c2=c3;
		}
		*point = (c1 + (c2<<1) + c2 + error) >> 2;
	}
}

static void filtrowf(float *point, int x)
{
	float c1,c2,c3;
	
	if (x>1){
		c1 = c2 = *point;
		for(x--;x>0;x--){
			c3 = point[4];
			c1 += (c2 * 2) + c3;
			*point = 0.25f*c1;
			point += 4;
			c1=c2;
			c2=c3;
		}
		*point = 0.25f*(c1 + (c2 * 2) + c2);
	}
}



static void filtcolum(unsigned char *point, int y, int skip)
{
	unsigned int c1,c2,c3,error;
	unsigned char *point2;

	if (y>1){
		c1 = c2 = *point;
		point2 = point;
		error = 2;
		for(y--;y>0;y--){
			point2 += skip;
			c3 = *point2;
			c1 += (c2<<1) + c3 +error;
			error = c1 & 3;
			*point = c1 >> 2;
			point=point2;
			c1=c2;
			c2=c3;
		}
		*point = (c1 + (c2<<1) + c2 + error) >> 2;
	}
}

static void filtcolumf(float *point, int y, int skip)
{
	float c1,c2,c3, *point2;
	
	if (y>1){
		c1 = c2 = *point;
		point2 = point;
		for(y--;y>0;y--){
			point2 += skip;
			c3 = *point2;
			c1 += (c2 * 2) + c3;
			*point = 0.25f*c1;
			point=point2;
			c1=c2;
			c2=c3;
		}
		*point = 0.25f*(c1 + (c2 * 2) + c2);
	}
}

void IMB_filtery(struct ImBuf *ibuf)
{
	unsigned char *point;
	float *pointf;
	int x, y, skip, do_float = 0;

	point = (unsigned char *)ibuf->rect;
	pointf = ibuf->rect_float;

	if (ibuf->rect_float != NULL) do_float = 1;

	x = ibuf->x;
	y = ibuf->y;
	skip = x<<2;

	for (;x>0;x--){
		if (ibuf->depth > 24) filtcolum(point,y,skip);
		point++;
		filtcolum(point,y,skip);
		point++;
		filtcolum(point,y,skip);
		point++;
		filtcolum(point,y,skip);
		point++;
		if (do_float) {
			if (ibuf->depth > 24) filtcolumf(pointf,y,skip);
			pointf++;
			filtcolumf(pointf,y,skip);
			pointf++;
			filtcolumf(pointf,y,skip);
			pointf++;
			filtcolumf(pointf,y,skip);
			pointf++;
		}
	}
}


void imb_filterx(struct ImBuf *ibuf)
{
	unsigned char *point;
	float *pointf;
	int x, y, skip, do_float =0;

	point = (unsigned char *)ibuf->rect;
	pointf = ibuf->rect_float;

	if (ibuf->rect_float != NULL) do_float = 1;

	x = ibuf->x;
	y = ibuf->y;
	skip = (x<<2) - 3;

	for (;y>0;y--){
		if (ibuf->depth > 24) filtrow(point,x);
		point++;
		filtrow(point,x);
		point++;
		filtrow(point,x);
		point++;
		filtrow(point,x);
		point+=skip;
		if (do_float) {
			if (ibuf->depth > 24) filtrowf(pointf,x);
			pointf++;
			filtrowf(pointf,x);
			pointf++;
			filtrowf(pointf,x);
			pointf++;
			filtrowf(pointf,x);
			pointf+=skip;
		}
	}
}

void IMB_filterN(ImBuf *out, ImBuf *in)
{
	register char *row1, *row2, *row3;
	register char *cp;
	int rowlen, x, y;
	
	rowlen= in->x;
	
	for(y=2; y<in->y; y++) {
		/* setup rows */
		row1= (char *)(in->rect + (y-2)*rowlen);
		row2= row1 + 4*rowlen;
		row3= row2 + 4*rowlen;
		
		cp= (char *)(out->rect + (y-1)*rowlen);
		cp[0]= row2[0];
		cp[1]= row2[1];
		cp[2]= row2[2];
		cp[3]= row2[3];
		cp+= 4;
		
		for(x=2; x<rowlen; x++) {
			cp[0]= (row1[0] + 2*row1[4] + row1[8] + 2*row2[0] + 4*row2[4] + 2*row2[8] + row3[0] + 2*row3[4] + row3[8])>>4;
			cp[1]= (row1[1] + 2*row1[5] + row1[9] + 2*row2[1] + 4*row2[5] + 2*row2[9] + row3[1] + 2*row3[5] + row3[9])>>4;
			cp[2]= (row1[2] + 2*row1[6] + row1[10] + 2*row2[2] + 4*row2[6] + 2*row2[10] + row3[2] + 2*row3[6] + row3[10])>>4;
			cp[3]= (row1[3] + 2*row1[7] + row1[11] + 2*row2[3] + 4*row2[7] + 2*row2[11] + row3[3] + 2*row3[7] + row3[11])>>4;
			cp+=4; row1+=4; row2+=4; row3+=4;
		}
	}
}

void IMB_filter(struct ImBuf *ibuf)
{
	IMB_filtery(ibuf);
	imb_filterx(ibuf);
}
