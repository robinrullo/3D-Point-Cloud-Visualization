cmake_minimum_required(VERSION 3.30)
project(generation_enrichissement_environnement2d3d)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

add_executable(generation_enrichissement_environnement2d3d main.cpp)

target_link_libraries(generation_enrichissement_environnement2d3d
        OpenGL::GL
        glfw
        GLEW
        "-framework GLUT"
)
