cmake_minimum_required(VERSION 3.16)

if(EXISTS "${CMAKE_SOURCE_DIR}/managed_components/espressif__mqtt/CMakeLists.txt")
    list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/managed_components/espressif__mqtt")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cookies_n_cream_esp32)
