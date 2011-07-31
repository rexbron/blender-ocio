
#include "BKE_colormanagement.h"

#include <string.h>

#include "DNA_windowmanager_types.h"

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_path_util.h"

#include "BKE_utildefines.h"
#include "BKE_global.h"
#include "BKE_main.h"
#include "BKE_context.h"

#include "RNA_define.h"

//#include "UI_resources.h"

#ifdef WITH_OCIO
#include "ocio-capi.h"

void cmInit(bContext *C)
{
	const char* configdir;
	char configfile[FILE_MAXDIR+FILE_MAXFILE];
	const char* name;
	const char* family;
	int nrColorSpaces, nrDisplays, nrViews, index, viewindex, viewindex2;
	ConstConfigRcPtr* config;
	
	G.main->colorspaces.first = NULL;
	G.main->colorspaces.last = NULL;
	
	G.main->color_managed_displays.first = NULL;
	G.main->color_managed_displays.last = NULL;
	
	configdir = BLI_get_folder(BLENDER_DATAFILES, "colormanagement");
	
	if(configdir)
	{
		BLI_join_dirfile(configfile, sizeof(configfile), configdir, BKE_COLORMANAGEMENT_PROFILE);
	}
	
	config = OCIO_configCreateFromFile(configfile);
	
	if(config)
	{
		
		OCIO_setCurrentConfig(config);
		
		// load colorspaces
		nrColorSpaces = OCIO_configGetNumColorSpaces(config);
		for (index = 0 ; index < nrColorSpaces; index ++)
		{
			ColorSpace* colorSpace;
			ConstColorSpaceRcPtr* ociocs;
			
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
			
			display->index = index;
			BLI_strncpy(display->display_name, displayname, 32);
			BLI_addtail(&G.main->color_managed_displays, display);
			
			
			// load views
			nrViews = OCIO_configGetNumViews(config, displayname);
			for (viewindex = 0 ; viewindex < nrViews; viewindex ++)
			{
				const char* viewname;
				ColorManagedView* view;
				
				viewname = OCIO_configGetView(config, displayname, viewindex);
				name = OCIO_configGetDisplayColorSpaceName(config, displayname, viewname);
				
				view = MEM_mallocN(sizeof(ColorManagedView), "ColorManagedView");
				//view->index = viewindex;
				view->index = viewindex2++;
				view->parent_display_index = index;
				BLI_strncpy(view->view_name, viewname, 32);
				BLI_strncpy(view->colorspace_name, name, 32);
				
				BLI_addtail(&display->views, view);
			}
		}
		OCIO_configRelease(config);
		
		//fix windows with bad display
		{
			wmWindowManager* wm = CTX_wm_manager(C);
			wmWindow* w = wm->windows.first;
			
			while(w)
			{
				ColorManagedDisplay* display = cmGetDisplay(w->colormanaged_display);
				if(!display)
				{
					display = cmGetDefaultDisplay();
					printf("Blender color management: Window display \"%s\" not found, setting default \"%s\".", w->colormanaged_display, display->display_name);
					BLI_strncpy(w->colormanaged_display, display->display_name, 32);
				}
				w = w->next;
			}
		}
		
		
		//fix space image with bad viewer
		{
			//todo
		}
		
		//fix images input/output with bad colorspaces
		{
			//todo
		}
	}
}

void cmExit(void)
{
	ColorSpace* cs;
	ColorManagedDisplay* cd;
	EnumPropertyItem* items;
	
	items = cmGetColorSpaces();
	MEM_freeN(items);
	
	items = cmGetDisplays();
	MEM_freeN(items);
	
	items = cmGetViews(0);
	MEM_freeN(items);
	
	cs = G.main->colorspaces.first;
	while(cs)
	{
		ColorSpace* cs2 = cs;
		cs = cs->next;
		MEM_freeN(cs2);
	}
	
	cd = G.main->color_managed_displays.first;
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


ColorSpace* cmGetColorSpace(const char* name)
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

ColorManagedDisplay* cmGetDisplay(const char* name)
{
	ColorManagedDisplay* cd = G.main->color_managed_displays.first;
	while(cd)
	{
		if( strcmp(cd->display_name, name) == 0 )
			return cd;
		cd = cd->next;
	}
	return 0;
}

ColorManagedDisplay* cmGetDisplayFromIndex(int index)
{
	ColorManagedDisplay* cd = G.main->color_managed_displays.first;
	while(cd)
	{
		if( cd->index == index )
			return cd;
		cd = cd->next;
	}
	return 0;
}

ColorManagedDisplay* cmGetDefaultDisplay(void)
{
	ConstConfigRcPtr* config = OCIO_getCurrentConfig();
	const char* display = OCIO_configGetDefaultDisplay(config);
	OCIO_configRelease(config);
	return cmGetDisplay(display);
}

ColorManagedView* cmGetView(ColorManagedDisplay* display, const char* name)
{
	if(display)
	{
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

ColorManagedView* cmGetViewFromName(const char* display, const char* name)
{
	ColorManagedDisplay* cd = cmGetDisplay(display);
	return cmGetView(cd, name);
}

ColorManagedView* cmGetViewFromIndex(int index)
{
	ColorManagedDisplay* cd = G.main->color_managed_displays.first;
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



// RNA helpers
//***********************************
EnumPropertyItem* cmGetColorSpaces(void)
{
	static EnumPropertyItem *items = 0;
	
	if(items == 0)
	{
		int totitem = 0;
		ColorSpace* cs = G.main->colorspaces.first;
		while(cs)
		{
			EnumPropertyItem item;
			
			item.value = cs->index;
			item.name = cs->name;
			item.identifier = cs->name;
			item.icon = 0; //ICON_COLOR;
			item.description = "";
			
			RNA_enum_item_add(&items, &totitem, &item);
			
			cs = cs->next;
		}
		
		RNA_enum_item_end(&items, &totitem);
	}
	
	return items;
}

EnumPropertyItem* cmGetDisplays(void)
{
	static EnumPropertyItem *items = 0;
	
	if(items == 0)
	{
		int totitem = 0;
		ColorManagedDisplay* cd = G.main->color_managed_displays.first;
		while(cd)
		{
			EnumPropertyItem item;
			
			item.value = cd->index;
			item.name = cd->display_name;
			item.identifier = cd->display_name;
			item.icon = 0; //ICON_COLOR;
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
			item.icon = 0; //ICON_COLOR;
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
	ColorManagedDisplay* display = cmGetDisplay(name);
	return cmGetViews(display);
}

#endif //WITH_OCIO

