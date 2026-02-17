#include "wifi_sniffer.h"
#include "spi_bridge.h"
#include <string.h>

static sniffer_stats_t cached_stats;
static wifi_sniffer_cb_t stream_cb = NULL;

static void wifi_sniffer_stream_cb(spi_id_t id, const uint8_t *payload, uint8_t len) {
    (void)id;
    if (!stream_cb || !payload || len < 3) return;
    const spi_wifi_sniffer_frame_t *frame = (const spi_wifi_sniffer_frame_t *)payload;
    uint16_t data_len = frame->len;
    if (data_len > (len - 3)) data_len = (len - 3);
    stream_cb(frame->data, data_len, frame->rssi, frame->channel);
}

static void update_stats(void) {
    spi_header_t resp;
    uint16_t magic_stats = SPI_DATA_INDEX_STATS;
    spi_bridge_send_command(SPI_ID_SYSTEM_DATA, (uint8_t*)&magic_stats, 2, &resp, (uint8_t*)&cached_stats, 1000);
}

bool wifi_sniffer_start(sniff_type_t type, uint8_t channel) {
    uint8_t payload[2];
    payload[0] = (uint8_t)type;
    payload[1] = channel;
    memset(&cached_stats, 0, sizeof(cached_stats));
    return (spi_bridge_send_command(SPI_ID_WIFI_APP_SNIFFER, payload, 2, NULL, NULL, 2000) == ESP_OK);
}

bool wifi_sniffer_start_stream(sniff_type_t type, uint8_t channel, wifi_sniffer_cb_t cb) {
    if (!cb) {
        return wifi_sniffer_start(type, channel);
    }
    stream_cb = cb;
    spi_bridge_register_stream_cb(SPI_ID_WIFI_APP_SNIFFER, wifi_sniffer_stream_cb);
    if (!wifi_sniffer_start(type, channel)) {
        spi_bridge_unregister_stream_cb(SPI_ID_WIFI_APP_SNIFFER);
        stream_cb = NULL;
        return false;
    }
    return true;
}

void wifi_sniffer_stop(void) {
    spi_bridge_send_command(SPI_ID_WIFI_APP_ATTACK_STOP, NULL, 0, NULL, NULL, 2000);
    spi_bridge_unregister_stream_cb(SPI_ID_WIFI_APP_SNIFFER);
    stream_cb = NULL;
}

uint32_t wifi_sniffer_get_packet_count(void) {
    update_stats();
    return cached_stats.packets;
}

uint32_t wifi_sniffer_get_deauth_count(void) {
    // Stats are updated in packet_count call, usually UI calls these together
    return cached_stats.deauths;
}

uint32_t wifi_sniffer_get_buffer_usage(void) {
    return cached_stats.buffer_usage;
}

bool wifi_sniffer_handshake_captured(void) {
    return cached_stats.handshake_captured;
}

bool wifi_sniffer_pmkid_captured(void) {
    return cached_stats.pmkid_captured;
}

// Stubs for UI flow control
bool wifi_sniffer_save_to_internal_flash(const char *filename) {
    if (!filename || filename[0] == '\0') return false;
    uint8_t payload[SPI_WIFI_SNIFFER_FILENAME_MAX];
    size_t name_len = strnlen(filename, SPI_WIFI_SNIFFER_FILENAME_MAX - 1);
    memset(payload, 0, sizeof(payload));
    memcpy(payload, filename, name_len);
    return (spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_SAVE_FLASH, payload, (uint8_t)name_len, NULL, NULL, 10000) == ESP_OK);
}

bool wifi_sniffer_save_to_sd_card(const char *filename) {
    if (!filename || filename[0] == '\0') return false;
    uint8_t payload[SPI_WIFI_SNIFFER_FILENAME_MAX];
    size_t name_len = strnlen(filename, SPI_WIFI_SNIFFER_FILENAME_MAX - 1);
    memset(payload, 0, sizeof(payload));
    memcpy(payload, filename, name_len);
    return (spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_SAVE_SD, payload, (uint8_t)name_len, NULL, NULL, 10000) == ESP_OK);
}

void wifi_sniffer_free_buffer(void) {
    spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_FREE_BUFFER, NULL, 0, NULL, NULL, 2000);
    cached_stats.buffer_usage = 0;
}

void wifi_sniffer_set_snaplen(uint16_t len) {
    uint8_t payload[2];
    memcpy(payload, &len, sizeof(len));
    spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_SET_SNAPLEN, payload, sizeof(payload), NULL, NULL, 2000);
}

void wifi_sniffer_set_verbose(bool verbose) {
    uint8_t payload = verbose ? 1 : 0;
    spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_SET_VERBOSE, &payload, 1, NULL, NULL, 2000);
}

bool wifi_sniffer_start_stream_sd(sniff_type_t type, uint8_t channel, const char *filename) {
    if (!filename || filename[0] == '\0') return false;
    uint8_t payload[2 + SPI_WIFI_SNIFFER_FILENAME_MAX];
    size_t name_len = strnlen(filename, SPI_WIFI_SNIFFER_FILENAME_MAX - 1);
    payload[0] = (uint8_t)type;
    payload[1] = channel;
    memset(payload + 2, 0, SPI_WIFI_SNIFFER_FILENAME_MAX);
    memcpy(payload + 2, filename, name_len);
    return (spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_STREAM_SD, payload, (uint8_t)(2 + name_len), NULL, NULL, 10000) == ESP_OK);
}

void wifi_sniffer_clear_pmkid(void) {
    spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_CLEAR_PMKID, NULL, 0, NULL, NULL, 2000);
    cached_stats.pmkid_captured = false;
}

void wifi_sniffer_get_pmkid_bssid(uint8_t out_bssid[6]) {
    if (!out_bssid) return;
    spi_header_t resp;
    uint8_t payload[6] = {0};
    if (spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_GET_PMKID_BSSID, NULL, 0, &resp, payload, 1000) == ESP_OK) {
        memcpy(out_bssid, payload, 6);
    } else {
        memset(out_bssid, 0, 6);
    }
}

void wifi_sniffer_clear_handshake(void) {
    spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_CLEAR_HANDSHAKE, NULL, 0, NULL, NULL, 2000);
    cached_stats.handshake_captured = false;
}

void wifi_sniffer_get_handshake_bssid(uint8_t out_bssid[6]) {
    if (!out_bssid) return;
    spi_header_t resp;
    uint8_t payload[6] = {0};
    if (spi_bridge_send_command(SPI_ID_WIFI_SNIFFER_GET_HANDSHAKE_BSSID, NULL, 0, &resp, payload, 1000) == ESP_OK) {
        memcpy(out_bssid, payload, 6);
    } else {
        memset(out_bssid, 0, 6);
    }
}
