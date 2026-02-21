// Copyright (c) 2025 HIGH CODE LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "st25r3916_aat.h"
#include "esp_log.h"

static const char* TAG = "st25r_aat";

hb_nfc_err_t st25r_aat_calibrate(st25r_aat_result_t* result)
{
    ESP_LOGW(TAG, "AAT not yet implemented (Phase 4)");
    if (result) {
        result->dac_a = 128;
        result->dac_b = 128;
        result->amplitude = 0;
        result->phase = 0;
    }
    return HB_NFC_OK;
}

hb_nfc_err_t st25r_aat_load_nvs(st25r_aat_result_t* result)
{
    (void)result;
    return HB_NFC_ERR_INTERNAL; /* Not implemented */
}

hb_nfc_err_t st25r_aat_save_nvs(const st25r_aat_result_t* result)
{
    (void)result;
    return HB_NFC_ERR_INTERNAL; /* Not implemented */
}
