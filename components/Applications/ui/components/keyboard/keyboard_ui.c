#include "keyboard_ui.h"
#include <string.h>
#include "core/lv_group.h"
#include "ui_manager.h"
#include "buzzer.h"

#define COLOR_BORDER        0x834EC6
#define COLOR_GRADIENT_TOP  0x000000
#define COLOR_GRADIENT_BOT  0x2E0157

static lv_obj_t * kb_overlay = NULL;
static lv_obj_t * kb_obj = NULL;
static lv_style_t style_kb;
static lv_style_t style_kb_btn;
static bool kb_styles_init = false;

extern lv_group_t * main_group;

static void init_kb_styles(void) {
    if(kb_styles_init) return;

    lv_style_init(&style_kb);
    lv_style_set_bg_color(&style_kb, lv_color_black());
    lv_style_set_bg_opa(&style_kb, LV_OPA_COVER);
    lv_style_set_border_width(&style_kb, 2);
    lv_style_set_border_color(&style_kb, lv_color_hex(COLOR_BORDER));
    lv_style_set_radius(&style_kb, 8);
    lv_style_set_pad_all(&style_kb, 4);

    lv_style_init(&style_kb_btn);
    lv_style_set_bg_color(&style_kb_btn, lv_color_hex(COLOR_GRADIENT_BOT));
    lv_style_set_bg_grad_color(&style_kb_btn, lv_color_hex(COLOR_GRADIENT_TOP));
    lv_style_set_bg_grad_dir(&style_kb_btn, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_kb_btn, 1);
    lv_style_set_border_color(&style_kb_btn, lv_color_hex(0x444444));
    lv_style_set_radius(&style_kb_btn, 4);
    lv_style_set_text_color(&style_kb_btn, lv_color_white());

    kb_styles_init = true;
}

static void kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * target_kb = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        buzzer_play_sound_file("buzzer_scroll_tick");
        uint16_t btn_id = lv_keyboard_get_selected_btn(target_kb);
        const char * txt = lv_keyboard_get_btn_text(target_kb, btn_id);
        
        if(txt != NULL && strcmp(txt, LV_SYMBOL_OK) == 0) {
            buzzer_play_sound_file("buzzer_hacker_confirm");
            keyboard_close();
        }
    }
    else if(code == LV_EVENT_CANCEL) {
        keyboard_close();
    }
}

void keyboard_open(lv_obj_t * target_textarea) {
    init_kb_styles();

    if(kb_overlay) keyboard_close();

    kb_overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(kb_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(kb_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(kb_overlay, LV_OPA_70, 0);
    lv_obj_clear_flag(kb_overlay, LV_OBJ_FLAG_SCROLLABLE);

    kb_obj = lv_keyboard_create(kb_overlay);
    lv_obj_set_size(kb_obj, 230, 140);
    lv_obj_align(kb_obj, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    lv_obj_add_style(kb_obj, &style_kb, 0);
    lv_obj_add_style(kb_obj, &style_kb_btn, LV_PART_ITEMS);
    
    lv_obj_set_style_bg_color(kb_obj, lv_color_hex(COLOR_BORDER), LV_PART_ITEMS | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_text_color(kb_obj, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_text_font(kb_obj, &lv_font_montserrat_14, 0);

    if(target_textarea) {
        lv_keyboard_set_textarea(kb_obj, target_textarea);
    }

    lv_obj_add_event_cb(kb_obj, kb_event_cb, LV_EVENT_ALL, NULL);

    if(main_group) {
        lv_group_add_obj(main_group, kb_obj);
        lv_group_focus_obj(kb_obj);
    }
}

void keyboard_close(void) {
    if(kb_overlay) {
        lv_obj_del(kb_overlay);
        kb_overlay = NULL;
        kb_obj = NULL;
    }
}