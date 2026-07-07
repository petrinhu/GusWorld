// gus/platform/fs/controls_file_store.hpp
//
// I/O REAL de controles (tela Controles, M2; ADR-007): "<perfil>_controls.json"
// dentro do MESMO diretorio de settings (gus::platform::fs::resolve_settings_dir(),
// reusado - mesmo ~/.gusworld). Formato JSON proprio dep-free (gus/domain/input/
// controls_json.hpp): PRETTY no disco (legivel/editavel a mao, intento do lider),
// parse ROBUSTO (JSON malformado/ausente NUNCA lanca, degrada pro
// default_controls() - mesma politica de settings_file_store.hpp).
//
// ESCOPO desta onda ("a ponte pro jogador" da tela Controles): so o load/save do
// arquivo achatado. A camada completa do ADR-007 (hash128/deteccao de
// adulteracao/backup-no-save/janela de aviso do diff) permanece PLATFORM/APP
// PENDENTE (ver ADR-007 "Pendencias PLATFORM/APP" + TODO.md item M2) - fora do
// escopo desta dispatch, que so precisa persistir/reler o remap do jogador.
// load_controls degrada com seguranca (arquivo ausente = 1a vez/perfil novo,
// JSON corrompido = alguem mexeu a mao) pro gus::domain::input::default_controls()
// - NUNCA lanca, NUNCA deixa o jogador sem controles validos.
//
// Cross-ref: gus/domain/input/controls_json.hpp (serialize/parse),
//            gus/domain/input/controls_restore.hpp (default_controls),
//            gus/domain/input/controls_name.hpp (sanitize + nome do arquivo),
//            gus/platform/fs/settings_file_store.hpp (resolve_settings_dir,
//            MESMO padrao de permissao 0700/0600).

#ifndef GUS_PLATFORM_FS_CONTROLS_FILE_STORE_HPP
#define GUS_PLATFORM_FS_CONTROLS_FILE_STORE_HPP

#include <string>

#include "gus/domain/input/input_binding.hpp"

namespace gus::platform::fs {

// Caminho do arquivo de controles de `profile` DENTRO de `dir` (dir/
// controls_file_name(profile), ver gus/domain/input/controls_name.hpp). Funcao
// pura (so concatena path) - usada tanto pela producao quanto pelos testes (que
// passam um dir temporario explicito em vez de resolve_settings_dir()).
[[nodiscard]] std::string controls_file_path(const std::string& dir,
                                              const std::string& profile);

// Carrega o config de controles de `profile` em `dir`. Degradacao segura em
// QUALQUER falha (arquivo ausente - 1a execucao/perfil novo; JSON corrompido;
// erro de leitura): devolve gus::domain::input::default_controls(), NUNCA lanca.
[[nodiscard]] gus::domain::input::InputRemapConfig load_controls(
    const std::string& dir, const std::string& profile);

// Grava `config` (forma PRETTY, legivel/editavel) em controls_file_path(dir,
// profile), criando o diretorio (permissao 0700) se necessario. Arquivo gravado
// com permissao 0600 (LGPD, mesma politica de settings_file_store). Devolve true
// em sucesso; false em qualquer falha de I/O (nunca lanca) - a config em
// MEMORIA continua valendo pro resto da sessao mesmo se a gravacao falhar.
[[nodiscard]] bool save_controls(const gus::domain::input::InputRemapConfig& config,
                                  const std::string& dir, const std::string& profile);

}  // namespace gus::platform::fs

#endif  // GUS_PLATFORM_FS_CONTROLS_FILE_STORE_HPP
