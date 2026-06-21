# gusengine_platform - fronteira Qt. UNICA camada (com app/) que conhece Qt.
#
# Esta lib PODE e DEVE linkar Qt. E aqui que a invariante das 4 camadas
# "se paga": toda dependencia de Qt fica confinada a platform/ e app/, de
# modo que core/ e domain/ permanecam puros e reutilizaveis.
#
# Subdominios previstos (engine-design.md secao 2, em M1+):
#   window/   janela + swap
#   render2d/ batcher de sprites + atlas + tilemap (Qt RHI)
#   input/    eventos Qt para acoes logicas (consome input_remap)
#   audio/    saida sobre Qt Multimedia (implementa o hook de core/)
#   fs/       I/O de arquivo (implementa save/i18n)
# No M0 so existe um stub que prova que o Qt linka (retorna qVersion()).

# AUTOMOC ligado so aqui (camada Qt). Mantido OFF nas libs puras.
set(CMAKE_AUTOMOC ON)

add_library(gusengine_platform STATIC
    src/platform_info.cpp
)

target_include_directories(gusengine_platform
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gusengine_platform
    PUBLIC
        gusengine_core
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::Multimedia
)

target_compile_features(gusengine_platform PUBLIC cxx_std_20)
