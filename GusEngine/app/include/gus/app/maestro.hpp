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
// voltam pra cidade com o inimigo intacto (pode tentar de novo). NAO inclui (fica pros
// Incrementos 2/3, ver ADR-012): fade preto + crossfade de musica (corte seco por ora); a
// posse do AudioEngine subir pra ca (continua em battle_preview.cpp); o flavor da derrota
// (reboot/bark/tela-xadrez) nem a regra Gus-centric de fim de combate.
//
// Cross-ref: gus/app/maestro_logic.hpp (a logica PURA/testavel que este orquestrador
//            consome); gus/app/sdl_window.hpp (a casca da cidade, agora com modo
//            "anexado" a uma janela externa); gus/app/screens/battle_preview.hpp
//            (run_battle_preview_embedded); docs/tech/adr/ADR-012-m7-paridade-jogavel-
//            plano.md (o plano completo do M7).

#ifndef GUS_APP_MAESTRO_HPP
#define GUS_APP_MAESTRO_HPP

#include <memory>

#include <SDL3/SDL.h>

#include "gus/app/maestro_logic.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/save/save_data.hpp"

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

    SDL_Window* window_ = nullptr;         // dono (a UNICA janela do app)
    std::unique_ptr<SdlWindow> city_;      // a cidade (OverworldSim vive aqui, sempre)

    // Inimigo FIXO (item 1 do escopo, ver maestro_logic.hpp): AABB + estado "derrotado"
    // em memoria. dado de app/ - NAO toca o formato .gmap/TileMap.
    gus::core::spatial::Aabb enemy_aabb_{};
    bool enemy_defeated_ = false;

    // "Save" desta onda: so uma instancia em MEMORIA (a persistencia real em disco e
    // M2-SAVE-IO, ADR-012 Onda 2). Usa SaveData::flags (ja existe, domain/save) em vez
    // de inventar um formato novo pra "inimigo1_derrotado".
    gus::domain::save::SaveData save_{};
};

}  // namespace gus::app

#endif  // GUS_APP_MAESTRO_HPP
