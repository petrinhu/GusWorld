# Fourth layer of the spine. Links only downward, to domain.
add_library(gusworld_app)

target_sources(gusworld_app PRIVATE layer.cpp)
target_link_libraries(gusworld_app PUBLIC gusworld_domain)

gusworld_apply_compile_options(gusworld_app)
