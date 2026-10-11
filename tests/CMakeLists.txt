# Test harness: in-house and minimal (ADR-025). No third-party test framework.

enable_testing()

# Must be set BEFORE include(CTest). CTest sets this as a cache entry at include time
# with a default of 1500 seconds, and the cache value set here is kept because the
# CTest set does not use FORCE. Fixing it after the include has no effect.
set(DART_TESTING_TIMEOUT 300 CACHE STRING
  "Maximum time allowed before CTest will kill the test.")

include(CTest)

add_library(gusworld_test_harness STATIC
  harness/check.cpp
  harness/test_registry.cpp
  harness/runner.cpp)
target_include_directories(gusworld_test_harness PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/harness)
gusworld_apply_compile_options(gusworld_test_harness)

# Self-test executables. Only gusworld_harness_selftest is a plain green gate.
# The red and empty ones exist to prove the harness refuses, and are run by
# expect_exit.cmake, which turns their expected failure into a passing test.
add_executable(gusworld_harness_selftest
  harness/main.cpp
  selftest/selftest_pass.cpp)
add_executable(gusworld_harness_selftest_red
  harness/main.cpp
  selftest/selftest_red.cpp)
add_executable(gusworld_harness_selftest_empty
  harness/main.cpp)

foreach(selftest_target IN ITEMS
    gusworld_harness_selftest
    gusworld_harness_selftest_red
    gusworld_harness_selftest_empty)
  target_link_libraries(${selftest_target} PRIVATE gusworld_test_harness)
  gusworld_apply_compile_options(${selftest_target})
endforeach()

set(expect_exit_script ${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_exit.cmake)

add_test(NAME harness_selftest
  COMMAND gusworld_harness_selftest)

add_test(NAME harness_selftest_list
  COMMAND ${CMAKE_COMMAND}
    -DEXE=$<TARGET_FILE:gusworld_harness_selftest>
    -DARGS_TEXT=--list
    -DEXPECT_RC=0
    -DREQUIRED_TEXT=selftest_pass_check|selftest_pass_second
    -P ${expect_exit_script})

add_test(NAME harness_selftest_case_filter
  COMMAND ${CMAKE_COMMAND}
    -DEXE=$<TARGET_FILE:gusworld_harness_selftest>
    -DARGS_TEXT=--case\ selftest_pass_check
    -DEXPECT_RC=0
    -DREQUIRED_TEXT=[\ PASS\ ]\ selftest_pass_check|gusworld\ tests:\ 1\ run,\ 0\ failed
    -P ${expect_exit_script})

add_test(NAME harness_selftest_unknown_case_fails
  COMMAND ${CMAKE_COMMAND}
    -DEXE=$<TARGET_FILE:gusworld_harness_selftest>
    -DARGS_TEXT=--case\ no_such_case
    -DEXPECT_RC=2
    -DREQUIRED_TEXT=no\ such\ case:\ no_such_case
    -P ${expect_exit_script})

add_test(NAME harness_selftest_usage_error_fails
  COMMAND ${CMAKE_COMMAND}
    -DEXE=$<TARGET_FILE:gusworld_harness_selftest>
    -DARGS_TEXT=--bogus
    -DEXPECT_RC=2
    -DREQUIRED_TEXT=usage:
    -P ${expect_exit_script})

add_test(NAME harness_selftest_red_is_detected
  COMMAND ${CMAKE_COMMAND}
    -DEXE=$<TARGET_FILE:gusworld_harness_selftest_red>
    -DEXPECT_RC=1
    -DREQUIRED_TEXT=CHECK\ failed:\ selftest_two\ ==\ 1|[\ FAIL\ ]\ selftest_red_check|[\ FAIL\ ]\ selftest_red_std_exception|unexpected\ std::exception:\ selftest|[\ FAIL\ ]\ selftest_red_foreign_throw|unexpected\ exception\ of\ a\ non-std\ type|[\ PASS\ ]\ selftest_red_then_pass|gusworld\ tests:\ 4\ run,\ 3\ failed
    -P ${expect_exit_script})

add_test(NAME harness_selftest_empty_registry_fails
  COMMAND ${CMAKE_COMMAND}
    -DEXE=$<TARGET_FILE:gusworld_harness_selftest_empty>
    -DEXPECT_RC=2
    -DREQUIRED_TEXT=no\ test\ case\ registered
    -P ${expect_exit_script})
