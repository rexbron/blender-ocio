
#include "BKE_colormanagement.h"

#include <string.h>
#include <math.h>

#include "DNA_windowmanager_types.h"
#include "DNA_userdef_types.h"
#include "DNA_screen_types.h"
#include "DNA_space_types.h"

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_path_util.h"
#include "BLI_utildefines.h"
#include "BLI_math_vector.h"
#include "BLI_rand.h"

#include "BKE_utildefines.h"
#include "BKE_global.h"
#include "BKE_main.h"
#include "BKE_context.h"

#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"

#include "RNA_define.h"

//#include "UI_resources.h"

//#ifdef WITH_OCIO
#include "ocio-capi.h"

void cmLoadConfig(ConstConfigRcPtr* config)
{
	int nrColorSpaces, nrDisplays, nrViews, index, viewindex, viewindex2;
	const char* name;
	const char* family;
	const char* role_lin;
	const char* role_picker;
	const char* role_texture;
	ConstColorSpaceRcPtr* ociocs;
	
	OCIO_setCurrentConfig(config);
	
	ociocs = OCIO_configGetColorSpace(config, OCIO_ROLE_SCENE_LINEAR);
	if(ociocs)
	{
		role_lin = OCIO_colorSpaceGetName(ociocs);
		OCIO_colorSpaceRelease(ociocs);
	}
	else
	{
		printf("Blender color management: Error could not find scene linear role.\n");
	}
	
	ociocs = OCIO_configGetColorSpace(config, OCIO_ROLE_COLOR_PICKING);
	if(ociocs)
	{
		role_picker = OCIO_colorSpaceGetName(ociocs);
		OCIO_colorSpaceRelease(ociocs);
	}
	else
	{
		printf("Blender color management: Error could not find color picking role.\n");
	}
	
	ociocs = OCIO_configGetColorSpace(config, OCIO_ROLE_TEXTURE_PAINT);
	if(ociocs)
	{
		role_texture = OCIO_colorSpaceGetName(ociocs);
		OCIO_colorSpaceRelease(ociocs);
	}
	else
	{
		printf("Blender color management: Error could not find texture paint role.\n");
	}
	
	// load colorspaces
	nrColorSpaces = OCIO_configGetNumColorSpaces(config);
	for (index = 0 ; index < nrColorSpaces; index ++)
	{
		ColorSpace* colorSpace;
		
		name = OCIO_configGetColorSpaceNameByIndex(config, index);
		ociocs = OCIO_configGetColorSpace(config, name);
		family = OCIO_colorSpaceGetFamily(ociocs);
		
		colorSpace = MEM_mallocN(sizeof(ColorSpace), "ColorSpace");
		colorSpace->index = index+1;
		
		BLI_strncpy(colorSpace->name, name, COLORMAN_MAX_COLORSPACE);
		BLI_strncpy(colorSpace->family, family, COLORMAN_MAX_FAMILY);
		
		colorSpace->flag=0;
		
		if(strcmp(name, role_lin)==0)
			colorSpace->flag |= COLORSPACE_IS_SCENE_LINEAR;
			
		if(strcmp(name, role_picker)==0)
			colorSpace->flag |= COLORSPACE_IS_COLOR_PICKING;
			
		if(strcmp(name, role_texture)==0)
			colorSpace->flag |= COLORSPACE_IS_TEXTURE_PAINT;
		
		BLI_addtail(&G.colorspaces, colorSpace);
		
		OCIO_colorSpaceRelease(ociocs);
	}
	
	// load displays
	viewindex2 = 0;
	nrDisplays = OCIO_configGetNumDisplays(config);
	for (index = 0 ; index < nrDisplays; index ++)
	{
		const char* displayname;
		ColorManagedDisplay* display;
		
		display = MEM_mallocN(sizeof(ColorManagedDisplay), "ColorManagedDisplay");
		displayname = OCIO_configGetDisplay(config, index);
		
		display->views.first = NULL;
		display->views.last = NULL;
		
		display->index = index+1;
		BLI_strncpy(display->display_name, displayname, COLORMAN_MAX_DISPLAY);
		BLI_addtail(&G.color_managed_displays, display);
		
		
		// load views
		nrViews = OCIO_configGetNumViews(config, displayname);
		for (viewindex = 0 ; viewindex < nrViews; viewindex++, viewindex2++)
		{
			const char* viewname;
			ColorManagedView* view;
			
			viewname = OCIO_configGetView(config, displayname, viewindex);
			name = OCIO_configGetDisplayColorSpaceName(config, displayname, viewname);
			
			view = MEM_mallocN(sizeof(ColorManagedView), "ColorManagedView");
			view->index = viewindex2+1;
			view->parent_display_index = display->index;
			BLI_strncpy(view->view_name, viewname, COLORMAN_MAX_VIEW);
			BLI_strncpy(view->colorspace_name, name, COLORMAN_MAX_COLORSPACE);
			
			BLI_addtail(&display->views, view);
		}
	}
}

