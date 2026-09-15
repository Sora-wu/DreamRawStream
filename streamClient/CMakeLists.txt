file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/* ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

find_package(ALSA REQUIRED)

set(FFMPEG_ROOT ${PROJECT_SOURCE_DIR}/third/build/${PLATFORM_NAME})
set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale)

add_executable(streamClient ${SRC})
target_include_directories(streamClient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/common
        ${FFMPEG_ROOT}/include
)
target_link_directories(streamClient PRIVATE ${FFMPEG_ROOT}/lib)
target_link_libraries(streamClient PRIVATE Dream::Net ${FFMPEG_LIBS})