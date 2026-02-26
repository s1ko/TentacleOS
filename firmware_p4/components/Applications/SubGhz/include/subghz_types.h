#ifndef SUBGHZ_TYPES_H
#define SUBGHZ_TYPES_H

#include <stdint.h>

/**
 * @brief Structured data for SubGhz protocols
 */
typedef struct {
    const char* protocol_name;
    uint32_t serial;
    uint8_t btn;
    uint8_t bit_count;
    uint32_t raw_value;
} subghz_data_t;

#endif // SUBGHZ_TYPES_H
