// gus/app/src/maestro.cpp
//
// Implementacao da Maestro (M7-COSTURA, ADR-012 Onda 1). Ver header.

#include "gus/app/maestro.hpp"

#include <iostream>
#include <string>

#include "gus/app/screens/battle_preview.hpp"  // run_battle_preview_embedded

namespace gus::app {

namespace {
// Mesma dimensao inicial da janela que a SdlWindow standalone usava (sdl_window.cpp) -
// duplicada de proposito (constante pequena, cada casca e dona da sua janela; a Maestro
// agora e quem cria a UNICA janela do modo normal).
constexpr int kWindowW = 1280;
constexpr int kWindowH = 720;

// Offset cardinal (em CELULAS) do inimigo fixo em relacao ao spawn do jogador (celula
// (15,1), passagem estreita logo abaixo do portal entrada_norte). O offset antigo (3,0)
// mirava a celula (18,1) - uma saleta isolada (canto sup-direito do mapa) - e a
// alcancabilidade (pick_fixed_enemy_position/flood-fill) caia de volta no fallback mais
// proximo: a celula (15,0), EM CIMA do proprio portal entrada_norte. Feio (decisao do
// lider, M7-COSTURA): trocado para (-5,+4), que mira a celula (10,5) - Chao aberto,
// alcancavel DIRETO (sem fallback), bem no meio do SALAO PRINCIPAL ESQUERDO onde o
// jogador cai ao descer a passagem central (cols1-13, rows1-9 de
// distritos_inferiores.csv) - a 5 celulas (chebyshev) do spawn, perto o bastante pra o
// lider esbarrar nele cedo no playtest ao vivo, longe o bastante do portal e da
// passagem estreita. Ver app/tests/maestro_logic_test.cpp (regressao com reproducao
// fiel do mapa real) para a prova headless de ambos os offsets.
constexpr int kEnemyOffsetTilesX = -5;
constexpr int kEnemyOffsetTilesY = 4;

// Chave da flag (SaveData::flags) que registra o inimigo fixo derrotado. Espelha o
// EncounterId::kFixedEnemy1 (unico valor desta onda) - quando houver mais encontros, a
// chave vira derivada do id (ponto unico a trocar).
constexpr std::string_view kEnemy1DefeatedFlag = "encounter_fixed_enemy1_defeated";
}  // namespace

Maestro::Maestro() = default;

Maestro::~Maestro() {
    city_.reset();  // destroi o renderer da cidade (se vivo); a janela NAO (ver abaixo)
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool Maestro::init() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("Maestro: SDL_Init falhou: %s", SDL_GetError());
        return false;
    }

    // A JANELA UNICA do app: criada com SDL_WINDOW_OPENGL desde o inicio (necessario
    // pra SDL_GL_CreateContext funcionar de forma portavel mais tarde, quando a batalha
    // entra - validado que NAO precisa ter os atributos de versao/profile/stencil
    // setados nesta hora, so o flag OPENGL; a batalha os seta a cada entrada, ver
    // run_battle_preview_embedded). SDL_WINDOW_RESIZABLE preserva o feel de sempre.
    window_ = SDL_CreateWindow("GusWorld", kWindowW, kWindowH,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window_ == nullptr) {
        SDL_Log("Maestro: SDL_CreateWindow falhou: %s", SDL_GetError());
        return false;
    }

    city_ = std::make_unique<SdlWindow>();
    if (!city_->init_attached(window_)) {
        SDL_Log("Maestro: falha ao inicializar o renderer/cidade.");
        return false;
    }

    // M7-COSTURA fix BUG-1 (playtest ao vivo do lider: "a batalha so ativou vindo do
    // sul"): pick_fixed_enemy_position devolve so o ANCHOR (celula-alvo, AABB minusculo
    // do tamanho do jogador); enemy_sprite_footprint_aabb deriva o AABB REAL (colisao E
    // visual) usando a MESMA formula que o marcador desenha o quad do androide - hitbox
    // e sprite visivel passam a COINCIDIR exatamente (ver maestro_logic.hpp).
    const gus::core::spatial::Aabb enemy_anchor = pick_fixed_enemy_position(
        city_->grid(), city_->player_aabb(), kEnemyOffsetTilesX, kEnemyOffsetTilesY);
    enemy_aabb_ = enemy_sprite_footprint_aabb(
        enemy_anchor, city_->tuning().player_sprite_height_tiles,
        city_->grid().tile_size());
    enemy_defeated_ = false;
    // M7-COSTURA Inc 2: o inimigo fixo agora e VISIVEL no mapa - o mesmo placeholder de
    // androide (retrato_inimigo.png) que a tela de BATALHA ja usa pros inimigos (ver
    // sdl_window.hpp/set_enemy_marker + overworld_sim.hpp). Sem isto, a colisao
    // continuava disparando a batalha "as cegas" (nada pro lider esbarrar de proposito).
    city_->set_enemy_marker(enemy_aabb_);
    std::cout << "Maestro: inimigo fixo (kFixedEnemy1) em (" << enemy_aabb_.x << ", "
              << enemy_aabb_.y << "); jogador em (" << city_->player_aabb().x << ", "
              << city_->player_aabb().y << ").\n";
    return true;
}

