
#ifndef BKE_COLORMANAGEMENT_H
#define BKE_COLORMANAGEMENT_H

#define BCM_CONFIG_FILE "config.ocio"

#include "DNA_color_types.h"

struct EnumPropertyItem;
struct ImBuf;
struct wmWindow;

void BCM_init(void);
void BCM_exit(void);

ColorSpace* BCM_get_colorspace(const char* name);
ColorSpace* BCM_get_colorspace_from_index(int index);
ColorSpace* BCM_get_default_imbuf_colorspace(struct ImBuf* ibuf);
ColorSpace* BCM_get_default_8bits_colorspace(void);
ColorSpace* BCM_get_default_16bits_colorspace(void);
ColorSpace* BCM_get_default_log_colorspace(void);
ColorSpace* BCM_get_default_float_colorspace(void);
ColorSpace* BCM_get_scene_linear_colorspace(void);
ColorSpace* BCM_get_color_picking_colorspace(void);
ColorSpace* BCM_get_texture_paint_colorspace(void);
ColorSpace* BCM_get_sequencer_colorspace(void);
ColorSpace* BCM_get_ui_colorspace(struct wmWindow* window);

ColorManagedDisplay* BCM_get_display(const char* name);
ColorManagedDisplay* BCM_get_display_from_index(int index);
ColorManagedDisplay* BCM_get_default_display(void);

ColorManagedView* BCM_get_view(ColorManagedDisplay* display, const char* name);
ColorManagedView* BCM_get_view_from_name(const char* display, const char* name);
ColorManagedView* BCM_get_view_from_index(int index);
ColorManagedView* BCM_get_default_view(ColorManagedDisplay* display);

void BCM_apply_transform(float* data, long w, long h, int channel, const char* src, const char* dst);
void BCM_apply_transform_to_byte(float* dataf, unsigned char *datac, long w, long h, const char* src, const char* dst);
void BCM_make_imbuf_float_linear(struct ImBuf * ibuf);

void BCM_apply_display_transform(struct ImBuf *ibuf, const char* display, const char* view);

/* create char buffer, color corrected if necessary, for ImBufs that lack one */
void IMB_rect_from_float(struct ImBuf *ibuf);
/* create linear float buffer for ImBufs that lack one */
void IMB_float_from_rect(struct ImBuf *ibuf);
/* force the colorspace of an Imbuf */
void IMB_convert_profile(struct ImBuf *ibuf, const ColorSpace* profile);

/* create char buffer for part of the image, color corrected if necessary,
   Changed part will be stored in buffer. This is expected to be used for texture painting updates */ 
void IMB_partial_rect_from_float(struct ImBuf *ibuf, float *buffer, int x, int y, int w, int h);



/* RNA helpers */
void BCM_add_colorspaces_items(struct EnumPropertyItem** items, int* totitem, int add_default, struct ImBuf *ibuf);
void BCM_add_displays_items(struct EnumPropertyItem** items, int* totitem);
void BCM_add_views_items(struct EnumPropertyItem** items, int* totitem, ColorManagedDisplay* display);
void BCM_add_views_items_from_display_name(struct EnumPropertyItem** items, int *totitem, const char* display);

#endif // BKE_COLORMANAGEMENT_H
