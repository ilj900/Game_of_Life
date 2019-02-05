cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)

project(Game_of_Life)

set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 11)

include_directories(include)
link_directories(libs)

set(SOURCES src/main.cpp
            src/glad.c
            src/shader.cpp)
set(INCLUDES    include/glfw3.h
                include/shader.h)
set(RESOURCES   res/shaders/simple.vertex.shader      res/shaders/simple.compute.shader     res/shaders/simple.fragment.shader)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

add_executable(Game_of_Life ${SOURCES} ${INCLUDES} ${RESOURCES})
target_link_libraries(Game_of_Life libglfw3.a)