void cmFreeConfig(void)
{
	ColorSpace* cs;
	ColorManagedDisplay* cd;
	
	cs = G.colorspaces.first;
	while(cs)
	{
		ColorSpace* cs2 = cs;
		cs = cs->next;
		MEM_freeN(cs2);
	}
	
	cd = G.color_managed_displays.first;
	while(cd)
	{
		ColorManagedDisplay* cd2 = cd;
		ColorManagedView* cv = cd->views.first;
		while(cv)
		{
			ColorManagedView* cv2 = cv;
			cv = cv->next;
			MEM_freeN(cv2);
		}
		
		cd = cd->next;
		MEM_freeN(cd2);
	}
}

void cmCheckConfigUse()
{
	//fix windows with bad display
	{
		wmWindowManager* wm = G.main->wm.first;
		wmWindow* w = wm->windows.first;
		
		while(w)
		{
			ColorManagedDisplay* display = BCM_get_display(w->colormanaged_display);
			if(!display)
			{
				display = BCM_get_default_display();
				printf("Blender color management: Window display \"%s\" not found, setting default \"%s\".\n", w->colormanaged_display, display->display_name);
				BLI_strncpy(w->colormanaged_display, display->display_name, COLORMAN_MAX_DISPLAY);
			}
			w = w->next;
		}
	}
	
	//fix space image with bad viewer
	{
		bScreen* sc;
		for(sc= G.main->screen.first; sc; sc= sc->id.next) {
			ScrArea* sa= sc->areabase.first;
			while(sa) {
				SpaceLink *sl;
				for (sl= sa->spacedata.first; sl; sl= sl->next) {
					if(sl->spacetype==SPACE_IMAGE) {
						SpaceImage* sima = (SpaceImage*)sl;
						
						ColorManagedDisplay* display = BCM_get_display(sima->colormanaged_display);
						if(!display)
						{
							display = BCM_get_default_display();
							printf("Blender color management: Window display \"%s\" not found, setting default \"%s\".\n", sima->colormanaged_display, display->display_name);
							BLI_strncpy(sima->colormanaged_display, display->display_name, COLORMAN_MAX_DISPLAY);
						}
						;
						ColorManagedView* view = BCM_get_view(display, sima->colormanaged_view);
						if(!view)
						{
							view = BCM_get_default_view(display);
							printf("Blender color management: Image viewer view \"%s\" not found, setting default \"%s\".\n", sima->colormanaged_view, view->view_name);
							BLI_strncpy(sima->colormanaged_view, view->view_name, COLORMAN_MAX_VIEW);
						}
					}
				}
				sa = sa->next;
			}
		}
	}
	
	//fix images input/output with bad colorspaces
	//...
}

