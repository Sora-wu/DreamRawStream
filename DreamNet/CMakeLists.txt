file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/* ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
add_library(DreamNet ${SRC})
target_include_directories(DreamNet PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
