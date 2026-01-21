#ifndef KEYBOARD_UI_H
#define KEYBOARD_UI_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void keyboard_open(lv_obj_t * target_textarea);
void keyboard_close(void);

#ifdef __cplusplus
}
#endif

#endif /*KEYBOARD_UI_H*/