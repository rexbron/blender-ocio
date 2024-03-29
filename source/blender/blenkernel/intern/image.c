/*  image.c
 * 
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
 *
 * Contributor(s): Blender Foundation, 2006, full recode
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/blenkernel/intern/image.c
 *  \ingroup bke
 */


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#ifndef WIN32 
#include <unistd.h>
#else
#include <io.h>
#endif

#include <time.h>

#ifdef _WIN32
#define open _open
#define close _close
#endif

#include "MEM_guardedalloc.h"

#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"

#ifdef WITH_OPENEXR
#include "intern/openexr/openexr_multi.h"
#endif

#include "DNA_packedFile_types.h"
#include "DNA_scene_types.h"
#include "DNA_object_types.h"
#include "DNA_camera_types.h"
#include "DNA_sequence_types.h"
#include "DNA_userdef_types.h"

#include "BLI_blenlib.h"
#include "BLI_threads.h"
#include "BLI_utildefines.h"

#include "BKE_bmfont.h"
#include "BKE_global.h"
#include "BKE_icons.h"
#include "BKE_image.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_packedFile.h"
#include "BKE_scene.h"
#include "BKE_node.h"
#include "BKE_sequencer.h" /* seq_foreground_frame_get() */
#include "BKE_utildefines.h"
#include "BKE_colormanagement.h"

#include "BLF_api.h"

#include "PIL_time.h"

#include "RE_pipeline.h"

#include "GPU_draw.h"

#include "BLO_sys_types.h" // for intptr_t support

/* max int, to indicate we don't store sequences in ibuf */
#define IMA_NO_INDEX	0x7FEFEFEF

/* quick lookup: supports 1 million frames, thousand passes */
#define IMA_MAKE_INDEX(frame, index)	((frame)<<10)+index
#define IMA_INDEX_FRAME(index)			(index>>10)
#define IMA_INDEX_PASS(index)			(index & ~1023)

/* ******** IMAGE PROCESSING ************* */

