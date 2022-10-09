cmake_minimum_required(VERSION 3.12)
project(air-combat)

set(CMAKE_C_FLAGS "-W -g")

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(air-combat
    src/main.c
    src/spatial.h
    src/spatial.c
    src/render.h
    src/render.c
    src/transform.h
    src/transform.c
)

include_directories(${GLEW_INCLUDE_DIRS})

target_link_libraries(air-combat glfw ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

add_custom_target(run
    COMMAND air-combat
    DEPENDS air-combat
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
