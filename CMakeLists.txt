# GusWorld root build. Orchestrates only: options, flags and targets live in the modules and subdirectories.
cmake_minimum_required(VERSION 4.1)

project(gusworld VERSION 0.0.0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GusworldCompileOptions)

# The test subsystem is enabled HERE, in the root scope, and not inside tests/.
# Measured in a container: with enable_testing() only in the subdirectory, the
# seven add_test() calls are written to the subdirectory file, the root one never
# references them, and `ctest --test-dir <build>` answers "No tests were found!!!"
# while still exiting with code ZERO. A silent false green.
#
# DART_TESTING_TIMEOUT must stay BEFORE include(CTest): CTest sets it as a cache
# entry at include time with a default of 1500 seconds, without FORCE, so the value
# set here survives and one set afterwards would have no effect.
set(DART_TESTING_TIMEOUT 300 CACHE STRING
  "Maximum time allowed before CTest will kill the test.")

include(CTest)

add_subdirectory(core)
add_subdirectory(content)
add_subdirectory(domain)
add_subdirectory(app)
add_subdirectory(present)

add_subdirectory(tests)
