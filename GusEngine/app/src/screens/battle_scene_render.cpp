// gus/app/src/screens/battle_scene_render.cpp
//
// AC-E11 A4 (TU-split, ADR-019): DEFINICOES de BattleScene::render() + draw_bar/draw_pips,
// junto da paleta canonica, das constantes de flavor/estilo e dos helpers de desenho que so
// o render usa (verb_color, log_line_color, intent_icon_tex, pick_defeat_reboot_line) -
// extraidos de battle_scene.cpp (que ficou grande demais). PURO TU-split: o header
// (battle_scene.hpp) NAO muda, nenhuma assinatura muda, battle_scene_test.cpp continua
// intocado. Ver battle_scene.cpp pro resto da classe (ctor, motor de turno, modo-mira,
// picker de ator, leituras publicas).
//
// kDefeatFlavorSeconds e a UNICA constante duplicada entre os dois arquivos (o valor
// tambem e lido por update()/defeat_flavor_active() em battle_scene.cpp, fora do
// render): e um float literal isolado (5.0f, Fibonacci), sem estado - duplicar 1 linha
// evita expor um symbol com external linkage so pra isso. Se o valor mudar, mudar nos 2
// lugares (comentado nos dois pontos).

#include "gus/app/screens/battle_scene.hpp"

#include <algorithm>  // std::remove_if (poda floaters mortos)
#include <cstdio>     // std::snprintf (numeros do painel)
#include <optional>
#include <string>
#include <utility>    // std::move
#include <vector>

#include "gus/app/screens/battle_floaters.hpp"
#include "gus/app/screens/battle_hud_model.hpp"
#include "gus/app/screens/battle_layout.hpp"
#include "gus/app/screens/sprite_anchor.hpp"  // pe real do sprite (W3: bbox do idle)
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"  // CombatState (preview_intent)
#include "gus/domain/combat/weakness_wheel.hpp"  // WeaknessWheel (pre-selecao D3 de mira)
#include "gus/platform/render2d/text_metrics.hpp"  // text_width (centrar floater)

