#ifndef __EMOJI_FONT_H__
#define __EMOJI_FONT_H__
#include "lvgl.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
void emoji_font_init(void);
extern lv_font_t *imgfont;
#ifdef __cplusplus
}
#endif

#endif
