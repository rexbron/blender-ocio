/*
 * $Id$
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 
 * The Original Code is: some of this file.
 *
 * ***** END GPL LICENSE BLOCK *****
 * */

/** \file blender/blenlib/intern/math_color.c
 *  \ingroup bli
 */


#include <assert.h>

#include "BLI_math.h"

void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b)
{
	int i;
	float f, p, q, t;

	if(s==0.0f) {
		*r = v;
		*g = v;
		*b = v;
	}
	else {
		h= (h - floorf(h))*6.0f;

		i = (int)floorf(h);
		f = h - i;
		p = v*(1.0f-s);
		q = v*(1.0f-(s*f));
		t = v*(1.0f-(s*(1.0f-f)));
		
		switch (i) {
		case 0 :
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1 :
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2 :
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3 :
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4 :
			*r = t;
			*g = p;
			*b = v;
			break;
		case 5 :
			*r = v;
			*g = p;
			*b = q;
			break;
		}
	}
}

void rgb_to_yuv(float r, float g, float b, float *ly, float *lu, float *lv)
{
	float y, u, v;
	y= 0.299f*r + 0.587f*g + 0.114f*b;
	u=-0.147f*r - 0.289f*g + 0.436f*b;
	v= 0.615f*r - 0.515f*g - 0.100f*b;
	
	*ly=y;
	*lu=u;
	*lv=v;
}

void yuv_to_rgb(float y, float u, float v, float *lr, float *lg, float *lb)
{
	float r, g, b;
	r=y+1.140f*v;
	g=y-0.394f*u - 0.581f*v;
	b=y+2.032f*u;
	
	*lr=r;
	*lg=g;
	*lb=b;
}

/* The RGB inputs are supposed gamma corrected and in the range 0 - 1.0f */
/* Output YCC have a range of 16-235 and 16-240 except with JFIF_0_255 where the range is 0-255 */
void rgb_to_ycc(float r, float g, float b, float *ly, float *lcb, float *lcr, int colorspace)
{
	float sr,sg, sb;
	float y = 128.f, cr = 128.f, cb = 128.f;
	
	sr=255.0f*r;
	sg=255.0f*g;
	sb=255.0f*b;
	
	switch (colorspace) {
	case BLI_YCC_ITU_BT601 :
		y=(0.257f*sr)+(0.504f*sg)+(0.098f*sb)+16.0f;
		cb=(-0.148f*sr)-(0.291f*sg)+(0.439f*sb)+128.0f;
		cr=(0.439f*sr)-(0.368f*sg)-(0.071f*sb)+128.0f;
		break;
	case BLI_YCC_ITU_BT709 :
		y=(0.183f*sr)+(0.614f*sg)+(0.062f*sb)+16.0f;
		cb=(-0.101f*sr)-(0.338f*sg)+(0.439f*sb)+128.0f;
		cr=(0.439f*sr)-(0.399f*sg)-(0.040f*sb)+128.0f;
		break;
	case BLI_YCC_JFIF_0_255 :
		y=(0.299f*sr)+(0.587f*sg)+(0.114f*sb);
		cb=(-0.16874f*sr)-(0.33126f*sg)+(0.5f*sb)+128.0f;
		cr=(0.5f*sr)-(0.41869f*sg)-(0.08131f*sb)+128.0f;
		break;
	default:
		assert(!"invalid colorspace");
	}
	
	*ly=y;
	*lcb=cb;
	*lcr=cr;
}


