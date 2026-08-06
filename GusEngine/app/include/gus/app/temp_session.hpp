// SPDX-License-Identifier: Apache-2.0
// gus/app/temp_session.hpp
//
// TEMPFILE-PROD-UNICO (2026-08-06): caminho temporario UNICO POR EXECUCAO para o codigo
// de PRODUCAO - o gemeo, do lado de producao, do que tmp_dir_test_support.hpp resolveu do
// lado dos testes.
//
// O DEFEITO QUE ISTO FECHA: as telas montavam seu stage/tempfile com nome FIXO ETERNO
// (`temp_directory_path() / "gusworld_glintfx_cockpit"`). Duas consequencias distintas:
//
//   (a) CORRIDA entre execucoes. Dois processos que exercitem a mesma tela gravam e leem
//       o MESMO arquivo. Medido no cockpit: dois TEST_CASEs gravam conteudo MUTUAMENTE
//       EXCLUSIVO (intro=true escreve o bloco de ABERTURA, intro=false o de COMBATE) no
//       mesmo `cockpit_baked.rml` e leem de volta - 0, 3 e 2 falhas em 12 pares
//       concorrentes, em tres rodadas. Nao aparece em rodada isolada (a suite roda em
//       serie); aparece com duas suites ao mesmo tempo, que e o fluxo real com agentes
//       em paralelo.
//   (b) NOME PREVISIVEL em diretorio de escrita publica e o vetor classico de ataque por
//       LINK SIMBOLICO: um terceiro cria o caminho apontando pra outro lugar antes do
//       jogo, e a escrita do jogo vai parar onde o atacante mandou. Em maquina de um dono
//       so o risco e baixo; o custo de arrumar e o mesmo nos dois casos.
//
// FIXO POR EXECUCAO, NAO UNICO POR CHAMADA (a escolha que o item TMPDIR-STAGE-FIXO-PRODUCAO
// mandou fazer caso a caso): o stage tem de ser DESCOBRIVEL - o humano depurando precisa
// achar o .rml gerado e abrir no editor -, e alguns chamadores pegam o dir por FORA depois
// que outro o preencheu (battle_preview.cpp le `glintfx_cockpit_stage_dir()` como base_url
// DEPOIS de `write_baked_cockpit_rml()` ter copiado os assets la dentro). Um path novo a
// cada chamada quebraria essa costura e espalharia lixo por frame. Entao: o PREFIXO
// continua o mesmo de sempre (o glob `gusworld_glintfx_cockpit*` acha), e o SUFIXO de
// execucao garante que duas execucoes nunca se cruzem.
//
// ENTROPIA EM PRODUCAO: `std::random_device` aqui NAO fere o determinismo do jogo. A regra
// da casa (motor 100% deterministico via IRandomSource injetavel) vale pra REGRA DE JOGO
// (core/domain); isto e nome de arquivo temporario, e o precedente ja existe em producao
// no mesmo espirito (`core/src/crypto/aead_xchacha20poly1305.cpp` tira nonce do CSPRNG do
// SO). PID sozinho nao basta: PID e reciclado, e duas execucoes com o mesmo PID em
// maquinas/containers diferentes montariam o mesmo nome.
//
// LIMPEZA: de proposito NAO ha remocao automatica no fim do processo - o motivo de existir
// o stage descobrivel e justamente poder olhar o arquivo DEPOIS que o jogo fechou. Custo
// medido: ~372 KB por execucao que monta o stage do cockpit (2 .ttf + 5 .png + 2 .rml).
// Quem cria stage em TESTE limpa o proprio (ScopedTempDir ou remove_all explicito).
//
// CAMADA: app/ - todos os consumidores sao telas/ferramentas de app/, e a peca nao toca
// SDL/GL/RmlUi/glintfx (so <filesystem> + a chamada de PID do SO). Header-only de
// proposito: sao ~3 funcoes curtas e assim nenhum CMakeLists precisa mudar.

#ifndef GUS_APP_TEMP_SESSION_HPP
#define GUS_APP_TEMP_SESSION_HPP

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <random>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <process.h>  // _getpid
#else
#include <unistd.h>  // getpid
#endif

namespace gus::app {

namespace detail {

inline unsigned long current_pid() {
#ifdef _WIN32
    return static_cast<unsigned long>(::_getpid());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

// 64 bits de entropia do SO, semeados UMA vez por THREAD (thread_local em vez de static
// simples: o gerador nao e thread-safe, e um `static` compartilhado seria corrida de dado
// se duas threads pedissem token ao mesmo tempo).
inline std::uint64_t random_word() {
    thread_local std::mt19937_64 rng{std::random_device{}()};
    return rng();
}

}  // namespace detail

// Token NOVO a cada chamada: <pid>_<contador>_<hex64>. O PID distingue processos
// concorrentes; o contador atomico distingue chamadas dentro do mesmo processo; o hex
// aleatorio fecha o caso de PID reciclado (dois processos na 1a chamada, mesmo contador).
[[nodiscard]] inline std::string new_temp_token() {
    static std::atomic<std::uint64_t> counter{0};
    const std::uint64_t n = counter.fetch_add(1, std::memory_order_relaxed);
    char buf[80];
    std::snprintf(buf, sizeof(buf), "%lu_%llu_%016llx", detail::current_pid(),
                  static_cast<unsigned long long>(n),
                  static_cast<unsigned long long>(detail::random_word()));
    return {buf};
}

// Token FIXO da execucao: o mesmo valor em todas as chamadas deste processo, diferente
// entre processos. E o que da a propriedade "fixo dentro da execucao, unico entre
// execucoes". Inicializacao de static local = thread-safe por garantia da linguagem.
[[nodiscard]] inline const std::string& temp_session_token() {
    static const std::string token = new_temp_token();
    return token;
}

// <tmp>/<prefixo>_<token da execucao>. NAO cria o diretorio (quem chama e quem cria, com
// o error_code que ja usa hoje).
[[nodiscard]] inline std::filesystem::path session_temp_dir(std::string_view prefixo) {
    return std::filesystem::temp_directory_path() /
           (std::string(prefixo) + "_" + temp_session_token());
}

// <tmp>/<prefixo>_<token da execucao><extensao> - o irmao ARQUIVO do de cima (ex.:
// prefixo "gusworld_glintfx_smoke" + extensao ".rml"). NAO cria o arquivo.
[[nodiscard]] inline std::filesystem::path session_temp_file(std::string_view prefixo,
                                                              std::string_view extensao) {
    std::filesystem::path p = session_temp_dir(prefixo);
    p += extensao;
    return p;
}

}  // namespace gus::app

#endif  // GUS_APP_TEMP_SESSION_HPP
