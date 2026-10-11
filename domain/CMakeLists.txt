# Third layer of the spine. Links only downward, to content.
add_library(gusworld_domain)

target_sources(gusworld_domain PRIVATE layer.cpp)
target_link_libraries(gusworld_domain PUBLIC gusworld_content)

gusworld_apply_compile_options(gusworld_domain)
