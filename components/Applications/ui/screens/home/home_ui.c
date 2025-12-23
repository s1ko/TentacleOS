#include "home_ui.h"
#include "header_ui.h"
#include "footer_ui.h"

#include "core/lv_group.h"
#include "ui_manager.h"
#include "lv_port_indev.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include <stdio.h>
#include <sys/stat.h>

#define BG_COLOR lv_color_black()
static const char *TAG = "HOME_UI";

static lv_obj_t * screen_home = NULL;
static lv_image_dsc_t * octobit_img_dsc = NULL;


static lv_image_dsc_t* load_bin_to_psram(const char * path, int32_t w, int32_t h) {
    struct stat st;
    if (stat(path, &st) != 0) {
        ESP_LOGE(TAG, "Arquivo nao encontrado: %s", path);
        return NULL;
    }

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    const int header_size = 12;
    long pixel_data_size = st.st_size - header_size;

    lv_image_dsc_t * dsc = (lv_image_dsc_t *)heap_caps_malloc(sizeof(lv_image_dsc_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint8_t * pixel_data = (uint8_t *)heap_caps_malloc(pixel_data_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!dsc || !pixel_data) {
        ESP_LOGE(TAG, "Falha de memoria PSRAM ao carregar %s", path);
        if (dsc) free(dsc);
        if (pixel_data) free(pixel_data);
        fclose(f);
        return NULL;
    }

    fseek(f, header_size, SEEK_SET);
    size_t read_bytes = fread(pixel_data, 1, pixel_data_size, f);
    fclose(f);

    if(read_bytes != pixel_data_size) {
        ESP_LOGE(TAG, "Erro na leitura dos dados: lido %d de %ld", read_bytes, pixel_data_size);
        free(dsc);
        free(pixel_data);
        return NULL;
    }

    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.cf = LV_COLOR_FORMAT_ARGB8888;
    dsc->header.w = w;
    dsc->header.h = h;
    dsc->header.stride = w * 4; 
    dsc->data_size = pixel_data_size;
    dsc->data = pixel_data;

    ESP_LOGI(TAG, "Imagem carregada com sucesso: %s (%dx%d)", path, w, h);
    return dsc;
}

static void home_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        if(key == LV_KEY_LEFT) {
            ui_switch_screen(SCREEN_MENU);
        }
    }
}

void ui_home_open(void)
{

    if(screen_home) {
        lv_obj_del(screen_home);
        screen_home = NULL;
    }

    // Criar nova tela
    screen_home = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_home, BG_COLOR, 0);
    lv_obj_set_style_bg_opa(screen_home, LV_OPA_COVER, 0);
    lv_obj_remove_flag(screen_home, LV_OBJ_FLAG_SCROLLABLE);

    if(octobit_img_dsc == NULL) {
        octobit_img_dsc = load_bin_to_psram("/assets/img/OCTOBIT.bin", 240, 275);
    }

    if(octobit_img_dsc) {
        lv_obj_t * img_obj = lv_image_create(screen_home);
        lv_image_set_src(img_obj, octobit_img_dsc);
        
        lv_obj_center(img_obj);
        
        lv_obj_set_style_opa(img_obj, LV_OPA_COVER, 0);
    } else {
        ESP_LOGE(TAG, "Nao foi possivel exibir o OCTOBIT (DSC nulo)");
    }

    header_ui_create(screen_home);
    footer_ui_create(screen_home);

    lv_obj_add_event_cb(screen_home, home_event_cb, LV_EVENT_KEY, NULL);
    
    if(main_group) {
        lv_group_add_obj(main_group, screen_home);
        lv_group_focus_obj(screen_home);
    }

    lv_screen_load(screen_home);
}