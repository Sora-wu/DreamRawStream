file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/* ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

find_package(ALSA REQUIRED)

add_executable(streamClient ${SRC})
target_include_directories(streamClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(streamClient PRIVATE Dream::Net)