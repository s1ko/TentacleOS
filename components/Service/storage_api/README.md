# Storage API

The **Storage API** provides a unified, backend-agnostic interface for file system operations in the Highboy project. It abstracts the underlying storage mechanism (LittleFS, SD Card, etc.), allowing developers to perform file and directory operations using a consistent set of functions without worrying about low-level details or mount points.

## Features

- **Unified Interface**: Same API for internal flash (LittleFS) and external SD cards.
- **Backend Abstraction**: Uses VFS layer underneath, works with any configured backend.
- **Automatic Path Resolution**: Automatically handles mount points - use relative paths.
- **Robustness**: Includes safety checks, recursive directory creation, and error handling.
- **High-Level Helpers**: Easy reading/writing of strings, lines, formatted text, and CSV data.

---

## Architecture

```
Application Code
      ↓
  Storage API  ← You are here (recommended layer)
      ↓
   VFS Core   ← Backend abstraction
      ↓
  SD Card / LittleFS / SPIFFS
```

**Dependencies:**
- Requires `vfs_core` to be initialized
- Backend selection is done in `vfs_config.h`

---

## Initialization

Before performing any operations, the storage system must be initialized.

```c
#include "storage_init.h"

// Initialize the storage system
// This calls vfs_init_auto() internally
esp_err_t ret = storage_init();
if (ret != ESP_OK) {
    // Handle error
}

// Check if mounted
if (storage_is_mounted()) {
    // Ready to use
}

// Deinitialize when done (rarely needed for main application)
storage_deinit();
```

### Default Directory Structure

The storage system automatically creates a standard directory tree on initialization:

```
<mount_point>/  (e.g., /sdcard or /littlefs)
├── config/     - Configuration files
├── data/       - Application data
├── logs/       - Log files
├── cache/      - Temporary cache
├── temp/       - Temporary files
├── backup/     - Backup files
├── certs/      - SSL/TLS certificates
└── scripts/    - Script files
```

These directories are defined in `storage_dirs.h` and can be accessed via macros:

```c
#include "storage_dirs.h"

// Macros automatically include the mount point
// Example: STORAGE_DIR_CONFIG expands to "/sdcard/config" or "/littlefs/config"

// Write to config directory
storage_write_string(STORAGE_DIR_CONFIG "/settings.json", json_data);

// Append to logs
storage_append_formatted(STORAGE_DIR_LOGS "/system.log", "[%lu] Event\n", timestamp);

// Save backup
storage_file_copy(STORAGE_DIR_DATA "/important.dat", STORAGE_DIR_BACKUP "/important.dat");
```

**Path Handling:**
- All Storage API functions accept **relative paths**
- Mount point is automatically prepended internally
- You can use either `"/config/file.txt"` or `STORAGE_DIR_CONFIG "/file.txt"`

**Note**: Directory creation is non-critical. If any directory fails to create, initialization continues successfully, and you can create directories manually later as needed.

---

## File Operations

Header: `storage_file.h`

### Basic Management

| Function | Description |
|----------|-------------|
| `bool storage_file_exists(const char *path)` | Checks if a file exists. |
| `esp_err_t storage_file_delete(const char *path)` | Deletes a file. |
| `esp_err_t storage_file_rename(const char *old, const char *new)` | Renames or moves a file. |
| `esp_err_t storage_file_copy(const char *src, const char *dst)` | Copies a file. |
| `esp_err_t storage_file_clear(const char *path)` | Clears file content (truncates to 0). |

### Information

```c
// File information structure
typedef struct {
    char path[256];           // Full path to file
    size_t size;              // File size in bytes
    time_t modified_time;     // Last modification time (Unix timestamp)
    bool is_directory;        // True if this is a directory
} storage_file_info_t;
```

| Function | Description |
|----------|-------------|
| `esp_err_t storage_file_get_size(const char *path, size_t *size)` | Gets file size in bytes. |
| `esp_err_t storage_file_is_empty(const char *path, bool *empty)` | Checks if a file is empty. |
| `esp_err_t storage_file_get_info(const char *path, storage_file_info_t *info)` | Gets detailed info (size, modification time, etc.). |

---

## Reading Data

Header: `storage_read.h`

The API provides various ways to read data depending on your needs.

### Strings & Binary

