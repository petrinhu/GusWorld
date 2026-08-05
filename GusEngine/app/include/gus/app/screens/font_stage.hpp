// SPDX-License-Identifier: Apache-2.0
// gus/app/screens/font_stage.hpp
//
// ASSETS-FONTE-TELAS-GEMEO (2026-08-05): staging das 2 fontes de UI (.ttf) pro
// diretorio de stage do glintfx - UM lugar, em vez das SETE copias identicas que viviam
// espalhadas pelas telas.
//
// O QUE ERA (o defeito que esta peca fecha): title_menu_loop, system_menu_loop,
// save_load_menu_loop, difficulty_menu_loop, npc_dialogue_loop_gl e battle_cockpit_rml
// (2 sitios neste ultimo) repetiam, cada um por conta propria:
//
//     std::string fonts_dir = GUSWORLD_FONTS_DIR;                       // macro
//     if (const char* e = std::getenv("GUSWORLD_FONTS")) { ... }        // env
//     if (!fonts_dir.empty()) { fs::copy_file(..., ec); }               // ec IGNORADO
//
// Tres problemas, todos reais: (1) liam a macro DIRETO, por FORA do porteiro de assets
// (FilesystemAssetSource), sem checar se o caminho EXISTE - e a macro carrega o caminho
// absoluto da MAQUINA DE BUILD, entao em qualquer outra maquina (o caso do AppImage) ela
// aponta pro nada e a cascata NUNCA descia pro relativo; (2) o std::error_code do
// copy_file era descartado, entao a falha era 100% SILENCIOSA - a UI simplesmente aparecia
// sem fonte, com o RmlUi caindo no fallback dele; (3) sete copias da mesma cadeia
// divergiam sozinhas com o tempo. Este e o GEMEO do defeito que ASSETS-PATH-CASCATA
// (commit b8628e05) fechou nas seis familias do porteiro.
//
// O QUE E AGORA: a resolucao do caminho e do PORTEIRO
// (FilesystemAssetSource::resolve_path, familia FONTES), que ja faz a cascata completa e
// checada `GUSWORLD_FONTS > GUSWORLD_ASSETS+"/fonts" > macro > CWD` (precedencia
// documentada em platform/src/assets/asset_source.cpp::resolve_font). A env GUSWORLD_FONTS
// so entrou naquela cadeia por causa desta migracao: sem ela, quem ja usava a env perderia
// o override em silencio.
//
// CAMADA: app/ (nao platform/) - o STAGE e um conceito do consumidor glintfx, e o porteiro
// e so-leitura por decisao (ADR-013: "staging de fonte pro glintfx e save NAO passam por
// aqui"). Esta peca ESCREVE (copy_file), entao mora do lado de quem escreve.

#ifndef GUS_APP_SCREENS_FONT_STAGE_HPP
#define GUS_APP_SCREENS_FONT_STAGE_HPP

#include <filesystem>

namespace gus::app::screens {

// Copia as 2 fontes de UI (kFontMonoRegularFile + kFontMonoBoldFile, core/asset_paths.hpp)
// pro `stage_dir`, ACHATADAS (so o nome do arquivo) - e assim que o glintfx as encontra:
// glintfx::UiLayer::load_font_face resolve caminho relativo contra a BaseUrlFileInterface
// apontada pro stage (FONT-EXTEND-GLITCH, 2026-07-29).
//
// NAO cria `stage_dir` (quem chama ja o criou com create_directories pro .rml) e NAO
// lanca: fonte ausente DEGRADA (o RmlUi usa o fallback dele), nunca derruba a tela. Cada
// copia que falha e LOGADA via glintfx::log(Warn) com o caminho de origem, o de destino e
// a mensagem do sistema - o que o codigo antigo engolia.
//
// Devolve true SE E SO SE as DUAS fontes chegaram ao stage. As telas hoje ignoram o
// retorno de proposito (nao ha o que fazer com a falha alem de logar, e isso a funcao ja
// fez); ele existe pro teste poder afirmar o resultado sem abrir janela nenhuma - por isso
// NAO e [[nodiscard]]. As duas copias sao SEMPRE tentadas: um bold ausente nao pode
// esconder um regular ausente no log.
bool stage_ui_fonts(const std::filesystem::path& stage_dir);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_FONT_STAGE_HPP
