cmake_minimum_required(VERSION 3.9)
project(reig
        VERSION 0.0.0
        LANGUAGES CXX)

set(CMAKE_CXX_COMPILER clang++)

# reig library
add_library(reig_lib
        lib/reig/stb_truetype.h
        lib/reig/fwd.h
        lib/reig/primitive.h lib/reig/primitive.cpp
        lib/reig/mouse.h lib/reig/mouse.cpp
        lib/reig/keyboard.h lib/reig/keyboard.cpp
        lib/reig/keyboard_shifted.cpp
        lib/reig/exception.h lib/reig/exception.cpp
        lib/reig/maths.h
        lib/reig/text.h
        lib/reig/context.h lib/reig/context.cpp
        lib/reig/reference_widget.h lib/reig/reference_widget.cpp
        lib/reig/reference_widget_list.h lib/reig/reference_widget_list.cpp
        lib/reig/reference_widget_slider.cpp
        lib/reig/reference_widget_entry.h lib/reig/reference_widget_entry.tcc
        lib/reig/config.h lib/reig/config.cpp
        lib/reig/window.h lib/reig/window.cpp
        )
add_library(my::reig_lib ALIAS reig_lib)
target_compile_features(reig_lib
        PUBLIC
            cxx_std_17)

# SDL 2 test bed
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/SDL-TestBed/cmake)
add_executable(sdl_bed
        SDL-TestBed/main.cpp
        SDL-TestBed/StopWatch.h
        SDL-TestBed/StopWatch.cpp)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
target_link_libraries(sdl_bed
        my::reig_lib
        SDL2_gfx
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARIES})
find_package(Boost)
target_include_directories(sdl_bed
        PUBLIC
        ${PROJECT_SOURCE_DIR}/lib
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS})

# Open GL 3 test bed
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/GL3-TestBed/cmake)
add_executable(gl3_bed
        GL3-TestBed/Camera.cpp
        GL3-TestBed/Camera.h
        GL3-TestBed/main.cpp
        GL3-TestBed/Shader.cpp
        GL3-TestBed/Shader.hpp)
target_link_libraries(gl3_bed
        my::reig_lib
        glfw
        GLEW
        GLU
        GL
        SOIL)
target_include_directories(gl3_bed
        PUBLIC
        ${PROJECT_SOURCE_DIR}/lib)