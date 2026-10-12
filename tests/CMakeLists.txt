# Test harness: in-house and minimal (ADR-025). No third-party test framework.

# enable_testing(), the timeout and include(CTest) live in the ROOT CMakeLists.txt:
# add_test() in a subdirectory is only visible to ctest when the test subsystem was
# enabled in the root scope.

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

# Warnings-as-errors gate (L-19 portao 1), proved on a fixture with deliberate
# warnings. Registered only when GUSWORLD_WERROR is ON, because only then is the
# red case a fact to expect: with the switch OFF it could never pass. Each case
# builds in its own directory, so the two sub-builds cannot contaminate each other.
#
# Diagnostic needles were measured on neither compiler here (L-35: nothing runs
# in this session). GCC prints "error: unused parameter 'x' [-Werror=unused-parameter]"
# and Clang prints "error: unused parameter 'x' [-Werror,-Wunused-parameter]", so
# "error: unused parameter" and "[-Werror" are the same text on both. Declared
# unverified until the CI runs them (GCC on Fedora is the CI compiler).
if(GUSWORLD_WERROR)
  set(werror_fixture_dir ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/werror)
  set(expect_build_script ${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect_build.cmake)

  # Red: the same source must fail, and fail because a warning was promoted.
  add_test(NAME werror_gate_red_fails_on_warning
    COMMAND ${CMAKE_COMMAND}
      -DSOURCE_DIR=${werror_fixture_dir}
      -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/werror_red
      -DWERROR=ON
      -DEXPECT_OK=FALSE
      -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DREQUIRED_TEXT=error:\ unused\ parameter|[-Werror
      -P ${expect_build_script})

  # Control: the same source with the switch OFF must pass, and must still
  # print the warning. Without it, a red could be blamed on a fixture that never compiled.
  add_test(NAME werror_gate_control_passes_without_werror
    COMMAND ${CMAKE_COMMAND}
      -DSOURCE_DIR=${werror_fixture_dir}
      -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/werror_control
      -DWERROR=OFF
      -DEXPECT_OK=TRUE
      -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DREQUIRED_TEXT=warning:\ unused\ parameter
      -P ${expect_build_script})
endif()
