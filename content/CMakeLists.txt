# Second layer of the spine. Links only downward, to core.
add_library(gusworld_content)

target_sources(gusworld_content PRIVATE layer.cpp)
target_link_libraries(gusworld_content PUBLIC gusworld_core)

gusworld_apply_compile_options(gusworld_content)
