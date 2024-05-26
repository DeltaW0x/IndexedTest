cmake_minimum_required(VERSION 3.28)
project(IndexedQuad)
include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY  https://github.com/thatcosmonaut/SDL.git
        GIT_TAG         gpu
)
FetchContent_MakeAvailable(SDL3)

add_executable(IndexedQuad Source/main.cpp
        Source/Context.cpp
        Source/Context.hpp
        Source/triangle_spirv.h
        Source/triangle_dxbc.h)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
