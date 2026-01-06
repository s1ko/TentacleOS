# Wi-Fi Service Component Documentation

This component manages Wi-Fi functionalities including Access Point (AP) mode, Station (STA) mode, scanning, and configuration persistence using JSON files.

## Functionality Overview

The service handles:
- **Initialization/Deinitialization:** Setup of NVS, Netif, Event Loops, and Wi-Fi drivers.
- **Access Point (AP):** Configurable SSID, password, max connections, and custom IP address.
- **Scanning:** Active scanning for nearby networks.
- **Station (STA):** Connecting to external Wi-Fi networks.
- **Hotspot Management:** Dynamic switching of AP configuration.
- **Configuration Persistence:** Loading and saving AP settings to/from `assets/config/wifi/wifi_ap.conf`.

## API Functions

### Initialization & Lifecycle

#### `wifi_init`
```c
void wifi_init(void);
```
Initializes the Wi-Fi stack in `APSTA` mode.
- Initializes NVS (performing erase if necessary).
- Sets up the default event loop and registers handlers.
- Loads AP configuration from storage (or uses defaults "Darth Maul"/"MyPassword123").
- Configures the static IP (default: 192.168.4.1) and starts the DHCP server.

#### `wifi_deinit`
```c
void wifi_deinit(void);
```
Completely shuts down the Wi-Fi service.
- Stops the Wi-Fi driver.
- Unregisters event handlers.
- Deinitializes the driver.
- Frees synchronization primitives (mutexes) and clears static data.

#### `wifi_start` / `wifi_stop`
```c
void wifi_start(void);
void wifi_stop(void);
```
Simple wrappers to start or stop the Wi-Fi driver without full deinitialization. `wifi_stop` also clears stored scan results.

### Scanning

#### `wifi_service_scan`
```c
void wifi_service_scan(void);
```
Performs an active Wi-Fi scan.
- Uses a mutex to ensure thread safety.
- Stores up to `WIFI_SCAN_LIST_SIZE` results internally.
- Provides visual feedback via LEDs (Green for AP connection, Red for failures, Blue for scan success).

#### `wifi_service_get_ap_count`
```c
uint16_t wifi_service_get_ap_count(void);
```
Returns the number of networks found in the last scan.

#### `wifi_service_get_ap_record`
```c
wifi_ap_record_t* wifi_service_get_ap_record(uint16_t index);
```
Retrieves a pointer to a specific scan result record. Returns `NULL` if the index is invalid.

### Connection & Management

#### `wifi_service_connect_to_ap`
```c
esp_err_t wifi_service_connect_to_ap(const char *ssid, const char *password);
```
Connects the device (as a station) to an external Access Point.
- Configures authentication mode based on the presence of a password (WPA2_PSK or OPEN).
- Disconnects any existing connection before attempting a new one.

#### `wifi_change_to_hotspot`
```c
void wifi_change_to_hotspot(const char *new_ssid);
```
Dynamically reconfigures the device's Access Point to an **Open** network with the specified SSID.
- Stops the Wi-Fi driver briefly to apply changes.
- Sets `authmode` to `WIFI_AUTH_OPEN`.
- Restarts Wi-Fi with the new configuration.

### Configuration Storage

#### `wifi_save_ap_config`
```c
esp_err_t wifi_save_ap_config(const char *ssid, const char *password, uint8_t max_conn, const char *ip_addr);
```
Saves the AP configuration to a JSON file (`/assets/config/wifi/wifi_ap.conf`).
- Uses `cJSON` to serialize settings.
- Persists data using the storage API.

**Internal Loader:** `wifi_load_ap_config` is called during initialization to read these settings.

## Internal Implementation Details

### Event Handling
A static `wifi_event_handler` manages Wi-Fi and IP events:
- **WIFI_EVENT_AP_STACONNECTED:** Logs the MAC of the connected station and blinks Green.
- **WIFI_EVENT_AP_STADISCONNECTED:** Blinks Red.
- **IP_EVENT_AP_STAIPASSIGNED:** Logs IP assignment and blinks Green.

### Thread Safety
A `wifi_mutex` (Semaphore) is used to protect the scanning process (`wifi_service_scan`), preventing concurrent scan requests which could lead to resource conflicts.

### Castings & Memory Management
- **cJSON:** Used extensively for parsing and generating configuration files.
- **Type Casting:** `event_data` is cast to specific event structures (e.g., `wifi_event_ap_staconnected_t*`) within handlers.
- **String Handling:** `strncpy` is used safely with explicit null-termination to prevent buffer overflows when handling SSIDs and passwords.
