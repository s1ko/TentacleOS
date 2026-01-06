# BadUSB Application Component

This component implements a "BadUSB" application capable of emulating a USB HID keyboard to execute automated payloads. It includes a complete DuckyScript parser, low-level keyboard control, and support for multiple layouts (US and ABNT2).

## Overview

- **Location:** `components/Applications/bad_usb/`
- **Main Headers:** 
    - `include/bad_usb.h` (Low-level control)
    - `include/ducky_parser.h` (Scripting engine)
- **Dependencies:** `tinyusb`, `tusb_desc`, `storage_assets`, `storage_read`

## Key Features

- **USB Emulation:** Acts as a standard HID Keyboard using TinyUSB.
- **DuckyScript Support:** Parses and executes standard DuckyScript commands.
- **Multi-Layout:** Native support for **US** and **ABNT2** (Brazilian Portuguese) keyboard layouts.
- **Storage Integration:** Can run scripts directly from internal assets or SD Card.
- **Progress Feedback:** Callback system to report execution progress to the UI.

## API Reference

### Initialization

#### `bad_usb_init` / `bad_usb_deinit`
```c
void bad_usb_init(void);
void bad_usb_deinit(void);
```
Initializes or removes the TinyUSB driver and HID descriptors.

#### `bad_usb_wait_for_connection`
```c
void bad_usb_wait_for_connection(void);
```
Blocks execution until the device is successfully mounted by a host computer.

### Low-Level Control

#### `bad_usb_press_key`
```c
void bad_usb_press_key(uint8_t keycode, uint8_t modifier);
```
Sends a single keystroke with optional modifiers (Shift, Ctrl, Alt, GUI). Automatically handles the "press" and "release" reports.

#### `type_string_us` / `type_string_abnt2`
```c
void type_string_us(const char* str);
void type_string_abnt2(const char* str);
```
Types a full string interpreting characters according to the specified layout.
- **ABNT2:** Handles dead keys (accents like `~`, `^`, `'`, `` ` ``) and special characters (`ç`).

### DuckyScript Parser (`ducky_parser.h`)

#### `ducky_parse_and_run`
```c
void ducky_parse_and_run(const char *script);
```
Parses a raw string containing DuckyScript commands and executes them line-by-line.

#### `ducky_run_from_assets` / `ducky_run_from_sdcard`
```c
esp_err_t ducky_run_from_assets(const char *filename);
esp_err_t ducky_run_from_sdcard(const char *path);
```
Helper functions to load script files from storage and execute them.

#### `ducky_set_layout`
```c
void ducky_set_layout(ducky_layout_t layout);
```
Switches the typing engine between `DUCKY_LAYOUT_US` and `DUCKY_LAYOUT_ABNT2`.

#### `ducky_abort`
```c
void ducky_abort(void);
```
Sets a flag to immediately stop the current script execution.

## Supported DuckyScript Commands

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `REM` | [Comment] | Comment line (ignored). |
| `DELAY` | [Milliseconds] | Pauses execution for the specified time. |
| `STRING` | [Text] | Types the following text using the active layout. |
| `ENTER` | - | Presses the Enter key. |
| `GUI` / `WINDOWS` | [Key] | Presses the Windows/Command key (optionally with a key). |
| `CTRL` | [Key] | Presses Control + Key. |
| `SHIFT` | [Key] | Presses Shift + Key. |
| `ALT` | [Key] | Presses Alt + Key. |
| `TAB` | - | Presses Tab. |
| `ESC` | - | Presses Escape. |
| `F1` - `F12` | - | Function keys. |
| ... | | Supports standard keys: `HOME`, `INSERT`, `DELETE`, `PAGEUP`, `PAGEDOWN`, Arrows, etc. |

## Implementation Details

- **ABNT2 Logic:** The `type_string_abnt2` function implements a state machine to handle accented characters (e.g., `á` becomes `'` + `a`) and specific keycodes for the Brazilian layout.
- **Tokenizing:** The parser uses `strtok_r` to process lines and arguments safely.
- **Delays:** A default delay of 10ms is added between key press/release, and 20ms between script lines to ensure the host registers inputs correctly.