void Maestro::run() {
    bool running = true;
    while (running) {
        if (!city_->step()) {
            running = false;
            break;
        }
        if (should_trigger_battle(city_->player_aabb(), enemy_aabb_,
                                   enemy_defeated_)) {
            // FIX BUG-3 (playtest ao vivo do lider: "cliquei no X pra fechar, a janela
            // reabre na dungeon e em poucos ms vira batalha de novo; precisei pkill"):
            // to_battle() devolve true SO quando o jogador pediu pra fechar a janela
            // DURANTE a batalha - um sinal DISTINTO de qualquer CombatOutcome. Antes,
            // esse quit era absorvido dentro de to_battle() (o SDL_EVENT_QUIT so parava
            // o loop da BATALHA) e a Maestro voltava pra cidade como se fosse Ongoing -
            // o jogador ainda sobre o inimigo, should_trigger_battle voltava true, e o
            // loop reabria a batalha (LOOP INFINITO ate o pkill). Agora o quit propaga:
            // encerra o `while (running)` da CIDADE tambem, sem re-renderizar nada.
            if (should_stop_running_after_battle(to_battle(EncounterId::kFixedEnemy1))) {
                running = false;
            }
        }
    }
}

bool Maestro::to_battle(EncounterId id) {
    (void)id;  // so 1 valor nesta onda (kFixedEnemy1) - o parametro ja existe pro futuro

    std::cout << "Maestro: [costura] esbarrou no inimigo -> ENTRANDO na batalha "
                 "(trocando SDL_Renderer da cidade por contexto GL, mesma janela)...\n";

    // TROCA ESCONDIDA ATRAS DO PRETO (corte seco neste incremento - fade/crossfade sao o
    // Incremento 2): libera o SDL_Renderer da cidade pra deixar a janela livre pro
    // contexto GL da batalha (a MESMA SDL_Window - decisao do lider, viabilidade
    // validada empiricamente na Onda 1).
    city_->release_renderer();

    gus::domain::combat::CombatOutcome outcome =
        gus::domain::combat::CombatOutcome::Ongoing;
    bool quit_requested = false;
    const int rc = gus::app::screens::run_battle_preview_embedded(
        window_, &outcome, &quit_requested);
    if (rc != 0) {
        SDL_Log(
            "Maestro: run_battle_preview_embedded devolveu %d (contexto GL/glad "
            "falhou) - outcome fica Ongoing, o inimigo NAO e marcado derrotado.",
            rc);
    }

    if (quit_requested) {
        // FIX BUG-3: o jogador fechou a janela DENTRO da batalha. NAO volta pra cidade
        // (on_battle_result/reacquire_renderer sao PULADOS de proposito - a janela vai
        // ser destruida pelo dtor da Maestro de qualquer jeito, reconstruir o renderer
        // da cidade agora so pra descarta-lo em seguida seria trabalho inutil). O
        // outcome fica descartado (nao importa mais: o programa esta encerrando).
        std::cout << "Maestro: [costura] janela fechada DURANTE a batalha -> "
                     "encerrando o programa (sem voltar pra cidade).\n";
        return true;
    }

    on_battle_result(outcome);

    if (!city_->reacquire_renderer()) {
        SDL_Log(
            "Maestro: falha ao reconstruir o renderer da cidade apos a batalha - a "
            "cidade segue rodando SEM desenhar (degradacao segura, sem crash).");
    }

    std::cout << "Maestro: [costura] VOLTANDO pra cidade no mesmo ponto ("
              << city_->player_aabb().x << ", " << city_->player_aabb().y
              << "); inimigo_derrotado=" << (enemy_defeated_ ? "sim" : "nao") << ".\n";
    return false;
}

void Maestro::on_battle_result(gus::domain::combat::CombatOutcome outcome) {
    const char* label = "Ongoing(janela fechada no meio)";
    switch (outcome) {
        case gus::domain::combat::CombatOutcome::Victory: label = "Victory"; break;
        case gus::domain::combat::CombatOutcome::Defeat: label = "Defeat"; break;
        case gus::domain::combat::CombatOutcome::Fled: label = "Fled"; break;
        case gus::domain::combat::CombatOutcome::Ongoing: break;
    }
    std::cout << "Maestro: [costura] outcome=" << label << "\n";

    if (outcome_marks_enemy_defeated(outcome)) {
        enemy_defeated_ = true;
        save_.flags[std::string(kEnemy1DefeatedFlag)] = true;
        // O marcador VISUAL some junto (M7-COSTURA Inc 2) - senao o androide continuaria
        // desenhado no mapa mesmo com should_trigger_battle ja desarmado (enemy_defeated_),
        // um fantasma visual que nao bate mais.
        city_->clear_enemy_marker();
        std::cout << "Maestro: [costura] inimigo kFixedEnemy1 DERROTADO - some do "
                     "mapa (flag '"
                  << kEnemy1DefeatedFlag << "'=true em memoria).\n";
    }
    // Defeat/Fled/Ongoing (janela fechada no meio): NAO marca - o inimigo continua no
    // mapa, o jogador volta pro MESMO ponto (o OverworldSim nunca foi destruido/
    // recarregado - ficou vivo e pausado durante a batalha) e pode tentar de novo. O
    // flavor da derrota (reboot/bark/tela-xadrez) e o Incremento 3.
}

}  // namespace gus::app
