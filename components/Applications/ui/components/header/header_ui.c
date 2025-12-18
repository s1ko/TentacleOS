#include "header_ui.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <stdio.h>
#include <sys/stat.h>

#define HEADER_HEIGHT 24
#define COLOR_PURPLE_MAIN    0x1A053D
#define HEADER_BORDER_COLOR  0x5E12A0

static const char *TAG = "HEADER_UI";

static lv_image_dsc_t* load_bin_to_psram(const char * path, int32_t w, int32_t h) {
    struct stat st;
    if (stat(path, &st) != 0) {
        ESP_LOGE(TAG, "File not found: %s", path);
        return NULL;
    }

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    // Header de 12 bytes padrão do LVGL 9
    int header_size = 12; 
    long pixel_data_size = st.st_size - header_size;

    // Alocação em PSRAM (MALLOC_CAP_SPIRAM)
    lv_image_dsc_t * dsc = (lv_image_dsc_t *)heap_caps_malloc(sizeof(lv_image_dsc_t), MALLOC_CAP_SPIRAM);
    uint8_t * pixel_data = (uint8_t *)heap_caps_malloc(pixel_data_size, MALLOC_CAP_SPIRAM);

    if (!dsc || !pixel_data) {
        ESP_LOGE(TAG, "Error: Insuficient PSRAM Memory");
        if (dsc) free(dsc);
        fclose(f);
        return NULL;
    }

    fseek(f, header_size, SEEK_SET);
    fread(pixel_data, 1, pixel_data_size, f);
    fclose(f);

    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.cf = LV_COLOR_FORMAT_ARGB8888; // Alterado para 32-bit com Alpha
    dsc->header.w = w;
    dsc->header.h = h;
    dsc->header.stride = w * 4; // ARGB8888 = 4 bytes por pixel
    dsc->data_size = pixel_data_size;
    dsc->data = pixel_data;

    ESP_LOGI(TAG, "Icon ARGB8888 loaded: %s (%dx%d)", path, w, h);
    return dsc;
}

void header_ui_create(lv_obj_t * parent)
{
    lv_obj_t * header = lv_obj_create(parent);
    lv_obj_set_size(header, lv_pct(100), HEADER_HEIGHT);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_remove_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_PURPLE_MAIN), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(header, 2, 0);
    lv_obj_set_style_border_color(header, lv_color_hex(HEADER_BORDER_COLOR), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    /* MENU (Esquerda) */
    lv_obj_t * img_menu = lv_image_create(header);
    lv_image_set_src(img_menu, "A:/label/HOME_MENU.png"); // Caminho corrigido conforme log
    lv_obj_align(img_menu, LV_ALIGN_LEFT_MID, 8, 0);

    /* Container de Ícones (Direita) */
    lv_obj_t * icon_cont = lv_obj_create(header);
    lv_obj_set_size(icon_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(icon_cont, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_obj_set_flex_flow(icon_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(icon_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(icon_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon_cont, 0, 0);
    lv_obj_set_style_pad_column(icon_cont, 6, 0);

    // Carregamento do ícone Bateria (agora em RGB565)
    static lv_image_dsc_t * bin_battery = NULL;
    if (bin_battery == NULL) {
        // Use o caminho absoluto que o seu log de boot confirmou (/assets)
        bin_battery = load_bin_to_psram("/assets/icons/BATTERY_ICON.bin", 20, 20);
    }

    if (bin_battery) {
        lv_obj_t * img_bat = lv_image_create(icon_cont);
        lv_image_set_src(img_bat, bin_battery);
    }

    const char * other_icons[] = {
        "A:/icons/BLE_ICON.png",
        "A:/icons/HEADPHONE_ICON.png",
        "A:/icons/STORAGE_ICON.png"
    };

    for(int i = 0; i < 3; i++) {
        lv_obj_t * img = lv_image_create(icon_cont);
        lv_image_set_src(img, other_icons[i]);
    }
}
