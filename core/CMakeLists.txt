# Bottom layer of the spine. Depends on no other GusWorld layer.
add_library(gusworld_core)

target_sources(gusworld_core PRIVATE layer.cpp error_code.cpp)

target_include_directories(gusworld_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

gusworld_apply_compile_options(gusworld_core)
