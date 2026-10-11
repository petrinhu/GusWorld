# GusWorld root build. Orchestrates only: options, flags and targets live in the modules and subdirectories.
cmake_minimum_required(VERSION 4.1)

project(gusworld VERSION 0.0.0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GusworldCompileOptions)

add_subdirectory(tests)
