
#include <string.h>

#include "BKE_colormanagement.h"
#include "BKE_utildefines.h"

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_path_util.h"

#include "BKE_global.h"
#include "BKE_main.h"

#include "RNA_define.h"

#include "UI_resources.h"

#ifdef WITH_OCIO
#include "ocio-capi.h"

void cmInit(void)
{
	const char* configdir;
	char configfile[FILE_MAXDIR+FILE_MAXFILE];
	const char* name;
	const char* family;
	const char* displayname;
	const char* viewname;
	int nrColorSpaces, nrDisplays, nrViews, index, viewindex;
	ConstConfigRcPtr* config;
	ConstColorSpaceRcPtr* ociocs;
	
	G.main->colorspaces.first = NULL;
	G.main->colorspaces.last = NULL;
	
	G.main->display_colorspaces.first = NULL;
	G.main->display_colorspaces.last = NULL;
	
	configdir = BLI_get_folder(BLENDER_DATAFILES, "colormanagement");
	
	if(configdir)
	{
		BLI_join_dirfile(configfile, sizeof(configfile), configdir, BKE_COLORMANAGEMENT_PROFILE);
	}
	
	config = OCIO_configCreateFromFile(configfile);
	
	if(!config)
		return;
	
	OCIO_setCurrentConfig(config);
	
	//colorspaces
	nrColorSpaces = OCIO_configGetNumColorSpaces(config);
	for (index = 0 ; index < nrColorSpaces; index ++)
	{
		ColorSpace* colorSpace;
		
		name = OCIO_configGetColorSpaceNameByIndex(config, index);
		ociocs = OCIO_configGetColorSpace(config, name);
		family = OCIO_colorSpaceGetFamily(ociocs);
		
		colorSpace = MEM_mallocN(sizeof(ColorSpace), "ColorSpace");
		colorSpace->index = index;
		
		BLI_strncpy(colorSpace->name, name, 32);
		BLI_strncpy(colorSpace->family, family, 32);
		
		BLI_addtail(&G.main->colorspaces, colorSpace);
		
		OCIO_colorSpaceRelease(ociocs);
	}
	
	//displays
	nrDisplays = OCIO_configGetNumDisplays(config);
	for (index = 0 ; index < nrDisplays; index ++)
	{
		DisplayColorSpace* colorSpace;
		displayname = OCIO_configGetDisplay(config, index);
			
		nrViews = OCIO_configGetNumViews(config, displayname);
		
		for (viewindex = 0 ; viewindex < nrViews; viewindex ++)
		{
			viewname = OCIO_configGetView(config, displayname, viewindex);
			name = OCIO_configGetDisplayColorSpaceName(config, displayname, viewname);
			ociocs = OCIO_configGetColorSpace(config, name);
			family = OCIO_colorSpaceGetFamily(ociocs);
			
			colorSpace = MEM_mallocN(sizeof(DisplayColorSpace), "ColorSpace");
			colorSpace->colorspace.index = OCIO_configGetIndexForColorSpace(config, name);
			
			BLI_strncpy(colorSpace->colorspace.name, name, 32);
			BLI_strncpy(colorSpace->colorspace.family, family, 32);
			
			BLI_snprintf(colorSpace->display_view_name, 64, "%s.%s", displayname, viewname);
			BLI_addtail(&G.main->display_colorspaces, colorSpace);
			
			OCIO_colorSpaceRelease(ociocs);
		}
	}
	
	OCIO_configRelease(config);
}

void cmExit(void)
{
	ColorSpace* cs;
	DisplayColorSpace* dcs;
	EnumPropertyItem* items;
	
	cs = G.main->colorspaces.first;
	while(cs)
	{
		ColorSpace* cs2 = cs;
		cs = cs->next;
		MEM_freeN(cs2);
	}
	
	dcs = G.main->display_colorspaces.first;
	while(dcs)
	{
		DisplayColorSpace* dcs2 = dcs;
		dcs = (DisplayColorSpace*) dcs->colorspace.next;
		MEM_freeN(dcs2);
	}
	
	items = cmGetColorSpaces();
	MEM_freeN(items);
	
	items = cmGetDisplayColorSpaces();
	MEM_freeN(items);
}


