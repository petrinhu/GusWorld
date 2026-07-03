// gus/app/maestro.hpp
//
// Maestro: orquestracao LEVE cidade<->batalha (M7-COSTURA, ADR-012 Onda 1). NAO e um
// gerenciador de cenas generico (anti over-engineering, decisao 4 do ADR-012) - so
// alterna entre as DUAS telas que existem hoje (city_scene via OverworldSim/SdlWindow,
// battle_scene via run_battle_preview_embedded) por um contrato pequeno: "contra quem"
// (to_battle) e "ganhou/perdeu" (on_battle_result).
//
// DONA de: a janela + o contexto de render corrente (troca por baixo entre SDL_Renderer
// da cidade e contexto GL da batalha, NA MESMA SDL_Window - decisao do lider, "trocar
// escondido atras do preto"; viabilidade validada empiricamente na Onda 1, ver relatorio
// do incremento); a instancia VIVA de OverworldSim (dentro de SdlWindow, mantida viva e
// PAUSADA - sem step_fixed - durante a batalha, entao a posicao do Gus e de GRACA: nao
// ha reposicionamento/serializacao, a cidade nunca foi destruida); e o estado em memoria
// que sobrevive a troca (flags de encontro - o I/O real em disco e M2-SAVE-IO, Onda 2).
//
// ESCOPO DO INCREMENTO 1 (esqueleto do loop, NAO o milestone inteiro): 1 inimigo FIXO no
// mapa; colisao dispara a batalha; Victory some o inimigo (flag em memoria); Defeat/Fled
// voltam pra cidade com o inimigo intacto (pode tentar de novo). NAO incluiu (pago no
// Incremento 2, ver abaixo): fade preto + crossfade de musica (era corte seco); a posse
// do AudioEngine subindo pra ca. Incremento 3 (ainda pendente): o flavor da derrota
// (reboot/bark/tela-xadrez) e a regra Gus-centric de fim de combate.
//
// ESCOPO DO INCREMENTO 2 (M7-COSTURA, ADR-012 decisao 5 + paga a divida do ADR-011
// "AudioEngine e dono da battle_preview"): a Maestro agora e DONA de 1 unica instancia
// do AudioEngine (audio_ abaixo), viva pro loop inteiro (cidade + todas as entradas/
// saidas de batalha - o device nao e mais reaberto a cada entrada). Toca o tema da
// cidade em loop enquanto na cidade; na troca cidade<->batalha, desenha um fade preto
// curto (gus/core/anim/fade_transition.hpp) POR CIMA da tela ATUAL e, no pico da
// opacidade (tela 100% preta), dispara o CROSSFADE de musica (gus/app/maestro_logic.hpp
// ::crossfade_music) - fecha o criterio de saida do M6 ("fade entre telas"). A
// battle_preview_embedded RECEBE o ponteiro do AudioEngine da Maestro (nao-dono, mesmo
// padrao de BattleScene::set_audio) e so o usa pro SFX do hit + o fade visual da
// PROPRIA tela de batalha - nunca toca musica quando chamada pela Maestro (ver o header
// de battle_preview.hpp).
//
// Cross-ref: gus/app/maestro_logic.hpp (a logica PURA/testavel que este orquestrador
//            consome); gus/app/sdl_window.hpp (a casca da cidade, agora com modo
//            "anexado" a uma janela externa + step_with_fade); gus/app/screens/
//            battle_preview.hpp (run_battle_preview_embedded); gus/core/anim/
//            fade_transition.hpp (a curva do overlay preto); gus/platform/audio/
//            audio_engine.hpp (AudioEngine, agora possuido aqui); docs/tech/adr/
//            ADR-012-m7-paridade-jogavel-plano.md (o plano completo do M7).

#ifndef GUS_APP_MAESTRO_HPP
#define GUS_APP_MAESTRO_HPP

#include <memory>

#include <SDL3/SDL.h>

#include "gus/app/maestro_logic.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/core/anim/fade_transition.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/platform/audio/audio_engine.hpp"

namespace gus::app {

class Maestro {
public:
    Maestro();
    ~Maestro();

    Maestro(const Maestro&) = delete;
    Maestro& operator=(const Maestro&) = delete;

    // SDL_Init (video+gamepad) + cria a JANELA UNICA (compartilhada entre cidade e
    // batalha) + a cidade (SdlWindow anexado) + posiciona o inimigo fixo. Devolve false
    // se qualquer passo falhar (o main reporta e sai != 0).
    [[nodiscard]] bool init();

