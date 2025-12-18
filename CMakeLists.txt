# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(highboy)

set(CONVERT_SCRIPT "${CMAKE_SOURCE_DIR}/png_conversor_to_bin.sh")

execute_process(
    COMMAND bash ${CONVERT_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE convert_result
)

if(NOT convert_result EQUAL 0)
  message(FATAL_ERROR "Fail to convert assets. Check the logs.")
endif()

littlefs_create_partition_image(assets assets FLASH_IN_PROJECT)
