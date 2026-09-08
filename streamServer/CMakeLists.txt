file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/* ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

find_package(ALSA REQUIRED)

add_executable(streamServer ${SRC})
target_include_directories(streamServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(streamServer PRIVATE Dream::Net ALSA::ALSA fdk-aac x264)