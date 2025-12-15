#include "menu_ui.h"
#include "core/lv_group.h"
#include "lv_conf_internal.h"
#include "misc/lv_palette.h"
#include "ui_manager.h"
#include "lv_port_indev.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "UI_MENU";

// --- PALETA LILÁS (Cyberpunk Purple) ---
#define BG_COLOR            lv_color_hex(0x0F0518) // Roxo muito escuro (Fundo)
#define CARD_COLOR          lv_color_hex(0x2D103A) // Roxo escuro (Botão inativo)
#define CARD_COLOR_FOCUSED  lv_color_hex(0xCF66FF) // Lilás Neon (Botão focado)
#define TEXT_COLOR_FOCUSED  lv_color_hex(0xFFFFFF) 
#define BORDER_COLOR        lv_color_hex(0x602080) // Borda sutil

// --- TAMANHOS ---
#define ITEM_SPACING 100    // Distância entre centros (Sobreposição visual)
#define WRAPPER_SIZE 130    // Tamanho da caixa invisível
#define BTN_MAX_SIZE 130    // Tamanho do botão focado
#define BTN_MIN_SIZE 90     // Tamanho do botão lateral

// --- CONSTANTES INFINITAS ---
// Uma largura virtual enorme permite rolar "para sempre"
#define VIRTUAL_WIDTH 16000 
#define START_OFFSET  8000  // Começamos no meio dessa largura

// --- IDs ---
typedef enum {
    MENU_ID_WIFI = 0,
    MENU_ID_BLUETOOTH,
    MENU_ID_INFRARED,
    MENU_ID_RADIO_RF,
    MENU_ID_BAD_USB,
    MENU_ID_MICRO_SD,
    MENU_ID_NFC,
    MENU_ID_RFID,
    MENU_ID_MAX // Total = 8
} menu_item_id_t;

static const char * menu_titles[] = {
    "WIFI ATTACKS", "BLUETOOTH", "INFRARED", "SUB-GHZ",
    "BAD USB", "MICRO SD", "NFC TOOLS", "RFID TAGS"
};

static lv_obj_t * screen_menu = NULL;
static lv_obj_t * cont_carousel = NULL;
static lv_obj_t * lbl_title = NULL;

// --- EFEITO VISUAL (INFINITO + GEOMÉTRICO + Z-INDEX) ---
static void carousel_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * cont = lv_event_get_target(e);

    if (code == LV_EVENT_SCROLL) {
        // Centro visual da tela
        lv_area_t cont_a;
        lv_obj_get_coords(cont, &cont_a);
        lv_coord_t cont_center_x = cont_a.x1 + lv_area_get_width(&cont_a) / 2;

        uint32_t child_cnt = lv_obj_get_child_cnt(cont);
        
        // Comprimento total de UM ciclo de ícones
        lv_coord_t loop_len = child_cnt * ITEM_SPACING; 

        lv_obj_t * closest_wrapper = NULL;
        lv_coord_t min_diff = 10000;

        for (uint32_t i = 0; i < child_cnt; i++) {
            lv_obj_t * wrapper = lv_obj_get_child(cont, i);
            lv_obj_t * btn = lv_obj_get_child(wrapper, 0);
            
            if(!btn) continue;

            // --- LÓGICA INFINITA (TELETRANSPORTE) ---
            lv_area_t child_a;
            lv_obj_get_coords(wrapper, &child_a);
            lv_coord_t child_center_abs = child_a.x1 + lv_area_get_width(&child_a) / 2;
            
            // Distância do item até o centro da tela
            lv_coord_t dist_visual = child_center_abs - cont_center_x;

            // Se o item saiu muito pela esquerda, move para o final da direita
            if (dist_visual < -(loop_len / 2)) {
                lv_obj_set_x(wrapper, lv_obj_get_x(wrapper) + loop_len);
            }
            // Se o item saiu muito pela direita, move para o final da esquerda
            else if (dist_visual > (loop_len / 2)) {
                lv_obj_set_x(wrapper, lv_obj_get_x(wrapper) - loop_len);
            }

            // --- REDESENHO (SIZE & COLOR) ---
            // Recalcula coords após possível teletransporte
            lv_obj_get_coords(wrapper, &child_a);
            lv_coord_t current_center = child_a.x1 + lv_area_get_width(&child_a) / 2;
            lv_coord_t diff_x = LV_ABS(cont_center_x - current_center);

            if (diff_x < min_diff) {
                min_diff = diff_x;
                closest_wrapper = wrapper;
            }

            // Modulação de tamanho
            int32_t target_size = BTN_MAX_SIZE - (diff_x / 3.5);
            if (target_size < BTN_MIN_SIZE) target_size = BTN_MIN_SIZE;

            if (lv_obj_get_width(btn) != target_size) {
                lv_obj_set_size(btn, target_size, target_size);
            }

            // Cores Lilás
            if (diff_x < (ITEM_SPACING / 1.5)) { 
                // Focado
                lv_obj_set_style_bg_color(btn, CARD_COLOR_FOCUSED, 0);
                lv_obj_set_style_border_color(btn, lv_color_white(), 0);
                
                // Efeito Glow (Sombra colorida)
                lv_obj_set_style_shadow_width(btn, 25, 0);
                lv_obj_set_style_shadow_color(btn, CARD_COLOR_FOCUSED, 0);
                lv_obj_set_style_shadow_opa(btn, LV_OPA_50, 0);

                intptr_t id = (intptr_t)lv_obj_get_user_data(wrapper);
                if (id >= 0 && id < MENU_ID_MAX && lbl_title) {
                    lv_label_set_text(lbl_title, menu_titles[id]);
                }
            } else {
                // Lateral (Inativo)
                lv_obj_set_style_bg_color(btn, CARD_COLOR, 0);
                lv_obj_set_style_border_color(btn, BORDER_COLOR, 0);
                lv_obj_set_style_shadow_width(btn, 0, 0);
            }
        }

        // --- Z-INDEX MANUAL ---
        // Garante que o item focado (closest) seja desenhado por último (na frente)
        if (closest_wrapper) {
            lv_obj_t * last_child = lv_obj_get_child(cont, child_cnt - 1);
            if (closest_wrapper != last_child) {
                lv_obj_move_foreground(closest_wrapper);
            }
        }
    }
}