void BCM_init()
{
	const char* ocio_env;
	const char* configdir;
	char configfile[FILE_MAXDIR+FILE_MAXFILE];
	ConstConfigRcPtr* config;
	
	G.colorspaces.first = NULL;
	G.colorspaces.last = NULL;
	
	G.color_managed_displays.first = NULL;
	G.color_managed_displays.last = NULL;
	
	ocio_env = getenv("OCIO");
	
	if(ocio_env)
	{
		config = OCIO_configCreateFromEnv();
	}
	else
	{
		configdir = BLI_get_folder(BLENDER_DATAFILES, "colormanagement");
		
		if(configdir)
		{
			BLI_join_dirfile(configfile, sizeof(configfile), configdir, BCM_CONFIG_FILE);
		}
		
		config = OCIO_configCreateFromFile(configfile);
	}
	
	if(config)
	{
		cmLoadConfig(config);
		cmCheckConfigUse();
	}
	
	OCIO_configRelease(config);
}

void BCM_exit(void)
{
	EnumPropertyItem* items;
	
	items = cmGetDisplays();
	MEM_freeN(items);
	
	items = cmGetViews(0);
	MEM_freeN(items);
	
	cmFreeConfig();
}


ColorSpace* BCM_get_colorspace(const char* name)
{
	ColorSpace* cs = G.colorspaces.first;
	while(cs)
	{
		if( strcmp(cs->name, name) == 0 )
			return cs;
		cs = cs->next;
	}
	return 0;
}

ColorSpace* BCM_get_colorspace_from_index(int index)
{
	ColorSpace* cs = G.colorspaces.first;
	while(cs)
	{
		if(cs->index == index)
			return cs;
		cs = cs->next;
	}
	return 0;
}

ColorSpace* BCM_get_default_colorspace_from_imbuf_ftype(int ftype)
{
/* OCIO TODO more file types here*/

	/* Log  */
#ifdef WITH_CINEOND
	if(ftype & CINEON || ftype & DPX )
		return BCM_get_default_log_colorspace()
#endif

	/* Floats */
	if(ftype & OPENEXR)
		return BCM_get_default_float_colorspace();
		
	/* 16 Bits */
#ifdef WITH_TIFF
	if(ftype & TIF && ftype & TIF_16BIT)
		return BCM_get_default_16bits_colorspace();
#endif
	
	/* 8 Bits */
	if(ftype & PNG || ftype & TGA || ftype & JPG || ftype & BMP)
		return BCM_get_default_8bits_colorspace();
#ifdef WITH_DDS
	if(ftype & DDS)
		return BCM_get_default_8bits_colorspace();
#endif
#ifdef WITH_QUICKTIME
	if(ftype & QUICKTIME)
		return BCM_get_default_8bits_colorspace();
#endif
		
	return 0;
}

ColorSpace* BCM_get_default_log_colorspace(void)
{
	return BCM_get_colorspace(U.colormanagement_options.default_log);
}

ColorSpace* BCM_get_default_float_colorspace(void)
{
	return BCM_get_colorspace(U.colormanagement_options.default_float);
}

ColorSpace* BCM_get_default_16bits_colorspace(void)
{
	return BCM_get_colorspace(U.colormanagement_options.default_16bits);
}

ColorSpace* BCM_get_default_8bits_colorspace(void)
{
	return BCM_get_colorspace(U.colormanagement_options.default_8bits);
}

ColorSpace* BCM_get_scene_linear_colorspace(void)
{
	ColorSpace* cs = G.colorspaces.first;
	while(cs)
	{
		if( cs->flag & COLORSPACE_IS_SCENE_LINEAR )
			return cs;
		cs = cs->next;
	}
	return 0;
}

ColorSpace* BCM_get_color_picking_colorspace(void)
{
	ColorSpace* cs = G.colorspaces.first;
	while(cs)
	{
		if( cs->flag & COLORSPACE_IS_COLOR_PICKING )
			return cs;
		cs = cs->next;
	}
	return 0;
}

ColorSpace* BCM_get_texture_paint_colorspace(void)
{
	ColorSpace* cs = G.colorspaces.first;
	while(cs)
	{
		if( cs->flag & COLORSPACE_IS_TEXTURE_PAINT )
			return cs;
		cs = cs->next;
	}
	return 0;
}

ColorSpace* BCM_get_sequencer_colorspace(void)
{
	return BCM_get_default_8bits_colorspace();
}

