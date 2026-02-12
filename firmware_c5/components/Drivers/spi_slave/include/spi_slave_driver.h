#ifndef SPI_SLAVE_DRIVER_H
#define SPI_SLAVE_DRIVER_H

#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t spi_slave_driver_init(void);
esp_err_t spi_slave_driver_transmit(const uint8_t *tx_data, uint8_t *rx_data, size_t len);
void spi_slave_driver_set_irq(int level);

#endif