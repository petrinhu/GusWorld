# Monocypher vendorizado (ADR-015 Onda 1, SAVE-CRYPTO-V2-VENDOR). 2 arquivos C
# (monocypher.c/.h), dominio publico de fato (dupla licenca CC0-1.0 / BSD-2-Clause),
# ZERO dependencia externa. Ver README-VENDOR.md deste diretorio para versao/commit
# exatos. NAO modificado (copia byte-a-byte do upstream, conferida por md5sum).
#
# E C99, nao C++: compilado como C (o root CMakeLists.txt habilita a linguagem C so
# para este target). gusengine_core (C++20) linka este alvo estatico.
add_library(monocypher STATIC monocypher.c)

target_include_directories(monocypher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(monocypher PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
