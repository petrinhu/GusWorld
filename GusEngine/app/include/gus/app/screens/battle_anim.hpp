// gus/app/screens/battle_anim.hpp
//
// ANIMACAO DE COMBATE (M5, W2 base funcional): POCO 100% testavel SEM SDL. Estado de
// animacao POR ATOR aplicado como OFFSET sobre a posicao-base do slot da arena
// (battle_layout/arena_rect_for_actor): a troca placeholder->sprite real depois NAO
// refaz esta logica (battle-anim.md par.1.1/3.2 - os GANCHOS de timing ja ficam
// validados com o retrato-placeholder).
//
// O QUE este modulo cobre (battle-anim.md, ratificado 2026-06-25):
//   - MELEE (par.2.2): desloca-golpeia-volta. Fases Approach (ida ate perto do alvo)
//     -> Hold (parado "no alvo": o CONTATO pode resolver) -> Return (volta ao repouso).
//     O ator SEMPRE termina exatamente em offset (0,0) - sem drift acumulado; o Return
//     parte do offset ATUAL (robusto a skip/frame-jitter).
//   - HIT-REACT (par.2.3): tranco pra tras (knockback VISUAL, cosmetico, todo hit que
//     conecta) e volta ao repouso. Curva seno 0->1->0: termina exatamente em (0,0).
//     FALHA (dano 0) NAO dispara hit-react - o gate fica no CONSUMIDOR (battle_scene
//     le o canal do golpe e simplesmente nao chama start_hit_react).
//   - MAGIA (par.2.1, MECANISMO DORMANTE): cast no lugar (CastWindup, bob sutil) +
//     PROJETIL placeholder (bolinha) que viaja do caster ao alvo e reporta o IMPACTO
//     (take_impacts) - e o gancho onde o hit-react/resolucao da carta vai plugar quando
//     COMPILAR sair do placeholder de UI. Hoje so o caminho de diagnostico usa
//     (BattleScene::debug_cast_demo, env-gated no host).
//
// TIMING: quem dita OS BEATS e o PacingDirector (battle_pacing) + a BattleScene; este
// modulo so anima offsets no tempo que o consumidor pedir. O encaixe canonico
// (battle-anim.md par.3.1): approach do inimigo dura o Beat 1 ANUNCIO
// (kPacingAnnounceSeconds); contato = inicio do Beat 2 (resolve_one_turn); Return +
// hit-react cabem no delay do Beat 2 (kPacingStepDelaySeconds).
//
// Cross-ref: docs/design/mecanicas/battle-anim.md (spec canonica);
//            gus/app/screens/battle_pacing.hpp (beats);
//            gus/app/screens/battle_scene.hpp (consumidor).

#ifndef GUS_APP_SCREENS_BATTLE_ANIM_HPP
#define GUS_APP_SCREENS_BATTLE_ANIM_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "gus/core/spatial/camera_clamp.hpp"  // gus::core::spatial::Vec2

