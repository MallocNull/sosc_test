cmake_minimum_required(VERSION 3.12)
project(ogl_test C)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

file(GLOB_RECURSE ogl_src
    "src/*.c"
    "src/*.h"
)

add_executable(ogl_test ${ogl_src})

target_include_directories(ogl_test
    PRIVATE ${SDL2_INCLUDE_DIRS}
    PRIVATE ${OPENGL_INCLUDE_DIR}
    PRIVATE ${GLEW_INCLUDE_DIR}
    PRIVATE ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(ogl_test
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(ogl_test m)
endif()

#TODO replace this with something that runs on build
#file(COPY src/data DESTINATION ./)
#file(COPY src/shaders DESTINATION ./)

add_executable(rbm_gen rbm/main.c)