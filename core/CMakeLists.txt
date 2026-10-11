# Bottom layer of the spine. Depends on no other GusWorld layer.
add_library(gusworld_core)

target_sources(gusworld_core PRIVATE layer.cpp)

gusworld_apply_compile_options(gusworld_core)