static void de_interlace_ng(struct ImBuf *ibuf)	/* neogeo fields */
{
	struct ImBuf * tbuf1, * tbuf2;
	
	if (ibuf == NULL) return;
	if (ibuf->flags & IB_fields) return;
	ibuf->flags |= IB_fields;
	
	if (ibuf->rect) {
		/* make copies */
		tbuf1 = IMB_allocImBuf(ibuf->x, (short)(ibuf->y >> 1), (unsigned char)32, (int)IB_rect);
		tbuf2 = IMB_allocImBuf(ibuf->x, (short)(ibuf->y >> 1), (unsigned char)32, (int)IB_rect);
		
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

static void de_interlace_st(struct ImBuf *ibuf)	/* standard fields */
{
	struct ImBuf * tbuf1, * tbuf2;
	
	if (ibuf == NULL) return;
	if (ibuf->flags & IB_fields) return;
	ibuf->flags |= IB_fields;
	
	if (ibuf->rect) {
		/* make copies */
		tbuf1 = IMB_allocImBuf(ibuf->x, (short)(ibuf->y >> 1), (unsigned char)32, IB_rect);
		tbuf2 = IMB_allocImBuf(ibuf->x, (short)(ibuf->y >> 1), (unsigned char)32, IB_rect);
		
		ibuf->x *= 2;
		
		IMB_rectcpy(tbuf1, ibuf, 0, 0, 0, 0, ibuf->x, ibuf->y);
		IMB_rectcpy(tbuf2, ibuf, 0, 0, tbuf2->x, 0, ibuf->x, ibuf->y);
		
		ibuf->x /= 2;
		IMB_rectcpy(ibuf, tbuf2, 0, 0, 0, 0, tbuf2->x, tbuf2->y);
		IMB_rectcpy(ibuf, tbuf1, 0, tbuf2->y, 0, 0, tbuf1->x, tbuf1->y);
		
		IMB_freeImBuf(tbuf1);
		IMB_freeImBuf(tbuf2);
	}
	ibuf->y /= 2;
}

void image_de_interlace(Image *ima, int odd)
{
	ImBuf *ibuf= BKE_image_get_ibuf(ima, NULL);
	if(ibuf) {
		if(odd)
			de_interlace_st(ibuf);
		else
			de_interlace_ng(ibuf);
	}
}

/* ***************** ALLOC & FREE, DATA MANAGING *************** */

static void image_free_buffers(Image *ima)
{
	ImBuf *ibuf;
	
	while((ibuf = ima->ibufs.first)) {
		BLI_remlink(&ima->ibufs, ibuf);
		
		if (ibuf->userdata) {
			MEM_freeN(ibuf->userdata);
			ibuf->userdata = NULL;
		}
		IMB_freeImBuf(ibuf);
	}
	
	if(ima->anim) IMB_free_anim(ima->anim);
	ima->anim= NULL;

	if(ima->rr) {
		RE_FreeRenderResult(ima->rr);
		ima->rr= NULL;
	}	
	
	GPU_free_image(ima);
	
	ima->ok= IMA_OK;
}

/* called by library too, do not free ima itself */
void free_image(Image *ima)
{
	int a;

	image_free_buffers(ima);
	if (ima->packedfile) {
		freePackedFile(ima->packedfile);
		ima->packedfile = NULL;
	}
	BKE_icon_delete(&ima->id);
	ima->id.icon_id = 0;

	BKE_previewimg_free(&ima->preview);

	for(a=0; a<IMA_MAX_RENDER_SLOT; a++) {
		if(ima->renders[a]) {
			RE_FreeRenderResult(ima->renders[a]);
			ima->renders[a]= NULL;
		}
	}
}

/* only image block itself */
static Image *image_alloc(const char *name, short source, short type)
{
	Image *ima;
	
	ima= alloc_libblock(&G.main->image, ID_IM, name);
	if(ima) {
		ima->ok= IMA_OK;
		
		ima->xrep= ima->yrep= 1;
		ima->aspx= ima->aspy= 1.0;
		ima->gen_x= 1024; ima->gen_y= 1024;
		ima->gen_type= 1;	/* no defines yet? */
		
		ima->source= source;
		ima->type= type;
	}
	return ima;
}

/* get the ibuf from an image cache, local use here only */
static ImBuf *image_get_ibuf(Image *ima, int index, int frame)
{
	/* this function is intended to be thread safe. with IMA_NO_INDEX this
	 * should be OK, but when iterating over the list this is more tricky
	 * */
	if(index==IMA_NO_INDEX)
		return ima->ibufs.first;
	else {
		ImBuf *ibuf;

		index= IMA_MAKE_INDEX(frame, index);
		for(ibuf= ima->ibufs.first; ibuf; ibuf= ibuf->next)
			if(ibuf->index==index)
				return ibuf;

		return NULL;
	}
}

/* no ima->ibuf anymore, but listbase */
static void image_remove_ibuf(Image *ima, ImBuf *ibuf)
{
	if(ibuf) {
		BLI_remlink(&ima->ibufs, ibuf);
		IMB_freeImBuf(ibuf);
	}
}


/* no ima->ibuf anymore, but listbase */
static void image_assign_ibuf(Image *ima, ImBuf *ibuf, int index, int frame)
{
	if(ibuf) {
		ImBuf *link;
		
		if(index!=IMA_NO_INDEX)
			index= IMA_MAKE_INDEX(frame, index);
		
		/* insert based on index */
		for(link= ima->ibufs.first; link; link= link->next)
			if(link->index>=index)
				break;

		ibuf->index= index;

		/* this function accepts link==NULL */
		BLI_insertlinkbefore(&ima->ibufs, link, ibuf);

		/* now we don't want copies? */
		if(link && ibuf->index==link->index)
			image_remove_ibuf(ima, link);
	}
}

/* empty image block, of similar type and filename */
Image *copy_image(Image *ima)
{
	Image *nima= image_alloc(ima->id.name+2, ima->source, ima->type);

	BLI_strncpy(nima->name, ima->name, sizeof(ima->name));

	nima->flag= ima->flag;
	nima->tpageflag= ima->tpageflag;
	
	nima->gen_x= ima->gen_x;
	nima->gen_y= ima->gen_y;
	nima->gen_type= ima->gen_type;

	nima->animspeed= ima->animspeed;

	nima->aspx= ima->aspx;
	nima->aspy= ima->aspy;

	return nima;
}

void BKE_image_merge(Image *dest, Image *source)
{
	ImBuf *ibuf;
	
	/* sanity check */
	if(dest && source && dest!=source) {
	
		while((ibuf= source->ibufs.first)) {
			BLI_remlink(&source->ibufs, ibuf);
			image_assign_ibuf(dest, ibuf, IMA_INDEX_PASS(ibuf->index), IMA_INDEX_FRAME(ibuf->index));
		}
		
		free_libblock(&G.main->image, source);
	}
}


/* checks if image was already loaded, then returns same image */
/* otherwise creates new. */
/* does not load ibuf itself */
/* pass on optional frame for #name images */
Image *BKE_add_image_file(const char *name)
{
	Image *ima;
	int file, len;
	const char *libname;
	char str[FILE_MAX], strtest[FILE_MAX];
	
	BLI_strncpy(str, name, sizeof(str));
	BLI_path_abs(str, G.main->name);
	
	/* exists? */
	file= open(str, O_BINARY|O_RDONLY);
	if(file== -1) return NULL;
	close(file);
	
	/* first search an identical image */
	for(ima= G.main->image.first; ima; ima= ima->id.next) {
		if(ima->source!=IMA_SRC_VIEWER && ima->source!=IMA_SRC_GENERATED) {
			BLI_strncpy(strtest, ima->name, sizeof(ima->name));
			BLI_path_abs(strtest, G.main->name);
			
			if( strcmp(strtest, str)==0 ) {
				if(ima->anim==NULL || ima->id.us==0) {
					BLI_strncpy(ima->name, name, sizeof(ima->name));	/* for stringcode */
					ima->id.us++;										/* officially should not, it doesn't link here! */
					if(ima->ok==0)
						ima->ok= IMA_OK;
			/* RETURN! */
					return ima;
				}
			}
		}
	}
	/* add new image */
	
	/* create a short library name */
	len= strlen(name);
	
	while (len > 0 && name[len - 1] != '/' && name[len - 1] != '\\') len--;
	libname= name+len;
	
	ima= image_alloc(libname, IMA_SRC_FILE, IMA_TYPE_IMAGE);
	BLI_strncpy(ima->name, name, sizeof(ima->name));
	
	if(BLI_testextensie_array(name, imb_ext_movie))
		ima->source= IMA_SRC_MOVIE;
	
	return ima;
}

static ImBuf *add_ibuf_size(unsigned int width, unsigned int height, const char *name, int depth, int floatbuf, short uvtestgrid, float color[4])
{
	ImBuf *ibuf;
	unsigned char *rect= NULL;
	float *rect_float= NULL;
	
	if (floatbuf) {
		ibuf= IMB_allocImBuf(width, height, depth, IB_rectfloat);
		rect_float= (float*)ibuf->rect_float;
	}
	else {
		ibuf= IMB_allocImBuf(width, height, depth, IB_rect);
		rect= (unsigned char*)ibuf->rect;
	}
	
	BLI_strncpy(ibuf->name, name, sizeof(ibuf->name));
	ibuf->userflags |= IB_BITMAPDIRTY;
	
	switch(uvtestgrid) {
	case 1:
		BKE_image_buf_fill_checker(rect, rect_float, width, height);
		break;
	case 2:
		BKE_image_buf_fill_checker_color(rect, rect_float, width, height);
		break;
	default:
		BKE_image_buf_fill_color(rect, rect_float, width, height, color);
	}

	return ibuf;
}

/* adds new image block, creates ImBuf and initializes color */
Image *BKE_add_image_size(unsigned int width, unsigned int height, const char *name, int depth, int floatbuf, short uvtestgrid, float color[4])
{
	/* on save, type is changed to FILE in editsima.c */
	Image *ima= image_alloc(name, IMA_SRC_GENERATED, IMA_TYPE_UV_TEST);
	
	if (ima) {
		ImBuf *ibuf;
		
		BLI_strncpy(ima->name, name, FILE_MAX);
		ima->gen_x= width;
		ima->gen_y= height;
		ima->gen_type= uvtestgrid;
		ima->gen_flag |= (floatbuf ? IMA_GEN_FLOAT : 0);
		
		ibuf= add_ibuf_size(width, height, name, depth, floatbuf, uvtestgrid, color);
		image_assign_ibuf(ima, ibuf, IMA_NO_INDEX, 0);
		
		ima->ok= IMA_OK_LOADED;
	}

	return ima;
}

/* creates an image image owns the imbuf passed */
Image *BKE_add_image_imbuf(ImBuf *ibuf)
{
	/* on save, type is changed to FILE in editsima.c */
	Image *ima;

	ima= image_alloc(BLI_path_basename(ibuf->name), IMA_SRC_FILE, IMA_TYPE_IMAGE);

	if (ima) {
		BLI_strncpy(ima->name, ibuf->name, FILE_MAX);
		image_assign_ibuf(ima, ibuf, IMA_NO_INDEX, 0);
		ima->ok= IMA_OK_LOADED;
	}

	return ima;
}

/* packs rect from memory as PNG */
void BKE_image_memorypack(Image *ima)
{
	ImBuf *ibuf= image_get_ibuf(ima, IMA_NO_INDEX, 0);
	
	if(ibuf==NULL)
		return;
	if (ima->packedfile) {
		freePackedFile(ima->packedfile);
		ima->packedfile = NULL;
	}
	
	ibuf->ftype= PNG;
	ibuf->depth= 32;
	
	IMB_saveiff(ibuf, ibuf->name, IB_rect | IB_mem);
	if(ibuf->encodedbuffer==NULL) {
		printf("memory save for pack error\n");
	}
	else {
		PackedFile *pf = MEM_callocN(sizeof(*pf), "PackedFile");
		
		pf->data = ibuf->encodedbuffer;
		pf->size = ibuf->encodedsize;
		ima->packedfile= pf;
		ibuf->encodedbuffer= NULL;
		ibuf->encodedsize= 0;
		ibuf->userflags &= ~IB_BITMAPDIRTY;
		
		if(ima->source==IMA_SRC_GENERATED) {
			ima->source= IMA_SRC_FILE;
			ima->type= IMA_TYPE_IMAGE;
		}
	}
}

void tag_image_time(Image *ima)
{
	if (ima)
		ima->lastused = (int)PIL_check_seconds_timer();
}

#if 0
static void tag_all_images_time() 
{
	Image *ima;
	int ctime = (int)PIL_check_seconds_timer();

	ima= G.main->image.first;
	while(ima) {
		if(ima->bindcode || ima->repbind || ima->ibufs.first) {
			ima->lastused = ctime;
		}
	}
}
#endif

void free_old_images(void)
{
	Image *ima;
	static int lasttime = 0;
	int ctime = (int)PIL_check_seconds_timer();
	
	/* 
	   Run garbage collector once for every collecting period of time 
	   if textimeout is 0, that's the option to NOT run the collector
	*/
	if (U.textimeout == 0 || ctime % U.texcollectrate || ctime == lasttime)
		return;

	/* of course not! */
	if (G.rendering)
		return;
	
	lasttime = ctime;

	ima= G.main->image.first;
	while(ima) {
		if((ima->flag & IMA_NOCOLLECT)==0 && ctime - ima->lastused > U.textimeout) {
			/*
			   If it's in GL memory, deallocate and set time tag to current time
			   This gives textures a "second chance" to be used before dying.
			*/
			if(ima->bindcode || ima->repbind) {
				GPU_free_image(ima);
				ima->lastused = ctime;
			}
			/* Otherwise, just kill the buffers */
			else if (ima->ibufs.first) {
				image_free_buffers(ima);
			}
		}
		ima = ima->id.next;
	}
}

static uintptr_t image_mem_size(Image *ima)
{
	ImBuf *ibuf, *ibufm;
	int level;
	uintptr_t size = 0;

	size= 0;
	
	/* viewers have memory depending on other rules, has no valid rect pointer */
	if(ima->source==IMA_SRC_VIEWER)
		return 0;
	
	for(ibuf= ima->ibufs.first; ibuf; ibuf= ibuf->next) {
		if(ibuf->rect) size += MEM_allocN_len(ibuf->rect);
		else if(ibuf->rect_float) size += MEM_allocN_len(ibuf->rect_float);

		for(level=0; level<IB_MIPMAP_LEVELS; level++) {
			ibufm= ibuf->mipmap[level];
			if(ibufm) {
				if(ibufm->rect) size += MEM_allocN_len(ibufm->rect);
				else if(ibufm->rect_float) size += MEM_allocN_len(ibufm->rect_float);
			}
		}
	}

	return size;
}

void BKE_image_print_memlist(void)
{
	Image *ima;
	uintptr_t size, totsize= 0;

	for(ima= G.main->image.first; ima; ima= ima->id.next)
		totsize += image_mem_size(ima);

	printf("\ntotal image memory len: %.3lf MB\n", (double)totsize/(double)(1024*1024));

	for(ima= G.main->image.first; ima; ima= ima->id.next) {
		size= image_mem_size(ima);

		if(size)
			printf("%s len: %.3f MB\n", ima->id.name+2, (double)size/(double)(1024*1024));
	}
}

void BKE_image_free_all_textures(void)
{
	Tex *tex;
	Image *ima;
	/* unsigned int totsize= 0; */
	
	for(ima= G.main->image.first; ima; ima= ima->id.next)
		ima->id.flag &= ~LIB_DOIT;
	
	for(tex= G.main->tex.first; tex; tex= tex->id.next)
		if(tex->ima)
			tex->ima->id.flag |= LIB_DOIT;
	
	for(ima= G.main->image.first; ima; ima= ima->id.next) {
		if(ima->ibufs.first && (ima->id.flag & LIB_DOIT)) {
			ImBuf *ibuf;
			
			for(ibuf= ima->ibufs.first; ibuf; ibuf= ibuf->next) {
				/* escape when image is painted on */
				if(ibuf->userflags & IB_BITMAPDIRTY)
					break;
				
				/* if(ibuf->mipmap[0]) 
					totsize+= 1.33*ibuf->x*ibuf->y*4;
				else
					totsize+= ibuf->x*ibuf->y*4;*/
				
			}
			if(ibuf==NULL)
				image_free_buffers(ima);
		}
	}
	/* printf("freed total %d MB\n", totsize/(1024*1024)); */
}

/* except_frame is weak, only works for seqs without offset... */
void BKE_image_free_anim_ibufs(Image *ima, int except_frame)
{
	ImBuf *ibuf, *nbuf;

	for(ibuf= ima->ibufs.first; ibuf; ibuf= nbuf) {
		nbuf= ibuf->next;
		if(ibuf->userflags & IB_BITMAPDIRTY)
			continue;
		if(ibuf->index==IMA_NO_INDEX)
			continue;
		if(except_frame!=IMA_INDEX_FRAME(ibuf->index)) {
			BLI_remlink(&ima->ibufs, ibuf);
			
			if (ibuf->userdata) {
				MEM_freeN(ibuf->userdata);
				ibuf->userdata = NULL;
			}
			IMB_freeImBuf(ibuf);
		}					
	}
}

void BKE_image_all_free_anim_ibufs(int cfra)
{
	Image *ima;
	
	for(ima= G.main->image.first; ima; ima= ima->id.next)
		if(ELEM(ima->source, IMA_SRC_SEQUENCE, IMA_SRC_MOVIE))
			BKE_image_free_anim_ibufs(ima, cfra);
}


/* *********** READ AND WRITE ************** */

int BKE_imtype_to_ftype(int imtype)
{
	if(imtype==R_TARGA)
		return TGA;
	else if(imtype==R_RAWTGA)
		return RAWTGA;
	else if(imtype== R_IRIS) 
		return IMAGIC;
#ifdef WITH_HDR
	else if (imtype==R_RADHDR)
		return RADHDR;
#endif
	else if (imtype==R_PNG)
		return PNG;
#ifdef WITH_DDS
	else if (imtype==R_DDS)
		return DDS;
#endif
	else if (imtype==R_BMP)
		return BMP;
#ifdef WITH_TIFF
	else if (imtype==R_TIFF)
		return TIF;
#endif
	else if (imtype==R_OPENEXR || imtype==R_MULTILAYER)
		return OPENEXR;
#ifdef WITH_CINEON
	else if (imtype==R_CINEON)
		return CINEON;
	else if (imtype==R_DPX)
		return DPX;
#endif
#ifdef WITH_OPENJPEG
	else if(imtype==R_JP2)
		return JP2;
#endif
	else
		return JPG|90;
}

int BKE_ftype_to_imtype(int ftype)
{
	if(ftype==0)
		return TGA;
	else if(ftype == IMAGIC) 
		return R_IRIS;
#ifdef WITH_HDR
	else if (ftype & RADHDR)
		return R_RADHDR;
#endif
	else if (ftype & PNG)
		return R_PNG;
#ifdef WITH_DDS
	else if (ftype & DDS)
		return R_DDS;
#endif
	else if (ftype & BMP)
		return R_BMP;
#ifdef WITH_TIFF
	else if (ftype & TIF)
		return R_TIFF;
#endif
	else if (ftype & OPENEXR)
		return R_OPENEXR;
#ifdef WITH_CINEON
	else if (ftype & CINEON)
		return R_CINEON;
	else if (ftype & DPX)
		return R_DPX;
#endif
	else if (ftype & TGA)
		return R_TARGA;
	else if(ftype & RAWTGA)
		return R_RAWTGA;
#ifdef WITH_OPENJPEG
	else if(ftype & JP2)
		return R_JP2;
#endif
	else
		return R_JPEG90;
}


int BKE_imtype_is_movie(int imtype)
{
	switch(imtype) {
	case R_AVIRAW:
	case R_AVIJPEG:
	case R_AVICODEC:
	case R_QUICKTIME:
	case R_FFMPEG:
	case R_H264:
	case R_THEORA:
	case R_XVID:
	case R_FRAMESERVER:
			return 1;
	}
	return 0;
}

int BKE_add_image_extension(char *string, int imtype)
{
	const char *extension= NULL;
	
	if(imtype== R_IRIS) {
		if(!BLI_testextensie(string, ".rgb"))
			extension= ".rgb";
	}
	else if(imtype==R_IRIZ) {
		if(!BLI_testextensie(string, ".rgb"))
			extension= ".rgb";
	}
#ifdef WITH_HDR
	else if(imtype==R_RADHDR) {
		if(!BLI_testextensie(string, ".hdr"))
			extension= ".hdr";
	}
#endif
	else if (ELEM5(imtype, R_PNG, R_FFMPEG, R_H264, R_THEORA, R_XVID)) {
		if(!BLI_testextensie(string, ".png"))
			extension= ".png";
	}
#ifdef WITH_DDS
	else if(imtype==R_DDS) {
		if(!BLI_testextensie(string, ".dds"))
			extension= ".dds";
	}
#endif
	else if(imtype==R_RAWTGA) {
		if(!BLI_testextensie(string, ".tga"))
			extension= ".tga";
	}
	else if(imtype==R_BMP) {
		if(!BLI_testextensie(string, ".bmp"))
			extension= ".bmp";
	}
#ifdef WITH_TIFF
	else if(imtype==R_TIFF) {
		if(!BLI_testextensie(string, ".tif") && 
			!BLI_testextensie(string, ".tiff")) extension= ".tif";
	}
#endif
#ifdef WITH_OPENEXR
	else if( ELEM(imtype, R_OPENEXR, R_MULTILAYER)) {
		if(!BLI_testextensie(string, ".exr"))
			extension= ".exr";
	}
#endif
#ifdef WITH_CINEON
	else if(imtype==R_CINEON){
		if (!BLI_testextensie(string, ".cin"))
			extension= ".cin";
	}
	else if(imtype==R_DPX){
		if (!BLI_testextensie(string, ".dpx"))
			extension= ".dpx";
	}
#endif
	else if(imtype==R_TARGA) {
		if(!BLI_testextensie(string, ".tga"))
			extension= ".tga";
	}
#ifdef WITH_OPENJPEG
	else if(imtype==R_JP2) {
		if(!BLI_testextensie(string, ".jp2"))
			extension= ".jp2";
	}
#endif
	else { //   R_AVICODEC, R_AVIRAW, R_AVIJPEG, R_JPEG90, R_QUICKTIME etc
		if(!( BLI_testextensie(string, ".jpg") || BLI_testextensie(string, ".jpeg")))
			extension= ".jpg";
	}

	if(extension) {
		/* prefer this in many cases to avoid .png.tga, but in certain cases it breaks */
		/* remove any other known image extension */
		if(BLI_testextensie_array(string, imb_ext_image)
				  || (G.have_quicktime && BLI_testextensie_array(string, imb_ext_image_qt))) {
			return BLI_replace_extension(string, FILE_MAX, extension);
		} else {
			strcat(string, extension);
			return TRUE;
		}
		
	}
	else {
		return FALSE;
	}
}

/* could allow access externally - 512 is for long names, 64 is for id names */
typedef struct StampData {
	char 	file[512];
	char 	note[512];
	char 	date[512];
	char 	marker[512];
	char 	time[512];
	char 	frame[512];
	char 	camera[64];
	char 	cameralens[64];
	char 	scene[64];
	char 	strip[64];
	char 	rendertime[64];
} StampData;

static void stampdata(Scene *scene, Object *camera, StampData *stamp_data, int do_prefix)
{
	char text[256];
	struct tm *tl;
	time_t t;

	if (scene->r.stamp & R_STAMP_FILENAME) {
		BLI_snprintf(stamp_data->file, sizeof(stamp_data->file), do_prefix ? "File %s":"%s", G.relbase_valid ? G.main->name:"<untitled>");
	} else {
		stamp_data->file[0] = '\0';
	}
	
	if (scene->r.stamp & R_STAMP_NOTE) {
		/* Never do prefix for Note */
		BLI_snprintf(stamp_data->note, sizeof(stamp_data->note), "%s", scene->r.stamp_udata);
	} else {
		stamp_data->note[0] = '\0';
	}
	
	if (scene->r.stamp & R_STAMP_DATE) {
		t = time(NULL);
		tl = localtime(&t);
		BLI_snprintf(text, sizeof(text), "%04d/%02d/%02d %02d:%02d:%02d", tl->tm_year+1900, tl->tm_mon+1, tl->tm_mday, tl->tm_hour, tl->tm_min, tl->tm_sec);
		BLI_snprintf(stamp_data->date, sizeof(stamp_data->date), do_prefix ? "Date %s":"%s", text);
	} else {
		stamp_data->date[0] = '\0';
	}
	
	if (scene->r.stamp & R_STAMP_MARKER) {
		char *name = scene_find_last_marker_name(scene, CFRA);
	
		if (name)	strcpy(text, name);
		else 		strcpy(text, "<none>");

		BLI_snprintf(stamp_data->marker, sizeof(stamp_data->marker), do_prefix ? "Marker %s":"%s", text);
	} else {
		stamp_data->marker[0] = '\0';
	}
	
	if (scene->r.stamp & R_STAMP_TIME) {
		int f = (int)(scene->r.cfra % scene->r.frs_sec);
		int s = (int)(scene->r.cfra / scene->r.frs_sec);
		int h= 0;
		int m= 0;

		if (s) {
			m = (int)(s / 60);
			s %= 60;

			if (m) {
				h = (int)(m / 60);
				m %= 60;
			}
		}

		if (scene->r.frs_sec < 100)
			BLI_snprintf(text, sizeof(text), "%02d:%02d:%02d.%02d", h, m, s, f);
		else
			BLI_snprintf(text, sizeof(text), "%02d:%02d:%02d.%03d", h, m, s, f);

		BLI_snprintf(stamp_data->time, sizeof(stamp_data->time), do_prefix ? "Time %s":"%s", text);
	} else {
		stamp_data->time[0] = '\0';
	}
	
	if (scene->r.stamp & R_STAMP_FRAME) {
		char format[32];
		int digits= 1;
		
		if(scene->r.efra>9)
			digits= 1 + (int) log10(scene->r.efra);

		BLI_snprintf(format, sizeof(format), do_prefix ? "Frame %%0%di":"%%0%di", digits);
		BLI_snprintf (stamp_data->frame, sizeof(stamp_data->frame), format, scene->r.cfra);
	} else {
		stamp_data->frame[0] = '\0';
	}

	if (scene->r.stamp & R_STAMP_CAMERA) {
		BLI_snprintf(stamp_data->camera, sizeof(stamp_data->camera), do_prefix ? "Camera %s":"%s", camera ? camera->id.name+2 : "<none>");
	} else {
		stamp_data->camera[0] = '\0';
	}

	if (scene->r.stamp & R_STAMP_CAMERALENS) {
		if (camera && camera->type == OB_CAMERA) {
			BLI_snprintf(text, sizeof(text), "%.2f", ((Camera *)camera->data)->lens);
		}
		else 		strcpy(text, "<none>");

		BLI_snprintf(stamp_data->cameralens, sizeof(stamp_data->cameralens), do_prefix ? "Lens %s":"%s", text);
	} else {
		stamp_data->cameralens[0] = '\0';
	}

	if (scene->r.stamp & R_STAMP_SCENE) {
		BLI_snprintf(stamp_data->scene, sizeof(stamp_data->scene), do_prefix ? "Scene %s":"%s", scene->id.name+2);
	} else {
		stamp_data->scene[0] = '\0';
	}
	
	if (scene->r.stamp & R_STAMP_SEQSTRIP) {
		Sequence *seq= seq_foreground_frame_get(scene, scene->r.cfra);
	
		if (seq) strcpy(text, seq->name+2);
		else 		strcpy(text, "<none>");

		BLI_snprintf(stamp_data->strip, sizeof(stamp_data->strip), do_prefix ? "Strip %s":"%s", text);
	} else {
		stamp_data->strip[0] = '\0';
	}

	{
		Render *re= RE_GetRender(scene->id.name);
		RenderStats *stats= re ? RE_GetStats(re):NULL;

		if (stats && (scene->r.stamp & R_STAMP_RENDERTIME)) {
			BLI_timestr(stats->lastframetime, text);

			BLI_snprintf(stamp_data->rendertime, sizeof(stamp_data->rendertime), do_prefix ? "RenderTime %s":"%s", text);
		} else {
			stamp_data->rendertime[0] = '\0';
		}
	}
}

void BKE_stamp_buf(Scene *scene, Object *camera, unsigned char *rect, float *rectf, int width, int height, int channels)
{
	struct StampData stamp_data;
	float w, h, pad;
	int x, y, y_ofs;
	float h_fixed;
	const int mono= blf_mono_font_render; // XXX

#define BUFF_MARGIN_X 2
#define BUFF_MARGIN_Y 1

	if (!rect && !rectf)
		return;
	
	stampdata(scene, camera, &stamp_data, 1);

	/* TODO, do_versions */
	if(scene->r.stamp_font_id < 8)
		scene->r.stamp_font_id= 12;

	/* set before return */
	BLF_size(mono, scene->r.stamp_font_id, 72);
	
	BLF_buffer(mono, rectf, rect, width, height, channels);
	BLF_buffer_col(mono, scene->r.fg_stamp[0], scene->r.fg_stamp[1], scene->r.fg_stamp[2], 1.0);
	pad= BLF_width_max(mono);

	/* use 'h_fixed' rather than 'h', aligns better */
	h_fixed= BLF_height_max(mono);
	y_ofs = -BLF_descender(mono);

	x= 0;
	y= height;

	if (stamp_data.file[0]) {
		/* Top left corner */
		BLF_width_and_height(mono, stamp_data.file, &w, &h); h= h_fixed;
		y -= h;

		/* also a little of space to the background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		/* and draw the text. */
		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.file);

		/* the extra pixel for background. */
		y -= BUFF_MARGIN_Y * 2;
	}

	/* Top left corner, below File */
	if (stamp_data.note[0]) {
		BLF_width_and_height(mono, stamp_data.note, &w, &h); h= h_fixed;
		y -= h;

		/* and space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, 0, y-BUFF_MARGIN_Y, w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.note);

		/* the extra pixel for background. */
		y -= BUFF_MARGIN_Y * 2;
	}
	
	/* Top left corner, below File (or Note) */
	if (stamp_data.date[0]) {
		BLF_width_and_height(mono, stamp_data.date, &w, &h); h= h_fixed;
		y -= h;

		/* and space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, 0, y-BUFF_MARGIN_Y, w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.date);

		/* the extra pixel for background. */
		y -= BUFF_MARGIN_Y * 2;
	}

	/* Top left corner, below File, Date or Note */
	if (stamp_data.rendertime[0]) {
		BLF_width_and_height(mono, stamp_data.rendertime, &w, &h); h= h_fixed;
		y -= h;

		/* and space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, 0, y-BUFF_MARGIN_Y, w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.rendertime);
	}

	x= 0;
	y= 0;

	/* Bottom left corner, leaving space for timing */
	if (stamp_data.marker[0]) {
		BLF_width_and_height(mono, stamp_data.marker, &w, &h); h= h_fixed;

		/* extra space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		/* and pad the text. */
		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.marker);

		/* space width. */
		x += w + pad;
	}
	
	/* Left bottom corner */
	if (stamp_data.time[0]) {
		BLF_width_and_height(mono, stamp_data.time, &w, &h); h= h_fixed;

		/* extra space for background */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y, x+w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		/* and pad the text. */
		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.time);

		/* space width. */
		x += w + pad;
	}
	
	if (stamp_data.frame[0]) {
		BLF_width_and_height(mono, stamp_data.frame, &w, &h); h= h_fixed;

		/* extra space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, x+w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		/* and pad the text. */
		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.frame);

		/* space width. */
		x += w + pad;
	}

	if (stamp_data.camera[0]) {
		BLF_width_and_height(mono, stamp_data.camera, &w, &h); h= h_fixed;

		/* extra space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, x+w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);
		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.camera);

		/* space width. */
		x += w + pad;
	}

	if (stamp_data.cameralens[0]) {
		BLF_width_and_height(mono, stamp_data.cameralens, &w, &h); h= h_fixed;

		/* extra space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, x+w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);
		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.cameralens);
	}
	
	if (stamp_data.scene[0]) {
		BLF_width_and_height(mono, stamp_data.scene, &w, &h); h= h_fixed;

		/* Bottom right corner, with an extra space because blenfont is too strict! */
		x= width - w - 2;

		/* extra space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, x+w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		/* and pad the text. */
		BLF_position(mono, x, y+y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.scene);
	}
	
	if (stamp_data.strip[0]) {
		BLF_width_and_height(mono, stamp_data.strip, &w, &h); h= h_fixed;

		/* Top right corner, with an extra space because blenfont is too strict! */
		x= width - w - pad;
		y= height - h;

		/* extra space for background. */
		buf_rectfill_area(rect, rectf, width, height, scene->r.bg_stamp, x-BUFF_MARGIN_X, y-BUFF_MARGIN_Y, x+w+BUFF_MARGIN_X, y+h+BUFF_MARGIN_Y);

		BLF_position(mono, x, y + y_ofs, 0.0);
		BLF_draw_buffer(mono, stamp_data.strip);
	}

	/* cleanup the buffer. */
	BLF_buffer(mono, NULL, NULL, 0, 0, 0);