ColorSpace* BCM_get_ui_colorspace(struct wmWindow *window)
{
	if(U.colormanagement_options.flag & COLORMAN_UI_USE_WINDOW_CS)
		return BCM_get_colorspace(window->colormanaged_display);
	else
		return BCM_get_color_picking_colorspace();
}

ColorManagedDisplay* BCM_get_display(const char* name)
{
	if( strcmp(name, "") == 0)
		return BCM_get_default_display();
	
	ColorManagedDisplay* cd = G.color_managed_displays.first;
	while(cd)
	{
		if( strcmp(cd->display_name, name) == 0 )
			return cd;
		cd = cd->next;
	}
	return 0;
}

ColorManagedDisplay* BCM_get_display_from_index(int index)
{
	ColorManagedDisplay* cd = G.color_managed_displays.first;
	while(cd)
	{
		if( cd->index == index )
			return cd;
		cd = cd->next;
	}
	return 0;
}

ColorManagedDisplay* BCM_get_default_display(void)
{
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	const char* display = OCIO_configGetDefaultDisplay(config);
	OCIO_configRelease(config);
	
	if( strcmp(display, "") == 0)
		return 0;
	return BCM_get_display(display);
}

ColorManagedView* BCM_get_view(ColorManagedDisplay* display, const char* name)
{

	if(display)
	{
		if( strcmp(name, "") == 0)
			return BCM_get_default_view(display);
		
		ColorManagedView* cv = display->views.first;
		while(cv)
		{
			if( strcmp(cv->view_name, name) == 0 )
				return cv;
			cv = cv->next;
		}
	}
	return 0;
}

ColorManagedView* BCM_get_view_from_name(const char* display, const char* name)
{
	ColorManagedDisplay* cd = BCM_get_display(display);
	return BCM_get_view(cd, name);
}

ColorManagedView* BCM_get_view_from_index(int index)
{
	ColorManagedDisplay* cd = G.color_managed_displays.first;
	while(cd)
	{
		ColorManagedView* cv = cd->views.first;
		while(cv)
		{
			if( cv->index == index )
				return cv;
			cv = cv->next;
		}
		cd = cd->next;
	}
	return 0;
}

ColorManagedView* BCM_get_default_view(ColorManagedDisplay *display)
{
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	const char* view = OCIO_configGetDefaultView(config, display->display_name);
	OCIO_configRelease(config);
	
	if( strcmp(view, "") == 0)
		return 0;
	return BCM_get_view(display, view);
}

void BCM_apply_transform(float* data, long w, long h, int channel, const char* src, const char* dst)
{
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	ConstProcessorRcPtr* processor = OCIO_configGetProcessorWithNames(config, src, dst);
	PackedImageDesc* img = OCIO_createPackedImageDesc(data, w, h, channel, sizeof(float), channel*sizeof(float), channel*sizeof(float)*w);
	OCIO_processorApply(processor, img);
	OCIO_packedImageDescRelease(img);
	OCIO_processorRelease(processor);
	OCIO_configRelease(config);
}

void BCM_apply_transform_to_byte(float* dataf, unsigned char *datac, long w, long h, const char* src, const char* dst)
{
	int i;
	float *rf= dataf;
	float rgba[4];
	unsigned char *rc= datac;
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	ConstProcessorRcPtr* processor = OCIO_configGetProcessorWithNames(config, src, dst);
	
	for(i=0; i<w*h; i++, rf+=4, rc+=4) {
		copy_v4_v4(rgba, rf);
		OCIO_processorApplyRGBA(processor, rgba);
		F4TOCHAR4(rgba, rc);
	}
	
	OCIO_processorRelease(processor);
	OCIO_configRelease(config);
}

void BCM_make_imbuf_float_linear(struct ImBuf * ibuf)
{
	if(ibuf->rect_float && !ibuf->is_float_linear && ibuf->channels >= 3)
	{
		ColorSpace* cs = BCM_get_colorspace_from_index(ibuf->profile);
		ColorSpace* cs2 = BCM_get_scene_linear_colorspace();
		if(cs)
		{
			BCM_apply_transform(ibuf->rect_float, ibuf->x, ibuf->y, ibuf->channels, cs->name, cs2->name);
			ibuf->is_float_linear = 1;
		}
	}
}