/* YCC input have a range of 16-235 and 16-240 except with JFIF_0_255 where the range is 0-255 */
/* RGB outputs are in the range 0 - 1.0f */
/* FIXME comment above must be wrong because BLI_YCC_ITU_BT601 y 16.0 cr 16.0 -> r -0.7009 */
void ycc_to_rgb(float y, float cb, float cr, float *lr, float *lg, float *lb, int colorspace)
{
	float r = 128.f, g = 128.f, b = 128.f;
	
	switch (colorspace) {
	case BLI_YCC_ITU_BT601 :
		r=1.164f*(y-16.0f)+1.596f*(cr-128.0f);
		g=1.164f*(y-16.0f)-0.813f*(cr-128.0f)-0.392f*(cb-128.0f);
		b=1.164f*(y-16.0f)+2.017f*(cb-128.0f);
		break;
	case BLI_YCC_ITU_BT709 :
		r=1.164f*(y-16.0f)+1.793f*(cr-128.0f);
		g=1.164f*(y-16.0f)-0.534f*(cr-128.0f)-0.213f*(cb-128.0f);
		b=1.164f*(y-16.0f)+2.115f*(cb-128.0f);
		break;
	case BLI_YCC_JFIF_0_255 :
		r=y+1.402f*cr - 179.456f;
		g=y-0.34414f*cb - 0.71414f*cr + 135.45984f;
		b=y+1.772f*cb - 226.816f;
		break;
	default:
		assert(!"invalid colorspace");
	}
	*lr=r/255.0f;
	*lg=g/255.0f;
	*lb=b/255.0f;
}

void hex_to_rgb(char *hexcol, float *r, float *g, float *b)
{
	unsigned int ri, gi, bi;

	if (hexcol[0] == '#') hexcol++;

	if (sscanf(hexcol, "%02x%02x%02x", &ri, &gi, &bi)==3) {
		*r = ri / 255.0f;
		*g = gi / 255.0f;
		*b = bi / 255.0f;
		CLAMP(*r, 0.0f, 1.0f);
		CLAMP(*g, 0.0f, 1.0f);
		CLAMP(*b, 0.0f, 1.0f);
	}
	else {
		/* avoid using un-initialized vars */
		*r= *g= *b= 0.0f;
	}
}

void rgb_to_hsv(float r, float g, float b, float *lh, float *ls, float *lv)
{
	float h, s, v;
	float cmax, cmin, cdelta;
	float rc, gc, bc;

	cmax = r;
	cmin = r;
	cmax = (g>cmax ? g:cmax);
	cmin = (g<cmin ? g:cmin);
	cmax = (b>cmax ? b:cmax);
	cmin = (b<cmin ? b:cmin);

	v = cmax;		/* value */
	if (cmax != 0.0f)
		s = (cmax - cmin)/cmax;
	else {
		s = 0.0f;
	}
	if (s == 0.0f)
		h = -1.0f;
	else {
		cdelta = cmax-cmin;
		rc = (cmax-r)/cdelta;
		gc = (cmax-g)/cdelta;
		bc = (cmax-b)/cdelta;
		if (r==cmax)
			h = bc-gc;
		else
			if (g==cmax)
				h = 2.0f+rc-bc;
			else
				h = 4.0f+gc-rc;
		h = h*60.0f;
		if (h < 0.0f)
			h += 360.0f;
	}
	
	*ls = s;
	*lh = h / 360.0f;
	if(*lh < 0.0f) *lh= 0.0f;
	*lv = v;
}

void rgb_to_hsv_compat(float r, float g, float b, float *lh, float *ls, float *lv)
{
	float orig_h= *lh;
	float orig_s= *ls;

	rgb_to_hsv(r, g, b, lh, ls, lv);

	if(*lv <= 0.0f) {
		*lh= orig_h;
		*ls= orig_s;
	}
	else if (*ls <= 0.0f) {
		*lh= orig_h;
	}

	if(*lh==0.0f && orig_h >= 1.0f) {
		*lh= 1.0f;
	}
}

/*http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html */

