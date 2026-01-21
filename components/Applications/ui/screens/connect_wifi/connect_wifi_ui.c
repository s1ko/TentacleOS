#include "connect_wifi_ui.h"
#include "header_ui.h"
#include "footer_ui.h"
#include "ui_theme.h"
#include "core/lv_group.h"
#include "ui_manager.h"
#include "buzzer.h"
#include "wifi_service.h" 
#include "esp_wifi.h"     

extern lv_group_t * main_group;

static lv_obj_t * screen_wifi_list = NULL;
static lv_style_t style_menu;
static lv_style_t style_item;
static bool styles_initialized = false;

static void init_styles(void) {
    if(styles_initialized) return;

    lv_style_init(&style_menu);
    lv_style_set_bg_opa(&style_menu, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_menu, 2);
    lv_style_set_border_color(&style_menu, current_theme.border_accent);
    lv_style_set_radius(&style_menu, 0); 
    lv_style_set_pad_all(&style_menu, 4);
    lv_style_set_pad_row(&style_menu, 4);

    lv_style_init(&style_item);
    lv_style_set_bg_color(&style_item, current_theme.bg_item_bot);
    lv_style_set_bg_grad_color(&style_item, current_theme.bg_item_top);
    lv_style_set_bg_grad_dir(&style_item, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_item, 1);
    lv_style_set_border_color(&style_item, current_theme.border_inactive);
    lv_style_set_radius(&style_item, 0);

    styles_initialized = true;
}

static void wifi_item_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * item = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        buzzer_play_sound_file("buzzer_scroll_tick");
        lv_obj_set_style_border_color(item, current_theme.border_accent, 0);
        lv_obj_set_style_border_width(item, 2, 0);
        lv_obj_scroll_to_view(item, LV_ANIM_ON);
    } 
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_style_border_color(item, current_theme.border_inactive, 0);
        lv_obj_set_style_border_width(item, 1, 0);
    }
    else if(code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);

        if(key == LV_KEY_ESC || key == LV_KEY_LEFT) {
            buzzer_play_sound_file("buzzer_click");
            ui_switch_screen(SCREEN_CONNECTION_SETTINGS);
            return;
        }

        if(key == LV_KEY_ENTER || key == LV_KEY_RIGHT) {
            buzzer_play_sound_file("buzzer_hacker_confirm");
        }
    }
}

void ui_connect_wifi_open(void) {
    init_styles();
    if(screen_wifi_list) lv_obj_del(screen_wifi_list);

    screen_wifi_list = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_wifi_list, current_theme.screen_base, 0);
    lv_obj_clear_flag(screen_wifi_list, LV_OBJ_FLAG_SCROLLABLE);

    header_ui_create(screen_wifi_list);
    footer_ui_create(screen_wifi_list);

    lv_obj_t * menu = lv_obj_create(screen_wifi_list);
    lv_obj_set_size(menu, 230, 160);
    lv_obj_align(menu, LV_ALIGN_CENTER, 0, 5);
    lv_obj_add_style(menu, &style_menu, 0);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * loading_label = lv_label_create(menu);
    lv_label_set_text(loading_label, "BUSCANDO REDES...");
    lv_obj_set_style_text_color(loading_label, current_theme.text_main, 0);
    lv_obj_set_width(loading_label, lv_pct(100));
    lv_obj_set_style_text_align(loading_label, LV_TEXT_ALIGN_CENTER, 0);
    
    lv_screen_load(screen_wifi_list);
    lv_refr_now(NULL); 

    wifi_start(); 
    vTaskDelay(pdMS_TO_TICKS(100)); 
    wifi_service_scan(); 
    lv_obj_del(loading_label);

    uint16_t ap_count = wifi_service_get_ap_count();

    if (ap_count == 0) {
        lv_obj_t * empty_label = lv_label_create(menu);
        lv_label_set_text(empty_label, "NENHUMA REDE");
        lv_obj_set_style_text_color(empty_label, current_theme.text_main, 0);
        lv_obj_set_width(empty_label, lv_pct(100));
        lv_obj_set_style_text_align(empty_label, LV_TEXT_ALIGN_CENTER, 0);
    } else {
        for(int i = 0; i < ap_count; i++) {
            wifi_ap_record_t * ap = wifi_service_get_ap_record(i);
            if(!ap) continue;

            lv_obj_t * item = lv_obj_create(menu);
            lv_obj_set_size(item, lv_pct(100), 40);
            lv_obj_add_style(item, &style_item, 0);
            lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t * icon = lv_label_create(item);
            lv_label_set_text(icon, LV_SYMBOL_WIFI);
            lv_obj_set_style_text_color(icon, current_theme.text_main, 0); 

            lv_obj_t * lbl_ssid = lv_label_create(item);
            lv_label_set_text(lbl_ssid, (char*)ap->ssid);
            lv_obj_set_style_text_color(lbl_ssid, current_theme.text_main, 0); 
            lv_obj_set_flex_grow(lbl_ssid, 1);
            lv_obj_set_style_margin_left(lbl_ssid, 8, 0);

            if(ap->authmode != WIFI_AUTH_OPEN) {
                lv_obj_t * lock = lv_label_create(item);
                lv_label_set_text(lock, LV_SYMBOL_SETTINGS); 
                lv_obj_set_style_text_color(lock, current_theme.text_main, 0); 
            }

            lv_obj_add_event_cb(item, wifi_item_event_cb, LV_EVENT_ALL, NULL);
            
            if(main_group) {
                lv_group_add_obj(main_group, item);
            }
        }
    }
}