namespace gus::app::screens {

// ---------------------------------------------------------------------------
// Constantes de tuning (px logico 960x540 / segundos). Valores PROPORCIONAIS ao
// que ja existe (slots de 54px; delays do pacing 0.7s/0.8s) - decisao documentada
// (battle-anim.md nao crava numeros finos; ajustar em playtest).
// ---------------------------------------------------------------------------

// Folga entre o atacante deslocado e o slot do alvo no contato (nao sobrepoe).
inline constexpr float kMeleeContactGapPx = 6.0f;
// Volta do melee do INIMIGO ao repouso: cabe FOLGADO no delay do Beat 2 (0.8s).
// So o INIMIGO usa esta (o jogador tem a sua propria, alongada - ver abaixo).
inline constexpr float kMeleeReturnSeconds = 0.4f;

// APROXIMACAO e VOLTA do melee do JOGADOR - DESACOPLADAS do ritmo do inimigo
// (regressao do playtest, lider 2026-07-02: "ele andou de costas" + "muito rapido,
// quase nao vejo nada"). Diagnostico: nao era ordem de frame nem pose - era ALIASING
// TEMPORAL (efeito roda-de-carroca): a corrida de perfil inteira (ida+soco+volta)
// passava em ~1.1s, rapido demais pro olho ler o ciclo de perna, e o cerebro invertia
// a leitura. A cura e DURACAO, nao arte.
//
// POR QUE UMA CONSTANTE PROPRIA (nao reusar kPacingAnnounceSeconds): a aproximacao do
// jogador ANTES pegava carona no kPacingAnnounceSeconds (0.7s) so por acaso de
// implementacao (start_melee_toward recebia a mesma duracao). Esse valor e o BEAT 1 /
// ANUNCIO do INIMIGO (telegraph do windup), aprovado AO VIVO no W1 - alonga-lo
// regridiria o inimigo. Ao contrario do inimigo, o approach do jogador NAO tem beat de
// pacing atrelado (o contato e disparado por anim_.melee_arrived(), nao por um timer do
// director - ver battle_scene::update), entao pode durar o que a LEITURA pedir sem
// mexer no ritmo. Por isso o jogador ganha durMs proprias, e kPacingAnnounceSeconds
// fica CRU, servindo so o inimigo.
//
// Valores (tuning; ajustaveis em playtest, o lider testa ao vivo): approach 1.3s
// (~1.86x o antigo 0.7s: "quase o dobro" pedido) da tempo de ler ~2 ciclos de corrida
// (run_east 0.5s/ciclo). Volta 0.7s (1.75x o antigo 0.4s do inimigo), mantendo a
// proporcao ida:volta ~1.86:1 (o inimigo tem 0.7:0.4 = 1.75:1). INVARIANTE: a volta do
// jogador CABE no delay do Beat 2 (kPacingStepDelaySeconds 0.8s) com folga (0.7 < 0.8):
// o Gus chega ao repouso ANTES de o proximo turno comecar, sem deslizar por cima do
// anuncio seguinte. Testado em battle_scene_test (constantes + comportamento).
inline constexpr float kPlayerMeleeApproachSeconds = 1.3f;
inline constexpr float kPlayerMeleeReturnSeconds = 0.7f;
// Tranco do hit-react (recuo + volta), dentro do delay do Beat 2.
inline constexpr float kHitReactSeconds = 0.3f;
// Amplitude do recuo do hit-react (~1/5 do slot de 54px).
inline constexpr float kHitReactKnockbackPx = 10.0f;
// Windup de conjuracao do caminho de magia (diagnostico; a carta real vai casar o
// beat proprio quando COMPILAR ligar).
inline constexpr float kCastWindupSeconds = 0.5f;
// Bob sutil do caster durante o windup (conjura NO LUGAR; so um respiro vertical).
inline constexpr float kCastBobPx = 4.0f;
// Viagem do projetil placeholder (caster -> alvo).
inline constexpr float kProjectileTravelSeconds = 0.35f;
// Raio da bolinha placeholder do projetil.
inline constexpr float kProjectileRadiusPx = 5.0f;

// Tipo de animacao corrente de um ator. None = em repouso (offset zero).
enum class ActorAnimKind : int {
    None = 0,
    MeleeApproach = 1,  // deslocando ATE o alvo (windup/anticipation do melee)
    MeleeHold = 2,      // chegou: parado no alvo, esperando o CONTATO resolver
    MeleeReturn = 3,    // voltando ao repouso (recovery/follow-through)
    CastWindup = 4,     // conjurando NO LUGAR (magia, par.2.1)
    HitReact = 5,       // tranco pra tras + volta (par.2.3)
};

// Projetil placeholder (bolinha) em voo: viaja (x0,y0) -> (x1,y1) em `duration`.
struct AnimProjectile {
    std::string target_id;  // ator que sofre o impacto ao chegar
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;
    float elapsed = 0.0f;
    float duration = kProjectileTravelSeconds;

    // Posicao corrente (lerp clampado 0..1 do progresso).
    [[nodiscard]] gus::core::spatial::Vec2 position() const noexcept;
};

// Diretor de animacao da arena. POCO: mapa ator->anim + projeteis + relogio via
// update(dt). NAO conhece o motor (domain/) nem SDL: o consumidor decide QUANDO
// iniciar cada anim (nos beats) e o que fazer nos impactos.
class BattleAnimDirector {
public:
    BattleAnimDirector() = default;

    // Avanca o relogio de TODAS as anims + projeteis. Approach que termina vira Hold
    // (fica no alvo ate begin_melee_return); Return/HitReact/CastWindup que terminam
    // voltam a None (offset exatamente zero). Projetil que chega e removido e seu
    // target_id enfileirado pra take_impacts().
    void update(float dt_seconds);

    // ---- melee (par.2.2) ----

