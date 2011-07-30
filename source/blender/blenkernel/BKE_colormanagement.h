
#ifndef BKE_COLORMANAGEMENT_H
#define BKE_COLORMANAGEMENT_H

#define BKE_COLORMANAGEMENT_PROFILE_PATH "/home/xavier/BlenderOCIOProfile/config.ocio"

#include "DNA_color_types.h"
//#include "RNA_types.h"

void cmInit(void);
void cmExit(void);

ColorSpace* cmGetColorSpaceFromName(const char* name);
ColorSpace* cmGetColorSpaceFromIndex(int index);
DisplayColorSpace* cmGetDisplayColorSpaceFromName(const char* name);
DisplayColorSpace* cmGetDisplayColorSpaceFromIndex(int index);
DisplayColorSpace* cmGetDefaultDisplayColorSpace(void);

void cmApplyTransform(float* data, long w, long h, long channels, const char* src, const char* dst);

struct EnumPropertyItem;
struct EnumPropertyItem* cmGetColorSpaces(void);
struct EnumPropertyItem* cmGetDisplayColorSpaces(void);

#endif // BKE_COLORMANAGEMENT_H