namespace gus::app::screens {

namespace {

using gus::core::spatial::Rect;
using gus::domain::combat::CardFamily;
using gus::domain::combat::CombatActor;
using gus::domain::combat::StatusId;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::IRenderer;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

// Tamanho de texto (px logico) das zonas. Pixel Operator e crisp a multiplos de 8/16. Os
// textos de corpo subiram 8px->16px com a resolucao 960x540 (lider 2026-06-25), mantendo
// a legibilidade proporcional ao canvas 1.5x maior (e crisp no x2 ate 1080p).
constexpr float kVerbTextPx = 12.0f;    // rotulo do verbo na caixa do menu (cockpit fino)
constexpr float kLogTextPx = 11.0f;     // linha do log (terminal fino)
constexpr float kPanelTextPx = 13.0f;   // numeros gerais
// Texto do COCKPIT (variante C): nome do ator, numeros, rotulos.
constexpr float kCockpitNamePx = 15.0f;   // nome grande do ator ativo
constexpr float kCockpitTextPx = 8.0f;    // numero "hp/max" sobre a barra (compacto)
constexpr float kCockpitLabelPx = 9.0f;   // rotulos "AP"/"MANA"/"ACAO"
constexpr float kCockpitResLabelW = 34.0f;  // largura reservada pro rotulo antes dos pips

// --- PALETA CANONICA "Tatico Cockpit" (variante C, HEX exatos do _common.css aprovado
//     pelo criador 2026-06-25). Cores em [0,1]. ---
constexpr DrawColor kBgColor{0.047f, 0.059f, 0.102f, 1.0f};        // #0c0f1a fundo
constexpr DrawColor kBg1Color{0.106f, 0.133f, 0.220f, 1.0f};       // #1B2238 cockpit topo
constexpr DrawColor kBg2Color{0.078f, 0.102f, 0.173f, 1.0f};       // #141a2c cockpit base
constexpr DrawColor kTerminalColor{0.055f, 0.075f, 0.133f, 1.0f};  // #0e1322 terminal/log
constexpr DrawColor kSlotDarkColor{0.039f, 0.051f, 0.086f, 1.0f};  // #0a0d16 slot escuro
constexpr DrawColor kCyan{0.133f, 0.827f, 0.933f, 1.0f};           // #22D3EE party/ativo/CRIT
constexpr DrawColor kCyanDim{0.082f, 0.369f, 0.420f, 1.0f};        // #155e6b
constexpr DrawColor kMagenta{0.882f, 0.114f, 0.455f, 1.0f};        // #E11D74 inimigo/intent
constexpr DrawColor kMagentaDim{0.361f, 0.071f, 0.188f, 1.0f};     // #5c1230
constexpr DrawColor kBrass{0.910f, 0.639f, 0.239f, 1.0f};          // #E8A33D Compilar/AP/fraqueza
constexpr DrawColor kHp{0.247f, 0.725f, 0.478f, 1.0f};             // #3FB97A HP verde
constexpr DrawColor kHpDim{0.114f, 0.361f, 0.235f, 1.0f};          // #1d5c3c
constexpr DrawColor kErr{0.957f, 0.247f, 0.369f, 1.0f};            // #F43F5E erro/FALHA
constexpr DrawColor kInk{0.812f, 0.902f, 0.933f, 1.0f};            // #cfe6ee tinta
constexpr DrawColor kInkDim{0.435f, 0.522f, 0.576f, 1.0f};         // #6f8593 tinta-dim
constexpr DrawColor kLine{0.165f, 0.204f, 0.314f, 1.0f};           // #2a3450 borda
constexpr DrawColor kWhite{1.0f, 1.0f, 1.0f, 1.0f};                // tint neutro

// Mapeamentos semanticos sobre a paleta canonica:
// (kCtbBandColor removido: a faixa/caixa de fundo da fila CTB foi aposentada - a fila
//  agora fica seamless sobre a vinheta da arena. Ver o bloco de render da fila CTB.)
constexpr DrawColor kCtbCellColor = kSlotDarkColor;   // celula CTB ocupada
constexpr DrawColor kCtbNextColor = kCyan;            // marca "proximo" (cyan)
constexpr DrawColor kPartyColor = kCyanDim;           // placeholder party (cyan dim)
constexpr DrawColor kEnemyColor = kMagentaDim;        // placeholder inimigo (magenta dim)
constexpr DrawColor kActiveHiColor = kCyan;           // highlight ator ativo (cyan)
constexpr DrawColor kPanelColor = kBg1Color;          // fundo cockpit (gradiente -> usa bg1)
constexpr DrawColor kLogColor = kTerminalColor;       // caixa de log (terminal)
constexpr DrawColor kHudBorderColor = kLine;          // contorno HUD (borda)
constexpr DrawColor kBarBackColor = kSlotDarkColor;   // fundo de barra
constexpr DrawColor kHpFillColor = kHp;               // HP cheio (verde)
constexpr DrawColor kApLitColor = kBrass;             // pip de AP aceso (latao)
constexpr DrawColor kApOffColor = kSlotDarkColor;     // pip de AP apagado
constexpr DrawColor kManaLitColor = kCyan;            // pip de Mana aceso (cyan)
constexpr DrawColor kManaOffColor = kSlotDarkColor;   // pip de Mana apagado
constexpr DrawColor kStatusBoxColor = kMagentaDim;    // placeholder de status
constexpr DrawColor kActiveCtbColor = kCyan;          // marca de ativo na fila (cyan)
constexpr DrawColor kVerbColor = kInk;                // texto de verbo neutro
constexpr DrawColor kVerbBoxColor{0.063f, 0.090f, 0.165f, 1.0f};   // #10172a fundo do verbo
constexpr DrawColor kVerbDisabledColor = kSlotDarkColor;           // verbo sem AP
constexpr DrawColor kVerbSelectColor = kCyan;         // verbo selecionado (cyan)
constexpr DrawColor kVerbPrimaryColor = kBrass;       // COMPILAR (latao)
// --- log: cor por categoria de linha (D7), na paleta canonica ---
constexpr DrawColor kLogSystemColor = kCyan;          // sistema (cyan, bold)
constexpr DrawColor kLogDamageColor = kInk;           // dano (tinta clara)
constexpr DrawColor kLogHealColor = kHp;              // cura (verde)
constexpr DrawColor kLogStatusColor = kMagenta;       // status (magenta)
constexpr DrawColor kLogDefeatColor = kErr;           // derrota (erro)

// Cor de DESTAQUE de um verbo (selecionado=cyan, Compilar=latao, demais=neutro).
DrawColor verb_color(BattleVerb v, bool selected) noexcept {
    if (selected) {
        return kVerbSelectColor;
    }
    if (v == BattleVerb::Compilar) {
        return kVerbPrimaryColor;
    }
    return kVerbColor;
}

// Intent (telegraph, incremento 5): tamanho do icone + cor da marca placeholder. Subiu
// com a resolucao 960x540 (lider 2026-06-25), proporcional ao slot de ator maior.
constexpr float kIntentIconSize = 18.0f;
constexpr DrawColor kIntentMarkColor{0.95f, 0.75f, 0.20f, 1.0f};  // placeholder ambar
// Numero flutuante de dano: tamanho do texto (px logico). 24px (1.5x os 16px do 640x360):
// Pixel Operator densa/nitida e GRANDE o bastante pra o criador VER o dano no canvas
// maior. Sobe sobre o alvo e some por fade.
constexpr float kFloaterTextPx = 24.0f;

// --- MODO-MIRA (§3.5, D3): destaque MULTIMODAL do inimigo mirado (Pillar 4/WCAG) ---
// A mira e CYAN na paleta canonica ("party/ativo/mira/CRIT"). Mas cor NUNCA basta
// (daltonismo): o alvo mirado ganha (1) CONTORNO reticulo (outline duplo, distinto do
// outline simples do ativo), (2) SETA em forma pura (caret ►, a esquerda do slot), e
// (3) NOME + HP em texto legivel. As 3 pistas sobrevivem em escala de cinza.
constexpr DrawColor kMiraColor = kCyan;         // cor da mira (cyan canonico)
constexpr float kMiraLabelPx = 12.0f;           // texto do nome/HP do alvo mirado
constexpr float kMiraCaretW = 3.0f;             // largura de cada coluna do caret ►
constexpr float kMiraFracoPipSize = 6.0f;       // pip do tier "fraco" (latao) se escaneado

// Banner de turno (D9/D10): tamanho + cor. Texto grande e centrado abaixo da fila CTB.
// 24px (1.5x os 16px do 640x360) pra dominar a tela maior na abertura.
constexpr float kBannerTextPx = 24.0f;
constexpr DrawColor kBannerBgColor{0.05f, 0.05f, 0.09f, 0.85f};   // faixa semi-opaca
constexpr DrawColor kBannerPlayerColor{0.55f, 0.95f, 0.65f, 1.0f};  // sua vez (verde)
constexpr DrawColor kBannerEnemyColor{0.95f, 0.55f, 0.45f, 1.0f};   // vez do inimigo
constexpr DrawColor kBannerIntroColor{0.98f, 0.92f, 0.45f, 1.0f};   // BATALHA! (ambar)

// --- FLAVOR DE DERROTA (M7-COSTURA Inc 3, docs/design/mecanicas/combat-flavor.md §3b
//     "Derrota"): o Gus reboota, nao morre (echo Batman, Pillar 1 "magia = software"). ---
// Duracao do overlay: 5.0s (Fibonacci - canon do easter egg pervasivo do projeto,
// project_fibonacci_easter_egg - nunca anunciado, so um numero da sequencia onde um
// timing generico serviria igual). SO TIMER (sem input): anti-OE, nao e cutscene.
// kDefeatFlavorSeconds: DUPLICADA de battle_scene.cpp (ver nota no topo do arquivo) -
// mesmo valor, mesma razao (Fibonacci - project_fibonacci_easter_egg), so a leitura
// aqui (o render pinta o overlay ate essa idade) muda.
constexpr float kDefeatFlavorSeconds = 5.0f;
constexpr DrawColor kDefeatVeilColor{0.008f, 0.008f, 0.012f, 0.90f};  // veu quase-opaco
constexpr float kDefeatRebootPx = 20.0f;  // "kernel panic" (literal, en fake-terminal)
constexpr float kDefeatBarkPx = 14.0f;    // falinha blase do companion
constexpr float kDefeatNotePx = 12.0f;    // nota-xadrez (explica o Gus-centric)

// Pool do "reboot de sistema": strings TECNICAS LITERAIS, NAO traduziveis (mesma
// convencao §4 do doc: o codigo de erro e autentico em qualquer locale - so o subtitulo
// pro leigo, aqui a nota-xadrez, passa por tr()). Subconjunto de §3b "Derrota" que LE
// como reboot/kill sistemico (nao erro de sintaxe de UMA carta, que e o acervo §3).
constexpr std::array<std::string_view, 4> kDefeatRebootLines{
    "Kernel panic - not syncing: Attempted to kill init",
    "Killed (signal 9: SIGKILL)",
    "No more processes left to schedule. System halted.",
    "Process finished with exit code 137",
};
// Alterna deterministicamente por um "seed" estavel do encontro (tamanho do log de
// combate ao fim da luta) - NAO consome IRandomSource do dominio (a UI nao sorteia nada
// do motor); so da variedade sem RNG novo nem estado extra pra guardar/testar.
std::string_view pick_defeat_reboot_line(std::size_t seed) noexcept {
    return kDefeatRebootLines[seed % kDefeatRebootLines.size()];
}

// Cor de uma linha do log pela categoria (D7).
DrawColor log_line_color(LogLineKind k) noexcept {
    switch (k) {
        case LogLineKind::System: return kLogSystemColor;
        case LogLineKind::Damage: return kLogDamageColor;
        case LogLineKind::Heal:   return kLogHealColor;
        case LogLineKind::Status: return kLogStatusColor;
        case LogLineKind::Defeat: return kLogDefeatColor;
    }
    return kLogSystemColor;
}

// Textura do icone de intent pra um IntentPreview (incremento 5). is_chaotic => ruido
// (Patch-Zero); senao pelo predicted_action_id (attack/defend/status). kInvalidTexture
// se o icone correspondente nao foi setado (o render cai na marca placeholder).
TextureId intent_icon_tex(const BattleIntentIconSet& icons,
                          const gus::domain::combat::IntentPreview& intent) noexcept {
    if (intent.is_chaotic) {
        return icons.ruido;
    }
    const std::string& a = intent.predicted_action_id;
    if (a.find("defend") != std::string::npos) {
        return icons.defender;
    }
    if (a.find("status") != std::string::npos || a.find("card") != std::string::npos) {
        return icons.aplicar_status;
    }
    // "attack"/"pass"/default: o telegraph mais comum e o ataque.
    return icons.atacar;
}

}  // namespace

namespace {

// Desenha uma barra (fundo + preenchimento por fracao 0..1, da esquerda). Pura no
// modelo (bar_fill_rect); aqui so emite os dois quads.
void draw_bar(IRenderer& r, const Rect& frame, float fraction,
              const DrawColor& fill) {
    r.draw_filled_rect(frame, kBarBackColor);
    r.draw_filled_rect(bar_fill_rect(frame, fraction), fill);
    r.draw_rect_outline(frame, kHudBorderColor, 1.0f);
}

// Desenha uma linha de pips de recurso (AP/Mana): total pips, lit acesos. Cores
// distintas pra aceso/apagado. Layout puro vem de resource_pips.
void draw_pips(IRenderer& r, float x, float y, int total, int lit, int max_pips,
               const DrawColor& on, const DrawColor& off) {
    for (const ResourcePip& p : resource_pips(x, y, total, lit, max_pips)) {
        r.draw_filled_rect(p.rect, p.lit ? on : off);
    }
}

}  // namespace

void BattleScene::render(IRenderer& renderer, float viewport_px_w,
                         float viewport_px_h) const {
    // Camera 1:1 no retangulo logico 960x540 (D1). begin_frame recebe os PIXELS REAIS
    // pro backend escalar por inteiro. Cada Rect de layout (px logico) == Rect de mundo.
    const Rect screen = battle_screen_rect();
    renderer.begin_frame(screen, static_cast<int>(viewport_px_w),
                         static_cast<int>(viewport_px_h));

    // --- fundo (D7: camera estatica) ---
    // O FUNDO da arena agora vem da VINHETA/glow radial do renderer (Render2dGl3 desenha
    // um quad full-window LOGO APOS o clear, dando profundidade: bordas escuras + leve
    // lift no centro). NAO repintamos um fill chapado #0c0f1a aqui senao ele cobriria a
    // vinheta. A cor-base canonica segue viva em kBgColor, hoje usada pelo cursor
    // do badge da CTB (ver draw_text mais abaixo, ~linha 716).

    // ====================================================================
    // VARIANTE C "Tatico Cockpit": cockpit lateral esq + arena a direita +
    // CTB topo + banner em faixa propria + terminal/log no rodape.
    // Ordem de render: fundo -> COCKPIT (opaco) -> CTB -> banner -> atores
    // -> log (opaco) -> floaters (por cima). O cockpit/log opacos garantem
    // que nenhum ator invada (e a arena ja vive a DIREITA do cockpit).
    // ====================================================================

    const CombatActor* active = active_actor();

    // --- COCKPIT lateral esquerdo (painel do ator ativo + verbos) ---
    // So mostra os DADOS do ator quando NAO e abertura (na abertura o ativo e o 1o da
    // fila por SPD = inimigo; o cockpit fica vazio/dim). A faixa do cockpit e sempre
    // desenhada (opaca) pra demarcar a coluna.
    // ADR-009: quando o HUD e EXTERNO (RmlUi-GL3), o cockpit inteiro (painel/retrato/
    // pips/menu) NAO e desenhado a mao - o RmlUi o desenha por cima. Evita 2 cockpits.
    if (!hud_external_) {
        const Rect cp = cockpit_rect();
        renderer.draw_filled_rect(cp, kPanelColor);
        renderer.draw_rect_outline(
            Rect{cp.x + cp.w - 1.0f, cp.y, 1.0f, cp.h}, kHudBorderColor, 1.0f);

        if (!is_intro() && active != nullptr) {
            const CombatActor* a = active;
            // Retrato grande do ator ativo (cyan border via highlight). Sem retrato => box.
            const Rect pr = cockpit_portrait_rect();
            const TextureId ptex = portraits_.find(a->id());
            if (ptex != kInvalidTexture) {
                renderer.draw_textured_rect(pr, ptex, UvRect{}, kWhite);
            } else {
                renderer.draw_filled_rect(pr, kSlotDarkColor);
            }
            renderer.draw_rect_outline(pr, kCyan, 1.0f);

            // Nome do ator (cyan), centrado sob o retrato.
            const std::string who = a->display_name();
            const float nw = gus::platform::render2d::text_width(who, kCockpitNamePx);
            renderer.draw_text(who.c_str(), pr.x + (pr.w - nw) * 0.5f,
                               pr.y + pr.h + 4.0f, kCockpitNamePx, kCyan, /*bold=*/true);

            // HP: barra (cockpit largura) + "hp/max" por cima, centrado.
            const Rect hp = cockpit_hp_bar_rect();
            draw_bar(renderer, hp, bar_fill(a->hp(), a->max_hp()), kHpFillColor);
            char num[40];
            std::snprintf(num, sizeof(num), "%d/%d", a->hp(), a->max_hp());
            const float hw = gus::platform::render2d::text_width(num, kCockpitTextPx);
            renderer.draw_text(num, hp.x + (hp.w - hw) * 0.5f,
                               hp.y + (hp.h - kCockpitTextPx) * 0.5f, kCockpitTextPx,
                               kInk, /*bold=*/false);

            // AP pips (latao) + rotulo "AP".
            const Vec2 apo = cockpit_ap_pips_origin();
            const std::string ap_label =
                translator_ != nullptr ? translator_->tr("HUD_AP_LABEL") : std::string("AP");
            renderer.draw_text(ap_label.c_str(), apo.x, apo.y - kCockpitLabelPx - 1.0f,
                               kCockpitLabelPx, kInkDim, /*bold=*/false);
            draw_pips(renderer, apo.x + kCockpitResLabelW, apo.y, a->max_ap(), a->ap(),
                      gus::domain::combat::combat_constants::kBaseApPerTurn,
                      kApLitColor, kApOffColor);
            // Mana pips (cyan) + rotulo "MANA".
            const Vec2 mno = cockpit_mana_pips_origin();
            const std::string mana_label =
                translator_ != nullptr ? translator_->tr("HUD_MANA_LABEL") : std::string("MANA");
            renderer.draw_text(mana_label.c_str(), mno.x, mno.y - kCockpitLabelPx - 1.0f,
                               kCockpitLabelPx, kInkDim, /*bold=*/false);
            draw_pips(renderer, mno.x + kCockpitResLabelW, mno.y, a->max_mana(),
                      a->mana(), gus::domain::combat::combat_constants::kManaCap,
                      kManaLitColor, kManaOffColor);

            // MENU de verbos EMPILHADO (so no turno de jogador vivo). Suprimido durante a
            // ESCOLHA DE ATOR (§4.1): o begin_turn esta deferido, entao nao ha turno cujo
            // menu mostrar - o jogador escolhe QUEM age primeiro (destaque na arena, abaixo).
            // Suprimido tambem com o GOLPE EM VOO (W2): o turno ja foi comandado; o menu
            // some enquanto o sprite desloca/golpeia (re-aparece no proximo turno).
            if (current_actor_is_player() && !combat_over() && !choosing_actor_ &&
                !player_strike_pending_) {
                const Rect mz = cockpit_menu_zone();
                const std::string action_label = translator_ != nullptr
                                                      ? translator_->tr("HUD_ACTION_LABEL")
                                                      : std::string("ACAO");
                renderer.draw_text(action_label.c_str(), mz.x, mz.y - kCockpitLabelPx - 2.0f,
                                   kCockpitLabelPx, kInkDim, /*bold=*/false);
                const auto items = menu_.layout(mz);
                for (int i = 0; i < kBattleVerbCount; ++i) {
                    const MenuItem& it = items[static_cast<std::size_t>(i)];
                    const bool sel = (i == menu_.selected_index());
                    // Caixa do verbo (fundo escuro) + borda na cor do verbo.
                    renderer.draw_filled_rect(
                        it.rect, it.enabled ? kVerbBoxColor : kVerbDisabledColor);
                    const DrawColor vc =
                        it.enabled ? verb_color(it.verb, sel) : kInkDim;
                    renderer.draw_rect_outline(it.rect, vc, sel ? 2.0f : 1.0f);
                    // Nome do verbo (na cor do verbo), centrado vertical, margem esq.
                    const std::string label = tr_verb_label(it.verb);
                    const float ty = it.rect.y + (it.rect.h - kVerbTextPx) * 0.5f;
                    renderer.draw_text(label.c_str(), it.rect.x + 6.0f, ty,
                                       kVerbTextPx, vc, /*bold=*/sel);
                }
            }
        }
    }

    // --- fila CTB (topo, a direita do cockpit; D4) ---
    // SEM FAIXA/CAIXA (lider 2026-06-30): a banda de fundo (kCtbBandColor = kBg2Color)
    // foi REMOVIDA. Ela pintava um retangulo de tom distinto do fundo da arena (que agora
    // e a VINHETA radial, nao mais o flat kBgColor), saltando como uma "caixa" de borda
    // dura atras dos retratos. Agora as celulas/retratos da fila ficam DIRETO sobre a
    // vinheta - seamless, igual a party e a coluna de inimigos (que nao tem faixa). As
    // CELULAS por-retrato (kCtbCellColor), a marca de ativo/proximo (cyan) e o banner
    // seguem intactos. kCtbBandColor deixa de ser usado aqui.
    {
        const CtbStrip strip = ctb_strip(queue_len());
        // JANELA ROTACIONADA (fix da fila CTB): os atores a mostrar vem de ctb_window(), que
        // COMECA no ator ATIVO e segue a ordem de jogo com wrap - NAO mais os top-5 fixos por
        // SPD (bug: atores de SPD baixa, ex. Jaci, nunca apareciam, e o destaque "ativo"
        // ficava preso no slot de maior SPD). window[0] = ativo; window[1] = proximo; etc.
        const std::vector<const CombatActor*> window = ctb_window();
        const CombatActor* active = active_actor();
        for (int i = 0; i < kCtbVisibleCells; ++i) {
            const CtbCell& cell = strip.cells[static_cast<std::size_t>(i)];
            if (!cell.occupied) {
                continue;
            }
            // O ator desta celula vem da janela rotacionada (a fila do motor ja exclui
            // mortos/incapacitados, que o motor remove em advance/prune: a fila CTB reflete
            // so quem ainda joga).
            const CombatActor* who = (i < static_cast<int>(window.size()))
                                         ? window[static_cast<std::size_t>(i)]
                                         : nullptr;

            // Retrato 48px se carregavel; senao retangulo da celula.
            TextureId tex = kInvalidTexture;
            if (who != nullptr) {
                tex = portraits_.find(who->id());
            }
            if (tex != kInvalidTexture) {
                renderer.draw_textured_rect(cell.rect, tex, UvRect{}, kWhite);
            } else {
                renderer.draw_filled_rect(cell.rect, kCtbCellColor);
            }
            // Marca do ator ATIVO (incremento 2): borda verde grossa onde o turno e.
            // Tem prioridade visual sobre a marca de "proximo" (que e o 1o da fila).
            if (who != nullptr && who == active) {
                renderer.draw_rect_outline(cell.rect, kActiveCtbColor, 3.0f);
            } else if (cell.is_next) {
                // "proximo" (D4): so destaca se NAO for ja o ativo.
                renderer.draw_rect_outline(cell.rect, kCtbNextColor, 2.0f);
            }
            // Overflow "+N" (D4): contorno discreto na 5a celula (rotulo vem com fonte).
            if (cell.is_overflow) {
                renderer.draw_rect_outline(cell.rect, kHudBorderColor, 1.0f);
            }
        }
    }

    // --- BANNER DE TURNO (D9/D10, incremento 6): quem joga, abaixo da fila CTB ---
    // "BATALHA!" na intro / "SUA VEZ: escolha uma acao" no jogador / "Vez do inimigo".
    // Resolve a chave via tr() (+ nome do ator ativo no turno do jogador). Sem translator
    // ou combate acabado, nao desenha (a casca degrada sem texto).
    if (!combat_over() && translator_ != nullptr) {
        const std::string_view key = turn_banner_key();
        if (!key.empty()) {
            std::string text = translator_->tr(std::string(key));
            // "Vez de {0}": interpola o NOME do ator ativo (D9; jogador e inimigo). O
            // banner de turno usa display_name (ex: "Vez de Gus", "Vez de Drone").
            const auto pos = text.find("{0}");
            if (pos != std::string::npos) {
                const auto* a = active_actor();
                text.replace(pos, 3, a != nullptr ? a->display_name() : "");
            }
            DrawColor col = kBannerIntroColor;
            if (current_actor_is_player()) {
                col = kBannerPlayerColor;
            } else if (!is_intro()) {
                col = kBannerEnemyColor;
            }
            // Banner na FAIXA PROPRIA do topo da arena (a direita do cockpit), centrado
            // NESSE espaco - nao sobre os atores. Na abertura, sobe pro centro da arena.
            const Rect bz = arena_banner_rect();
            const float center_x = bz.x + bz.w * 0.5f;
            const float by = is_intro() ? (static_cast<float>(kArenaTop) + 40.0f)
                                        : (bz.y + (bz.h - kBannerTextPx) * 0.5f);
            const float bigPx = is_intro() ? kBannerTextPx + 6.0f : kBannerTextPx;
            const float tw = gus::platform::render2d::text_width(text, bigPx);
            renderer.draw_text(text.c_str(), center_x - tw * 0.5f, by, bigPx, col,
                               /*bold=*/true);

            // ABERTURA: prompt de input abaixo do "BATALHA!" (lider). A luta so comeca
            // quando o jogador ENCARA. "[Enter] Encarar" sempre; "[Q] Resolver sem
            // encarar" so se oferecido (TRASH). Centrado na faixa da arena.
            if (is_intro()) {
                float py = by + bigPx + 12.0f;
                const auto draw_prompt = [&](const char* tr_key, DrawColor pc) {
                    const std::string t = translator_->tr(tr_key);
                    const float pw =
                        gus::platform::render2d::text_width(t, kPanelTextPx);
                    renderer.draw_text(t.c_str(), center_x - pw * 0.5f, py,
                                       kPanelTextPx, pc, /*bold=*/false);
                    py += kPanelTextPx + 6.0f;
                };
                draw_prompt("COMBAT_INTRO_ENCARAR", kCyan);  // Encarar = cyan (primary)
                if (offers_auto_resolve()) {
                    draw_prompt("COMBAT_INTRO_AUTORESOLVE", kInkDim);
                }
            }
        }
    }

    // --- arena: party (esq-da-arena) e inimigos (dir), distribuidos space-around ---
    {
        const ArenaLayout arena =
            arena_layout(party_count(), enemy_count(), gus_party_index());

        // Mapeia o slot da party/inimigo de volta pro ator (mesma ordem dos vivos) pra
        // saber QUEM e o ator ativo (highlight D7).
        std::vector<const CombatActor*> alive_party;
        std::vector<const CombatActor*> alive_enemies;
        for (const CombatActor* a : machine_->queue().order()) {
            if (a == nullptr || !a->is_alive()) {
                continue;
            }
            (a->is_player_side() ? alive_party : alive_enemies).push_back(a);
        }

        // Desenha um lado da arena: placeholder do ator + mini-barra de HP REAL sob ele
        // (incremento 2) + highlight se for o ativo (D7). A i-esima slot casa o i-esimo
        // ator vivo daquele lado (mesma ordem do layout/arena_layout).
        // Ator na arena = RETRATO (placeholder; sprite de corpo depois). Moldura na cor
        // do lado (cyan party / magenta inimigo), highlight cyan no ativo (D7/D9),
        // mini-barra de HP (verde party / magenta inimigo via o body color), nome curto.
        auto draw_side = [&](int count, const auto& slots,
                             const std::vector<const CombatActor*>& alive,
                             const DrawColor& side, const DrawColor& hp_fill,
                             bool is_party) {
            for (int i = 0; i < count; ++i) {
                const Rect& r = slots[static_cast<std::size_t>(i)].rect;
                if (i >= static_cast<int>(alive.size())) {
                    continue;
                }
                const CombatActor* a = alive[static_cast<std::size_t>(i)];
                // ANIMACAO (W2, battle-anim.md par.3.2): o ATOR (retrato-placeholder;
                // sprite depois) desenha na posicao-base do slot + o OFFSET de animacao
                // (melee/hit-react/cast). Os elementos de HUD ancorados no ator (barra
                // de HP, status, intent, mira, picker) FICAM no slot-base: o sprite se
                // desloca, a leitura tatica nao dança junto (camera estatica, D7).
                const Vec2 aoff = anim_.offset_for(a->id());
                const Rect ra{r.x + aoff.x, r.y + aoff.y, r.w, r.h};
                const bool act = (active != nullptr && a == active);
                // SPRITE ANIMADO (W3, battle-anim.md par.1.1/3.2): ator COM sprite set
                // desenha o FRAME corrente da animacao da fase (idle/run/golpe/hurt),
                // ESCALADO (frame 256x256 quadrado -> quad de kActorSpriteScale x o
                // slot, proporcao mantida) e ANCORADO PELO PE na BASE do slot ("em pe
                // no chao", nao centrado). O pe REAL vem do alpha-bbox do frame 0 do
                // Idle (referencia ESTAVEL - ancorar pelo idle nao faz o tronco pular
                // entre frames; mesma disciplina do overworld/sprite_anchor). Headless/
                // Null (bbox invalido) degrada pro anchor legado (margem 0). O offset
                // de animacao (aoff) desloca o sprite; o HUD ancorado (HP/status/
                // intent/mira) SEGUE no slot-base, como no placeholder.
                bool drew_sprite = false;
                {
                    BattleClipId sclip = BattleClipId::Idle;
                    float selapsed = 0.0f;
                    const SpriteClip* clip =
                        resolve_sprite_clip(a->id(), &sclip, &selapsed);
                    if (clip != nullptr) {
                        const int fi = clip_frame_index(
                            static_cast<int>(clip->frames.size()), clip->fps,
                            clip->loop, selapsed);
                        const TextureId ftex =
                            clip->frames[static_cast<std::size_t>(fi)];
                        if (ftex != kInvalidTexture) {
                            const float quad = static_cast<float>(kActorSlotH) *
                                               kActorSpriteScale;
                            // Pe real: margem inferior transparente do Idle f0.
                            float foot = 0.0f;
                            const auto sit = sprites_.find(a->id());
                            const SpriteClip* idle =
                                sit != sprites_.end()
                                    ? sit->second.find(BattleClipId::Idle)
                                    : nullptr;
                            const TextureId anchor_tex =
                                idle != nullptr ? idle->frames.front() : ftex;
                            const auto bbox =
                                renderer.texture_content_bbox(anchor_tex);
                            if (bbox.valid()) {
                                foot = bottom_margin_fraction(bbox.bottom_margin(),
                                                              bbox.canvas_h);
                            }
                            const float top_y =
                                sprite_top_y(r.y + r.h, quad, foot,
                                             /*manual_offset_world=*/0.0f);
                            const Rect quad_rect{r.x + (r.w - quad) * 0.5f + aoff.x,
                                                 top_y + aoff.y, quad, quad};
                            renderer.draw_textured_rect(quad_rect, ftex, UvRect{},
                                                        kWhite);
                            // Highlight do ATIVO (D7) segue multimodal: contorno cyan
                            // no quad do sprite. A moldura fina de "lado" do
                            // placeholder NAO se aplica (a silhueta do sprite ja
                            // marca o ator) - decisao W3, lider valida ao vivo.
                            if (act) {
                                renderer.draw_rect_outline(quad_rect, kCyan, 2.0f);
                            }
                            drew_sprite = true;
                        }
                    }
                }
                if (!drew_sprite) {
                    // Retrato (ou slot escuro de fallback) - placeholder de hoje.
                    const TextureId tex = portraits_.find(a->id());
                    if (tex != kInvalidTexture) {
                        renderer.draw_textured_rect(ra, tex, UvRect{}, kWhite);
                    } else {
                        renderer.draw_filled_rect(ra, kSlotDarkColor);
                    }
                    // Moldura: ativo = cyan grossa; senao a cor do lado, fina.
                    renderer.draw_rect_outline(ra, act ? kCyan : side,
                                               act ? 2.0f : 1.0f);
                }
                // Mini-barra de HP sob o ator.
                const Rect hpbar = arena_hp_bar_frame(r);
                draw_bar(renderer, hpbar, bar_fill(a->hp(), a->max_hp()), hp_fill);
                // STATUSROW: icones de status do ator (sob a barra de HP), como no mock.
                {
                    float sx = r.x;
                    const float sy = hpbar.y + hpbar.h + 2.0f;
                    for (const auto& st : a->status_effects()) {
                        const Rect sbox{sx, sy, static_cast<float>(kStatusIconSize),
                                        static_cast<float>(kStatusIconSize)};
                        const TextureId stex = status_icons_.find(st.id);
                        if (stex != kInvalidTexture) {
                            renderer.draw_textured_rect(sbox, stex, UvRect{}, kWhite);
                        } else {
                            renderer.draw_filled_rect(sbox, kStatusBoxColor);
                        }
                        sx += static_cast<float>(kStatusIconSize + kStatusIconGap);
                        if (sx > r.x + r.w) {
                            break;  // nao vaza a largura do slot
                        }
                    }
                }
                // INTENT sobre cada INIMIGO vivo (telegraph). Sem icone => marca magenta.
                if (!is_party) {
                    const auto intent = intent_for(*a);
                    if (intent.has_value()) {
                        const Rect ibox{r.x + (r.w - kIntentIconSize) * 0.5f,
                                        r.y - kIntentIconSize - 2.0f,
                                        kIntentIconSize, kIntentIconSize};
                        const TextureId itex =
                            intent_icon_tex(intent_icons_, *intent);
                        if (itex != kInvalidTexture) {
                            renderer.draw_textured_rect(ibox, itex, UvRect{}, kWhite);
                        } else {
                            renderer.draw_filled_rect(ibox, kMagentaDim);
                            renderer.draw_rect_outline(ibox, kMagenta, 1.0f);
                        }
                    }
                }

                // MIRA (§3.5, D3): destaque MULTIMODAL do inimigo mirado (contorno + seta
                // + nome/HP), NUNCA so cor (Pillar 4/WCAG). Reusa as primitivas de outline/
                // rect/text (mesma tecnica do highlight do ativo). So o alvo mirado.
                if (!is_party && aiming_ && a == aim_target()) {
                    // (1) CONTORNO: reticulo = outline DUPLO (externo grosso + interno
                    //     fino), distinto do outline simples do ator ativo.
                    renderer.draw_rect_outline(r, kMiraColor, 3.0f);
                    const Rect inner{r.x + 3.0f, r.y + 3.0f, r.w - 6.0f, r.h - 6.0f};
                    renderer.draw_rect_outline(inner, kMiraColor, 1.0f);
                    // (2) SETA: caret >> (forma PURA por rects, nao so cor), a ESQUERDA do
                    //     slot, centrado na vertical - nao colide com o intent (acima) nem
                    //     a barra de HP (abaixo). 3 colunas de altura decrescente = aponta
                    //     pro slot.
                    const float acy = r.y + r.h * 0.5f;
                    for (int c = 0; c < 3; ++c) {
                        const float ch = 12.0f - static_cast<float>(c) * 4.0f;  // 12,8,4
                        const Rect bar{r.x - 12.0f + static_cast<float>(c) * kMiraCaretW,
                                       acy - ch * 0.5f, kMiraCaretW, ch};
                        renderer.draw_filled_rect(bar, kMiraColor);
                    }
                    // (3) NOME + HP do alvo em TEXTO legivel (WCAG): nome acima do slot
                    //     (sobre o intent), "hp/max" a direita. Usa display_name direto (sem
                    //     translator), como o cockpit faz com o ator ativo.
                    const std::string nm = a->display_name();
                    renderer.draw_text(nm.c_str(), r.x,
                                       r.y - kIntentIconSize - kMiraLabelPx - 3.0f,
                                       kMiraLabelPx, kMiraColor, /*bold=*/true);
                    char hpn[40];
                    std::snprintf(hpn, sizeof(hpn), "%d/%d", a->hp(), a->max_hp());
                    renderer.draw_text(hpn, r.x + r.w + 2.0f,
                                       r.y + (r.h - kMiraLabelPx) * 0.5f, kMiraLabelPx,
                                       kMiraColor, /*bold=*/false);
                    // (4) TIER DE FRAQUEZA (§3.5 "se ja escaneado, o tier vs a acao"): alvo
                    //     ESCANEADO e FRACO a familia da acao ganha um pip LATAO (paleta:
                    //     "latao = fraqueza"). APRESENTACAO PURA: usa o fallback de familia
                    //     (action_family) - o ataque BASICO nao aplica a roda no motor.
                    if (a->is_scanned()) {
                        const std::optional<CardFamily> fam = action_family(aim_verb_);
                        if (fam.has_value() && !a->is_universal_compiler() &&
                            gus::domain::combat::WeaknessWheel::multiplier(
                                *fam, a->family()) ==
                                gus::domain::combat::combat_constants::kMultFraco) {
                            const Rect fp{r.x + r.w + 2.0f, r.y, kMiraFracoPipSize,
                                          kMiraFracoPipSize};
                            renderer.draw_filled_rect(fp, kBrass);
                        }
                    }
                    // (5) PREVIA DE DANO (feedback do lider no display): SO em [Atacar]
                    //     (Scan e utilitario, nao bate). Mostra a perda de HP PREVISTA no
                    //     alvo mirado ANTES de confirmar, atualizando AO VIVO por alvo.
                    //     Numero PURO do motor (preview_basic_attack_damage: dano bruto -
                    //     absorcao de Shield, piso 0) - a cena NUNCA recalcula regra, so LE.
                    //     "-N" = HP que sai (mesma convencao "-N" da narracao do log); cor =
                    //     a do numero de dano COMUM do floater (destaca do HP ciano). Pillar
                    //     4/WCAG: e NUMERO, nao so cor. Fecha o cluster de info logo ABAIXO
                    //     do "hp/max", no lado direito do slot.
                    if (aim_verb_ == BattleVerb::Atacar) {
                        const CombatActor* attacker = active_actor();
                        if (attacker != nullptr) {
                            const int dano =
                                machine().preview_basic_attack_damage(*attacker, *a);
                            char dmg[24];
                            std::snprintf(dmg, sizeof(dmg), "-%d", dano);
                            const float hp_y = r.y + (r.h - kMiraLabelPx) * 0.5f;
                            renderer.draw_text(
                                dmg, r.x + r.w + 2.0f, hp_y + kMiraLabelPx + 2.0f,
                                kMiraLabelPx, floater_color_for_channel(HitChannel::Common),
                                /*bold=*/true);
                        }
                    }
                }

                // ESCOLHA DE ATOR (§4.1, comando-livre 1B): destaque MULTIMODAL dos membros
                // ELEGIVEIS da party e do PRE-SELECIONADO/cursor. NUNCA so cor (Pillar 4/WCAG):
                // (a) TODO elegivel ganha um BADGE numerado (= a tecla-atalho 1/2/3) + contorno;
                // (b) o CURSOR ganha reticulo (outline duplo) + caret + NOME. So no lado da
                // party. Reusa a tecnica do realce da mira (kMiraColor/caret/nome), aqui em cyan
                // (cor da party) - sem conflito visual com a mira (inimigos, a direita).
                if (is_party && choosing_actor_) {
                    int choice_idx = -1;
                    for (int ci = 0; ci < static_cast<int>(actor_choices_.size()); ++ci) {
                        if (actor_choices_[static_cast<std::size_t>(ci)] == a) {
                            choice_idx = ci;
                            break;
                        }
                    }
                    if (choice_idx >= 0) {
                        const bool is_cursor = (a == actor_pick_target());
                        // (1) CONTORNO cyan. Elegivel = fino; cursor = reticulo (outline duplo).
                        renderer.draw_rect_outline(r, kMiraColor, is_cursor ? 3.0f : 1.0f);
                        if (is_cursor) {
                            const Rect inr{r.x + 3.0f, r.y + 3.0f, r.w - 6.0f, r.h - 6.0f};
                            renderer.draw_rect_outline(inr, kMiraColor, 1.0f);
                        }
                        // (2) BADGE numerado (tecla-atalho): quadrado no canto sup-esq + o
                        //     digito (i+1). Forma PURA (rect) + TEXTO (digito) => multimodal; o
                        //     digito ENSINA a tecla 1/2/3. Cursor = badge PREENCHIDO (contraste).
                        const Rect badge{r.x - 2.0f, r.y - 2.0f, 14.0f, 14.0f};
                        renderer.draw_filled_rect(
                            badge, is_cursor ? kMiraColor : kSlotDarkColor);
                        renderer.draw_rect_outline(badge, kMiraColor, 1.0f);
                        char num[4];
                        std::snprintf(num, sizeof(num), "%d", choice_idx + 1);
                        renderer.draw_text(num, badge.x + 4.0f, badge.y + 2.0f, 10.0f,
                                           is_cursor ? kBgColor : kMiraColor, /*bold=*/true);
                        // (3) CURSOR: caret (forma pura, 3 colunas) a ESQUERDA do slot +
                        //     NOME (texto legivel) acima. Mesmo vocabulario da mira (WCAG).
                        if (is_cursor) {
                            const float acy = r.y + r.h * 0.5f;
                            for (int c = 0; c < 3; ++c) {
                                const float ch = 12.0f - static_cast<float>(c) * 4.0f;
                                const Rect bar{
                                    r.x - 12.0f + static_cast<float>(c) * kMiraCaretW,
                                    acy - ch * 0.5f, kMiraCaretW, ch};
                                renderer.draw_filled_rect(bar, kMiraColor);
                            }
                            const std::string nm = a->display_name();
                            renderer.draw_text(nm.c_str(), r.x, r.y - kMiraLabelPx - 3.0f,
                                               kMiraLabelPx, kMiraColor, /*bold=*/true);
                        }
                    }
                }
            }
        };
        draw_side(arena.party_count, arena.party, alive_party, kCyan, kHp,
                  /*is_party=*/true);
        draw_side(arena.enemy_count, arena.enemies, alive_enemies, kMagenta, kMagenta,
                  /*is_party=*/false);
    }

    // --- PROJETEIS de magia (W2, battle-anim.md par.2.1), por cima dos atores ---
    // Bolinha placeholder NEUTRA de proposito (so valida o esqueleto de timing; o VFX
    // por familia entra depois, vfx-combate-familias.md). O IRenderer so tem rects:
    // a bolinha e aproximada por 3 fatias horizontais (circulo chunky pixel-art).
    for (const auto& p : anim_.projectiles()) {
        const Vec2 pos = p.position();
        const float pr = kProjectileRadiusPx;
        renderer.draw_filled_rect(
            Rect{pos.x - pr, pos.y - pr * 0.5f, 2.0f * pr, pr}, kInk);
        renderer.draw_filled_rect(
            Rect{pos.x - pr * 0.6f, pos.y - pr, 1.2f * pr, pr * 0.5f}, kInk);
        renderer.draw_filled_rect(
            Rect{pos.x - pr * 0.6f, pos.y + pr * 0.5f, 1.2f * pr, pr * 0.5f}, kInk);
    }

    // --- caixa de log (base): ESTRUTURA por evento (D7), 1 marca colorida por linha ---
    // FIX (lider no display): NAO renderiza na ABERTURA - na intro o log esta vazio e a
    // caixa virava um "quadrao preto" grande. So aparece DEPOIS de Encarar (quando ha
    // narracao). A abertura fica limpa (CTB + arena + banner + prompt).
    // ADR-009: com HUD externo (RmlUi), o log/terminal e 100% RmlUi - nao desenha a mao.
    if (!is_intro() && !hud_external_) {
        const Rect l = log_panel_rect();
        renderer.draw_filled_rect(l, kLogColor);
        renderer.draw_rect_outline(l, kHudBorderColor, 1.0f);

        // Linha de log = TEXTO (incremento 3.5) da message real do motor, na cor da
        // categoria (D7). Uma marca curta colorida a esquerda ancora a cor (legivel
        // mesmo se a fonte faltar: a marca sobrevive headless/sem-fonte). Bold pra
        // sistema/dano notavel (criticos/COMPILADO).
        constexpr float kLogLineH = kLogTextPx;  // altura da linha = altura do glifo
        constexpr float kLogLineGap = 2.0f;
        const float pad = 2.0f;
        const int capacity = static_cast<int>(
            (l.h - 2.0f * pad) / (kLogLineH + kLogLineGap));
        if (capacity > 0) {
            const std::vector<LogLine> lines = log_lines(capacity);
            float ly = l.y + pad;
            for (const LogLine& line : lines) {
                const DrawColor col = log_line_color(line.kind);
                // Marca de cor (ancora a categoria; fallback sem-fonte).
                const Rect mark{l.x + pad, ly + 1.0f, 3.0f, kLogLineH - 2.0f};
                renderer.draw_filled_rect(mark, col);
                // Texto da message (sistema/dano em bold pra enfase).
                const bool bold = line.kind == LogLineKind::System ||
                                  line.kind == LogLineKind::Damage ||
                                  line.kind == LogLineKind::Defeat;
                renderer.draw_text(line.text.c_str(), l.x + pad + 5.0f, ly,
                                   kLogTextPx, col, bold);
                ly += kLogLineH + kLogLineGap;
            }
        }
    }

    // --- NUMEROS FLUTUANTES de dano (incremento 5), POR CIMA de tudo ---
    // Cada floater sobe + some pela idade (battle_floaters). Cor por canal; CRIT em bold.
    // O texto e centrado sobre o alvo (origin_x e o centro). Alpha = fade pela idade.
    for (const Floater& f : floaters_) {
        if (!floater_alive(f.age)) {
            continue;
        }
        DrawColor col = floater_color_for_channel(f.channel);
        col.a = floater_alpha(f.age);  // fade
        const float w = gus::platform::render2d::text_width(f.text, kFloaterTextPx);
        const float fx = f.origin_x - w * 0.5f;  // centra o texto sobre o alvo
        const float fy = f.origin_y + floater_offset_y(f.age);  // sobe
        const bool bold = f.channel == HitChannel::Crit;
        renderer.draw_text(f.text.c_str(), fx, fy, kFloaterTextPx, col, bold);
    }

    // --- FLAVOR DE DERROTA (M7-COSTURA Inc 3), POR CIMA de tudo (ultimo bloco) ---
    // O Gus reboota, nao morre (Pillar 1 "magia = software"). 3 linhas centradas sobre um
    // veu quase-opaco: (1) o "kernel panic" (literal tecnico do pool, NAO traduzido -
    // mesma convencao §4 de combat-flavor.md); (2) a falinha blase de um companion vivo
    // (tr(), ou a variante _GENERIC se ninguem sobrou pra falar - wipe-total tambem e
    // Defeat, ver check_end); (3) a nota-xadrez explicando o Gus-centric ao jogador.
    if (defeat_flavor_active()) {
        const Rect full = battle_screen_rect();
        renderer.draw_filled_rect(full, kDefeatVeilColor);

        const float cx = full.x + full.w * 0.5f;
        float ty = full.y + full.h * 0.5f - 40.0f;

        const std::string_view reboot =
            pick_defeat_reboot_line(machine_->log().size());
        const float rw = gus::platform::render2d::text_width(reboot, kDefeatRebootPx);
        renderer.draw_text(std::string(reboot).c_str(), cx - rw * 0.5f, ty,
                           kDefeatRebootPx, kErr, /*bold=*/true);
        ty += kDefeatRebootPx + 16.0f;

        if (translator_ != nullptr) {
            // (2) falinha blase: nomeia o 1o companion AINDA vivo (o Gus, caido pelo
            // Gus-centric, nunca e ele mesmo aqui). Sem ninguem vivo (wipe-total), cai
            // pra variante SEM nome (nao ha quem falar).
            const CombatActor* speaker = first_alive_player();
            std::string bark;
            if (speaker != nullptr) {
                bark = translator_->tr("COMBAT_DEFEAT_BARK");
                const auto pos = bark.find("{0}");
                if (pos != std::string::npos) {
                    bark.replace(pos, 3, speaker->display_name());
                }
            } else {
                bark = translator_->tr("COMBAT_DEFEAT_BARK_GENERIC");
            }
            const float bw = gus::platform::render2d::text_width(bark, kDefeatBarkPx);
            renderer.draw_text(bark.c_str(), cx - bw * 0.5f, ty, kDefeatBarkPx, kInk,
                               /*bold=*/false);
            ty += kDefeatBarkPx + 12.0f;

            // (3) nota-xadrez: "o Rei caiu, a partida acaba" - explica o Gus-centric.
            const std::string note = translator_->tr("COMBAT_DEFEAT_CHESS_NOTE");
            const float nw = gus::platform::render2d::text_width(note, kDefeatNotePx);
            renderer.draw_text(note.c_str(), cx - nw * 0.5f, ty, kDefeatNotePx, kInkDim,
                               /*bold=*/false);
        }
    }

    renderer.end_frame();
}

}  // namespace gus::app::screens
