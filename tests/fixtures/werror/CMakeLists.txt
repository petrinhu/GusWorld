# Werror fixture: a standalone project that proves the warnings-as-errors gate.
# Only the sub-builds started by tests/CMakeLists.txt configure it (C5c); the
# root build never adds it, so its deliberate warnings reach no other target.
cmake_minimum_required(VERSION 4.1)

project(gusworld_werror_fixture LANGUAGES CXX)

# The real options and flags of the project, not a copy: a -Werror typed here
# would prove the fixture and not the gate. Path derived from this file, so the
# fixture works on any checkout.
include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/GusworldCompileOptions.cmake)

add_library(gusworld_werror_fixture STATIC werror_fixture.cpp)
gusworld_apply_compile_options(gusworld_werror_fixture)
