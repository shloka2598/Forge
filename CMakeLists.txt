cmake_minimum_required(VERSION 3.20)

project(
    Forge
    VERSION 0.1.0
    LANGUAGES CXX
)

include(FetchContent)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

list(REMOVE_ITEM SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_library(forge_core ${SOURCES})

add_executable(forge src/main.cpp)

target_link_libraries(forge PRIVATE forge_core)

target_include_directories(forge_core PUBLIC
    src
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.10.0
)

FetchContent_MakeAvailable(Catch2)

enable_testing()

add_subdirectory(tests)

if(MSVC)
    target_compile_options(forge_core PRIVATE /W4)
else()
    target_compile_options(forge_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()