ColorSpace* cmGetColorSpaceFromName(const char* name)
{
	ColorSpace* cs = G.main->colorspaces.first;
	while(cs)
	{
		if( strcmp(cs->name, name) == 0 )
			return cs;
		cs = cs->next;
	}
	return 0;
}

ColorSpace* cmGetColorSpaceFromIndex(int index)
{
	ColorSpace* cs = G.main->colorspaces.first;
	while(cs)
	{
		if(cs->index == index)
			return cs;
		cs = cs->next;
	}
	return 0;
}

DisplayColorSpace* cmGetDisplayColorSpaceFromName(const char* name)
{
	DisplayColorSpace* cs = G.main->display_colorspaces.first;
	while(cs)
	{
		if( strcmp(cs->display_view_name, name) == 0 )
			return cs;
		cs = (DisplayColorSpace*) cs->colorspace.next;
	}
	return 0;
}

DisplayColorSpace* cmGetDisplayColorSpaceFromIndex(int index)
{
	DisplayColorSpace* cs = G.main->display_colorspaces.first;
	while(cs)
	{
		if( cs->colorspace.index == index )
			return cs;
		cs = (DisplayColorSpace*) cs->colorspace.next;
	}
	return 0;
}

DisplayColorSpace* cmGetDefaultDisplayColorSpace(void)
{
	char display_view[64];
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	const char* display = OCIO_configGetDefaultDisplay(config);
	const char* view = OCIO_configGetDefaultView(config, display);
	BLI_snprintf(display_view, 64, "%s.%s", display, view);
	OCIO_configRelease(config);
	return cmGetDisplayColorSpaceFromName(display_view);
}

void cmApplyTransform(float* data, long w, long h, long channels, const char* src, const char* dst)
{
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	ConstProcessorRcPtr* processor = OCIO_configGetProcessorWithNames(config, src, dst);
	PackedImageDesc* img = OCIO_createPackedImageDesc(data, w, h, channels, sizeof(float), 4*sizeof(float), 4*sizeof(float)*w);
	OCIO_processorApply(processor, img);
	OCIO_packedImageDescRelease(img);
	OCIO_processorRelease(processor);
	OCIO_configRelease(config);
}


EnumPropertyItem* cmGetColorSpaces(void)
{
	static EnumPropertyItem *items = 0;
	static int totitem = 0;
	
	if(items == 0)
	{
		ColorSpace* cs = G.main->colorspaces.first;
		while(cs)
		{
			EnumPropertyItem item;
			
			item.value = cs->index;
			item.name = cs->name;
			item.identifier = cs->name;
			item.icon = ICON_COLOR;
			item.description = "";
			
			RNA_enum_item_add(&items, &totitem, &item);
			
			cs = cs->next;
		}
		
		RNA_enum_item_end(&items, &totitem);
	}
	
	return items;
}

EnumPropertyItem* cmGetDisplayColorSpaces(void)
{
	static EnumPropertyItem *items = 0;
	static int totitem = 0;
	
	if(items == 0)
	{
		DisplayColorSpace* cs = G.main->display_colorspaces.first;
		while(cs)
		{
			EnumPropertyItem item;
			
			item.value = cs->colorspace.index;
			item.name = cs->display_view_name;
			item.identifier = cs->colorspace.name;
			item.icon = ICON_COLOR;
			item.description = "";
			
			RNA_enum_item_add(&items, &totitem, &item);
			
			cs = (DisplayColorSpace*)cs->colorspace.next;
		}
		
		RNA_enum_item_end(&items, &totitem);
	}
	
	return items;
}

#endif //WITH_OCIO

