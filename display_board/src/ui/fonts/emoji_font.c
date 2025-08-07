#include "lvgl.h"
#include <stdio.h>

LV_IMAGE_DECLARE(f602);
LV_IMAGE_DECLARE(f609);
LV_IMAGE_DECLARE(f60d);
LV_IMAGE_DECLARE(f60f);
LV_IMAGE_DECLARE(f618);
LV_IMAGE_DECLARE(f620);
LV_IMAGE_DECLARE(f62f);
LV_IMAGE_DECLARE(f633);
LV_IMAGE_DECLARE(f636);
LV_IMAGE_DECLARE(f644);
LV_IMAGE_DECLARE(f924);
LV_IMAGE_DECLARE(f266a);
LV_IMAGE_DECLARE(f606);
LV_IMAGE_DECLARE(f60c);
LV_IMAGE_DECLARE(f60e);
LV_IMAGE_DECLARE(f614);
LV_IMAGE_DECLARE(f61c);
LV_IMAGE_DECLARE(f62d);
LV_IMAGE_DECLARE(f631);
LV_IMAGE_DECLARE(f634);
LV_IMAGE_DECLARE(f642);
LV_IMAGE_DECLARE(f914);
LV_IMAGE_DECLARE(f92a);
// typedef const void * (*lv_imgfont_get_path_cb_t)(const lv_font_t * font,
//                                                  uint32_t unicode, uint32_t unicode_next,
//                                                  int32_t * offset_y, void * user_data);
static const void *get_imgfont_path(const lv_font_t *font,
                                    uint32_t unicode, uint32_t unicode_next,
                                    int32_t *offset_y, void *user_data)
{

    // printf("unicode: %lx\n", unicode);
    // LV_LOG_WARN("unicode: %lx", unicode);

    switch (unicode)
    {
    case 0x266A:
        return &f266a;
    case 0x1f914:
        return &f914;
    case 0x1f92a:
        return &f92a;
    case 0x1f602:
        return &f602;
    case 0x1f609:
        return &f609;
    case 0x1f60d:
        return &f60d;
    case 0x1f60f:
        return &f60f;
    case 0x1f618:
        return &f618;
    case 0x1f620:
        return &f620;
    case 0x1f62f:
        return &f62f;
    case 0x1f633:
        return &f633;
    case 0x1f636:
        return &f636;
    case 0x1f644:
        return &f644;
    case 0x1f924:
        return &f924;
    case 0x1f606:
        return &f606;
    case 0x1f60c:
        return &f60c;
    case 0x1f60e:
        return &f60e;
    case 0x1f614:
        return &f614;
    case 0x1f61c:
        return &f61c;
    case 0x1f62d:
        return &f62d;
    case 0x1f631:
        return &f631;
    case 0x1f634:
        return &f634;
    case 0x1f642:
        return &f642;
    default:
        break;
    }

    return false;
}

lv_font_t *imgfont;

void emoji_font_init(void)
{
    imgfont = lv_imgfont_create(44, get_imgfont_path, NULL);
    if (imgfont == NULL)
    {
        LV_LOG_ERROR("imgfont init error");
        return;
    }
    imgfont->base_line = 0;

    imgfont->fallback = LV_FONT_DEFAULT;
}
