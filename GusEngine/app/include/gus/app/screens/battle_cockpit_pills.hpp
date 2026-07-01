// gus/app/screens/battle_cockpit_pills.hpp
//
// GEOMETRIA PURA dos 6 PILLS de verbo do COCKPIT LIVE (RmlUi/glintfx), POCO 100%
// testavel SEM SDL/janela/glintfx. Fonte UNICA da posicao dos pills (SCAN/GAMBITO/
// ATACAR/DEFENDER/COMPILAR/FUGIR) em ESPACO 'dp' (o canvas logico 960x540 do RCSS do
// cockpit_otimo) para o HIT-TEST DE MOUSE (Incremento A2). O glintfx NAO expoe API de
// hit-test/geometria de elemento (so data-model + process_event), entao o host testa o
// clique VOCE-MESMO em coordenadas de tela -> converte pra dp -> consulta esta geometria.
//
// POR QUE UM MODULO SEPARADO (e nao battle_layout.hpp): battle_layout.hpp modela o
// cockpit DESENHADO-A-MAO (variante C, coluna 174px logico, headless/testes). ESTES
// pills sao do cockpit RCSS "cockpit_otimo" (coluna 252dp), desenhado pelo glintfx SO no
// app vivo (hud_external). Sao DUAS geometrias distintas no mesmo espaco 960x540; manter
// esta separada evita confundir a coluna a-mao com a coluna RCSS.
//
// ================= SINCRONIA COM A RCSS (fonte cruzada, LER ANTES DE MEXER) =================
// Os numeros abaixo REPLICAM valores hoje HARDCODED na string RCSS de load_cockpit_rml()
// (app/src/screens/battle_preview.cpp), regra `.verb` + `#cockpit`. NAO da pra unificar
// 100% (a origem-Y do 1o pill EMERGE do fluxo do RmlUi: retrato+nome+papel+vitals+fontes,
// nao ha numero literal). Se editar a RCSS do menu, ATUALIZE AQUI (e vice-versa):
//   RCSS `.verb { width: 110dp; height: 18dp; margin-bottom: 4dp; padding: 0dp 12dp;
//                 border: 1dp; }`  -> kCockpitPillContentWidthDp / ...ContentHeightDp /
//                                     ...MarginBottomDp / ...PaddingHDp / ...BorderDp
//   RCSS `#cockpit { padding: 10dp 12dp 0dp 12dp; }` (padding-left) -> kCockpitPillLeftDp
//   RCSS `.menu { margin-top: 18dp; }`               -> kCockpitMenuMarginTopDp (informativo)
// A ORIGEM-Y (kCockpitFirstPillTopDp) foi MEDIDA numa captura do cockpit vivo em combate
// (subpixel ~279.6dp; ver relatorio do Incremento A2). E CONSTANTE: retrato/nome/papel/
// vitals tem altura fixa (nao dependem de HP/nome). Ao mudar a RCSS ACIMA do menu, RE-MEDIR.
// ==========================================================================================
//
// Cross-ref: app/src/screens/battle_preview.cpp (load_cockpit_rml, RCSS + o hit-test SDL);
//            gus/app/screens/battle_menu.hpp (MODELO do menu; BattleVerb / ordem dos verbos);
//            gus/core/spatial/camera_clamp.hpp (Rect / Vec2).

#ifndef GUS_APP_SCREENS_BATTLE_COCKPIT_PILLS_HPP
#define GUS_APP_SCREENS_BATTLE_COCKPIT_PILLS_HPP

#include "gus/app/screens/battle_menu.hpp"     // kBattleVerbCount / BattleVerb (ordem)
#include "gus/core/spatial/camera_clamp.hpp"   // gus::core::spatial::Rect / Vec2

