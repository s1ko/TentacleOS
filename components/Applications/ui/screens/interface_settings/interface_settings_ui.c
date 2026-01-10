#include "interface_settings_ui.h"
#include "header_ui.h"
#include "footer_ui.h"
#include "core/lv_group.h"
#include "ui_manager.h"
#include "lv_port_indev.h"
#include "buzzer.h"
#include "esp_log.h"

#define BG_COLOR            lv_color_black()
#define COLOR_BORDER        0x834EC6
#define COLOR_GRADIENT_TOP  0x000000
#define COLOR_GRADIENT_BOT  0x2E0157

static lv_obj_t * screen_interface = NULL;
static lv_style_t style_menu;
static lv_style_t style_item;
static bool styles_initialized = false;

static int color_idx = 0;
const char * color_options[] = {"PURPLE", "CYAN", "RED", "GREEN"};
static int header_idx = 0;
const char * header_options[] = {"MINIMAL", "FULL", "NONE"};
static bool hide_footer = false;
static int lang_idx = 0;
const char * lang_options[] = {"EN", "PT", "ES", "FR"};

static void init_styles(void) {
    if(styles_initialized) return;

    lv_style_init(&style_menu);
    lv_style_set_bg_opa(&style_menu, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_menu, 2);
    lv_style_set_border_color(&style_menu, lv_color_hex(COLOR_BORDER));
    lv_style_set_radius(&style_menu, 6);
    lv_style_set_pad_all(&style_menu, 8);
    lv_style_set_pad_row(&style_menu, 8);

    lv_style_init(&style_item);
    lv_style_set_bg_color(&style_item, lv_color_hex(COLOR_GRADIENT_BOT));
    lv_style_set_bg_grad_color(&style_item, lv_color_hex(COLOR_GRADIENT_TOP));
    lv_style_set_bg_grad_dir(&style_item, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_item, 1);
    lv_style_set_border_color(&style_item, lv_color_hex(0x333333));
    lv_style_set_radius(&style_item, 4);

    styles_initialized = true;
}

static void update_footer_switch(lv_obj_t * cont) {
    lv_obj_t * b1 = lv_obj_get_child(cont, 0);
    lv_obj_t * b2 = lv_obj_get_child(cont, 1);
    lv_obj_set_style_bg_opa(b1, !hide_footer ? LV_OPA_COVER : LV_OPA_20, 0);
    lv_obj_set_style_bg_opa(b2, hide_footer ? LV_OPA_COVER : LV_OPA_20, 0);
}

static void interface_item_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * item = lv_event_get_target(e);
    int type = (int)lv_event_get_user_data(e);

    if(code == LV_EVENT_FOCUSED) {
        buzzer_scroll_tick();
        lv_obj_set_style_border_color(item, lv_color_hex(COLOR_BORDER), 0);
        lv_obj_set_style_border_width(item, 2, 0);
        lv_obj_scroll_to_view(item, LV_ANIM_ON);
    } 
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_style_border_color(item, lv_color_hex(0x333333), 0);
        lv_obj_set_style_border_width(item, 1, 0);
    }
    else if(code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        lv_obj_t * input_obj = lv_obj_get_child(item, 2);

        if(type == 0) {
            if(key == LV_KEY_RIGHT) color_idx = (color_idx + 1) % 4;
            if(key == LV_KEY_LEFT) color_idx = (color_idx - 1 + 4) % 4;
            lv_label_set_text_fmt(input_obj, "< %s >", color_options[color_idx]);
            buzzer_scroll_tick();
        }
        else if(type == 1) {
            if(key == LV_KEY_RIGHT) header_idx = (header_idx + 1) % 3;
            if(key == LV_KEY_LEFT) header_idx = (header_idx - 1 + 3) % 3;
            lv_label_set_text_fmt(input_obj, "< %s >", header_options[header_idx]);
            buzzer_scroll_tick();
        }
        else if(type == 2) {
            if(key == LV_KEY_ENTER || key == LV_KEY_RIGHT || key == LV_KEY_LEFT) {
                hide_footer = !hide_footer;
                update_footer_switch(input_obj);
                buzzer_hacker_confirm();
            }
        }
        else if(type == 3) {
            if(key == LV_KEY_RIGHT) lang_idx = (lang_idx + 1) % 4;
            if(key == LV_KEY_LEFT) lang_idx = (lang_idx - 1 + 4) % 4;
            lv_label_set_text_fmt(input_obj, "< %s >", lang_options[lang_idx]);
            buzzer_scroll_tick();
        }
    }
}