```c
// Read entire file into a string buffer
char buffer[128];
storage_read_string("/config/settings.txt", buffer, sizeof(buffer));

// Read binary data
uint8_t data[64];
size_t bytes_read;
storage_read_binary("/data/image.bin", data, sizeof(data), &bytes_read);
```

### Line-by-Line

```c
// Read specific line (1-based index)
char line[64];
storage_read_line("/logs/system.log", line, sizeof(line), 5);

// Read first/last line helpers
storage_read_first_line("/logs/system.log", line, sizeof(line));
storage_read_last_line("/logs/system.log", line, sizeof(line));

// Iterate over all lines using a callback
void my_line_callback(const char *line, void *user_data) {
    printf("Read line: %s\n", line);
}
storage_read_lines("/data/list.txt", my_line_callback, NULL);
```

### Typed Data

```c
int32_t count;
storage_read_int("/config/boot_count", &count);

float temperature;
storage_read_float("/config/temp_threshold", &temperature);
```

---

## Writing Data

Header: `storage_write.h`

All write functions automatically create parent directories if they don't exist (recursive mkdir).

### Strings & Binary

```c
// Write (overwrite) a string to a file
storage_write_string("/data/status.txt", "System Ready");

// Append to a file
storage_append_string("/logs/app.log", "Event occurred");

// Write binary data
uint8_t raw_data[] = {0x01, 0x02, 0x03};
storage_write_binary("/data/blob.bin", raw_data, sizeof(raw_data));
```

### Formatted Output

Similar to `printf`, useful for logs or human-readable data.

```c
storage_write_formatted("/logs/info.txt", "Boot count: %d\nTime: %u", count, timestamp);
storage_append_formatted("/logs/events.log", "[INFO] Sensor %s: %.2f\n", sensor_name, value);
```

### CSV Support

Helper for writing structured data.

```c
const char *row[] = {"Timestamp", "Value", "Unit"};
storage_append_csv_row("/data/sensors.csv", row, 3);
// Writes: Timestamp,Value,Unit\n
```

---

## Directory Operations

Header: `storage_dir.h`

| Function | Description |
|----------|-------------|
| `esp_err_t storage_dir_create(const char *path)` | Creates a directory (recursive). |
| `esp_err_t storage_dir_remove(const char *path)` | Removes an empty directory. |
| `esp_err_t storage_dir_remove_recursive(const char *path)` | Removes a directory and all contents. |
| `esp_err_t storage_dir_list(const char *path, storage_dir_callback_t cb, void *user_data)` | Lists directory contents via callback. |
| `esp_err_t storage_dir_count(const char *path, uint32_t *file_count, uint32_t *dir_count)` | Counts files and subdirectories. |

---

## Storage Information

Header: `storage_info.h`

Monitor storage usage and health.

```c
// Print detailed usage report to log
storage_print_info_detailed();

// Get raw values
uint64_t total, free, used;
storage_get_total_space(&total);
storage_get_free_space(&free);
storage_get_used_space(&used);

// Get usage percentage
float percent;
storage_get_usage_percent(&percent);
```

---

## Example Usage

```c
#include "storage_api.h" // Includes all necessary headers

void app_main() {
    // Initialize storage (calls vfs_init_auto internally)
    if (storage_init() != ESP_OK) {
        printf("Storage init failed!\n");
        return;
    }

    // Check for config file
    if (storage_file_exists("/config/settings.json")) {
        char config[1024];
        storage_read_string("/config/settings.json", config, sizeof(config));
        // Process config...
    } else {
        // Create default config
        storage_write_string("/config/settings.json", "{ \"defaults\": true }");
    }

    // Log startup event
    storage_append_formatted(STORAGE_DIR_LOGS "/boot.log", 
                            "System started at %lu\n", xTaskGetTickCount());
    
    // Check storage health
    float usage;
    storage_get_usage_percent(&usage);
    printf("Storage usage: %.1f%%\n", usage);
}
```

---

## Best Practices

1. **Always use relative paths** - Let the API handle mount points
2. **Use directory macros** - `STORAGE_DIR_CONFIG` instead of hardcoded `"/config"`
3. **Check return values** - All functions return `esp_err_t` for error handling
4. **Close files properly** - Though most helpers do this automatically
5. **Monitor storage** - Use `storage_get_usage_percent()` to prevent full disk
6. **Use appropriate read functions** - Line-by-line for logs, binary for images