static void menu_btn_event_cb(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * wrapper = lv_obj_get_parent(btn);
    
    if (code == LV_EVENT_FOCUSED) {
        lv_obj_scroll_to_view(wrapper, LV_ANIM_ON);
    }
    else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_ENTER) {
            intptr_t id = (intptr_t)lv_obj_get_user_data(wrapper); 
            ESP_LOGI(TAG, "ID Selecionado: %d", (int)id);
            // Implementar switch(id) aqui...
        }
        else if (key == LV_KEY_ESC) {
            ui_switch_screen(SCREEN_HOME);
        }
    }
}

static void create_card(lv_obj_t * parent, const char * symbol, menu_item_id_t id){
    // Cria o wrapper
    lv_obj_t * wrapper = lv_obj_create(parent);
    lv_obj_set_size(wrapper, WRAPPER_SIZE, WRAPPER_SIZE);
    lv_obj_remove_style_all(wrapper);
    lv_obj_set_style_bg_opa(wrapper, LV_OPA_TRANSP, 0);
    
    // --- POSICIONAMENTO ABSOLUTO INICIAL ---
    // Coloca os itens no meio da área virtual
    lv_obj_set_pos(wrapper, START_OFFSET + (id * ITEM_SPACING), 0);

    // O wrapper usa FLEX internamente só para centralizar o botão
    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wrapper, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_set_user_data(wrapper, (void*)(intptr_t)id);

    // Cria o botão
    lv_obj_t * btn = lv_btn_create(wrapper);
    lv_obj_set_size(btn, BTN_MIN_SIZE, BTN_MIN_SIZE);
    
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_bg_color(btn, CARD_COLOR, 0);
    
    lv_obj_t * icon = lv_label_create(btn);
    lv_label_set_text(icon, symbol);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0); 
    lv_obj_set_style_text_color(icon, lv_color_white(), 0);
    lv_obj_center(icon);

    lv_obj_add_event_cb(btn, menu_btn_event_cb, LV_EVENT_ALL, NULL);

    if (main_group) lv_group_add_obj(main_group, btn);
}

void ui_menu_open(void){
    if (main_group) lv_group_remove_all_objs(main_group);
    if (screen_menu) { lv_obj_del(screen_menu); screen_menu = NULL; }

    screen_menu = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_menu, BG_COLOR, 0);
    if(main_group) lv_group_remove_obj(screen_menu);

    cont_carousel = lv_obj_create(screen_menu);
    lv_obj_set_size(cont_carousel, LV_PCT(100), 180);
    lv_obj_align(cont_carousel, LV_ALIGN_CENTER, 0, -20);
    
    // --- LARGURA VIRTUAL ---
    // Define área gigante para o scroll infinito
    lv_obj_set_width(cont_carousel, VIRTUAL_WIDTH);
    
    // NOTA: Não definimos Flex Flow no cont_carousel. 
    // Por padrão ele aceita posicionamento absoluto (que usamos no create_card).

    lv_obj_set_scroll_snap_x(cont_carousel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont_carousel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(cont_carousel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont_carousel, 0, 0);
    
    lv_obj_add_event_cb(cont_carousel, carousel_event_cb, LV_EVENT_SCROLL, NULL);

    lbl_title = lv_label_create(screen_menu);
    lv_label_set_text(lbl_title, "SELECT");
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_title, TEXT_COLOR_FOCUSED, 0);
    lv_obj_align(lbl_title, LV_ALIGN_BOTTOM_MID, 0, -30);

    // Cria os itens
    create_card(cont_carousel, LV_SYMBOL_WIFI,      MENU_ID_WIFI);
    create_card(cont_carousel, LV_SYMBOL_BLUETOOTH, MENU_ID_BLUETOOTH);
    create_card(cont_carousel, LV_SYMBOL_EYE_OPEN,  MENU_ID_INFRARED);
    create_card(cont_carousel, LV_SYMBOL_AUDIO,     MENU_ID_RADIO_RF);
    create_card(cont_carousel, LV_SYMBOL_USB,       MENU_ID_BAD_USB);
    create_card(cont_carousel, LV_SYMBOL_SD_CARD,   MENU_ID_MICRO_SD);
    create_card(cont_carousel, LV_SYMBOL_LOOP,      MENU_ID_NFC);
    create_card(cont_carousel, LV_SYMBOL_BELL,      MENU_ID_RFID);

    // Inicialização
    if (main_group && lv_obj_get_child_cnt(cont_carousel) > 0) {
        // Pula direto para o meio da área virtual (onde os botões estão)
        lv_obj_scroll_to_x(cont_carousel, START_OFFSET, LV_ANIM_OFF);

        // Foca no primeiro
        lv_obj_t * first_wrapper = lv_obj_get_child(cont_carousel, 0);
        lv_obj_t * first_btn = lv_obj_get_child(first_wrapper, 0);
        
        lv_group_focus_obj(first_btn);
        
        // Força o cálculo inicial de posição e tamanho
        lv_obj_send_event(cont_carousel, LV_EVENT_SCROLL, NULL); 
    }

    lv_screen_load(screen_menu);
}
