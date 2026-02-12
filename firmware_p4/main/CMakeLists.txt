


idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
    Core
    Drivers
    Applications
    Service
    driver
)

# Embed C5 Firmware Binary
# The build script must ensure this file exists before P4 compilation
set(C5_BIN_PATH "${CMAKE_SOURCE_DIR}/../firmware_c5/build/firmware_c5.bin")
if(EXISTS ${C5_BIN_PATH})
    target_add_binary_data(${COMPONENT_LIB} "${C5_BIN_PATH}" BINARY)
    message(STATUS "Embedding C5 Firmware: ${C5_BIN_PATH}")
else()
    message(WARNING "C5 Firmware binary not found at ${C5_BIN_PATH}. Firmware update feature will be disabled.")
endif()


