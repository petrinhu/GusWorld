# Top layer of the spine. Off by default: it stays blocked until the upstream
# engine provides input and text, so its target is only defined when the option is ON.
option(GUSWORLD_PRESENT "Build the presentation layer (blocked until its prerequisites exist)" OFF)

if(GUSWORLD_PRESENT)
  add_library(gusworld_present)

  target_sources(gusworld_present PRIVATE layer.cpp)
  target_link_libraries(gusworld_present PUBLIC gusworld_app)

  gusworld_apply_compile_options(gusworld_present)
endif()
