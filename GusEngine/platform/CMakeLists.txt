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
    src/input/key_translation.cpp
    src/input/input_mapper.cpp
    src/render2d/viewport_transform.cpp
    src/render2d/render2d_rhi.cpp
)

# Shaders do render2d: GLSL -> .qsb (via qsb/ShaderTools), embutidos como recurso
# Qt (prefixo /gusengine/shaders). O Render2dRhi carrega de
# :/gusengine/shaders/quad.{vert,frag}.qsb em runtime.
qt6_add_shaders(gusengine_platform "gusengine_render2d_shaders"
    PREFIX "/gusengine"
    FILES
        render2d/shaders/quad.vert
        render2d/shaders/quad.frag
    OUTPUTS
        shaders/quad.vert.qsb
        shaders/quad.frag.qsb
)

target_include_directories(gusengine_platform
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# domain/ entra aqui: input_mapper consome o mapa logico puro (InputRemapConfig).
# Qt6::GuiPrivate expoe os headers do QRhi (API semi-privada do Qt; usada por
# window/ e render2d/ no M1, atras da interface IRenderer - risco R1 mitigado).
target_link_libraries(gusengine_platform
    PUBLIC
        gusengine_core
        gusengine_domain
        Qt6::Core
        Qt6::Gui
        Qt6::GuiPrivate
        Qt6::Quick
        Qt6::Multimedia
)

target_compile_features(gusengine_platform PUBLIC cxx_std_20)

# Testes Qt da camada platform/ (headless). Mesma condicao do raiz (BUILD_TESTING).
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
