# Sanitizer fixtures: one executable per deliberate defect, so one proof cannot hide
# another. Only tests/CMakeLists.txt adds this directory, and only when
# GUSWORLD_SANITIZER_FIXTURES is ON.
#
# The sanitizer flags do not come from this file: the sanitizers job passes them in
# CMAKE_CXX_FLAGS. Built without them, these programs run to success, so a plain
# configure must never turn the switch on.

add_executable(gusworld_sanitizer_fixture_heap_buffer_overflow heap_buffer_overflow.cpp)
gusworld_apply_compile_options(gusworld_sanitizer_fixture_heap_buffer_overflow)

add_executable(gusworld_sanitizer_fixture_signed_overflow signed_overflow.cpp)
gusworld_apply_compile_options(gusworld_sanitizer_fixture_signed_overflow)

add_executable(gusworld_sanitizer_fixture_leak leak.cpp)
gusworld_apply_compile_options(gusworld_sanitizer_fixture_leak)
