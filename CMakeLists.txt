cmake_minimum_required(VERSION 3.21)
project(DreamRawStream LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(PLATFORM_NAME "linux")
    add_compile_definitions(PLATFORM_LINUX)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(PLATFORM_NAME "win")
    add_compile_definitions(PLATFORM_WINDOWS)
endif()

add_compile_definitions($<$<CONFIG:Debug>:CONFIG_DEBUG>)
set(BUILD_SHARED_LIBS ON)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/code/third/build/${PLATFORM_NAME})

add_subdirectory(DreamNet)