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
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third/build/${PLATFORM_NAME})

add_compile_definitions($<$<CONFIG:Debug>:CONFIG_DEBUG>)

option(DREAM_FROM_SOURCE "Build DreamNet from source for debugging" ON)

if(DREAM_FROM_SOURCE)
    add_subdirectory(DreamNet)
else()
    find_package(Dream COMPONENTS Net REQUIRED)
endif()

add_executable(echoServer echoServer/echoServer.cpp)
add_executable(echoClient echoServer/echoClient.cpp)
target_link_libraries(echoServer PRIVATE Dream::Net)
target_link_libraries(echoClient PRIVATE Dream::Net)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(streamServer)