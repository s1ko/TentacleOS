// Copyright (c) 2025 HIGH CODE LLC
#ifndef UI_BADUSB_BROWSER_H
#define UI_BADUSB_BROWSER_H

typedef enum {
    BADUSB_STORAGE_INTERNAL = 0,
    BADUSB_STORAGE_SDCARD = 1
} badusb_storage_t;

void ui_badusb_browser_open(void);
void ui_badusb_browser_set_storage(badusb_storage_t storage);

#endif // UI_BADUSB_BROWSER_H