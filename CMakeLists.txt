cmake_minimum_required(VERSION 3.20)

project(
    Forge
    VERSION 0.1.0
    LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(forge ${SOURCES})

target_include_directories(forge PRIVATE
    src
)

if(MSVC)
    target_compile_options(forge PRIVATE /W4)
else()
    target_compile_options(forge PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()