#undef BUFF_MARGIN_X
#undef BUFF_MARGIN_Y
}

void BKE_stamp_info(Scene *scene, Object *camera, struct ImBuf *ibuf)
{
	struct StampData stamp_data;

	if (!ibuf)	return;
	
	/* fill all the data values, no prefix */
	stampdata(scene, camera, &stamp_data, 0);
	
	if (stamp_data.file[0])		IMB_metadata_change_field (ibuf, "File",		stamp_data.file);
	if (stamp_data.note[0])		IMB_metadata_change_field (ibuf, "Note",		stamp_data.note);
	if (stamp_data.date[0])		IMB_metadata_change_field (ibuf, "Date",		stamp_data.date);
	if (stamp_data.marker[0])	IMB_metadata_change_field (ibuf, "Marker",	stamp_data.marker);
	if (stamp_data.time[0])		IMB_metadata_change_field (ibuf, "Time",		stamp_data.time);
	if (stamp_data.frame[0])	IMB_metadata_change_field (ibuf, "Frame",	stamp_data.frame);
	if (stamp_data.camera[0])	IMB_metadata_change_field (ibuf, "Camera",	stamp_data.camera);
	if (stamp_data.cameralens[0]) IMB_metadata_change_field (ibuf, "Lens",	stamp_data.cameralens);
	if (stamp_data.scene[0])	IMB_metadata_change_field (ibuf, "Scene",	stamp_data.scene);
	if (stamp_data.strip[0])	IMB_metadata_change_field (ibuf, "Strip",	stamp_data.strip);
	if (stamp_data.rendertime[0]) IMB_metadata_change_field (ibuf, "RenderTime", stamp_data.rendertime);
}

