// gus/app/screens/battle_cockpit_rml.hpp
//
// AC-E11 A2 (ADR-019, decomposicao atomica de battle_preview.cpp): montagem do RML/RCSS
// do cockpit de batalha (conversao do mock scratchpad/cockpit_otimo/index.html, com os
// efeitos nativos do backend GL3 - gradientes, box-shadow/glow, @keyframes/animation,
// transform). Reune as 3 variantes que a casca consome (SMOKE trivial de diagnostico,
// BAKED com valores literais, LIVE data-model) mais o resolvedor do retrato ativo do
// cockpit (cockpit_retrato_flat_for) e os rotulos dos verbos (kVerbLabels). Extraido
// verbatim de battle_preview.cpp - ZERO mudanca de comportamento, so de arquivo.
//
// O RML AUTORADO (load_cockpit_rml) e o diretorio base dos assets do cockpit
// (cockpit_asset_base_dir) permanecem INTERNOS a battle_cockpit_rml.cpp (namespace
// anonimo, mesma encapsulacao que ja tinham em battle_preview.cpp) - so as 3 variantes
// finais (smoke/baked/live) + os 2 helpers cruzados por outros modulos sao expostos aqui.
//
// Depende de gus/app/screens/battle_assets.hpp (resolve_asset_dir/join, pra montar o
// stage de assets do cockpit) - por isso este corte (A2) so faz sentido DEPOIS do corte
// A3 na sequencia de extracao (ver nota de decisao autonoma em battle_assets.hpp).
//
// Cross-ref: gus/app/screens/battle_preview.hpp (casca dona do loop, unico chamador de
//            producao); gus/app/screens/battle_assets.hpp (resolucao de asset que este
//            modulo consome); docs/tech/adr/ADR-019.

#ifndef GUS_APP_SCREENS_BATTLE_COCKPIT_RML_HPP
#define GUS_APP_SCREENS_BATTLE_COCKPIT_RML_HPP

#include <array>
#include <string>
#include <string_view>

namespace gus::domain::combat {
class CombatActor;
}  // namespace gus::domain::combat

namespace gus::app::screens {

// ADR-010 F2b: rotulos dos verbos do menu (ordem = BattleVerb: Scan/Gambito/Atacar/
// Defender/Compilar/Flee), em MAIUSCULA, espelhando os <span class="lbl"> do RML. Usado
// pra alimentar o binding {{verb}} (verbo SELECIONADO no motor) a cada frame. Self-contido
// na casca (o spike nao depende do Translator pra isto).
inline constexpr std::array<std::string_view, 6> kVerbLabels = {
    "SCAN", "GAMBITO", "ATACAR", "DEFENDER", "COMPILAR", "FUGIR"};

// ADR-010 F2b RETRATO-VIVO: nome FLAT do retrato da MOLDURA do cockpit para o ator ATIVO.
// O retrato segue o ator (motor = autoridade): inimigo -> retrato_inimigo.png (a MESMA
// cabeca generica que a fila CTB / coluna usa pros inimigos, via retrato_file_for); party
// -> o retrato do membro. Excecao do Gus: usa a versao NO-BG (recorte sem fundo) que ja
// encaixa limpa na moldura (a versao da fila tem fundo). O stage achata os caminhos, entao
// devolvemos so o nome do arquivo (resolvido contra o base-url do stage). NUNCA inventa um
// caminho novo: reusa retrato_file_for (fonte unica dos retratos da fila, battle_assets.hpp).
[[nodiscard]] std::string cockpit_retrato_flat_for(
    const gus::domain::combat::CombatActor& actor);

// ADR-010 F1 SMOKE: escreve um RML TRIVIAL (sem assets externos, sem fonte) num tempfile e
// devolve o path. O glintfx v0.2.1 carrega por PATH (load()) e NAO resolve base-url nem
// carrega fonte default (so a v0.2.2) - por isso o smoke e um DIV com gradiente
// semitransparente + glow (decorator/box-shadow nativos), SEM texto. Prova so o compose do
// embed mode por cima da arena. Unidades em 'px' (deterministico; dp-ratio fica pra v0.2.2).
[[nodiscard]] std::string write_smoke_glintfx_rml();

// ADR-010 F2a: diretorio de STAGE dos assets do cockpit BAKED/LIVE (tempfile). Por que
// stage: o glintfx carrega o doc por PATH e o RmlUi canonicaliza os caminhos RELATIVOS
// contra o DIR do documento ANTES de chamar o FileInterface (entao um base-url sozinho
// nao pega caminho ja-canonicalizado, e caminho ABSOLUTO arrisca perder a barra inicial).
// A receita robusta (= ao teste base_url_sanity do glintfx): juntar doc + assets num dir
// unico, com referencias RELATIVAS achatadas. As fontes (GusEngine/assets/) e os sprites
// (resources/sprites/icons-m5/) vivem em ARVORES diferentes; o stage os reune.
[[nodiscard]] std::string glintfx_cockpit_stage_dir();

// ADR-010 F2a: produz a variante BAKED (valores ESTATICOS) do cockpit REAL pelo
// glintfx::UiLayer. REUSA o RML/RCSS autorado (gradientes/glow/molduras/keyframes
// intactos) e so TRANSFORMA por string: (1) injeta @font-face (o UiLayer nao expoe
// Rml::LoadFontFace - so o doc carrega fonte); (2) remove o data-model; (3) troca
// {{bindings}} por literais de um encontro-exemplo (Gus 55/55, papel); (4) achata o
// caminho do retrato; (5) escolhe o estado (combate por padrao, ou intro/brasao). Copia
// os 4 assets (2 fontes + moldura + retrato) pro stage dir e escreve o .rml la. Devolve o
// path do .rml.
[[nodiscard]] std::string write_baked_cockpit_rml(bool intro);

// ADR-010 F2b: variante LIVE (data-model) do cockpit REAL pelo glintfx::UiLayer. Diferente
// do BAKED (F2a, valores literais): aqui o RML MANTEM o data-model="hud" + os {{bindings}}
// + os data-if(intro/!intro), e a casca alimenta os valores VIVOS por frame (set_*). REUSA
// o RML/RCSS autorado (gradientes/glow/moldura/keyframes intactos) e so TRANSFORMA por
// string: (1) injeta @font-face; (2) achata o caminho do retrato; (3) acrescenta
// tab-index/nav ao .verb (foco navegavel) + data-class-sel por verbo (a SELECAO do motor
// vira a classe .sel, motor = fonte de verdade); (4) troca o log hardcoded por
// data-for("line : log") + uma now-line com {{verb}}/{{alvo}} (verbo selecionado + alvo,
// vivos). Copia os 4 assets (2 fontes + moldura + retrato) pro stage e escreve o .rml la.
// Devolve o path.
[[nodiscard]] std::string write_live_cockpit_rml();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_COCKPIT_RML_HPP