void xyz_to_rgb(float xc, float yc, float zc, float *r, float *g, float *b, int colorspace)
{
	switch (colorspace) { 
	case BLI_XYZ_SMPTE:
		*r = (3.50570f	 * xc) + (-1.73964f	 * yc) + (-0.544011f * zc);
		*g = (-1.06906f	 * xc) + (1.97781f	 * yc) + (0.0351720f * zc);
		*b = (0.0563117f * xc) + (-0.196994f * yc) + (1.05005f	 * zc);
		break;
	case BLI_XYZ_REC709_SRGB:
		*r = (3.240476f	 * xc) + (-1.537150f * yc) + (-0.498535f * zc);
		*g = (-0.969256f * xc) + (1.875992f  * yc) + (0.041556f  * zc);
		*b = (0.055648f	 * xc) + (-0.204043f * yc) + (1.057311f  * zc);
		break;
	case BLI_XYZ_CIE:
		*r = (2.28783848734076f	* xc) + (-0.833367677835217f	* yc) + (-0.454470795871421f	* zc);
		*g = (-0.511651380743862f * xc) + (1.42275837632178f * yc) + (0.0888930017552939f * zc);
		*b = (0.00572040983140966f	* xc) + (-0.0159068485104036f	* yc) + (1.0101864083734f	* zc);
		break;
	}
}

/* we define a 'cpack' here as a (3 byte color code) number that can be expressed like 0xFFAA66 or so.
   for that reason it is sensitive for endianness... with this function it works correctly
*/

unsigned int hsv_to_cpack(float h, float s, float v)
{
	short r, g, b;
	float rf, gf, bf;
	unsigned int col;
	
	hsv_to_rgb(h, s, v, &rf, &gf, &bf);
	
	r= (short)(rf*255.0f);
	g= (short)(gf*255.0f);
	b= (short)(bf*255.0f);
	
	col= ( r + (g*256) + (b*256*256) );
	return col;
}


unsigned int rgb_to_cpack(float r, float g, float b)
{
	int ir, ig, ib;
	
	ir= (int)floor(255.0f*r);
	if(ir<0) ir= 0; else if(ir>255) ir= 255;
	ig= (int)floor(255.0f*g);
	if(ig<0) ig= 0; else if(ig>255) ig= 255;
	ib= (int)floor(255.0f*b);
	if(ib<0) ib= 0; else if(ib>255) ib= 255;
	
	return (ir+ (ig*256) + (ib*256*256));
}

void cpack_to_rgb(unsigned int col, float *r, float *g, float *b)
{
	
	*r= (float)((col)&0xFF);
	*r /= 255.0f;

	*g= (float)(((col)>>8)&0xFF);
	*g /= 255.0f;

	*b= (float)(((col)>>16)&0xFF);
	*b /= 255.0f;
}

void rgb_byte_to_float(const unsigned char *in, float *out)
{
	out[0]= ((float)in[0]) / 255.0f;
	out[1]= ((float)in[1]) / 255.0f;
	out[2]= ((float)in[2]) / 255.0f;
}

void rgb_float_to_byte(const float *in, unsigned char *out)
{
	int r, g, b;
	
	r= (int)(in[0] * 255.0f);
	g= (int)(in[1] * 255.0f);
	b= (int)(in[2] * 255.0f);
	
	out[0]= (char)((r <= 0)? 0 : (r >= 255)? 255 : r);
	out[1]= (char)((g <= 0)? 0 : (g >= 255)? 255 : g);
	out[2]= (char)((b <= 0)? 0 : (b >= 255)? 255 : b);
}


/* ********************************* color transforms ********************************* */


void gamma_correct(float *c, float gamma)
{
	*c = powf((*c), gamma);
}

float rec709_to_linearrgb(float c)
{
	if (c < 0.081f)
		return (c < 0.0f)? 0.0f: c * (1.0f/4.5f);
	else
		return powf((c + 0.099f)*(1.0f/1.099f), (1.0f/0.45f));
}

float linearrgb_to_rec709(float c)
{
	if (c < 0.018f)
		return (c < 0.0f)? 0.0f: c * 4.5f;
	else
		return 1.099f * powf(c, 0.45f) - 0.099f;
}

float srgb_to_linearrgb(float c)
{
	if (c < 0.04045f)
		return (c < 0.0f)? 0.0f: c * (1.0f/12.92f);
	else
		return powf((c + 0.055f)*(1.0f/1.055f), 2.4f);
}

