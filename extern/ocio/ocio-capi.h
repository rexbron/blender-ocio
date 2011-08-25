
#ifndef OCIO_CAPI_H
#define OCIO_CAPI_H



#ifdef __cplusplus
using namespace OCIO_NAMESPACE;
extern "C" { 
#endif


#ifndef OCIO_CAPI_IMPLEMENTATION
	#define OCIO_ROLE_SCENE_LINEAR	"scene_linear"
	#define OCIO_ROLE_COLOR_PICKING	"color_picking"
	#define OCIO_ROLE_TEXTURE_PAINT	"texture_paint"
	
	typedef void ConstConfigRcPtr;
	typedef void ConstColorSpaceRcPtr;
	typedef void ConstProcessorRcPtr;
	typedef void ConstContextRcPtr;
	typedef void PackedImageDesc;
	typedef void DisplayTransformRcPtr;
	typedef void ConstTransformRcPtr;
#endif


extern ConstConfigRcPtr* OCIO_getCurrentConfig(void);
extern void OCIO_setCurrentConfig(const ConstConfigRcPtr* config);

extern ConstConfigRcPtr* OCIO_configCreateFromEnv(void);
extern ConstConfigRcPtr* OCIO_configCreateFromFile(const char* filename);

extern void OCIO_configRelease(ConstConfigRcPtr* config);

extern int OCIO_configGetNumColorSpaces(ConstConfigRcPtr* config);
extern const char* OCIO_configGetColorSpaceNameByIndex(ConstConfigRcPtr* config, int index);
extern ConstColorSpaceRcPtr* OCIO_configGetColorSpace(ConstConfigRcPtr* config, const char* name);
extern int OCIO_configGetIndexForColorSpace(ConstConfigRcPtr* config, const char* name);

extern void OCIO_colorSpaceRelease(ConstColorSpaceRcPtr* cs);

extern const char* OCIO_configGetDefaultDisplay(ConstConfigRcPtr* config);
extern int         OCIO_configGetNumDisplays(ConstConfigRcPtr* config);
extern const char* OCIO_configGetDisplay(ConstConfigRcPtr* config, int index);
extern const char* OCIO_configGetDefaultView(ConstConfigRcPtr* config, const char* display);
extern int         OCIO_configGetNumViews(ConstConfigRcPtr* config, const char* display);
extern const char* OCIO_configGetView(ConstConfigRcPtr* config, const char* display, int index);
extern const char* OCIO_configGetDisplayColorSpaceName(ConstConfigRcPtr* config, const char* display, const char* view);

extern ConstProcessorRcPtr* OCIO_configGetProcessorWithNames(ConstConfigRcPtr* config, const char* srcName, const char* dstName);
extern ConstProcessorRcPtr* OCIO_configGetProcessor(ConstConfigRcPtr* config, ConstTransformRcPtr* transform);

extern void OCIO_processorApply(ConstProcessorRcPtr* processor, PackedImageDesc* img);
extern void OCIO_processorApplyRGB(ConstProcessorRcPtr* processor, float* pixel);
extern void OCIO_processorApplyRGBA(ConstProcessorRcPtr* processor, float* pixel);

extern void OCIO_processorRelease(ConstProcessorRcPtr* p);


extern const char* OCIO_colorSpaceGetName(ConstColorSpaceRcPtr* cs);
extern const char* OCIO_colorSpaceGetFamily(ConstColorSpaceRcPtr* cs);

extern DisplayTransformRcPtr* OCIO_createDisplayTransform(void);
extern void OCIO_displayTransformSetInputColorSpaceName(DisplayTransformRcPtr* dt, const char * name);
extern void OCIO_displayTransformSetDisplay(DisplayTransformRcPtr* dt, const char * name);
extern void OCIO_displayTransformSetView(DisplayTransformRcPtr* dt, const char * name);
extern void OCIO_displayTransformRelease(DisplayTransformRcPtr* dt);

PackedImageDesc* OCIO_createPackedImageDesc(float * data, long width, long height, long numChannels,
											long chanStrideBytes, long xStrideBytes, long yStrideBytes);

extern void OCIO_packedImageDescRelease(PackedImageDesc* p);

#ifdef __cplusplus
}
#endif

#endif //OCIO_CAPI_H

