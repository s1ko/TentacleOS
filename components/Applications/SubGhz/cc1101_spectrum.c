#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>
#include "cc1101.h"

void cc1101_spectrum_task(void *pvParameters) {
  int log_counter = 0;

  // Centro: 433.92 MHz
  // Range: 600 kHz / 80 samples = 7.5 kHz por passo (High Resolution)
  const uint32_t CENTER_FREQ = 433920000;
  const uint32_t SPAN_HZ = 600000; 
  const uint32_t STEP_HZ = SPAN_HZ / SPECTRUM_SAMPLES;
  const uint32_t START_FREQ = CENTER_FREQ - (SPAN_HZ / 2);

  while (1) {
    float sweep_max_dbm = -130.0; 

    for (int i = 0; i < SPECTRUM_SAMPLES; i++) {
      uint32_t current_freq = START_FREQ + (i * STEP_HZ);

      cc1101_strobe(CC1101_SIDLE);

      cc1101_set_frequency(current_freq);

      cc1101_strobe(CC1101_SRX);

      ets_delay_us(800); 

      float local_max = -130.0;
      for(int k=0; k<5; k++) {
        uint8_t raw = cc1101_read_reg(CC1101_RSSI | 0x40);
        float val = cc1101_convert_rssi(raw);
        if(val > local_max) local_max = val;
        ets_delay_us(50); 
      }

      spectrum_data[i] = local_max;

      if(local_max > sweep_max_dbm) {
        sweep_max_dbm = local_max;
      }

      if (i % 10 == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }

    if (++log_counter >= 5) { 
      log_counter = 0;
    }        
  }
}


