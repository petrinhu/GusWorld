# expect_build.cmake - configures and builds SOURCE_DIR with GUSWORLD_WERROR as
# given, then checks that the build passes or fails as expected and prints the
# text it must print.
#
# Used by tests/CMakeLists.txt to prove the warnings-as-errors gate (L-19 portao 1):
# the red case must fail because a warning became an error, and the control must
# pass with the same source, so a failure cannot be blamed on the fixture alone.
#
# Inputs:
#   SOURCE_DIR     directory holding the CMakeLists.txt to configure
#   BUILD_DIR      build directory; deleted and recreated, so an old cache cannot leak in
#   WERROR         ON or OFF, passed as GUSWORLD_WERROR
#   EXPECT_OK      TRUE when the build must pass, FALSE when it must fail
#   CXX_COMPILER   optional, compiler the configure must use (the one the root build uses)
#   REQUIRED_TEXT  optional, text that must appear in the output, one needle per
#                  "|" separator (needles may contain spaces)
#
# Three outcomes are kept apart (L-49 of the global laws):
#   - configure failed: never a valid red, because the fixture never reached the compiler
#   - build result differs from EXPECT_OK: reported with the full output
#   - process killed by a signal, or exit code 128 or more: a tool dying, reported
#     apart and never taken as a legitimate failure

if(NOT DEFINED SOURCE_DIR OR NOT DEFINED BUILD_DIR OR NOT DEFINED WERROR OR NOT DEFINED EXPECT_OK)
  message(FATAL_ERROR "expect_build.cmake needs SOURCE_DIR, BUILD_DIR, WERROR and EXPECT_OK")
endif()
if(NOT DEFINED CXX_COMPILER)
  set(CXX_COMPILER "")
endif()
if(NOT DEFINED REQUIRED_TEXT)
  set(REQUIRED_TEXT "")
endif()

# A list, not a string: an empty value must add no argument at all.
set(compiler_args "")
if(NOT "${CXX_COMPILER}" STREQUAL "")
  list(APPEND compiler_args "-DCMAKE_CXX_COMPILER=${CXX_COMPILER}")
endif()

# Refuse a BUILD_DIR that could take the source tree with it (L-53 of the global
# laws: deletion here cannot be undone, so the check runs before REMOVE_RECURSE
# and fails closed). Paths are normalized LEXICALLY, not with REALPATH, because
# REALPATH needs the path to exist and BUILD_DIR usually does not on a first run.
# Not covered: symbolic links are not resolved, so a BUILD_DIR that is a link into
# the source tree is not refused. An unset HOME skips only the home-directory check.
if("${SOURCE_DIR}" STREQUAL "")
  message(FATAL_ERROR "refuses an empty SOURCE_DIR: there is no source to protect")
endif()
if("${BUILD_DIR}" STREQUAL "")
  message(FATAL_ERROR "refuses an empty BUILD_DIR: an empty target must not reach REMOVE_RECURSE")
endif()

cmake_path(ABSOLUTE_PATH BUILD_DIR NORMALIZE OUTPUT_VARIABLE build_norm)
cmake_path(ABSOLUTE_PATH SOURCE_DIR NORMALIZE OUTPUT_VARIABLE source_norm)
cmake_path(ABSOLUTE_PATH "${CMAKE_CURRENT_LIST_DIR}/../.." NORMALIZE OUTPUT_VARIABLE repo_norm)

# A trailing separator would make "/a/b/" and "/a/b" compare unequal. The filesystem
# root "/" becomes empty here, and that empty value is refused as the root below.
string(REGEX REPLACE "/+$" "" build_norm "${build_norm}")
string(REGEX REPLACE "/+$" "" source_norm "${source_norm}")
string(REGEX REPLACE "/+$" "" repo_norm "${repo_norm}")

if("${build_norm}" STREQUAL "")
  message(FATAL_ERROR "refuses BUILD_DIR at the filesystem root: deleting it would delete everything")
endif()
if("${build_norm}" STREQUAL "${repo_norm}")
  message(FATAL_ERROR "refuses BUILD_DIR equal to the repository root")
endif()
if(NOT "$ENV{HOME}" STREQUAL "")
  cmake_path(ABSOLUTE_PATH "$ENV{HOME}" NORMALIZE OUTPUT_VARIABLE home_norm)
  string(REGEX REPLACE "/+$" "" home_norm "${home_norm}")
  if("${build_norm}" STREQUAL "${home_norm}")
    message(FATAL_ERROR "refuses BUILD_DIR equal to the home directory")
  endif()
endif()
if("${build_norm}" STREQUAL "${source_norm}")
  message(FATAL_ERROR "refuses BUILD_DIR equal to SOURCE_DIR")
endif()
# BUILD_DIR is the same as or an ancestor of SOURCE_DIR: deleting it deletes the sources.
# The slashes make the test about whole path components, so "/a/b" never matches "/a/bc".
string(FIND "${source_norm}/" "${build_norm}/" build_is_ancestor)
if(build_is_ancestor EQUAL 0)
  message(FATAL_ERROR "refuses BUILD_DIR that contains SOURCE_DIR: deleting it would delete the sources")
endif()

file(REMOVE_RECURSE "${BUILD_DIR}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -S "${SOURCE_DIR}" -B "${BUILD_DIR}"
    "-DGUSWORLD_WERROR=${WERROR}" ${compiler_args}
  RESULT_VARIABLE configure_rc
  OUTPUT_VARIABLE configure_out
  ERROR_VARIABLE configure_err
  TIMEOUT 120)

set(configure_all "${configure_out}${configure_err}")
if(NOT "${configure_rc}" STREQUAL "0")
  message(FATAL_ERROR "configure failed (result ${configure_rc}): the fixture never reached the compiler, so this is not a proof\n${configure_all}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}"
  RESULT_VARIABLE build_rc
  OUTPUT_VARIABLE build_out
  ERROR_VARIABLE build_err
  TIMEOUT 150)

set(build_all "${build_out}${build_err}")

# A signal death reports text such as "Segmentation fault", and 128 or more is
# the same kind of death. Neither is a warning promoted to an error.
set(build_died FALSE)
if(NOT "${build_rc}" MATCHES "^[0-9]+$")
  set(build_died TRUE)
elseif(build_rc GREATER_EQUAL 128)
  set(build_died TRUE)
endif()
if(build_died)
  message(FATAL_ERROR "build process died (result ${build_rc}): tool death, not a legitimate failure\n${build_all}")
endif()

if(EXPECT_OK AND NOT "${build_rc}" STREQUAL "0")
  message(FATAL_ERROR "build should pass, got result ${build_rc}\n${build_all}")
endif()
if(NOT EXPECT_OK AND "${build_rc}" STREQUAL "0")
  message(FATAL_ERROR "build should fail, got result 0: the warning did not become an error\n${build_all}")
endif()

string(REPLACE "|" ";" needles "${REQUIRED_TEXT}")
foreach(needle IN LISTS needles)
  string(FIND "${build_all}" "${needle}" needle_pos)
  if(needle_pos EQUAL -1)
    message(FATAL_ERROR "missing in build output: ${needle}\n${build_all}")
  endif()
endforeach()
