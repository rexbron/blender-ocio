/*
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
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 * allocimbuf.c
 *
 * $Id$
 */

/** \file blender/imbuf/intern/divers.c
 *  \ingroup imbuf
 */


#include "BLI_blenlib.h"
#include "BLI_rand.h"
#include "BLI_math.h"
#include "BLI_utildefines.h"

#include "imbuf.h"
#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"
#include "IMB_allocimbuf.h"

#include "BKE_colortools.h"
#include "BKE_colormanagement.h"

#include "MEM_guardedalloc.h"

void IMB_de_interlace(struct ImBuf *ibuf)
{
	struct ImBuf * tbuf1, * tbuf2;
	
	if (ibuf == NULL) return;
	if (ibuf->flags & IB_fields) return;
	ibuf->flags |= IB_fields;
	
	if (ibuf->rect) {
		/* make copies */
		tbuf1 = IMB_allocImBuf(ibuf->x, ibuf->y / 2, 32, IB_rect);
		tbuf2 = IMB_allocImBuf(ibuf->x, ibuf->y / 2, 32, IB_rect);
		
		ibuf->x *= 2;	
		IMB_rectcpy(tbuf1, ibuf, 0, 0, 0, 0, ibuf->x, ibuf->y);
		IMB_rectcpy(tbuf2, ibuf, 0, 0, tbuf2->x, 0, ibuf->x, ibuf->y);
	
		ibuf->x /= 2;
		IMB_rectcpy(ibuf, tbuf1, 0, 0, 0, 0, tbuf1->x, tbuf1->y);
		IMB_rectcpy(ibuf, tbuf2, 0, tbuf2->y, 0, 0, tbuf2->x, tbuf2->y);
		
		IMB_freeImBuf(tbuf1);
		IMB_freeImBuf(tbuf2);
	}
	ibuf->y /= 2;
}

void IMB_interlace(struct ImBuf *ibuf)
{
	struct ImBuf * tbuf1, * tbuf2;

	if (ibuf == NULL) return;
	ibuf->flags &= ~IB_fields;

	ibuf->y *= 2;

	if (ibuf->rect) {
		/* make copies */
		tbuf1 = IMB_allocImBuf(ibuf->x, ibuf->y / 2, 32, IB_rect);
		tbuf2 = IMB_allocImBuf(ibuf->x, ibuf->y / 2, 32, IB_rect);

		IMB_rectcpy(tbuf1, ibuf, 0, 0, 0, 0, ibuf->x, ibuf->y);
		IMB_rectcpy(tbuf2, ibuf, 0, 0, 0, tbuf2->y, ibuf->x, ibuf->y);

		ibuf->x *= 2;
		IMB_rectcpy(ibuf, tbuf1, 0, 0, 0, 0, tbuf1->x, tbuf1->y);
		IMB_rectcpy(ibuf, tbuf2, tbuf2->x, 0, 0, 0, tbuf2->x, tbuf2->y);
		ibuf->x /= 2;

		IMB_freeImBuf(tbuf1);
		IMB_freeImBuf(tbuf2);
	}
}


static void imb_float_from_rect_nonlinear(struct ImBuf *ibuf, float *fbuf)
{
	float *tof = fbuf;
	int i;
	unsigned char *to = (unsigned char *) ibuf->rect;

	for (i = ibuf->x * ibuf->y; i > 0; i--) 
	{
		tof[0] = ((float)to[0])*(1.0f/255.0f);
		tof[1] = ((float)to[1])*(1.0f/255.0f);
		tof[2] = ((float)to[2])*(1.0f/255.0f);
		tof[3] = ((float)to[3])*(1.0f/255.0f);
		to += 4; 
		tof += 4;
	}
}

static void imb_rect_from_float_nonlinear(struct ImBuf *ibuf, char *cbuf)
{
	char *toc = cbuf;
	int i;
	float *to = (float *) ibuf->rect_float;

	for (i = ibuf->x * ibuf->y; i > 0; i--) 
	{
		toc[0] = FTOCHAR(to[0]);
		toc[1] = FTOCHAR(to[1]);
		toc[2] = FTOCHAR(to[2]);
		toc[3] = FTOCHAR(to[3]);
		to += 4; 
		toc += 4;
	}
}

/* no profile conversion */
void IMB_float_from_rect_simple(struct ImBuf *ibuf)
{
	if(ibuf->rect_float==NULL)
		imb_addrectfloatImBuf(ibuf);
	imb_float_from_rect_nonlinear(ibuf, ibuf->rect_float);
	ibuf->is_float_linear = 0;
}

void IMB_rect_from_float_simple(struct ImBuf *ibuf)
{
	if(ibuf->rect==NULL)
		imb_addrectImBuf(ibuf);
	imb_rect_from_float_nonlinear(ibuf, (char*)ibuf->rect);
}



/* no profile conversion */
void IMB_color_to_bw(struct ImBuf *ibuf)
{
	float *rctf= ibuf->rect_float;
	unsigned char *rct= (unsigned char *)ibuf->rect;
	int i;
	if(rctf) {
		for (i = ibuf->x * ibuf->y; i > 0; i--, rctf+=4) {
			rctf[0]= rctf[1]= rctf[2]= rgb_to_grayscale(rctf);
		}
	}

	if(rct) {
		for (i = ibuf->x * ibuf->y; i > 0; i--, rct+=4) {
			rct[0]= rct[1]= rct[2]= rgb_to_grayscale_byte(rct);
		}
	}
}
