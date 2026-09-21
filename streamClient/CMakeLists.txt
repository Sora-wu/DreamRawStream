file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/* ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets OpenGLWidgets Multimedia)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

set(FFMPEG_ROOT ${PROJECT_SOURCE_DIR}/third/build/${PLATFORM_NAME})
set(FFMPEG_LIBS avcodec avdevice avfilter avformat avutil postproc swresample swscale)

qt_add_executable(streamClient ${SRC})
target_sources(streamClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shader.qrc)
set_target_properties(streamClient PROPERTIES
    AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms
)
target_include_directories(streamClient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/common
        ${FFMPEG_ROOT}/include
)
target_link_directories(streamClient PRIVATE ${FFMPEG_ROOT}/lib)
target_link_libraries(streamClient PRIVATE Qt::Core Qt6::Multimedia Qt::OpenGLWidgets Qt::Widgets Dream::Net ${FFMPEG_LIBS})

install(TARGETS streamClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET streamClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})