
#ifndef BKE_COLORMANAGEMENT_H
#define BKE_COLORMANAGEMENT_H

#define BKE_COLORMANAGEMENT_PROFILE "config.ocio"

#include "DNA_color_types.h"

struct bContext;
struct EnumPropertyItem;

void cmInit(struct bContext* C);
void cmExit(void);

ColorSpace* cmGetColorSpace(const char* name);
ColorSpace* cmGetColorSpaceFromIndex(int index);

ColorManagedDisplay* cmGetDisplay(const char* name);
ColorManagedDisplay* cmGetDisplayFromIndex(int index);
ColorManagedDisplay* cmGetDefaultDisplay(void);

ColorManagedView* cmGetView(ColorManagedDisplay* display, const char* name);
ColorManagedView* cmGetViewFromName(const char* display, const char* name);
ColorManagedView* cmGetViewFromIndex(int index);
//ColorManagedView* cmGetDefaultView(ColorManagedDisplay* display);

void cmApplyTransform(float* data, long w, long h, long channels, const char* src, const char* dst);

struct EnumPropertyItem* cmGetColorSpaces(void);
struct EnumPropertyItem* cmGetDisplays(void);
struct EnumPropertyItem* cmGetViews(ColorManagedDisplay* display);
struct EnumPropertyItem* cmGetViewsFromDisplayName(const char* display);

#endif // BKE_COLORMANAGEMENT_H
