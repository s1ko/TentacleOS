#ifndef BRIDGE_MANAGER_H
#define BRIDGE_MANAGER_H

#include "esp_err.h"

/**
 * @brief Synchronizes P4 and C5 versions and initializes the bridge.
 * This should be called during P4 system initialization.
 */
esp_err_t bridge_manager_init(void);

/**
 * @brief Manually triggers a C5 firmware update.
 */
esp_err_t bridge_manager_force_update(void);

#endif
