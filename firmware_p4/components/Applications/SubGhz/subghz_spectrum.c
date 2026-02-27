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

#include "subghz_spectrum.h"
#include "cc1101.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <rom/ets_sys.h>
#include <string.h>

static const char *TAG = "SUBGHZ_SPECTRUM";
static TaskHandle_t spectrum_task_handle = NULL;
static SemaphoreHandle_t spectrum_mutex = NULL;
static volatile bool s_stop_requested = false;
static volatile bool s_is_running = false;

static subghz_spectrum_line_t s_latest_line = {0};

void subghz_spectrum_task(void *pvParameters) {
  s_is_running = true;

  uint32_t center = s_latest_line.center_freq;
  uint32_t span = s_latest_line.span_hz;
  uint32_t step = span / SPECTRUM_SAMPLES;
  uint32_t start = center - (span / 2);

  ESP_LOGI(TAG, "Spectrum Task Started: Center=%lu, Span=%lu, Step=%lu", 
           (unsigned long)center, (unsigned long)span, (unsigned long)step);

  while (!s_stop_requested) {
    float current_sweep[SPECTRUM_SAMPLES];

    for (int i = 0; i < SPECTRUM_SAMPLES; i++) {
      if (s_stop_requested) break;

      uint32_t current_freq = start + (i * step);

      cc1101_strobe(CC1101_SIDLE);
      cc1101_set_frequency(current_freq);
      cc1101_strobe(CC1101_SRX);

      // Stabilization delay
      ets_delay_us(400); 

      // Peak detection (3 samples to mitigate OOK pulsing)
      float local_max = -130.0;
      for(int k=0; k<3; k++) {
        uint8_t raw = cc1101_read_reg(CC1101_RSSI | 0x40);
        float val = cc1101_convert_rssi(raw);
        if(val > local_max) local_max = val;
        ets_delay_us(20);
      }
      current_sweep[i] = local_max;

      if (i % 16 == 0) vTaskDelay(1);
    }

    // Update global state safely
    if (xSemaphoreTake(spectrum_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      memcpy(s_latest_line.dbm_values, current_sweep, sizeof(current_sweep));
      s_latest_line.timestamp = esp_timer_get_time();
      s_latest_line.start_freq = start;
      s_latest_line.step_hz = step;
      xSemaphoreGive(spectrum_mutex);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }

  cc1101_strobe(CC1101_SIDLE);
  s_is_running = false;
  vTaskDelete(NULL);
}

void subghz_spectrum_start(uint32_t center_freq, uint32_t span_hz) {
  if (s_is_running) return;

  if (spectrum_mutex == NULL) {
    spectrum_mutex = xSemaphoreCreateMutex();
  }

  s_latest_line.center_freq = center_freq;
  s_latest_line.span_hz = span_hz;
  s_stop_requested = false;

  xTaskCreatePinnedToCore(subghz_spectrum_task, "subghz_spectrum", 4096, NULL, 1, &spectrum_task_handle, 1);
}

void subghz_spectrum_stop(void) {
  if (s_is_running) {
    s_stop_requested = true;
    // Task will delete itself
  }
}

bool subghz_spectrum_get_line(subghz_spectrum_line_t* out_line) {
  if (!out_line || !spectrum_mutex) return false;

  if (xSemaphoreTake(spectrum_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    memcpy(out_line, &s_latest_line, sizeof(subghz_spectrum_line_t));
    xSemaphoreGive(spectrum_mutex);
    return true;
  }
  return false;
}

