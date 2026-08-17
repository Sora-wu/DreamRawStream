cmake_minimum_required(VERSION 3.21)
project(DreamNet LANGUAGES C CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/* ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
add_library(DreamNet ${SRC})
add_library(Dream::Net ALIAS DreamNet)
set_target_properties(DreamNet PROPERTIES EXPORT_NAME Net)

target_include_directories(DreamNet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(Dream_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Dream)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DreamConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DreamConfig.cmake
    INSTALL_DESTINATION ${Dream_INSTALL_CMAKEDIR}
    NO_SET_AND_CHECK_MACRO
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DreamConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/DreamConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/DreamConfigVersion.cmake
    DESTINATION ${Dream_INSTALL_CMAKEDIR}
)

install(
    EXPORT DreamNetTargets
    FILE DreamNetTargets.cmake
    NAMESPACE Dream::
    DESTINATION ${Dream_INSTALL_CMAKEDIR}
)

install(
    TARGETS DreamNet
    EXPORT DreamNetTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/DreamNet
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)