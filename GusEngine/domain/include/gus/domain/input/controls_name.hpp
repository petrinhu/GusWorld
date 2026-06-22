// gus/domain/input/controls_name.hpp
//
// Sanitize PURO do nome de perfil de jogador + formacao do nome de arquivo de
// controles (ADR-007, decisao do lider: multi-perfil, "[player]_controls.json").
// POCO puro, ZERO Qt, ZERO I/O: aqui so se forma o NOME LOGICO. O I/O real (path,
// escrever em ~/.gusworld/saves, permissoes 0700/0600) e PLATFORM, fora daqui.
//
// Regra de sanitize (decisao do lider):
//   - minusculas (ASCII);
//   - espaco e caracteres problematicos de filesystem ( / \ : * ? " < > | e
//     qualquer caractere de controle / nao-ASCII ) -> '_';
//   - "Jose Silva" -> "jose_silva" -> arquivo "jose_silva_controls.json".
//
// Fallback: um perfil multi-jogador precisa SEMPRE de um nome valido; nome vazio ou
// composto so de caracteres invalidos (ex.: "///") cai no perfil "default".
//
// Cross-ref: docs/tech/adr/ADR-007-controls-json-hash128-save-v4.md (fork 3 -> b).

#ifndef GUS_DOMAIN_INPUT_CONTROLS_NAME_HPP
#define GUS_DOMAIN_INPUT_CONTROLS_NAME_HPP

#include <string>
#include <string_view>

namespace gus::domain::input {

// Perfil de fallback quando o nome do jogador vira vazio/invalido apos sanitize.
inline constexpr std::string_view kDefaultProfile = "default";

// Sanitiza o nome do jogador para um identificador de perfil seguro em filesystem.
// Minusculas; espaco e caracteres problematicos -> '_'; vazio/so-invalido ->
// "default". Funcao pura, deterministica.
[[nodiscard]] std::string sanitize_profile_name(std::string_view raw_name);

// Nome de arquivo de controles do perfil: "<perfil saneado>_controls.json".
// Funcao pura.
[[nodiscard]] std::string controls_file_name(std::string_view raw_name);

}  // namespace gus::domain::input

#endif  // GUS_DOMAIN_INPUT_CONTROLS_NAME_HPP