    // Loop principal: roda a cidade ate o jogador fechar a janela OU esbarrar no
    // inimigo (dispara to_battle); ao voltar da batalha, segue rodando a cidade no
    // MESMO ponto. Retorna quando a janela fecha (em qualquer tela).
    void run();

private:
    // Troca cidade->batalha: libera o SDL_Renderer da cidade (a janela fica livre pro
    // contexto GL), roda o loop da BattleScene na MESMA janela ate o jogador
    // resolver/fugir/fechar, le o outcome final e chama on_battle_result. Ao final,
    // reconstroi o renderer da cidade (recarrega sprites - handles antigos nao
    // sobrevivem, ver sdl_window.hpp/reacquire_renderer). FIX BUG-3 (playtest ao vivo do
    // lider: fechar a janela DURANTE a batalha reabria a cidade em LOOP INFINITO):
    // devolve true se o jogador pediu pra FECHAR A JANELA durante a batalha (sinal
    // DISTINTO de qualquer CombatOutcome) - o chamador (run()) DEVE encerrar o programa
    // na hora, sem voltar a renderizar a cidade (por isso on_battle_result/
    // reacquire_renderer sao PULADOS nesse caso: a janela esta fechando de qualquer
    // jeito). false = retorno normal pra cidade (Victory/Defeat/Fled/Ongoing).
    [[nodiscard]] bool to_battle(EncounterId id);

    // Roteamento outcome->acao (delega a logica pura pra maestro_logic.hpp): Victory
    // marca o inimigo derrotado (flag em memoria + some do mapa - nao dispara mais
    // should_trigger_battle); Defeat/Fled nao marcam (o inimigo continua la).
    void on_battle_result(gus::domain::combat::CombatOutcome outcome);

    // M7-COSTURA Inc 2 (ADR-012 decisao 5): roda o FADE PRETO sobre a CIDADE
    // (city_->step_with_fade, um frame real por vez - a cidade continua VIVA/animada
    // durante o fade, so com o overlay preto crescendo/decrescendo por cima) pela
    // duracao dada, na direcao pedida. Devolve false se o jogador fechou a janela
    // DURANTE o fade (o chamador propaga como quit, mesmo contrato de to_battle()).
    // duration_seconds<=0 e um no-op que devolve true na hora (sem rodar nenhum frame
    // extra) - symmetrico ao fade_overlay_alpha(duration<=0) do core.
    [[nodiscard]] bool run_city_fade(gus::core::anim::FadeDirection direction,
                                      float duration_seconds);

    SDL_Window* window_ = nullptr;         // dono (a UNICA janela do app)
    std::unique_ptr<SdlWindow> city_;      // a cidade (OverworldSim vive aqui, sempre)

    // AUDIO (M7-COSTURA Inc 2): a Maestro e DONA - 1 instancia viva pro loop inteiro
    // (paga a divida do ADR-011 "AudioEngine e dono da battle_preview" - o device nao
    // e mais reaberto a cada entrada na batalha). device_active=true tenta o hardware
    // real; degrada com seguranca se indisponivel (mesma API no-op de sempre).
    gus::platform::audio::AudioEngine audio_{/*device_active=*/true};
    // Tema da cidade (kCityThemeFile), carregado UMA vez em init(). O crossfade
    // (maestro_logic.hpp::crossfade_music) cruza PRA este id nos dois sentidos - o kit
    // CC0 desta onda so tem 1 faixa (ver a nota honesta em core/asset_paths.hpp), entao
    // o mecanismo cruza pra ELA MESMA (riqueza musical fica pra onda de audio dedicada).
    gus::platform::audio::SoundId city_music_id_ =
        gus::platform::audio::kInvalidSound;

    // Inimigo FIXO (item 1 do escopo, ver maestro_logic.hpp): AABB + estado "derrotado"
    // em memoria. dado de app/ - NAO toca o formato .gmap/TileMap.
    gus::core::spatial::Aabb enemy_aabb_{};
    bool enemy_defeated_ = false;

    // EDGE-TRIGGER (M7-COSTURA BUG-6): estado "havia overlap jogador x inimigo no frame
    // ANTERIOR". A batalha so dispara na TRANSICAO nao-overlap -> overlap (rising edge,
    // ver should_trigger_battle_on_edge em maestro_logic.hpp) - senao, na FUGA/DERROTA (o
    // inimigo PERMANECE e o jogador volta pra cidade AINDA sobre ele) o overlap
    // continuo re-dispararia a batalha em loop. Comeca false (o jogador nasce longe do
    // inimigo, ver o offset em init()); apos uma batalha que NAO remove o inimigo, e
    // forcado a true (o jogador esta em cima) pra exigir SAIR e RE-ENTRAR na hitbox.
    bool was_overlapping_enemy_ = false;

    // "Save" desta onda: so uma instancia em MEMORIA (a persistencia real em disco e
    // M2-SAVE-IO, ADR-012 Onda 2). Usa SaveData::flags (ja existe, domain/save) em vez
    // de inventar um formato novo pra "inimigo1_derrotado".
    gus::domain::save::SaveData save_{};
};

}  // namespace gus::app

#endif  // GUS_APP_MAESTRO_HPP
