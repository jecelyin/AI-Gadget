#include "ui_player.h"
#include "ui_presenter.h"
#include "ui_player_list.h"
#include "ui_player_crtl.h"

#define INTRO_TIME 2000
#define BAR_COLOR1 lv_color_hex(0xe9dbfc)
#define BAR_COLOR2 lv_color_hex(0x6f8af6)
#define BAR_COLOR3 lv_color_hex(0xffffff)




void ui_player_page(lv_obj_t *parent) {
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN); // 垂直排列
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);

  // lv_obj_set_style_bg_color(title_label, lv_color_white(), 0);
  // lv_obj_set_style_bg_opa(title_label, LV_OPA_COVER, 0);

  create_ctrl_box(cont);
  player_list_create(cont);


#if LV_USE_SDL
  folders.push_back(Folder{"folder1"});
  folders.push_back(Folder{"folder2"});

  audioFiles.push_back(AudioFile{"audio1.mp3", 1024, 300});
  audioFiles.push_back(AudioFile{"audio2.mp3", 2048, 320});
  audioFiles.push_back(AudioFile{"audio3.mp3", 1500, 280});

  ui_player_update_list();
#endif

}
