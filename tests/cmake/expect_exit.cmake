# expect_exit.cmake - runs EXE and checks its exit code and the text it must print.
#
# Used by tests/CMakeLists.txt to prove the harness goes red on purpose, and
# refuses an unknown or empty run, without making the build itself red.
#
# Inputs:
#   EXE            executable to run
#   EXPECT_RC      exact exit code expected, as a decimal string
#   ARGS_TEXT      optional, command line arguments separated by spaces
#   REQUIRED_TEXT  text that must appear in stdout or stderr, one needle per
#                  "|" separator (needles may contain spaces)

if(NOT DEFINED EXE OR NOT DEFINED EXPECT_RC OR NOT DEFINED REQUIRED_TEXT)
  message(FATAL_ERROR "expect_exit.cmake needs EXE, EXPECT_RC and REQUIRED_TEXT")
endif()
if(NOT DEFINED ARGS_TEXT)
  set(ARGS_TEXT "")
endif()

separate_arguments(run_args UNIX_COMMAND "${ARGS_TEXT}")
execute_process(
  COMMAND "${EXE}" ${run_args}
  RESULT_VARIABLE run_rc
  OUTPUT_VARIABLE run_out
  ERROR_VARIABLE run_err
  TIMEOUT 120)

set(run_all "${run_out}${run_err}")

# A signal death reports a string such as "Segmentation fault", not the number
# expected, so a crash never passes as an intended failure.
if(NOT "${run_rc}" STREQUAL "${EXPECT_RC}")
  message(FATAL_ERROR "exit code is ${run_rc}, expected ${EXPECT_RC}\n${run_all}")
endif()

string(REPLACE "|" ";" needles "${REQUIRED_TEXT}")
foreach(needle IN LISTS needles)
  string(FIND "${run_all}" "${needle}" needle_pos)
  if(needle_pos EQUAL -1)
    message(FATAL_ERROR "missing in output: ${needle}\n${run_all}")
  endif()
endforeach()