void BCM_apply_display_transform(struct ImBuf *ibuf, const char* display, const char* view)
{
	if(ibuf->rect_float)
	{
		ColorSpace* inputcs;
		ConstConfigRcPtr* config = OCIO_getCurrentConfig();
		ConstProcessorRcPtr* processor;
		DisplayTransformRcPtr* dt = OCIO_createDisplayTransform();
		PackedImageDesc* img = OCIO_createPackedImageDesc(ibuf->rect_float, ibuf->x, ibuf->y, ibuf->channels, sizeof(float), ibuf->channels*sizeof(float), ibuf->channels*sizeof(float)*ibuf->x);
		
		inputcs = NULL;
		if(!ibuf->is_float_linear)
		{
			inputcs = BCM_get_colorspace_from_index(ibuf->profile);
		}
		if(!inputcs)
		{
			inputcs = BCM_get_scene_linear_colorspace();
		}
		
		OCIO_displayTransformSetInputColorSpaceName(dt, inputcs->name);
		OCIO_displayTransformSetDisplay(dt, display);
		OCIO_displayTransformSetView(dt, view);
		
		processor = OCIO_configGetProcessor(config, dt);
		if(processor)
		{
			OCIO_processorApply(processor, img);
			ibuf->is_float_linear = 0;
		}
		
		OCIO_packedImageDescRelease(img);
		OCIO_displayTransformRelease(dt);
		OCIO_processorRelease(processor);
		OCIO_configRelease(config);
	}
	
}

void IMB_rect_from_float(struct ImBuf *ibuf)
{
	float *tof = (float *)ibuf->rect_float;
	float dither= ibuf->dither / 255.0f;
	float rgba[4];
	int i, channels= ibuf->channels;
	unsigned char *to = (unsigned char *) ibuf->rect;
	
	if(tof==NULL) return;
	if(to==NULL) {
		imb_addrectImBuf(ibuf);
		to = (unsigned char *) ibuf->rect;
	}
	
	if(channels==1 || !ibuf->is_float_linear) {
		IMB_rect_from_float_simple(ibuf);
	}
	else {
	
		/* init per pixel transform */
		ColorSpace* cs = BCM_get_colorspace_from_index(ibuf->profile);
		ColorSpace* floatcs = BCM_get_scene_linear_colorspace();
		ConstConfigRcPtr* config = OCIO_getCurrentConfig();
		ConstProcessorRcPtr* proc = OCIO_configGetProcessorWithNames(config, floatcs->name, cs->name);
		
		if(channels == 3) {
			for (i = ibuf->x * ibuf->y; i > 0; i--, to+=4, tof+=3)
			{
				copy_v3_v3(rgba, tof);
				OCIO_processorApplyRGB(proc, rgba);
				
				to[0] = FTOCHAR(rgba[0]);
				to[1] = FTOCHAR(rgba[1]);
				to[2] = FTOCHAR(rgba[2]);
				to[3] = 255;
				
			}
		}
		else if (channels == 4) {
			for (i = ibuf->x * ibuf->y; i > 0; i--, to+=4, tof+=4)
			{
				copy_v4_v4(rgba, tof);
				OCIO_processorApplyRGBA(proc, rgba);
				
				if (dither != 0.f) {
					const float d = (BLI_frand()-0.5f)*dither;
					add_v4_fl(rgba, d);
				}
				
				to[0] = FTOCHAR(rgba[0]);
				to[1] = FTOCHAR(rgba[1]);
				to[2] = FTOCHAR(rgba[2]);
				to[3] = FTOCHAR(rgba[3]);
			}
		}
		
		OCIO_processorRelease(proc);
		OCIO_configRelease(config);
	}
	
	/* ensure user flag is reset */
	ibuf->userflags &= ~IB_RECT_INVALID;
}

 

