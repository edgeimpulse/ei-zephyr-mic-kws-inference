cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/model)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ei-zephyr-mic-kws-inference)

# Needed for colorful output
zephyr_compile_options(-fdiagnostics-color=always)

# Disable warnings as errors for Edge Impulse SDK
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-error=maybe-uninitialized>)

# Project include directories
set(INCLUDES
    .
    src
    )
include_directories(${INCLUDES})

# add all sources to the project
target_sources(app PRIVATE src/main.cpp)
target_sources(app PRIVATE src/sensors/ei_microphone.cpp)
target_sources(app PRIVATE src/inference/inferencing.cpp)
