
#ifndef OCIO_CAPI_H
#define OCIO_CAPI_H



#ifdef __cplusplus
using namespace OCIO_NAMESPACE;
extern "C" { 
#endif


#ifndef OCIO_CAPI_IMPLEMENTATION
	typedef void ConstConfigRcPtr;
	typedef void ConstColorSpaceRcPtr;
	typedef void ConstProcessorRcPtr;
	typedef void ConstContextRcPtr;
	typedef void PackedImageDesc;
#endif


extern ConstConfigRcPtr* OCIO_getCurrentConfig(void);
extern void OCIO_setCurrentConfig(const ConstConfigRcPtr* config);

//extern ConfigRcPtr* OCIO_configCreate();
extern ConstConfigRcPtr* OCIO_configCreateFromEnv(void);
extern ConstConfigRcPtr* OCIO_configCreateFromFile(const char* filename);

extern void OCIO_configRelease(ConstConfigRcPtr* config);

extern int OCIO_configGetNumColorSpaces(ConstConfigRcPtr* config);
extern const char* OCIO_configGetColorSpaceNameByIndex(ConstConfigRcPtr* config, int index);
extern ConstColorSpaceRcPtr* OCIO_configGetColorSpace(ConstConfigRcPtr* config, const char* name);
extern int OCIO_configGetIndexForColorSpace(ConstConfigRcPtr* config, const char* name);

extern void OCIO_colorSpaceRelease(ConstColorSpaceRcPtr* cs);

////extern void OCIO_configSetRole(ConfigRcPtr* config, const char* role, const char* colorSpaceName);
//extern int OCIO_configGetNumRoles(ConstConfigRcPtr* config);
//extern int OCIO_configHasRole(ConstConfigRcPtr* config, const char* role);
//extern const char* OCIO_configGetRoleName(ConstConfigRcPtr* config, int index);

extern const char* OCIO_configGetDefaultDisplay(ConstConfigRcPtr* config);
extern int         OCIO_configGetNumDisplays(ConstConfigRcPtr* config);
extern const char* OCIO_configGetDisplay(ConstConfigRcPtr* config, int index);
extern const char* OCIO_configGetDefaultView(ConstConfigRcPtr* config, const char* display);
extern int         OCIO_configGetNumViews(ConstConfigRcPtr* config, const char* display);
extern const char* OCIO_configGetView(ConstConfigRcPtr* config, const char* display, int index);
extern const char* OCIO_configGetDisplayColorSpaceName(ConstConfigRcPtr* config, const char* display, const char* view);
//extern const char* OCIO_configGetDisplayLooks(ConstConfigRcPtr* config, const char* display, const char* view);
////extern void OCIO_configAddDisplay(ConfigRcPtr* config, const char* display, const char* view, const char* colorSpaceName, const char* looks);
////extern void OCIO_configClearDisplays(ConfigRcPtr* config);
////extern void OCIO_configSetActiveDisplays(ConfigRcPtr* config, const char* displays);
//extern const char* OCIO_configGetActiveDisplays(ConstConfigRcPtr* config);
////extern void OCIO_configSetActiveViews(ConfigRcPtr* config, const char* displays);
//extern const char* OCIO_configGetActiveViews(ConstConfigRcPtr* config);

//extern ConstProcessorRcPtr* OCIO_configGetProcessorWithContext(ConstConfigRcPtr* config, const ConstContextRcPtr* context, const ConstColorSpaceRcPtr* srcColorSpace, const ConstColorSpaceRcPtr* dstColorSpace);
//extern ConstProcessorRcPtr* OCIO_configGetProcessor(ConstConfigRcPtr* config, const ConstColorSpaceRcPtr* srcColorSpace, const ConstColorSpaceRcPtr* dstColorSpace);
extern ConstProcessorRcPtr* OCIO_configGetProcessorWithNames(ConstConfigRcPtr* config, const char* srcName, const char* dstName);
//extern ConstProcessorRcPtr* OCIO_configGetProcessorWithContextAndNames(ConstConfigRcPtr* config, const ConstContextRcPtr* context, const char* srcName, const char* dstName);

extern void OCIO_processorApply(ConstProcessorRcPtr* processor, PackedImageDesc* img);
extern void OCIO_processorApplyRGB(ConstProcessorRcPtr* processor, float* pixel);
extern void OCIO_processorApplyRGBA(ConstProcessorRcPtr* processor, float* pixel);

extern void OCIO_processorRelease(ConstProcessorRcPtr* p);

////extern ColorSpaceRcPtr OCIO_colorSpaceCreate(ColorSpaceRcPtr* cs);
////extern void OCIO_colorSpaceSetName(ColorSpaceRcPtr* cs, const char* name);
////extern void OCIO_colorSpaceSetFamily(ColorSpaceRcPtr* cs, const char* family);
////extern void COCIO_clorSpaceSetDescription(ColorSpaceRcPtr* cs, const char* description);
////extern void OCIO_colorSpaceSetBitDepth(ColorSpaceRcPtr* cs, BitDepth bitDepth);
////extern ColorSpaceRcPtr OCIO_colorSpaceCreateEditableCopy(ConstColorSpaceRcPtr* cs);
//extern const char* OCIO_colorSpaceGetName(ConstColorSpaceRcPtr* cs);
extern const char* OCIO_colorSpaceGetFamily(ConstColorSpaceRcPtr* cs);
//extern const char* OCIO_colorSpaceGetDescription(ConstColorSpaceRcPtr* cs);
//extern OCIO_BitDepth OCIO_colorSpaceGetBitDepth(ConstColorSpaceRcPtr* cs);

PackedImageDesc* OCIO_createPackedImageDesc(float * data, long width, long height, long numChannels,
											long chanStrideBytes, long xStrideBytes, long yStrideBytes);

extern void OCIO_packedImageDescRelease(PackedImageDesc* p);

#ifdef __cplusplus
}
#endif

#endif //OCIO_CAPI_H