/* converts from linear float to gamma corrected byte for part of the texture, buffer will hold the changed part */
void IMB_partial_rect_from_float(struct ImBuf *ibuf,float *buffer, int x, int y, int w, int h)
{
	/* indices to source and destination image pixels */
	float *srcFloatPxl;
	unsigned char *dstBytePxl;
	/* buffer index will fill buffer */
	float *bufferIndex;

	/* convenience pointers to start of image buffers */
	float *init_srcFloatPxl = (float *)ibuf->rect_float;
	unsigned char *init_dstBytePxl = (unsigned char *) ibuf->rect;

	/* Dithering factor */
	float dither= ibuf->dither / 255.0f;
	/* respective attributes of image */
	int channels= ibuf->channels;
	
	int i, j;
	
	/*
		if called -only- from GPU_paint_update_image this test will never fail
		but leaving it here for better or worse
	*/
	if(init_srcFloatPxl==NULL || (buffer == NULL)){
		return;
	}
	if(init_dstBytePxl==NULL) {
		imb_addrectImBuf(ibuf);
		init_dstBytePxl = (unsigned char *) ibuf->rect;
	}
	if(channels==1) {
			for (j = 0; j < h; j++){
				bufferIndex = buffer + w*j*4;
				dstBytePxl = init_dstBytePxl + (ibuf->x*(y + j) + x)*4;
				srcFloatPxl = init_srcFloatPxl + (ibuf->x*(y + j) + x);
				for(i = 0;  i < w; i++, dstBytePxl+=4, srcFloatPxl++, bufferIndex+=4) {
					dstBytePxl[1]= dstBytePxl[2]= dstBytePxl[3]= dstBytePxl[0] = FTOCHAR(srcFloatPxl[0]);
					bufferIndex[0] = bufferIndex[1] = bufferIndex[2] = bufferIndex[3] = srcFloatPxl[0];
				}
			}
	}
	else {
	
		/* init per pixel transform */
		ColorSpace* cs = BCM_get_colorspace_from_index(ibuf->profile);
		ColorSpace* floatcs = BCM_get_scene_linear_colorspace();
		ConstConfigRcPtr* config = OCIO_getCurrentConfig();
		ConstProcessorRcPtr* proc = OCIO_configGetProcessorWithNames(config, floatcs->name, cs->name);
		
		if(channels == 3) {
			for (j = 0; j < h; j++){
				bufferIndex = buffer + w*j*4;
				dstBytePxl = init_dstBytePxl + (ibuf->x*(y + j) + x)*4;
				srcFloatPxl = init_srcFloatPxl + (ibuf->x*(y + j) + x)*3;
				for(i = 0;  i < w; i++, dstBytePxl+=4, srcFloatPxl+=3, bufferIndex += 4) {
					copy_v3_v3(bufferIndex, srcFloatPxl);
					OCIO_processorApplyRGB(proc, bufferIndex);
					F3TOCHAR4(bufferIndex, dstBytePxl);
					bufferIndex[3]= 1.0;
				}
			}
		}
		else if (channels == 4) {
			for (j = 0; j < h; j++){
				bufferIndex = buffer + w*j*4;
				dstBytePxl = init_dstBytePxl + (ibuf->x*(y + j) + x)*4;
				srcFloatPxl = init_srcFloatPxl + (ibuf->x*(y + j) + x)*4;
				for(i = 0;  i < w; i++, dstBytePxl+=4, srcFloatPxl+=4, bufferIndex+=4) {
					copy_v4_v4(bufferIndex, srcFloatPxl);
					OCIO_processorApplyRGBA(proc, bufferIndex);
					
					if (dither != 0.f) {
						const float d = (BLI_frand()-0.5f)*dither;
						add_v4_fl(bufferIndex, d);
					}
					
					F4TOCHAR4(bufferIndex, dstBytePxl);
				}
			}
		}
		
		OCIO_processorRelease(proc);
		OCIO_configRelease(config);
	}
	
	/* ensure user flag is reset */
	ibuf->userflags &= ~IB_RECT_INVALID;
}




void IMB_float_from_rect(struct ImBuf *ibuf)
{
	IMB_float_from_rect_simple(ibuf);
	BCM_make_imbuf_float_linear(ibuf);
}


