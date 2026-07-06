// gus/app/screens/npc_dialogue_rml.hpp
//
// RML/RCSS da caixa de dialogo "QUENTE" (M7-DIALOGO, DIALOGO-TERMINAL): moldura de
// latao + retrato + nome + fala + escolhas, traduzida do mock JA APROVADO
// (docs/design/mockups/05-dialogo-bertoldo-retrato-real.html, MESMA .warm-box/
// .warm-corner/.warm-portrait/.warm-name/.warm-line de 03-.../04-... - o registro
// "quente" e IDENTICO nas 2 propostas de terminal, so o Bertoldo usa warm nesta
// onda). Substitui o overlay funcional simples de texto (render_dialogue_overlay_
// frame/npc_dialogue_overlay_lines) como APRESENTACAO - a LOGICA de selecao/input
// (apply_npc_dialogue_input, npc_dialogue_move_selection) e o rotulo do falante
// (npc_dialogue_actor_display_name) continuam em npc_dialogue_overlay.hpp,
// REUSADOS aqui sem duplicar.
//
// MESMO ESTILO DE AUTORIA de gus/app/screens/system_menu_rml.hpp: RML/RCSS como
// string C++, decorators nativos do backend GL3 (linear-gradient/radial-gradient/
// polygon/box-shadow) - build_npc_dialogue_rml e uma funcao PURA (sem I/O, sem
// GL): devolve uma string a partir do DialogueNode + Translator + selecao +
// nome-flat-do-retrato-ja-resolvido. O CHAMADOR (npc_dialogue_loop_gl.cpp) e quem
// resolve o retrato (npc_dialogue_portrait_file abaixo), copia o arquivo pro stage
// dir (MESMA receita de write_system_menu_rml_file/write_baked_cockpit_rml) e
// escreve o .rml em disco pro glintfx::UiLayer::load() consumir.
//
// DECISAO DE APRESENTACAO NAO COBERTA PELO MOCK (a `.choices` do mock estava
// display:none - nenhuma escolha visual foi desenhada la): a lista de opcoes
// (no de ESCOLHA, node.options.size()>=2) e renderizada DENTRO da propria
// .warm-box (nao um bloco flutuante separado, que sairia da tela no
// enquadramento 960x540) - cada opcao e uma linha `.warm-choice`, a
// SELECIONADA ganha um marcador ("> ") + cor mais clara (MESMA convencao "> "/
// "  " ja usada por npc_dialogue_overlay_lines, so a apresentacao troca de
// texto puro pra RCSS). Simples e coerente com o resto do documento (nenhuma
// API nova); ajustavel ao vivo se o lider preferir outra disposicao.
//
// BOTAO "Continuar" (pedido do lider, pos-DIALOGO-TERMINAL): o no LINEAR (sem
// opcoes) mostrava so um textinho pequeno (`.warm-continue-hint`, so avancava
// por Enter/Espaco) - virou um BOTAO DE VERDADE (`.warm-continue-btn`,
// id="npcdlg-continue-btn"), MESMA receita visual/estrutural de .btn-back
// (system_menu_rml.cpp): box-sizing:border-box, `:hover` nativo do glintfx
// DECLARADO ANTES de `.pressed` (mesma ordem/motivo documentado la - o empate
// de especificidade cai a favor do estado real), `.pressed` = flash da moldura
// de latao (borda+fundo dourados) tocado pelo CHAMADOR (npc_dialogue_loop_gl.cpp,
// MESMO padrao de flash_pressed() do menu - efeito NOSSO, nao nativo do
// glintfx). DECISAO DE POSICAO NAO COBERTA PELO MOCK (so o texto existia):
// mantido na MESMA posicao/lugar do antigo hint (fim do #npcdlg-body, so
// virando `<div>` clicavel) - simples e coerente, ajustavel ao vivo se o lider
// preferir outro lugar. `continue_pressed` so importa em no LINEAR (mesma
// condicao de `options.empty()` que decide mostrar o botao).
//
// Cross-ref: gus/app/screens/npc_dialogue_overlay.hpp (logica pura reusada);
//            gus/app/screens/npc_dialogue_loop_gl.hpp (o loop GL que gera o
//            stage dir, copia o retrato e chama esta funcao a cada mudanca de no);
//            gus/app/screens/system_menu_rml.hpp (MESMO estilo de autoria);
//            gus/core/asset_paths.hpp (kRetratosDir - pasta-fonte dos retratos).

#ifndef GUS_APP_SCREENS_NPC_DIALOGUE_RML_HPP
#define GUS_APP_SCREENS_NPC_DIALOGUE_RML_HPP

#include <string>

#include "gus/app/i18n/translator.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"

namespace gus::app::screens {

// Resolve o ARQUIVO de retrato (nome flat, dentro de kRetratosDir) para um
// `speaker_id` de dialogo. Convencao DEFAULT: "retrato_<speaker_id>.png" (MESMA
// convencao de nomenclatura de ACTOR_<ID> em pt_br.md). GENERICO POR DESIGN
// (pedido do lider): funciona pra QUALQUER speaker_id, nao so o Bertoldo -
// speaker_id="gus" resolve "retrato_gus.png" (arquivo ja existente), sem
// nenhuma excecao cadastrada.
//
// EXCECOES CADASTRADAS (o arquivo REAL em resources/sprites/icons-m5/retratos/
// NAO bate 1:1 com o speaker_id usado no .dlg.txt - conferido em disco antes de
// escrever isto): "bertoldo" (speaker_id do npc_intro_bertoldo.dlg.txt) ->
// "retrato_seu_bertoldo_caim.png" ("retrato_bertoldo.png" NAO existe). Se um
// speaker_id sem excecao cadastrada nao tiver o arquivo default no disco, o
// CHAMADOR degrada com seguranca (mesmo racional de load_enemy_marker_texture/
// load_npc_bertoldo_marker_texture: TextureId invalido = nao desenha, sem
// crashar) - esta funcao so RESOLVE O NOME, nunca toca o disco.
[[nodiscard]] std::string npc_dialogue_portrait_file(const std::string& speaker_id);

// Gera o RML/RCSS completo da caixa quente pro no ATUAL. `portrait_file` e o NOME
// FLAT (ja copiado pro stage dir pelo CHAMADOR, ver header acima) devolvido por
// npc_dialogue_portrait_file - esta funcao so o REFERENCIA num `decorator: image(
// <portrait_file> cover )`, MESMA tecnica ja usada pro retrato do cockpit
// (battle_preview.cpp, #pic). `selected_option` so importa em no de ESCOLHA
// (node.options nao vazio); em no LINEAR mostra tr("DIALOGUE_CONTINUE") num
// BOTAO de verdade (id="npcdlg-continue-btn", classe "warm-continue-btn" - ver
// header). `continue_pressed` (default false) marca o botao com ".pressed"
// (flash de confirmacao, MESMO padrao de pressed_index em
// build_system_menu_rml/system_menu_rml.hpp) - so tem efeito visual em no
// LINEAR (nao ha botao pra marcar em no de ESCOLHA).
[[nodiscard]] std::string build_npc_dialogue_rml(
    const gus::domain::dialogue::DialogueNode& node,
    const gus::app::i18n::Translator& translator, int selected_option,
    const std::string& portrait_file, bool continue_pressed = false);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_NPC_DIALOGUE_RML_HPP
