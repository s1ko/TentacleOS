#include "wifi_scan_ui.h"
#include "ui_theme.h"
#include "ui_manager.h"
#include "esp_log.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "wifi_service.h"
#include "freertos/task.h"

static const char *TAG = "UI_SCAN";

static lv_obj_t * screen_scan = NULL;
static lv_obj_t * spinner = NULL;
static lv_obj_t * lbl_status = NULL;

static void scan_worker_task(void *arg)
{
    ESP_LOGI(TAG, "Iniciando Scan em Background...");

    wifi_service_scan();
    
    if (ui_acquire()) {
        if (spinner) {
            lv_obj_del(spinner);
            spinner = NULL;
        }

        if (lbl_status) {
            lv_label_set_text(lbl_status, "Scan Concluido!");
            lv_obj_set_style_text_color(lbl_status, current_theme.text_main, 0);
            lv_obj_align(lbl_status, LV_ALIGN_CENTER, 0, 0);
            
            lv_obj_t * icon = lv_label_create(screen_scan);
            lv_label_set_text(icon, LV_SYMBOL_OK);
            lv_obj_set_style_text_color(icon, current_theme.text_main, 0);
            lv_obj_align(icon, LV_ALIGN_CENTER, 0, -30);
        }
        
        ui_release();
    }

    vTaskDelay(pdMS_TO_TICKS(1500));
    ui_switch_screen(SCREEN_WIFI_MENU); 

    vTaskDelete(NULL);
}

void ui_wifi_scan_open(void)
{
    if (screen_scan) {
        lv_obj_del(screen_scan);
        screen_scan = NULL;
    }

    screen_scan = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_scan, current_theme.screen_base, 0);

    spinner = lv_spinner_create(screen_scan);
    lv_obj_set_size(spinner, 80, 80);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -20);
    
    // Tema aplicado ao Spinner
    lv_obj_set_style_arc_color(spinner, current_theme.border_accent, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, current_theme.border_inactive, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);

    lbl_status = lv_label_create(screen_scan);
    lv_label_set_text(lbl_status, "Scanning...");
    lv_obj_set_style_text_color(lbl_status, current_theme.text_main, 0);
    lv_obj_align(lbl_status, LV_ALIGN_CENTER, 0, 40);

    lv_screen_load(screen_scan);

    xTaskCreate(scan_worker_task, "WifiScanWorker", 4096, NULL, 5, NULL);
}