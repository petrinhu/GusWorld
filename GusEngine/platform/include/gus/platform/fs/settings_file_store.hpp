// gus/platform/fs/settings_file_store.hpp
//
// I/O REAL em disco de SystemSettings (MENU-PAUSA-CONFIG-SOM, M7-COSTURA):
// ~/.gusworld/settings.json, arquivo JSON PROPRIO (dep-free, reusa o parser/
// serializer de gus/domain/settings/system_settings_json.hpp). Distinto do save
// do JOGO (gus/domain/save/, binario+HMAC, dentro do formato .gmap/save slots) -
// settings.json e uma PREFERENCIA de conforto do jogador (volume), fora de
// qualquer partida, formato legivel/editavel a mao de proposito.
//
// PERMISSOES (LGPD - dado do usuario so ele le/escreve, mesma politica de
// diretorios de config de outros apps *nix): o diretorio e criado com 0700
// (rwx so o dono) e o arquivo com 0600 (rw so o dono, sem exec) - ver
// save_system_settings(). Falha de I/O (disco cheio, permissao negada por outro
// motivo, etc.) NUNCA lanca: degrada com seguranca (load_system_settings cai nos
// defaults do SystemSettings; save_system_settings devolve false).
//
// Cross-ref: gus/domain/settings/system_settings.hpp (o struct),
//            gus/domain/settings/system_settings_json.hpp (serialize/parse),
//            gus/app/maestro.hpp (carrega no boot, salva quando o config muda).

#ifndef GUS_PLATFORM_FS_SETTINGS_FILE_STORE_HPP
#define GUS_PLATFORM_FS_SETTINGS_FILE_STORE_HPP

#include <string>

#include "gus/domain/settings/system_settings.hpp"

namespace gus::platform::fs {

// Resolve o DIRETORIO de settings pra uso REAL do jogo: env GUSWORLD_HOME (override
// de teste/CI, mesmo espirito de GUSWORLD_ASSETS) > $HOME/.gusworld (default). So
// monta a STRING (nao cria nada em disco) - quem quer o diretorio de fato criado
// chama save_system_settings, que cria sob demanda.
[[nodiscard]] std::string resolve_settings_dir();

// Caminho do arquivo settings.json DENTRO do diretorio dado (dir/settings.json).
// Funcao pura (so concatena path) - usada tanto pela produção quanto pelos testes
// (que passam um dir temporario explicito em vez de resolve_settings_dir()).
[[nodiscard]] std::string settings_file_path(const std::string& dir);

// Carrega o settings.json de `dir` (settings_file_path(dir)). Degradacao segura em
// QUALQUER falha (arquivo ausente - 1a execucao do jogo; JSON corrompido; erro de
// leitura): devolve SystemSettings nos DEFAULTS, nunca lanca.
[[nodiscard]] gus::domain::settings::SystemSettings load_system_settings(
    const std::string& dir);

// Grava `settings` em settings_file_path(dir), criando o diretorio (permissao
// 0700) se ainda nao existir. O arquivo e escrito com permissao 0600. Devolve
// true em sucesso; false em qualquer falha de I/O (nunca lanca) - o chamador
// decide se avisa o jogador ou so loga (a config em MEMORIA continua valendo
// pro resto da sessao mesmo se a gravacao falhar).
[[nodiscard]] bool save_system_settings(
    const gus::domain::settings::SystemSettings& settings, const std::string& dir);

}  // namespace gus::platform::fs

#endif  // GUS_PLATFORM_FS_SETTINGS_FILE_STORE_HPP
