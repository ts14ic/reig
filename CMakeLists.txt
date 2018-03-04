cmake_minimum_required(VERSION 3.9)
project(reig)

set(CMAKE_CXX_STANDARD 14)

# reig library
set(LIB_SRC lib/reig.h lib/reig.cpp)
add_library(reig_lib ${LIB_SRC})

# SDL 2 test bed
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/SDL-TestBed/cmake)
set(SDL_BED_SRC
        SDL-TestBed/main.cpp
        SDL-TestBed/StopWatch.h
        SDL-TestBed/StopWatch.cpp)
add_executable(sdl_bed ${SDL_BED_SRC})
target_link_libraries(sdl_bed
        reig_lib
        SDL2_gfx
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARIES})
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
include_directories(
        ${PROJECT_SOURCE_DIR}/lib
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIRS})

# Open GL 3 test bed
# TODO: Link opengl
# TODO: Transition to target based cmake format
set(GL3_BED_SRC
        GL3-TestBed/Camera.cpp
        GL3-TestBed/Camera.h
        GL3-TestBed/main.cpp
        GL3-TestBed/Shader.cpp
        GL3-TestBed/Shader.hpp)
add_executable(gl3_bed ${GL3_BED_SRC})
target_link_libraries(gl3_bed reig_lib)