static void screen_back_event_cb(lv_event_t * e) {
    uint32_t key = lv_event_get_key(e);
    if(key == LV_KEY_ESC || key == LV_KEY_LEFT) {
        ui_switch_screen(SCREEN_SETTINGS);
    }
}

static lv_obj_t * create_menu_item(lv_obj_t * parent, const char * symbol, const char * name) {
    lv_obj_t * item = lv_obj_create(parent);
    lv_obj_set_size(item, lv_pct(100), 48);
    lv_obj_add_style(item, &style_item, 0);
    lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(item, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * icon = lv_label_create(item);
    lv_label_set_text(icon, symbol);
    lv_obj_set_style_text_color(icon, lv_color_white(), 0);

    lv_obj_t * label = lv_label_create(item);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_flex_grow(label, 1);
    lv_obj_set_style_margin_left(label, 10, 0);

    return item;
}

void ui_interface_settings_open(void) {
    init_styles();
    if(screen_interface) lv_obj_del(screen_interface);

    screen_interface = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_interface, BG_COLOR, 0);
    lv_obj_clear_flag(screen_interface, LV_OBJ_FLAG_SCROLLABLE);

    header_ui_create(screen_interface);
    footer_ui_create(screen_interface);

    lv_coord_t menu_h = 240 - 60;

    lv_obj_t * menu = lv_obj_create(screen_interface);
    lv_obj_set_size(menu, 230, menu_h);
    lv_obj_align(menu, LV_ALIGN_CENTER, 0, 5);
    lv_obj_add_style(menu, &style_menu, 0);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(menu, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_OFF);

    // COLORS
    lv_obj_t * item_color = create_menu_item(menu, LV_SYMBOL_TINT, "COLORS");
    lv_obj_t * color_label = lv_label_create(item_color);
    lv_label_set_text_fmt(color_label, "< %s >", color_options[color_idx]);
    lv_obj_set_style_text_color(color_label, lv_color_white(), 0); // Garantido Branco
    lv_obj_add_event_cb(item_color, interface_item_event_cb, LV_EVENT_ALL, (void*)0);

    // HEADER
    lv_obj_t * item_head = create_menu_item(menu, LV_SYMBOL_UP, "HEADER");
    lv_obj_t * head_label = lv_label_create(item_head);
    lv_label_set_text_fmt(head_label, "< %s >", header_options[header_idx]);
    lv_obj_set_style_text_color(head_label, lv_color_white(), 0); // Garantido Branco
    lv_obj_add_event_cb(item_head, interface_item_event_cb, LV_EVENT_ALL, (void*)1);

    // FOOTER
    lv_obj_t * item_foot = create_menu_item(menu, LV_SYMBOL_DOWN, "FOOTER");
    lv_obj_t * sw_cont = lv_obj_create(item_foot);
    lv_obj_set_size(sw_cont, 56, 30);
    lv_obj_set_style_bg_opa(sw_cont, 0, 0);
    lv_obj_set_style_border_width(sw_cont, 0, 0);
    lv_obj_set_flex_flow(sw_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sw_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(sw_cont, 6, 0);
    lv_obj_clear_flag(sw_cont, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < 2; i++) {
        lv_obj_t * b = lv_obj_create(sw_cont);
        lv_obj_set_size(b, 22, 26);
        lv_obj_set_style_bg_color(b, lv_color_white(), 0);
        lv_obj_set_style_radius(b, 2, 0);
        lv_obj_set_style_border_width(b, 0, 0);
        lv_obj_clear_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    }
    update_footer_switch(sw_cont);
    lv_obj_add_event_cb(item_foot, interface_item_event_cb, LV_EVENT_ALL, (void*)2);

    // LANGUAGE
    lv_obj_t * item_lang = create_menu_item(menu, LV_SYMBOL_DUMMY, "LANGUAGE");
    lv_obj_t * lang_label = lv_label_create(item_lang);
    lv_label_set_text_fmt(lang_label, "< %s >", lang_options[lang_idx]);
    lv_obj_set_style_text_color(lang_label, lv_color_white(), 0); // Garantido Branco
    lv_obj_add_event_cb(item_lang, interface_item_event_cb, LV_EVENT_ALL, (void*)3);

    if(main_group) {
        lv_group_add_obj(main_group, item_color);
        lv_group_add_obj(main_group, item_head);
        lv_group_add_obj(main_group, item_foot);
        lv_group_add_obj(main_group, item_lang);
    }

    lv_obj_add_event_cb(screen_interface, screen_back_event_cb, LV_EVENT_KEY, NULL);
    lv_screen_load(screen_interface);
}