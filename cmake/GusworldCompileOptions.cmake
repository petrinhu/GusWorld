# Compile contract for every GusWorld target: C++23 without compiler extensions,
# strong warnings, and an opt-in warnings-as-errors switch that only CI turns on.

option(GUSWORLD_WERROR "Treat compiler warnings as errors" OFF)

function(gusworld_set_standard target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
endfunction()

function(gusworld_set_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(GUSWORLD_WERROR)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(GUSWORLD_WERROR)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

function(gusworld_apply_compile_options target)
  gusworld_set_standard(${target})
  gusworld_set_warnings(${target})
endfunction()