namespace gus::app::screens {

using gus::core::spatial::Rect;

// ----------------------------------------------------------------------------
// Constantes de AUTORIA da RCSS (espelho dos numeros da regra `.verb`/`#cockpit`).
// Tudo em 'dp' (o dp do RCSS == px logico do canvas 960x540; o dp_ratio do glintfx
// escala pro backbuffer real). Ponto unico de sincronia com load_cockpit_rml().
// ----------------------------------------------------------------------------

// `.verb { width: 110dp; }` -- LARGURA DE CONTEUDO (content-box; o RmlUi soma padding+border).
inline constexpr float kCockpitPillContentWidthDp = 110.0f;
// `.verb { height: 18dp; }` -- ALTURA DE CONTEUDO.
inline constexpr float kCockpitPillContentHeightDp = 18.0f;
// `.verb { margin-bottom: 4dp; }` -- o "gap" vertical entre pills.
inline constexpr float kCockpitPillMarginBottomDp = 4.0f;
// `.verb { padding: 0dp 12dp; }` -- padding HORIZONTAL (cada lado).
inline constexpr float kCockpitPillPaddingHDp = 12.0f;
// `.verb { border: 1dp; }` -- espessura da borda (NAO-selecionado; o .sel usa 2dp e fica
// 2dp mais alto/largo, empurrando os pills ABAIXO dele em +2dp -- absorvido pelo hit-test).
inline constexpr float kCockpitPillBorderDp = 1.0f;
// `#cockpit { padding-left: 12dp; }` -- a coluna do menu ancora aqui (margin-box left do pill).
inline constexpr float kCockpitPillLeftDp = 12.0f;
// `.menu { margin-top: 18dp; }` -- informativo (ja embutido em kCockpitFirstPillTopDp).
inline constexpr float kCockpitMenuMarginTopDp = 18.0f;

// TOPO (margin-box) do 1o pill (SCAN), em dp. EMPIRICO: medido na captura do cockpit vivo
// (~279.6dp; arredondado). Emerge do fluxo RmlUi acima do menu (retrato 104x134 + nome +
// papel + vitals: HP + AP + Mana), cujas alturas dependem das metricas da fonte -- por isso
// NAO ha numero literal na RCSS pra copiar. Constante (nao varia com HP/nome/estado).
inline constexpr float kCockpitFirstPillTopDp = 280.0f;

// Quantidade de pills == quantidade de verbos (ordem = BattleVerb: Scan..Flee).
inline constexpr int kCockpitPillCount = kBattleVerbCount;

// --- Derivados (box-model content-box do RmlUi/CSS) ---

// Largura BORDER-BOX (o retangulo VISIVEL): conteudo + 2*padding-h + 2*borda = 110+24+2 = 136.
inline constexpr float kCockpitPillBorderBoxWidthDp =
    kCockpitPillContentWidthDp + 2.0f * kCockpitPillPaddingHDp + 2.0f * kCockpitPillBorderDp;
// Altura BORDER-BOX (visivel): conteudo + 2*borda = 18+2 = 20.
inline constexpr float kCockpitPillBorderBoxHeightDp =
    kCockpitPillContentHeightDp + 2.0f * kCockpitPillBorderDp;
// PASSO vertical topo->topo: altura border-box + margin-bottom = 20+4 = 24.
inline constexpr float kCockpitPillPitchDp =
    kCockpitPillBorderBoxHeightDp + kCockpitPillMarginBottomDp;

// ----------------------------------------------------------------------------
// Funcoes puras. Determinismo total: mesmo indice/ponto -> mesma saida.
// ----------------------------------------------------------------------------

// Retangulo VISIVEL (border-box) do pill do verbo `verb_index` (0..kCockpitPillCount-1), em
// dp. left = kCockpitPillLeftDp; top = 1o topo + index*passo; w/h = border-box. Usa a
// posicao NOMINAL (borda 1dp em todos): o +2dp do pill selecionado NAO entra aqui (e um
// detalhe de realce, absorvido pelo hit-test generoso). index fora de faixa -> Rect vazio.
[[nodiscard]] Rect cockpit_pill_rect(int verb_index) noexcept;

// HIT-TEST: indice do pill (0..kCockpitPillCount-1) cujo BAND vertical contem o ponto 'dp'
// (dp_x, dp_y), ou -1 se fora. Os bands LADRILHAM o passo (24dp cada) SEM zonas mortas: cada
// pill "possui" seu pill visivel + o gap de 4dp abaixo dele. Isso (a) da clique tolerante e
// (b) absorve o desvio de +-2dp do realce (.sel 2dp empurra os pills abaixo). Em X exige o
// ponto dentro da largura border-box do pill [left, left+borderBoxW]. Fora da coluna/da pilha
// -> -1 (o clique nao faz nada; nao "erra" pro pill errado). PURA: nao sabe de motor/estado.
[[nodiscard]] int cockpit_pill_index_at(float dp_x, float dp_y) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_COCKPIT_PILLS_HPP