void IMB_convert_profile(struct ImBuf *ibuf, const ColorSpace* profile)
{
	ColorSpace* fromcs = BCM_get_colorspace_from_index(ibuf->profile);
	ConstConfigRcPtr* config;
	ConstProcessorRcPtr* proc;
	
	if(ibuf->profile == profile->index)
		return;
	
	config = OCIO_getCurrentConfig();
	proc = OCIO_configGetProcessorWithNames(config, fromcs->name, profile->name);
	
	if(ibuf->rect_float) {
		PackedImageDesc* img = OCIO_createPackedImageDesc(ibuf->rect_float, ibuf->x, ibuf->y, 4, sizeof(float), 4*sizeof(float), 4*sizeof(float)*ibuf->y);
		OCIO_processorApply(proc, img);
		OCIO_packedImageDescRelease(img);
	}
	if(ibuf->rect) {
		int i;
		unsigned char *rct= (unsigned char *)ibuf->rect;
		for (i = ibuf->x * ibuf->y; i > 0; i--, rct+=4) {
			float rgb[3];
			
			rgb[0] = (float)rct[0]/255.0f;
			rgb[1] = (float)rct[1]/255.0f;
			rgb[2] = (float)rct[2]/255.0f;
			
			OCIO_processorApplyRGB(proc, rgb);
			
			F3TOCHAR3(rgb, rct);
		}
	}

	OCIO_processorRelease(proc);
	OCIO_configRelease(config);
	
	ibuf->profile= profile->index;
	
	if(profile->flag && COLORSPACE_IS_SCENE_LINEAR)
		ibuf->is_float_linear = 1;
	else 
		ibuf->is_float_linear = 0;
}



/* RNA helpers */
/***********************************/
void BCM_add_colorspaces_items(EnumPropertyItem** items, int* totitem, int add_default)
{
	ColorSpace* cs = G.colorspaces.first;
	
	if(add_default)
	{	
		EnumPropertyItem item;
		
		item.value = 0;
		item.name = "(default)";
		item.identifier = "(default)";
		item.icon = 0;
		item.description = "Use the default colorspace for this image";
		
		RNA_enum_item_add(items, totitem, &item);
	}
	
	while(cs)
	{
		EnumPropertyItem item;
		
		item.value = cs->index;
		item.name = cs->name;
		item.identifier = cs->name;
		item.icon = 0;
		item.description = "";
		
		RNA_enum_item_add(items, totitem, &item);
		
		cs = cs->next;	
	}
}

EnumPropertyItem* cmGetDisplays(void)
{
	static EnumPropertyItem *items = 0;
	
	if(items == 0)
	{
		int totitem = 0;
		ColorManagedDisplay* cd = G.color_managed_displays.first;
		while(cd)
		{
			EnumPropertyItem item;
			
			item.value = cd->index;
			item.name = cd->display_name;
			item.identifier = cd->display_name;
			item.icon = 0;
			item.description = "";
			
			RNA_enum_item_add(&items, &totitem, &item);
			
			cd = cd->next;
		}
		
		RNA_enum_item_end(&items, &totitem);
	}
	
	return items;
}

EnumPropertyItem* cmGetViews(ColorManagedDisplay* display)
{
	static EnumPropertyItem *items = 0;
	static int totitem = 0;
	
	if(items)
	{
		MEM_freeN(items);
		items = 0;
		totitem = 0;
	}
	
	if(display)
	{
		ColorManagedView* cv = display->views.first;
		while(cv)
		{
			EnumPropertyItem item;
			
			item.value = cv->index;
			item.name = cv->view_name;
			item.identifier = cv->view_name;
			item.icon = 0;
			item.description = "";
			
			RNA_enum_item_add(&items, &totitem, &item);
			
			cv = cv->next;
		}
	}
	
	RNA_enum_item_end(&items, &totitem);
	
	return items;
}

EnumPropertyItem* cmGetViewsFromDisplayName(const char* name)
{
	ColorManagedDisplay* display = BCM_get_display(name);
	return cmGetViews(display);
}

//#endif //WITH_OCIO