float linearrgb_to_srgb(float c)
{
	if (c < 0.0031308f)
		return (c < 0.0f)? 0.0f: c * 12.92f;
	else
		return  1.055f * powf(c, 1.0f/2.4f) - 0.055f;
}

void srgb_to_linearrgb_v3_v3(float *col_to, float *col_from)
{
	col_to[0] = srgb_to_linearrgb(col_from[0]);
	col_to[1] = srgb_to_linearrgb(col_from[1]);
	col_to[2] = srgb_to_linearrgb(col_from[2]);
}

void linearrgb_to_srgb_v3_v3(float *col_to, float *col_from)
{
	col_to[0] = linearrgb_to_srgb(col_from[0]);
	col_to[1] = linearrgb_to_srgb(col_from[1]);
	col_to[2] = linearrgb_to_srgb(col_from[2]);
}



void minmax_rgb(short c[])
{
	if(c[0]>255) c[0]=255;
	else if(c[0]<0) c[0]=0;
	if(c[1]>255) c[1]=255;
	else if(c[1]<0) c[1]=0;
	if(c[2]>255) c[2]=255;
	else if(c[2]<0) c[2]=0;
}

/*If the requested RGB shade contains a negative weight for
  one of the primaries, it lies outside the color gamut 
  accessible from the given triple of primaries.  Desaturate
  it by adding white, equal quantities of R, G, and B, enough
  to make RGB all positive.  The function returns 1 if the
  components were modified, zero otherwise.*/
int constrain_rgb(float *r, float *g, float *b)
{
	float w;

	/* Amount of white needed is w = - min(0, *r, *g, *b) */

	w = (0 < *r) ? 0 : *r;
	w = (w < *g) ? w : *g;
	w = (w < *b) ? w : *b;
	w = -w;

	/* Add just enough white to make r, g, b all positive. */

	if (w > 0) {
		*r += w;  *g += w; *b += w;
		return 1;                     /* Color modified to fit RGB gamut */
	}

	return 0;                         /* Color within RGB gamut */
}

float rgb_to_grayscale(float rgb[3])
{
	return 0.3f*rgb[0] + 0.58f*rgb[1] + 0.12f*rgb[2];
}

unsigned char rgb_to_grayscale_byte(unsigned char rgb[3])
{
	return (76*(unsigned short)rgb[0] + 148*(unsigned short)rgb[1] + 31*(unsigned short)rgb[2]) / 255;
}

/* ********************************* lift/gamma/gain / ASC-CDL conversion ********************************* */

void lift_gamma_gain_to_asc_cdl(float *lift, float *gamma, float *gain, float *offset, float *slope, float *power)
{
	int c;
	for(c=0; c<3; c++) {
		offset[c]= lift[c]*gain[c];
		slope[c]=  gain[c]*(1.0f-lift[c]);
		if(gamma[c] == 0)
			power[c]= FLT_MAX;
		else
			power[c]= 1.0f/gamma[c];
	}
}

/* ******************************************** other ************************************************* */

/* Applies an hue offset to a float rgb color */
void rgb_float_set_hue_float_offset(float rgb[3], float hue_offset)
{
	float hsv[3];
	
	rgb_to_hsv(rgb[0], rgb[1], rgb[2], hsv, hsv+1, hsv+2);
	
	hsv[0]+= hue_offset;
	if(hsv[0] > 1.0f)		hsv[0] -= 1.0f;
	else if(hsv[0] < 0.0f)	hsv[0] += 1.0f;
	
	hsv_to_rgb(hsv[0], hsv[1], hsv[2], rgb, rgb+1, rgb+2);
}

/* Applies an hue offset to a byte rgb color */
void rgb_byte_set_hue_float_offset(unsigned char rgb[3], float hue_offset)
{
	float rgb_float[3];
	
	rgb_byte_to_float(rgb, rgb_float);
	rgb_float_set_hue_float_offset(rgb_float, hue_offset);
	rgb_float_to_byte(rgb_float, rgb);
}
