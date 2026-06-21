# gusworld_app - executavel GusWorld-specific.
#
# Linka as tres libs; Qt entra transitivamente via gusengine_platform.
# No M0 e so um main que prova que tudo linka (imprime qVersion() e a versao
# da engine e retorna 0). Sem janela: a janela e M1 (platform/window).
#
# Subdominios previstos (engine-design.md secao 2):
#   screens/ Overworld / Battle / Menu (maquina de estados de tela)
#   main.cpp ponto de entrada

# main.cpp usa apenas Qt Core (QCoreApplication); AUTOMOC nao e necessario no
# M0, mas fica ligado para conveniencia quando screens/ entrar.
set(CMAKE_AUTOMOC ON)

add_executable(gusworld_app
    main.cpp
)

target_link_libraries(gusworld_app
    PRIVATE
        gusengine_core
        gusengine_domain
        gusengine_platform
)

target_compile_features(gusworld_app PRIVATE cxx_std_20)
