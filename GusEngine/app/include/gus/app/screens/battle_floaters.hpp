// gus/app/screens/battle_floaters.hpp
//
// MODELO PURO dos NUMEROS FLUTUANTES de dano (M5, incremento 5): POCO 100% testavel SEM
// SDL. Um numero (ex: 25) SOBE sobre o ator alvo e some por fade (~700ms, D7), com COR
// por CANAL (combat.md par.11): COMUM claro, CRIT ciano, FALHA (dano 0) vermelho-erro,
// CURA verde com sinal +. O motor (CombatLogEntry.value + message) carrega o resultado
// real; parse_hit le o canal a partir dos sufixos que a FSM ja produz ([CRITICO] /
// FALHA DE COMPILACAO). A animacao (subida + fade) e funcao pura do TEMPO de vida do
// floater; a BattleScene avanca a idade no update(dt) e desenha em render.
//
// Cross-ref: docs/design/mecanicas/combat.md par.11 (canais de dano);
//            docs/design/mecanicas/battle-screen.md par.5 (D7: numero flutuante);
//            gus/domain/combat/combat_records.hpp (CombatLogEntry.value/message).

#ifndef GUS_APP_SCREENS_BATTLE_FLOATERS_HPP
#define GUS_APP_SCREENS_BATTLE_FLOATERS_HPP

#include <string>

#include "gus/platform/render2d/i_renderer.hpp"  // DrawColor

namespace gus::app::screens {

// Tempo de vida de um floater em segundos. D7 pediu ~700ms, mas o criador testou no
// display e o numero "nao aparecia" (fade rapido demais + texto pequeno); subido pra
// ~1.1s pra ser PERCEPTIVEL durante o turno (ainda curto, nao acumula na tela).
inline constexpr float kFloaterLifeSeconds = 1.1f;

// Quanto o floater sobe (px logico) ao longo da vida (deslocamento total pra cima).
inline constexpr float kFloaterRisePx = 20.0f;

// Canal de um golpe (combat.md par.11) -> cor do numero.
enum class HitChannel : int {
    Common = 0,  // dano comum (branco/claro)
    Crit = 1,    // critico (ciano + bold)
    Fail = 2,    // falha de compilacao, dano 0 (vermelho-erro #F43F5E)
    Heal = 3,    // cura/Regen (verde, +N)
};

// Resultado de um golpe ja classificado pro floater: canal + valor + texto a exibir.
struct HitResult {
    HitChannel channel = HitChannel::Common;
    int value = 0;
    std::string text;  // "25", "+12", "FALHA"
};

// Le o canal + texto de um golpe a partir da message do motor + o value. is_heal marca
// cura (o motor nao sufixa cura; a deteccao vem do tipo de acao/efeito no caller).
// Regras: contem "[CRITICO]" => Crit; contem "FALHA DE COMPILACAO" (ou value 0 sem cura)
// => Fail (texto "FALHA"); is_heal => Heal (texto "+N"); senao Common (texto "N").
[[nodiscard]] HitResult parse_hit(const std::string& message, int value,
                                  bool is_heal = false);

// Cor do floater por canal (DrawColor [0,1]).
[[nodiscard]] gus::platform::render2d::DrawColor floater_color_for_channel(
    HitChannel channel) noexcept;

// true se o floater de idade age (segundos) ainda esta vivo (0 <= age <= vida).
[[nodiscard]] bool floater_alive(float age_seconds) noexcept;

// Deslocamento VERTICAL (px logico) do floater pela idade: 0 no nascimento, sobe ate
// -kFloaterRisePx no fim (eixo +Y pra baixo, entao "subir" e negativo). Clampa na vida.
[[nodiscard]] float floater_offset_y(float age_seconds) noexcept;

// Alpha (1 -> 0) do floater pela idade (fade linear ao longo da vida). Clampa [0,1].
[[nodiscard]] float floater_alpha(float age_seconds) noexcept;

// Um floater ATIVO na cena: texto + canal + posicao de nascimento (px logico, ja sobre
// o alvo) + idade acumulada. A BattleScene guarda uma lista destes; update(dt) envelhece
// e poda os mortos; render desenha com offset/alpha pela idade.
struct Floater {
    std::string text;
    HitChannel channel = HitChannel::Common;
    float origin_x = 0.0f;  // canto do texto no nascimento (px logico)
    float origin_y = 0.0f;
    float age = 0.0f;       // segundos desde o nascimento
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_FLOATERS_HPP