    // Inicia o deslocamento do ator: do offset ATUAL ate `displacement` (offset-alvo
    // relativo a posicao de repouso) em `seconds`. Substitui qualquer anim corrente
    // do ator (uma anim por ator).
    void start_melee_approach(const std::string& id,
                              gus::core::spatial::Vec2 displacement, float seconds);

    // true se o ator CHEGOU (Hold): o contato pode resolver (o consumidor casa isso
    // com o inicio do Beat 2 / a resolucao do turno do jogador).
    [[nodiscard]] bool melee_arrived(const std::string& id) const;

    // true se o ator esta no caminho de melee (Approach OU Hold) - ainda nao voltou.
    [[nodiscard]] bool melee_in_flight(const std::string& id) const;

    // Inicia a VOLTA ao repouso: do offset ATUAL ate (0,0) em `seconds`. Robusto a
    // jitter: mesmo se o Approach nao terminou (skip/frames), volta do ponto onde
    // esta. NO-OP se o ator nao esta em Approach/Hold (nao interrompe hit-react).
    void begin_melee_return(const std::string& id, float seconds);

    // ---- hit-react (par.2.3) ----

    // Tranco pra tras: offset x = dir_x * knock_px * sin(pi*t) (0 -> pico -> 0).
    // dir_x = -1 recua pra ESQUERDA (party), +1 pra DIREITA (inimigo). Substitui a
    // anim corrente do ator. O gate de FALHA (sem hit-react) fica no consumidor.
    void start_hit_react(const std::string& id, float dir_x, float seconds,
                         float knock_px);

    // ---- magia: cast + projetil (par.2.1, mecanismo dormante) ----

    // Conjura NO LUGAR por `seconds` (bob vertical sutil; sem deslocamento).
    void start_cast(const std::string& id, float seconds);

    // true enquanto o windup de conjuracao esta tocando.
    [[nodiscard]] bool is_casting(const std::string& id) const;

    // Spawna a bolinha placeholder viajando (x0,y0) -> (x1,y1) em `seconds`; ao
    // chegar, `target_id` aparece em take_impacts() (gancho do hit-react/resolucao).
    void spawn_projectile(const std::string& target_id, float x0, float y0, float x1,
                          float y1, float seconds);

    // Projeteis em voo (leitura pro render: bolinha em position()).
    [[nodiscard]] const std::vector<AnimProjectile>& projectiles() const noexcept {
        return projectiles_;
    }

    // Drena os alvos IMPACTADOS desde o ultimo take (cada projetil reporta 1 vez).
    [[nodiscard]] std::vector<std::string> take_impacts();

    // ---- leitura (render/testes) ----

    // Offset corrente do ator (px logico) a somar na posicao-base do slot. (0,0) se
    // em repouso/desconhecido.
    [[nodiscard]] gus::core::spatial::Vec2 offset_for(const std::string& id) const;

    // Anim corrente do ator (None se em repouso/desconhecido).
    [[nodiscard]] ActorAnimKind kind_for(const std::string& id) const;

    // Tempo RESTANTE (s) da fase corrente do ator: duration - elapsed, clampado em
    // >= 0. 0 em repouso/desconhecido e no MeleeHold (segura indefinido, duration 0).
    // Gancho do SPRITE (W3): o consumidor comeca o swing do attack_melee quando a
    // cauda do Approach cabe o clip (battle_sprite_anim::clip_for_kind).
    [[nodiscard]] float phase_remaining_seconds(const std::string& id) const;

    // true se ha QUALQUER anim/projetil ativo (util pra diagnostico/captura).
    [[nodiscard]] bool any_active() const noexcept {
        return !anims_.empty() || !projectiles_.empty();
    }

private:
    struct ActorAnim {
        ActorAnimKind kind = ActorAnimKind::None;
        float elapsed = 0.0f;
        float duration = 0.0f;
        gus::core::spatial::Vec2 from{};  // offset no inicio da fase
        gus::core::spatial::Vec2 to{};    // offset-alvo da fase
        float knock_dir_x = 0.0f;         // hit-react: direcao do tranco
        float knock_px = 0.0f;            // hit-react: amplitude
    };

    // Anims correntes por ator (ausente = repouso). Uma anim por ator por vez.
    std::unordered_map<std::string, ActorAnim> anims_;
    std::vector<AnimProjectile> projectiles_;
    std::vector<std::string> impacts_;  // alvos atingidos desde o ultimo take
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_ANIM_HPP
