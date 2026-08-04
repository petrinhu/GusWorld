// SPDX-License-Identifier: Apache-2.0
// GusEngine/tests/support/tmp_dir_test_support.hpp (test helper)
//
// Diretorio temporario UNICO por CHAMADA E por PROCESSO, para toda a suite (app/tests
// e platform/tests). Motivo: catch_discover_tests roda 1 PROCESSO por TEST_CASE, entao
// um contador `static` (que comeca em 0 a cada processo) NAO evita colisao entre
// TEST_CASEs concorrentes - dois processos calculam o MESMO path e disputam a MESMA
// pasta em disco. Medido: 91-100 falhas em 100 pares concorrentes com contador static
// OU nome fixo, 0 em 200 isolado (nao e lentidao/timeout, e colisao deterministica).
// Agravado nesta maquina porque TMPDIR e herdado do systemd de usuario (compartilhado
// entre QUALQUER sessao, o cenario real de 2 agentes rodando teste ao mesmo tempo).
//
// unique_temp_dir(tag) compoe PID (distingue processos concorrentes) + contador
// atomico por-processo (distingue chamadas dentro do MESMO processo/TEST_CASE) + um
// sufixo aleatorio (fecha o caso raro de PID reciclado por outro processo pegando o
// MESMO contador - ambos na 1a chamada). O `tag` continua no nome so para
// legibilidade humana (ex.: sobra de pasta o path ainda diz de qual teste ela era).
//
// Aleatoriedade de sistema (std::random_device/mt19937_64) e ACEITAVEL AQUI porque
// isto e codigo de TESTE (nao dominio) - o motor do jogo continua 100% deterministico
// via IRandomSource injetavel (core/domain). Nao copiar este padrao pra dominio.
//
// ScopedTempDir cria o diretorio no construtor e o remove no destrutor (RAII) - a
// limpeza dispara mesmo quando o teste falha no meio (Catch2 desenrola a pilha via
// excecao em REQUIRE), entao nao troca flaky por lixo acumulando em disco.

#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <random>
#include <string>
#include <system_error>

#ifdef _WIN32
#include <process.h>  // _getpid
#else
#include <unistd.h>  // getpid
#endif

namespace gus::test_support {

namespace detail {

inline unsigned long current_pid() {
#ifdef _WIN32
    return static_cast<unsigned long>(::_getpid());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

// Sufixo aleatorio de 64 bits (hex), semeado uma vez por processo. So para teste -
// ver o comentario no topo do arquivo sobre por que aleatoriedade de sistema e
// aceitavel aqui e nao no dominio.
inline std::uint64_t random_suffix() {
    static std::mt19937_64 rng{std::random_device{}()};
    return rng();
}

}  // namespace detail

// Devolve um path AINDA NAO CRIADO, unico por processo + chamada:
// <tmp>/<tag>_<pid>_<contador>_<random-hex>.
inline std::filesystem::path unique_temp_dir(const std::string& tag) {
    static std::atomic<std::uint64_t> counter{0};
    const std::uint64_t n = counter.fetch_add(1);
    const unsigned long pid = detail::current_pid();
    char rand_hex[17];
    std::snprintf(rand_hex, sizeof(rand_hex), "%016llx",
                  static_cast<unsigned long long>(detail::random_suffix()));
    return std::filesystem::temp_directory_path() /
           (tag + "_" + std::to_string(pid) + "_" + std::to_string(n) + "_" + rand_hex);
}

// Devolve um path de ARQUIVO (nao diretorio) unico por processo + chamada, com a
// extensao dada (ex.: ".wav") - para testes que gravam um unico arquivo solto no tmp
// (em vez de uma arvore inteira). NAO cria o arquivo; quem chama e quem grava (e quem
// remove - sem RAII aqui, ver ScopedTempDir se precisar de limpeza garantida).
inline std::filesystem::path unique_temp_file(const std::string& tag,
                                               const std::string& extension) {
    std::filesystem::path p = unique_temp_dir(tag);
    p += extension;
    return p;
}

// Controla se ScopedTempDir cria o diretorio no construtor (ver enum abaixo). Alguns
// testes (ex.: "diretorio AUSENTE devolve default" ou "save_X cria o diretorio com
// permissao 0700") dependem do path NAO existir ainda quando o codigo sob teste roda -
// pre-criar quebraria exatamente o que esses testes provam. kCreateNow e o default
// (a maioria dos testes so quer um lugar hermetico pra escrever).
enum class TempDirCreate { kCreateNow, kDeferCreation };

// RAII: por padrao cria o diretorio (create_directories) no construtor; com
// TempDirCreate::kDeferCreation, so calcula o path unico e deixa o codigo sob teste
// criar (ou nao) o diretorio. Em QUALQUER modo, remove tudo (remove_all) no destrutor
// - mesmo se o teste falhar/REQUIRE lancar no meio, e mesmo se o diretorio nunca
// chegou a ser criado (remove_all de path inexistente e no-op, ec ignorado).
class ScopedTempDir {
public:
    explicit ScopedTempDir(const std::string& tag,
                            TempDirCreate mode = TempDirCreate::kCreateNow)
        : path_(unique_temp_dir(tag)) {
        if (mode == TempDirCreate::kCreateNow) {
            std::error_code ec;
            std::filesystem::create_directories(path_, ec);
        }
    }

    ~ScopedTempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    ScopedTempDir(const ScopedTempDir&) = delete;
    ScopedTempDir& operator=(const ScopedTempDir&) = delete;
    ScopedTempDir(ScopedTempDir&&) = delete;
    ScopedTempDir& operator=(ScopedTempDir&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }
    [[nodiscard]] std::string string() const { return path_.string(); }

private:
    std::filesystem::path path_;
};

}  // namespace gus::test_support