int BKE_alphatest_ibuf(ImBuf *ibuf)
{
	int tot;
	if(ibuf->rect_float) {
		float *buf= ibuf->rect_float;
		for(tot= ibuf->x * ibuf->y; tot--; buf+=4) {
			if(buf[3] < 1.0f) {
				return TRUE;
			}
		}
	}
	else if (ibuf->rect) {
		unsigned char *buf= (unsigned char *)ibuf->rect;
		for(tot= ibuf->x * ibuf->y; tot--; buf+=4) {
			if(buf[3] != 255) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

int BKE_write_ibuf(ImBuf *ibuf, const char *name, int imtype, int subimtype, int quality)
{
	int ok;
	(void)subimtype; /* quies unused warnings */

	if(imtype == -1) {
		/* use whatever existing image type is set by 'ibuf' */
	}
	else if(imtype== R_IRIS) {
		ibuf->ftype= IMAGIC;
	}
#ifdef WITH_HDR
	else if (imtype==R_RADHDR) {
		ibuf->ftype= RADHDR;
	}
#endif
	else if (ELEM5(imtype, R_PNG, R_FFMPEG, R_H264, R_THEORA, R_XVID)) {
		ibuf->ftype= PNG;

		if(imtype==R_PNG)
			ibuf->ftype |= quality;  /* quality is actually compression 0-100 --> 0-9 */

	}
#ifdef WITH_DDS
	else if (imtype==R_DDS) {
		ibuf->ftype= DDS;
	}
#endif
	else if (imtype==R_BMP) {
		ibuf->ftype= BMP;
	}
#ifdef WITH_TIFF
	else if (imtype==R_TIFF) {
		ibuf->ftype= TIF;

		if(subimtype & R_TIFF_16BIT)
			ibuf->ftype |= TIF_16BIT;
	}
#endif
#ifdef WITH_OPENEXR
	else if (imtype==R_OPENEXR || imtype==R_MULTILAYER) {
		ibuf->ftype= OPENEXR;
		if(subimtype & R_OPENEXR_HALF)
			ibuf->ftype |= OPENEXR_HALF;
		ibuf->ftype |= (quality & OPENEXR_COMPRESS);
		
		if(!(subimtype & R_OPENEXR_ZBUF))
			ibuf->zbuf_float = NULL;	/* signal for exr saving */
		
	}
#endif
#ifdef WITH_CINEON
	else if (imtype==R_CINEON) {
		ibuf->ftype = CINEON;
	}
	else if (imtype==R_DPX) {
		ibuf->ftype = DPX;
	}
#endif
	else if (imtype==R_TARGA) {
		ibuf->ftype= TGA;
	}
	else if(imtype==R_RAWTGA) {
		ibuf->ftype= RAWTGA;
	}
#ifdef WITH_OPENJPEG
	else if(imtype==R_JP2) {
		if(quality < 10) quality= 90;
		ibuf->ftype= JP2|quality;
		
		if (subimtype & R_JPEG2K_16BIT) {
			ibuf->ftype |= JP2_16BIT;
		} else if (subimtype & R_JPEG2K_12BIT) {
			ibuf->ftype |= JP2_12BIT;
		}
		
		if (subimtype & R_JPEG2K_YCC) {
			ibuf->ftype |= JP2_YCC;
		}
		
		if (subimtype & R_JPEG2K_CINE_PRESET) {
			ibuf->ftype |= JP2_CINE;
			if (subimtype & R_JPEG2K_CINE_48FPS)
				ibuf->ftype |= JP2_CINE_48FPS;
		}
	}
#endif
	else {
		/* R_JPEG90, etc. default we save jpegs */
		if(quality < 10) quality= 90;
		ibuf->ftype= JPG|quality;
		if(ibuf->depth==32) ibuf->depth= 24;	/* unsupported feature only confuses other s/w */
	}
	
	BLI_make_existing_file(name);
	
	ok = IMB_saveiff(ibuf, name, IB_rect | IB_zbuf | IB_zbuffloat);
	if (ok == 0) {
		perror(name);
	}
	
	return(ok);
}

int BKE_write_ibuf_stamp(Scene *scene, struct Object *camera, ImBuf *ibuf, const char *name, int imtype, int subimtype, int quality)
{
	if(scene && scene->r.stamp & R_STAMP_ALL)
		BKE_stamp_info(scene, camera, ibuf);

	return BKE_write_ibuf(ibuf, name, imtype, subimtype, quality);
}


void BKE_makepicstring(char *string, const char *base, int frame, int imtype, const short use_ext, const short use_frames)
{
	if (string==NULL) return;
	BLI_strncpy(string, base, FILE_MAX - 10);	/* weak assumption */
	BLI_path_abs(string, G.main->name);

	if(use_frames)
		BLI_path_frame(string, frame, 4);

	if(use_ext)
		BKE_add_image_extension(string, imtype);
		
}

/* used by sequencer too */
struct anim *openanim(char *name, int flags)
{
	struct anim *anim;
	struct ImBuf *ibuf;
	
	anim = IMB_open_anim(name, flags);
	if (anim == NULL) return NULL;

	ibuf = IMB_anim_absolute(anim, 0);
	if (ibuf == NULL) {
		if(BLI_exists(name))
			printf("not an anim: %s\n", name);
		else
			printf("anim file doesn't exist: %s\n", name);
		IMB_free_anim(anim);
		return NULL;
	}
	IMB_freeImBuf(ibuf);
	
	return(anim);
}

/* ************************* New Image API *************** */


/* Notes about Image storage 
- packedfile
  -> written in .blend
- filename
  -> written in .blend
- movie
  -> comes from packedfile or filename
- renderresult
  -> comes from packedfile or filename
- listbase
  -> ibufs from exrhandle
- flipbook array
  -> ibufs come from movie, temporary renderresult or sequence
- ibuf
  -> comes from packedfile or filename or generated

*/


/* forces existence of 1 Image for renderout or nodes, returns Image */
/* name is only for default, when making new one */
Image *BKE_image_verify_viewer(int type, const char *name)
{
	Image *ima;
	
	for(ima=G.main->image.first; ima; ima= ima->id.next)
		if(ima->source==IMA_SRC_VIEWER)
			if(ima->type==type)
				break;
	
	if(ima==NULL)
		ima= image_alloc(name, IMA_SRC_VIEWER, type);
	
	/* happens on reload, imagewindow cannot be image user when hidden*/
	if(ima->id.us==0)
		id_us_plus(&ima->id);

	return ima;
}

void BKE_image_assign_ibuf(Image *ima, ImBuf *ibuf)
{
	image_assign_ibuf(ima, ibuf, IMA_NO_INDEX, 0);
}

void BKE_image_signal(Image *ima, ImageUser *iuser, int signal)
{
	if(ima==NULL)
		return;
	
	switch(signal) {
	case IMA_SIGNAL_FREE:
		image_free_buffers(ima);
		if(iuser)
			iuser->ok= 1;
		break;
	case IMA_SIGNAL_SRC_CHANGE:
		if(ima->type == IMA_TYPE_UV_TEST)
			if(ima->source != IMA_SRC_GENERATED)
				ima->type= IMA_TYPE_IMAGE;

		if(ima->source==IMA_SRC_GENERATED) {
			if(ima->gen_x==0 || ima->gen_y==0) {
				ImBuf *ibuf= image_get_ibuf(ima, IMA_NO_INDEX, 0);
				if(ibuf) {
					ima->gen_x= ibuf->x;
					ima->gen_y= ibuf->y;
				}
			}
		}

		/* force reload on first use, but not for multilayer, that makes nodes and buttons in ui drawing fail */
		if(ima->type!=IMA_TYPE_MULTILAYER)
			image_free_buffers(ima);

		ima->ok= 1;
		if(iuser)
			iuser->ok= 1;
		break;
			
	case IMA_SIGNAL_RELOAD:
		/* try to repack file */
		if(ima->packedfile) {
			PackedFile *pf;
			pf = newPackedFile(NULL, ima->name);
			if (pf) {
				freePackedFile(ima->packedfile);
				ima->packedfile = pf;
				image_free_buffers(ima);
			} else {
				printf("ERROR: Image not available. Keeping packed image\n");
			}
		}
		else
			image_free_buffers(ima);
		
		if(iuser)
			iuser->ok= 1;
		
		break;
	case IMA_SIGNAL_USER_NEW_IMAGE:
		if(iuser) {
			iuser->ok= 1;
			if(ima->source==IMA_SRC_FILE || ima->source==IMA_SRC_SEQUENCE) {
				if(ima->type==IMA_TYPE_MULTILAYER) {
					iuser->multi_index= 0;
					iuser->layer= iuser->pass= 0;
				}
			}
		}
		break;
	}
	
	/* dont use notifiers because they are not 100% sure to succseed
	 * this also makes sure all scenes are accounted for. */
	{
		Scene *scene;
		for(scene= G.main->scene.first; scene; scene= scene->id.next) {
			if(scene->nodetree) {
				NodeTagIDChanged(scene->nodetree, &ima->id);
			}
		}
	}
}

/* if layer or pass changes, we need an index for the imbufs list */
/* note it is called for rendered results, but it doesnt use the index! */
/* and because rendered results use fake layer/passes, don't correct for wrong indices here */
RenderPass *BKE_image_multilayer_index(RenderResult *rr, ImageUser *iuser)
{
	RenderLayer *rl;
	RenderPass *rpass= NULL;
	
	if(rr==NULL) 
		return NULL;
	
	if(iuser) {
		short index= 0, rl_index= 0, rp_index;
		
		for(rl= rr->layers.first; rl; rl= rl->next, rl_index++) {
			rp_index= 0;
			for(rpass= rl->passes.first; rpass; rpass= rpass->next, index++, rp_index++)
				if(iuser->layer==rl_index && iuser->pass==rp_index)
					break;
			if(rpass)
				break;
		}
		
		if(rpass)
			iuser->multi_index= index;
		else 
			iuser->multi_index= 0;
	}
	if(rpass==NULL) {
		rl= rr->layers.first;
		if(rl)
			rpass= rl->passes.first;
	}
	
	return rpass;
}

RenderResult *BKE_image_acquire_renderresult(Scene *scene, Image *ima)
{
	if(ima->rr) {
		return ima->rr;
	}
	else if(ima->type==IMA_TYPE_R_RESULT) {
		if(ima->render_slot == ima->last_render_slot)
			return RE_AcquireResultRead(RE_GetRender(scene->id.name));
		else
			return ima->renders[ima->render_slot];
	}
	else
		return NULL;
}

void BKE_image_release_renderresult(Scene *scene, Image *ima)
{
	if(ima->rr);
	else if(ima->type==IMA_TYPE_R_RESULT) {
		if(ima->render_slot == ima->last_render_slot)
			RE_ReleaseResult(RE_GetRender(scene->id.name));
	}
}

void BKE_image_backup_render(Scene *scene, Image *ima)
{
	/* called right before rendering, ima->renders contains render
	   result pointers for everything but the current render */
	Render *re= RE_GetRender(scene->id.name);
	int slot= ima->render_slot, last= ima->last_render_slot;

	if(slot != last) {
		if(ima->renders[slot]) {
			RE_FreeRenderResult(ima->renders[slot]);
			ima->renders[slot]= NULL;
		}

		ima->renders[last]= NULL;
		RE_SwapResult(re, &ima->renders[last]);
	}

	ima->last_render_slot= slot;
}

/* after imbuf load, openexr type can return with a exrhandle open */
/* in that case we have to build a render-result */
static void image_create_multilayer(Image *ima, ImBuf *ibuf, int framenr)
{
	
	ima->rr= RE_MultilayerConvert(ibuf->userdata, ibuf->x, ibuf->y);

#ifdef WITH_OPENEXR
	IMB_exr_close(ibuf->userdata);
#endif

	ibuf->userdata= NULL;
	if(ima->rr)
		ima->rr->framenr= framenr;
}

static void image_apply_colormanagement_after_load(Image *ima, ImBuf *ibuf)
{
	ColorSpace* colorspace;
	colorspace = BCM_get_colorspace(ima->colorspace);
	if(!colorspace)
	{
		if( strcmp(ima->colorspace, "")!=0 )
		{
			printf("Color management Warning: Image using unknown colorspace \"%s\"", ima->colorspace);
			BLI_strncpy(ima->colorspace, "", 32);	
		}
		/* default colorspace according to image bith depth */
		/* 8bit, 16bit, log, float*/
		colorspace = BCM_get_default_imbuf_colorspace(ibuf);
	}
	if(colorspace)
		ibuf->profile = colorspace->index;
	else
		ibuf->profile = IB_PROFILE_NONE;
	
	ibuf->is_float_linear = 0;
	BCM_make_imbuf_float_linear(ibuf);
}

/* common stuff to do with images after loading */
static void image_initialize_after_load(Image *ima, ImBuf *ibuf)
{
	image_apply_colormanagement_after_load(ima, ibuf);
	
	/* preview is NULL when it has never been used as an icon before */
	if(G.background==0 && ima->preview==NULL)
		BKE_icon_changed(BKE_icon_getid(&ima->id));

	/* fields */
	if (ima->flag & IMA_FIELDS) {
		if(ima->flag & IMA_STD_FIELD) de_interlace_st(ibuf);
		else de_interlace_ng(ibuf);
	}
	/* timer */
	ima->lastused = clock() / CLOCKS_PER_SEC;
	
	ima->ok= IMA_OK_LOADED;
	
}

static ImBuf *image_load_sequence_file(Image *ima, ImageUser *iuser, int frame)
{
	struct ImBuf *ibuf;
	unsigned short numlen;
	char name[FILE_MAX], head[FILE_MAX], tail[FILE_MAX];
	int flag;
	
	/* XXX temp stuff? */
	if(ima->lastframe != frame)
		ima->tpageflag |= IMA_TPAGE_REFRESH;

	ima->lastframe= frame;
	BLI_strncpy(name, ima->name, sizeof(name));
	BLI_stringdec(name, head, tail, &numlen);
	BLI_stringenc(name, head, tail, numlen, frame);

	if(ima->id.lib)
		BLI_path_abs(name, ima->id.lib->filepath);
	else
		BLI_path_abs(name, G.main->name);
	
	flag= IB_rect|IB_multilayer;
	if(ima->flag & IMA_DO_PREMUL)
		flag |= IB_premul;

	/* read ibuf */
	ibuf = IMB_loadiffname(name, flag);

#if 0
	if(ibuf) {
		printf(AT" loaded %s\n", name);
	} else {
		printf(AT" missed %s\n", name);
	}
#endif

	if (ibuf) {
#ifdef WITH_OPENEXR
		/* handle multilayer case, don't assign ibuf. will be handled in BKE_image_get_ibuf */
		if (ibuf->ftype==OPENEXR && ibuf->userdata) {
			image_create_multilayer(ima, ibuf, frame);	
			ima->type= IMA_TYPE_MULTILAYER;
			IMB_freeImBuf(ibuf);
			ibuf= NULL;
		}
		else {
			image_initialize_after_load(ima, ibuf);
			image_assign_ibuf(ima, ibuf, 0, frame);
		}
#else
		image_initialize_after_load(ima, ibuf);
		image_assign_ibuf(ima, ibuf, 0, frame);
#endif
	}
	else
		ima->ok= 0;
	
	if(iuser)
		iuser->ok= ima->ok;
	
	return ibuf;
}

static ImBuf *image_load_sequence_multilayer(Image *ima, ImageUser *iuser, int frame)
{
	struct ImBuf *ibuf= NULL;
	
	/* either we load from RenderResult, or we have to load a new one */
	
	/* check for new RenderResult */
	if(ima->rr==NULL || frame!=ima->rr->framenr) {
		/* copy to survive not found multilayer image */
		RenderResult *oldrr= ima->rr;
	
		ima->rr= NULL;
		ibuf = image_load_sequence_file(ima, iuser, frame);
		
		if(ibuf) { /* actually an error */
			ima->type= IMA_TYPE_IMAGE;
			printf("error, multi is normal image\n");
		}
		// printf("loaded new result %p\n", ima->rr);
		/* free result if new one found */
		if(ima->rr) {
			// if(oldrr) printf("freed previous result %p\n", oldrr);
			if(oldrr) RE_FreeRenderResult(oldrr);
		}
		else {
			ima->rr= oldrr;
		}

	}
	if(ima->rr) {
		RenderPass *rpass= BKE_image_multilayer_index(ima->rr, iuser);
		
		if(rpass) {
			// printf("load from pass %s\n", rpass->name);
			/* since we free  render results, we copy the rect */
			ibuf= IMB_allocImBuf(ima->rr->rectx, ima->rr->recty, 32, 0);
			ibuf->rect_float= MEM_dupallocN(rpass->rect);
			ibuf->flags |= IB_rectfloat;
			ibuf->mall= IB_rectfloat;
			ibuf->channels= rpass->channels;
			ibuf->profile = IB_PROFILE_LINEAR_RGB;
			
			image_initialize_after_load(ima, ibuf);
			image_assign_ibuf(ima, ibuf, iuser?iuser->multi_index:0, frame);
			
		}
		// else printf("pass not found\n");
	}
	else
		ima->ok= 0;
	
	if(iuser)
		iuser->ok= ima->ok;
	
	return ibuf;
}


static ImBuf *image_load_movie_file(Image *ima, ImageUser *iuser, int frame)
{
	struct ImBuf *ibuf= NULL;
	
	ima->lastframe= frame;
	
	if(ima->anim==NULL) {
		char str[FILE_MAX];
		
		BLI_strncpy(str, ima->name, FILE_MAX);
		if(ima->id.lib)
			BLI_path_abs(str, ima->id.lib->filepath);
		else
			BLI_path_abs(str, G.main->name);
		
		ima->anim = openanim(str, IB_rect);
		
		/* let's initialize this user */
		if(ima->anim && iuser && iuser->frames==0)
			iuser->frames= IMB_anim_get_duration(ima->anim);
	}
	
	if(ima->anim) {
		int dur = IMB_anim_get_duration(ima->anim);
		int fra= frame-1;
		
		if(fra<0) fra = 0;
		if(fra>(dur-1)) fra= dur-1;
		ibuf = IMB_anim_absolute(ima->anim, fra);
		
		if(ibuf) {
			image_initialize_after_load(ima, ibuf);
			image_assign_ibuf(ima, ibuf, 0, frame);
		}
		else
			ima->ok= 0;
	}
	else
		ima->ok= 0;
	
	if(iuser)
		iuser->ok= ima->ok;
	
	return ibuf;
}

/* warning, 'iuser' can be NULL */
static ImBuf *image_load_image_file(Image *ima, ImageUser *iuser, int cfra)
{
	struct ImBuf *ibuf;
	char str[FILE_MAX];
	int assign = 0, flag;
	
	/* always ensure clean ima */
	image_free_buffers(ima);
	
	/* is there a PackedFile with this image ? */
	if (ima->packedfile) {
		flag = IB_rect|IB_multilayer;
		if(ima->flag & IMA_DO_PREMUL) flag |= IB_premul;
		
		ibuf = IMB_ibImageFromMemory((unsigned char*)ima->packedfile->data, ima->packedfile->size, flag);
	} 
	else {
		flag= IB_rect|IB_multilayer|IB_metadata;
		if(ima->flag & IMA_DO_PREMUL)
			flag |= IB_premul;
			
		/* get the right string */
		BLI_strncpy(str, ima->name, sizeof(str));
		if(ima->id.lib)
			BLI_path_abs(str, ima->id.lib->filepath);
		else
			BLI_path_abs(str, G.main->name);
		
		/* read ibuf */
		ibuf = IMB_loadiffname(str, flag);
	}
	
	if (ibuf) {
		/* handle multilayer case, don't assign ibuf. will be handled in BKE_image_get_ibuf */
		if (ibuf->ftype==OPENEXR && ibuf->userdata) {
			image_create_multilayer(ima, ibuf, cfra);	
			ima->type= IMA_TYPE_MULTILAYER;
			IMB_freeImBuf(ibuf);
			ibuf= NULL;
		}
		else {
			image_initialize_after_load(ima, ibuf);
			assign= 1;

			/* check if the image is a font image... */
			detectBitmapFont(ibuf);
			
			/* make packed file for autopack */
			if ((ima->packedfile == NULL) && (G.fileflags & G_AUTOPACK))
				ima->packedfile = newPackedFile(NULL, str);
		}
	}
	else
		ima->ok= 0;
	
	if(assign)
		image_assign_ibuf(ima, ibuf, IMA_NO_INDEX, 0);

	if(iuser)
		iuser->ok= ima->ok;
	
	return ibuf;
}

static ImBuf *image_get_ibuf_multilayer(Image *ima, ImageUser *iuser)
{
	ImBuf *ibuf= NULL;
	
	if(ima->rr==NULL) {
		ibuf = image_load_image_file(ima, iuser, 0);
		if(ibuf) { /* actually an error */
			ima->type= IMA_TYPE_IMAGE;
			return ibuf;
		}
	}
	if(ima->rr) {
		RenderPass *rpass= BKE_image_multilayer_index(ima->rr, iuser);

		if(rpass) {
			ibuf= IMB_allocImBuf(ima->rr->rectx, ima->rr->recty, 32, 0);
			
			image_initialize_after_load(ima, ibuf);
			
			ibuf->rect_float= rpass->rect;
			ibuf->flags |= IB_rectfloat;
			ibuf->channels= rpass->channels;
			ibuf->profile = IB_PROFILE_LINEAR_RGB;

			image_assign_ibuf(ima, ibuf, iuser?iuser->multi_index:IMA_NO_INDEX, 0);
		}
	}
	
	if(ibuf==NULL) 
		ima->ok= 0;
	if(iuser)
		iuser->ok= ima->ok;
	
	return ibuf;
}


/* showing RGBA result itself (from compo/sequence) or
   like exr, using layers etc */
/* always returns a single ibuf, also during render progress */
static ImBuf *image_get_render_result(Image *ima, ImageUser *iuser, void **lock_r)
{
	Render *re;
	RenderResult rres;
	float *rectf, *rectz;
	unsigned int *rect;
	float dither;
	int channels, layer, pass;
	ImBuf *ibuf;
	int from_render= (ima->render_slot == ima->last_render_slot);

	if(!(iuser && iuser->scene))
		return NULL;

	/* if we the caller is not going to release the lock, don't give the image */
	if(!lock_r)
		return NULL;

	re= RE_GetRender(iuser->scene->id.name);

	channels= 4;
	layer= (iuser)? iuser->layer: 0;
	pass= (iuser)? iuser->pass: 0;

	if(from_render) {
		RE_AcquireResultImage(re, &rres);
	}
	else if(ima->renders[ima->render_slot]) {
		rres= *(ima->renders[ima->render_slot]);
		rres.have_combined= rres.rectf != NULL;
	}
	else
		memset(&rres, 0, sizeof(RenderResult));
	
	if(!(rres.rectx > 0 && rres.recty > 0)) {
		if(from_render)
			RE_ReleaseResultImage(re);
		return NULL;
	}

	/* release is done in BKE_image_release_ibuf using lock_r */
	if(from_render) {
		BLI_lock_thread(LOCK_VIEWER);
		*lock_r= re;
	}

	/* this gives active layer, composite or seqence result */
	rect= (unsigned int *)rres.rect32;
	rectf= rres.rectf;
	rectz= rres.rectz;
	dither= iuser->scene->r.dither_intensity;

	/* combined layer gets added as first layer */
	if(rres.have_combined && layer==0);
	else if(rres.layers.first) {
		RenderLayer *rl= BLI_findlink(&rres.layers, layer-(rres.have_combined?1:0));
		if(rl) {
			RenderPass *rpass;

			/* there's no combined pass, is in renderlayer itself */
			if(pass==0) {
				rectf= rl->rectf;
			}
			else {
				rpass= BLI_findlink(&rl->passes, pass-1);
				if(rpass) {
					channels= rpass->channels;
					rectf= rpass->rect;
					dither= 0.0f; /* don't dither passes */
				}
			}

			for(rpass= rl->passes.first; rpass; rpass= rpass->next)
				if(rpass->passtype == SCE_PASS_Z)
					rectz= rpass->rect;
		}
	}

	ibuf= image_get_ibuf(ima, IMA_NO_INDEX, 0);

	/* make ibuf if needed, and initialize it */
	if(ibuf==NULL) {
		ibuf= IMB_allocImBuf(rres.rectx, rres.recty, 32, 0);
		image_assign_ibuf(ima, ibuf, IMA_NO_INDEX, 0);
	}

	ibuf->x= rres.rectx;
	ibuf->y= rres.recty;
	
	if(ibuf->rect_float!=rectf || rect) /* ensure correct redraw */
		imb_freerectImBuf(ibuf);

	if(rect)
		ibuf->rect= rect;
	
	if(rectf) {
		ibuf->rect_float= rectf;
		ibuf->flags |= IB_rectfloat;
		ibuf->channels= channels;
	}
	else {
		ibuf->rect_float= NULL;
		ibuf->flags &= ~IB_rectfloat;
	}

	if(rectz) {
		ibuf->zbuf_float= rectz;
		ibuf->flags |= IB_zbuffloat;
	}
	else {
		ibuf->zbuf_float= NULL;
		ibuf->flags &= ~IB_zbuffloat;
	}

	/* since its possible to access the buffer from the image directly, set the profile [#25073] */
	ibuf->profile= (iuser->scene->r.color_mgt_flag & R_COLOR_MANAGEMENT) ? IB_PROFILE_LINEAR_RGB : IB_PROFILE_NONE;

	ibuf->dither= dither;

	ima->ok= IMA_OK_LOADED;

	return ibuf;
}

static ImBuf *image_get_ibuf_threadsafe(Image *ima, ImageUser *iuser, int *frame_r, int *index_r)
{
	ImBuf *ibuf = NULL;
	int frame = 0, index = 0;

	/* see if we already have an appropriate ibuf, with image source and type */
	if(ima->source==IMA_SRC_MOVIE) {
		frame= iuser?iuser->framenr:ima->lastframe;
		ibuf= image_get_ibuf(ima, 0, frame);
		/* XXX temp stuff? */
		if(ima->lastframe != frame)
			ima->tpageflag |= IMA_TPAGE_REFRESH;
		ima->lastframe = frame;
	}
	else if(ima->source==IMA_SRC_SEQUENCE) {
		if(ima->type==IMA_TYPE_IMAGE) {
			frame= iuser?iuser->framenr:ima->lastframe;
			ibuf= image_get_ibuf(ima, 0, frame);
			
			/* XXX temp stuff? */
			if(ima->lastframe != frame) {
				ima->tpageflag |= IMA_TPAGE_REFRESH;
			}
			ima->lastframe = frame;
		}	
		else if(ima->type==IMA_TYPE_MULTILAYER) {
			frame= iuser?iuser->framenr:ima->lastframe;
			index= iuser?iuser->multi_index:IMA_NO_INDEX;
			ibuf= image_get_ibuf(ima, index, frame);
		}
	}
	else if(ima->source==IMA_SRC_FILE) {
		if(ima->type==IMA_TYPE_IMAGE)
			ibuf= image_get_ibuf(ima, IMA_NO_INDEX, 0);
		else if(ima->type==IMA_TYPE_MULTILAYER)
			ibuf= image_get_ibuf(ima, iuser?iuser->multi_index:IMA_NO_INDEX, 0);
	}
	else if(ima->source == IMA_SRC_GENERATED) {
		ibuf= image_get_ibuf(ima, IMA_NO_INDEX, 0);
	}
	else if(ima->source == IMA_SRC_VIEWER) {
		/* always verify entirely, not that this shouldn't happen
		 * as part of texture sampling in rendering anyway, so not
		 * a big bottleneck */
	}

	*frame_r = frame;
	*index_r = index;

	return ibuf;
}

/* Checks optional ImageUser and verifies/creates ImBuf. */
/* use this one if you want to get a render result in progress,
 * if not, use BKE_image_get_ibuf which doesn't require a release */
ImBuf *BKE_image_acquire_ibuf(Image *ima, ImageUser *iuser, void **lock_r)
{
	ImBuf *ibuf= NULL;
	float color[] = {0, 0, 0, 1};
	int frame= 0, index= 0;

	/* This function is intended to be thread-safe. It postpones the mutex lock
	 * until it needs to load the image, if the image is already there it
	 * should just get the pointer and return. The reason is that a lot of mutex
	 * locks appears to be very slow on certain multicore macs, causing a render
	 * with image textures to actually slow down as more threads are used.
	 *
	 * Note that all the image loading functions should also make sure they do
	 * things in a threadsafe way for image_get_ibuf_threadsafe to work correct.
	 * That means, the last two steps must be, 1) add the ibuf to the list and
	 * 2) set ima/iuser->ok to 0 to IMA_OK_LOADED */
	
	if(lock_r)
		*lock_r= NULL;

	/* quick reject tests */
	if(ima==NULL) 
		return NULL;
	if(iuser) {
		if(iuser->ok==0)
			return NULL;
	}
	else if(ima->ok==0)
		return NULL;
	
	/* try to get the ibuf without locking */
	ibuf= image_get_ibuf_threadsafe(ima, iuser, &frame, &index);

	if(ibuf == NULL) {
		/* couldn't get ibuf and image is not ok, so let's lock and try to
		 * load the image */
		BLI_lock_thread(LOCK_IMAGE);

		/* need to check ok flag and loading ibuf again, because the situation
		 * might have changed in the meantime */
		if(iuser) {
			if(iuser->ok==0) {
				BLI_unlock_thread(LOCK_IMAGE);
				return NULL;
			}
		}
		else if(ima->ok==0) {
			BLI_unlock_thread(LOCK_IMAGE);
			return NULL;
		}

		ibuf= image_get_ibuf_threadsafe(ima, iuser, &frame, &index);

		if(ibuf == NULL) {
			/* we are sure we have to load the ibuf, using source and type */
			if(ima->source==IMA_SRC_MOVIE) {
				/* source is from single file, use flipbook to store ibuf */
				ibuf= image_load_movie_file(ima, iuser, frame);
			}
			else if(ima->source==IMA_SRC_SEQUENCE) {
				if(ima->type==IMA_TYPE_IMAGE) {
					/* regular files, ibufs in flipbook, allows saving */
					ibuf= image_load_sequence_file(ima, iuser, frame);
				}
				/* no else; on load the ima type can change */
				if(ima->type==IMA_TYPE_MULTILAYER) {
					/* only 1 layer/pass stored in imbufs, no exrhandle anim storage, no saving */
					ibuf= image_load_sequence_multilayer(ima, iuser, frame);
				}
			}
			else if(ima->source==IMA_SRC_FILE) {
				
				if(ima->type==IMA_TYPE_IMAGE)
					ibuf= image_load_image_file(ima, iuser, frame);	/* cfra only for '#', this global is OK */
				/* no else; on load the ima type can change */
				if(ima->type==IMA_TYPE_MULTILAYER)
					/* keeps render result, stores ibufs in listbase, allows saving */
					ibuf= image_get_ibuf_multilayer(ima, iuser);
					
			}
			else if(ima->source == IMA_SRC_GENERATED) {
				/* generated is: ibuf is allocated dynamically */
				/* UV testgrid or black or solid etc */
				if(ima->gen_x==0) ima->gen_x= 1024;
				if(ima->gen_y==0) ima->gen_y= 1024;
				ibuf= add_ibuf_size(ima->gen_x, ima->gen_y, ima->name, 24, (ima->gen_flag & IMA_GEN_FLOAT) != 0, ima->gen_type, color);
				image_assign_ibuf(ima, ibuf, IMA_NO_INDEX, 0);
				ima->ok= IMA_OK_LOADED;
			}
			else if(ima->source == IMA_SRC_VIEWER) {
				if(ima->type==IMA_TYPE_R_RESULT) {
					/* always verify entirely, and potentially
					   returns pointer to release later */
					ibuf= image_get_render_result(ima, iuser, lock_r);
				}
				else if(ima->type==IMA_TYPE_COMPOSITE) {
					/* requires lock/unlock, otherwise don't return image */
					if(lock_r) {
						/* unlock in BKE_image_release_ibuf */
						BLI_lock_thread(LOCK_VIEWER);
						*lock_r= ima;

						/* XXX anim play for viewer nodes not yet supported */
						frame= 0; // XXX iuser?iuser->framenr:0;
						ibuf= image_get_ibuf(ima, 0, frame);

						if(!ibuf) {
							/* Composite Viewer, all handled in compositor */
							/* fake ibuf, will be filled in compositor */
							ibuf= IMB_allocImBuf(256, 256, 32, IB_rect);
							image_assign_ibuf(ima, ibuf, 0, frame);
						}
					}
				}
			}
		}

		BLI_unlock_thread(LOCK_IMAGE);
	}

	tag_image_time(ima);

	return ibuf;
}

void BKE_image_release_ibuf(Image *ima, void *lock)
{
	/* for getting image during threaded render / compositing, need to release */
	if(lock == ima) {
		BLI_unlock_thread(LOCK_VIEWER); /* viewer image */
	}
	else if(lock) {
		RE_ReleaseResultImage(lock); /* render result */
		BLI_unlock_thread(LOCK_VIEWER); /* view image imbuf */
	}
}

/* warning, this can allocate generated images */
ImBuf *BKE_image_get_ibuf(Image *ima, ImageUser *iuser)
{
	/* here (+fie_ima/2-1) makes sure that division happens correctly */
	return BKE_image_acquire_ibuf(ima, iuser, NULL);
}

int BKE_image_user_get_frame(const ImageUser *iuser, int cfra, int fieldnr)
{
	const int len= (iuser->fie_ima*iuser->frames)/2;

	if(len==0) {
		return 0;
	}
	else {
		int framenr;
		cfra= cfra - iuser->sfra+1;

		/* cyclic */
		if(iuser->cycl) {
			cfra= ( (cfra) % len );
			if(cfra < 0) cfra+= len;
			if(cfra==0) cfra= len;
		}

		if(cfra<0) cfra= 0;
		else if(cfra>len) cfra= len;

		/* convert current frame to current field */
		cfra= 2*(cfra);
		if(fieldnr) cfra++;

		/* transform to images space */
		framenr= (cfra+iuser->fie_ima-2)/iuser->fie_ima;
		if(framenr>iuser->frames) framenr= iuser->frames;
		framenr+= iuser->offset;

		if(iuser->cycl) {
			framenr= ( (framenr) % len );
			while(framenr < 0) framenr+= len;
			if(framenr==0) framenr= len;
		}

		return framenr;
	}
}

void BKE_image_user_calc_frame(ImageUser *iuser, int cfra, int fieldnr)
{
	const int framenr= BKE_image_user_get_frame(iuser, cfra, fieldnr);

	/* allows image users to handle redraws */
	if(iuser->flag & IMA_ANIM_ALWAYS)
		if(framenr!=iuser->framenr)
			iuser->flag |= IMA_ANIM_REFRESHED;

	iuser->framenr= framenr;
	if(iuser->ok==0) iuser->ok= 1;
}
