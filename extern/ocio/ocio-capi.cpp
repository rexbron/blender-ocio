
#include <iostream>

#include "OpenColorIO/OpenColorIO.h"


#define OCIO_CAPI_IMPLEMENTATION
#include "ocio-capi.h"


ConstConfigRcPtr* OCIO_getCurrentConfig(void)
{
	try
	{
		ConstConfigRcPtr* config =  new ConstConfigRcPtr();
		*config = GetCurrentConfig();
		return config;
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

void OCIO_setCurrentConfig(const ConstConfigRcPtr* config)
{
	if(config)
	{
		try
		{
			SetCurrentConfig(*config);
		}
		catch(Exception & exception)
		{
			std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
		}
	}
}

ConstConfigRcPtr* OCIO_configCreateFromEnv(void)
{
	try
	{
		ConstConfigRcPtr* config =  new ConstConfigRcPtr();
		*config = Config::CreateFromEnv();
		return config;
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}


ConstConfigRcPtr* OCIO_configCreateFromFile(const char* filename)
{
	try
	{
		ConstConfigRcPtr* config =  new ConstConfigRcPtr();
		*config = Config::CreateFromFile(filename);
		return config;
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

void OCIO_configRelease(ConstConfigRcPtr* config)
{
	if(config){
		delete config;
		config =0;
	}
}

int OCIO_configGetNumColorSpaces(ConstConfigRcPtr* config)
{
	try
	{
		return (*config)->getNumColorSpaces();
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

const char* OCIO_configGetColorSpaceNameByIndex(ConstConfigRcPtr* config, int index)
{
	try
	{
		return (*config)->getColorSpaceNameByIndex(index);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

ConstColorSpaceRcPtr* OCIO_configGetColorSpace(ConstConfigRcPtr* config, const char* name)
{
	try
	{
		ConstColorSpaceRcPtr* cs =  new ConstColorSpaceRcPtr();
		*cs = (*config)->getColorSpace(name);
		return cs;
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

int OCIO_configGetIndexForColorSpace(ConstConfigRcPtr* config, const char* name)
{
	try
	{
		return (*config)->getIndexForColorSpace(name);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return -1;
}

const char* OCIO_configGetDefaultDisplay(ConstConfigRcPtr* config)
{
	try
	{
		return (*config)->getDefaultDisplay();
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

int OCIO_configGetNumDisplays(ConstConfigRcPtr* config)
{
	try
	{
		return (*config)->getNumDisplays();
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

const char* OCIO_configGetDisplay(ConstConfigRcPtr* config, int index)
{
	try
	{
		return (*config)->getDisplay(index);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

const char* OCIO_configGetDefaultView(ConstConfigRcPtr* config, const char* display)
{
	try
	{
		return (*config)->getDefaultView(display);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

int OCIO_configGetNumViews(ConstConfigRcPtr* config, const char* display)
{
	try
	{
		return (*config)->getNumViews(display);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

const char* OCIO_configGetView(ConstConfigRcPtr* config, const char* display, int index)
{
	try
	{
		return (*config)->getView(display, index);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

const char* OCIO_configGetDisplayColorSpaceName(ConstConfigRcPtr* config, const char* display, const char* view)
{
	try
	{
		return (*config)->getDisplayColorSpaceName(display, view);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}




void OCIO_colorSpaceRelease(ConstColorSpaceRcPtr* cs)
{
	if(cs){
		delete cs;
		cs =0;
	}
}





ConstProcessorRcPtr* OCIO_configGetProcessorWithNames(ConstConfigRcPtr* config, const char* srcName, const char* dstName)
{
	try
	{
		ConstProcessorRcPtr* p =  new ConstProcessorRcPtr();
		*p = (*config)->getProcessor(srcName, dstName);
		return p;
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

void OCIO_processorApply(ConstProcessorRcPtr* processor, PackedImageDesc* img)
{
	try
	{
		(*processor)->apply(*img);
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
}

extern void OCIO_processorApplyRGB(ConstProcessorRcPtr* processor, float* pixel)
{
	(*processor)->applyRGB(pixel);
}

extern void OCIO_processorApplyRGBA(ConstProcessorRcPtr* processor, float* pixel)
{
	(*processor)->applyRGBA(pixel);
}

extern void OCIO_processorRelease(ConstProcessorRcPtr* p)
{
	if(p){
		delete p;
		p = 0;
	}
}



const char* OCIO_colorSpaceGetFamily(ConstColorSpaceRcPtr* cs)
{
	return (*cs)->getFamily();
}

PackedImageDesc* OCIO_createPackedImageDesc(float * data, long width, long height, long numChannels,
											long chanStrideBytes, long xStrideBytes, long yStrideBytes)
{
	try
	{
		PackedImageDesc* id = new PackedImageDesc(data, width, height, numChannels, chanStrideBytes, xStrideBytes, yStrideBytes);
		return id;
	}
	catch(Exception & exception)
	{
		std::cerr << "OpenColorIO Error: " << exception.what() << std::endl;
	}
	return 0;
}

extern void OCIO_packedImageDescRelease(PackedImageDesc* id)
{
	if(id){
		delete id;
		id = 0;
	}
}

