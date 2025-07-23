cmake_minimum_required(VERSION 3.16)
project(a35d C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Get version from git
execute_process(
    COMMAND git describe --tags --dirty --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_VERSION)
    set(GIT_VERSION "unknown")
endif()

set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(GENERATE
    OUTPUT ${GENERATED_DIR}/version.h
    CONTENT "#pragma once
#define VERSION \"${GIT_VERSION}\"
")

add_library(project_options INTERFACE)

target_compile_options(project_options INTERFACE
    $<$<CONFIG:Debug>:-Wall -Wextra -Werror -g -O0>
    $<$<CONFIG:Release>:-O2>
)

target_compile_definitions(project_options INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

target_include_directories(project_options INTERFACE
    ${GENERATED_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Build options
option(BUILD_POC "Build POC" ON)
option(BUILD_CORE "Build Core" OFF)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

if(BUILD_POC)
    add_executable(poc POC/poc.c)
    target_link_libraries(poc PRIVATE project_options Threads::Threads)

    add_executable(size POC/size.c)
    target_link_libraries(size PRIVATE project_options)
endif()
