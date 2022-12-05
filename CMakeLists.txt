cmake_minimum_required(VERSION 3.12)
project(asteroids)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(asteroids
    src/main.c
    src/spatial.h
    src/spatial.c
    src/render.h
    src/render.c
    src/transform.h
    src/transform.c
    src/game.h
    src/game.c
    src/player.h
    src/player.c
    src/camera.h
    src/camera.c
    src/input.h
    src/input.c
    src/asset.h
    src/asset.c
    src/platform.h
    src/platform.c
    src/hashmap.h
    src/hashmap.c
    src/shader.h
    src/shader.c
    src/world.h
    src/world.c
    src/collide.h
    src/collide.c
    src/log.h
    src/log.c
    src/menu.h
    src/menu.c
    src/audio.h
    src/audio.c
    src/calc.h
    src/calc.c
    src/timer.h
    src/timer.c
    src/vertex.h
    src/vertex.c
    src/actor.h
    src/actor.c
    src/color.h
    src/color.c
    src/texture.h
    src/texture.c
    src/mesh.h
    src/mesh.c
    src/font.h
    src/font.c
    src/particle.h
    src/particle.c
    src/orb.h
    src/orb.c
)

include_directories(. ${GLEW_INCLUDE_DIRS})

target_link_libraries(asteroids m glfw ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

add_custom_target(run
    COMMAND asteroids
    DEPENDS asteroids
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
