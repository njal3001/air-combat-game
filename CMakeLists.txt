cmake_minimum_required(VERSION 3.12)
project(ww1-air-combat)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

message("${GLFW3_LIBRARIES}")

add_executable(ww1-air-combat
    src/main.c
)

target_link_libraries(ww1-air-combat glfw ${OPENGL_LIBRARIES})

add_custom_target(run
    COMMAND ww1-air-combat
    DEPENDS ww1-air-combat
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
