// gus/app/src/maestro.cpp
//
// Implementacao da Maestro (M7-COSTURA, ADR-012 Onda 1). Ver header.

#include "gus/app/maestro.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

#include "gus/app/dialogue/npc_dialogue_catalog.hpp"  // M7-DIALOGO: I/O do .dlg.txt
#include "gus/app/screens/battle_preview.hpp"    // run_battle_preview_embedded
// DIALOGO-TERMINAL: loop GL real (caixa quente com retrato) - substitui o overlay
// funcional simples de texto (npc_dialogue_loop.hpp, aposentado - ver seu header).
#include "gus/app/screens/npc_dialogue_loop_gl.hpp"
#include "gus/app/screens/system_menu_loop.hpp"  // MENU-PAUSA-CONFIG-SOM: Esc na cidade
#include "gus/domain/dialogue/dialogue_runtime.hpp"
#include "gus/domain/settings/system_settings.hpp"
#include "gus/platform/fs/settings_file_store.hpp"

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

// M7-DIALOGO (NPC-MVP): offset cardinal (em CELULAS) do Bertoldo em relacao ao
// spawn do jogador (celula (15,1) - MESMA base do inimigo acima). Mira a celula
// (10,14): Chao aberto da SALA SUL de distritos_inferiores.gmap (alcancavel via a
// passagem estreita cols14-16/row10 que liga o salao principal - rows1-9 - a essa
// sala), DISTANTE da celula do inimigo fixo (10,5) - nenhum dos dois marcadores
// disputa a mesma area. Ver app/tests/maestro_logic_test.cpp (reproducao fiel do
// mapa real) pra a prova headless de reachability/distincao.
constexpr int kNpcBertoldoOffsetTilesX = -5;
constexpr int kNpcBertoldoOffsetTilesY = 13;

// Chave da flag (SaveData::flags) que registra o inimigo fixo derrotado. Espelha o
// EncounterId::kFixedEnemy1 (unico valor desta onda) - quando houver mais encontros, a
// chave vira derivada do id (ponto unico a trocar).
constexpr std::string_view kEnemy1DefeatedFlag = "encounter_fixed_enemy1_defeated";

// M7-COSTURA Inc 2 (ADR-012 decisao 5, "fade preto curto ~0.3-0.5s cada lado"): duracao
// de CADA metade visual da transicao (fade-out sobre a tela de saida + fade-in sobre a
// tela de entrada). Usada nos DOIS sentidos (cidade->batalha e volta) e em AMBOS os
// lados (cidade via run_city_fade; batalha via os parametros fade_in/fade_out_seconds
// de run_battle_preview_embedded) - um unico valor, sem duplicar o numero em 4 lugares.
constexpr float kTransitionFadeSeconds = 0.4f;

// Duracao do CROSSFADE de audio (stop_music+play_music, maestro_logic.hpp::
// crossfade_music), disparado no PICO da opacidade (tela 100% preta). Um pouco mais
// longo que kTransitionFadeSeconds de proposito: a musica cruza enquanto a tela SEGUE
// preta (o resto do fade-in da tela NOVA) - suaviza o corte sem estender o fade visual
// em si. Heuristica inicial; o lider ajusta ao vivo se sentir curto/longo demais.
constexpr float kAudioCrossfadeSeconds = 0.8f;

// Fade-in de BOOT (cidade, ao ligar o app): mais generoso que o da costura - nao ha
// pressa nenhuma no boot, e um fade suave "acordando" o tema evita o pop abrupto de
// volume cheio no frame 1. Nao faz parte do fade preto (nao ha tela preta no boot).
constexpr float kBootMusicFadeInSeconds = 1.0f;

// FUNDO REAL CONGELADO (M7-DIALOGO/MENU-PAUSA-CONFIG-SOM, decisao do lider via
// AskUserQuestion): caminho do PNG onde a Maestro deixa o ultimo frame da cidade
// (SdlWindow::capture_frame_to_png) ANTES de abrir o dialogo do NPC ou o menu de
// pausa - os dois loops GL leem o MESMO arquivo (nunca simultaneamente: sao modais
// exclusivos, orquestrados em serie por Maestro::run(), o 2o capture sempre
// sobrescreve o 1o antes de ser lido de novo).
//
// AC-E3 (AUDITORIA-COMPLETA-2026-07-06): antes vivia em /tmp compartilhado com nome
// FIXO (gusworld_frozen_city.png) - em maquina multiusuario, outro usuario podia
// pre-criar o arquivo ou um symlink com esse nome (classe classica de vulnerabilidade
// de /tmp), alem de colidir entre 2 instancias do jogo. Agora vive dentro do
// diretorio de settings do jogador (gus::platform::fs::resolve_settings_dir(),
// MESMA politica 0700 - rwx so o dono - de settings_file_store.cpp/
// save_file_store.cpp, LGPD leve): dado PRIVADO do jogador, o risco de symlink
// desaparece. create_directories/permissions aqui (nao so em save/settings) porque a
// captura pode acontecer ANTES de qualquer save ou settings.json terem sido
// gravados (1a execucao do jogo).
std::string frozen_city_snapshot_path() {
    const std::string dir = gus::platform::fs::resolve_settings_dir();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    std::filesystem::permissions(dir, std::filesystem::perms::owner_all,
                                  std::filesystem::perm_options::replace, ec);
    // Falha silenciosa aqui (ec ignorado de proposito): se o diretorio nao puder
    // ser criado/permissionado, a captura seguinte (capture_frame_to_png) falha
    // sozinha e degrada pra vinheta (frozen_ok==false) - mesmo contrato de sempre.
    return (std::filesystem::path(dir) / "frozen_city.png").string();
}
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

    // MENU-PAUSA-CONFIG-SOM (INTEGRACAO FINAL): carrega settings.json (ou os DEFAULTS
    // se for a 1a execucao/arquivo ausente/corrompido - load_system_settings degrada
    // com seguranca) e aplica o volume no AudioEngine ANTES de tocar qualquer musica
    // logo abaixo - senao o boot tocaria 1 frame em volume cheio (100%) antes do
    // valor salvo "chegar", um pop perceptivel se o jogador tinha baixado o volume.
    const std::string settings_dir = gus::platform::fs::resolve_settings_dir();
    const gus::domain::settings::SystemSettings loaded_settings =
        gus::platform::fs::load_system_settings(settings_dir);
    audio_.set_music_volume(loaded_settings.music_volume);
    audio_.set_sfx_volume(loaded_settings.sfx_volume);
    std::cout << "Maestro: [settings] carregado de " << settings_dir
              << " - music_volume=" << loaded_settings.music_volume
              << " sfx_volume=" << loaded_settings.sfx_volume << "\n";

    // Traducao (i18n) do MENU DE PAUSA/CONFIG - carregada 1 vez aqui, reusada em toda
    // abertura do menu pela CIDADE (open_pause_from_city). Ausencia => fallback (o
    // Translator devolve a propria chave), mesma degradacao do resto do app/.
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    translator_.load_from_file(tr_path);

    // AUDIO (M7-COSTURA Inc 2, ADR-012 decisao 5 + paga a divida do ADR-011 "AudioEngine
    // e dono da battle_preview"): a Maestro carrega o tema da cidade UMA vez aqui (audio_
    // ja construida no default-member-initializer do header) e toca em LOOP - critério
    // "musica da CIDADE toca enquanto na cidade". Fade-in de boot suave (nao ha tela
    // preta no boot, entao nao e o mesmo fade da costura - so evita o pop de volume).
    const std::string city_music_path = gus::app::screens::resolve_music_path(
        gus::core::assets::kCityThemeFile);
    city_music_id_ = audio_.load_music(city_music_path.c_str());
    audio_.play_music(city_music_id_, /*loop=*/true, kBootMusicFadeInSeconds);
    std::cout << "Maestro: [audio] device "
              << (audio_.available() ? "disponivel" : "INDISPONIVEL (mudo)")
              << " - tema da cidade "
              << (city_music_id_ != gus::platform::audio::kInvalidSound ? "carregado"
                                                                         : "AUSENTE")
              << ".\n";

    // M7-COSTURA Inc 3: carrega TAMBEM o tema da ARENA (kBattleThemeFile) - mesmo
    // padrao do load acima, so o nome do arquivo muda (resolve_music_path
    // generalizado, ver battle_preview.hpp). NAO toca ainda (so a cidade toca no
    // boot); o crossfade em to_battle() dispara este id. Se o load falhar
    // (kInvalidSound), degrada com seguranca: os crossfades caem de volta pra
    // city_music_id_ (ver to_battle/on_battle_result abaixo) em vez de tocar silencio.
    const std::string battle_music_path = gus::app::screens::resolve_music_path(
        gus::core::assets::kBattleThemeFile);
    battle_music_id_ = audio_.load_music(battle_music_path.c_str());
    std::cout << "Maestro: [audio] tema da arena "
              << (battle_music_id_ != gus::platform::audio::kInvalidSound
                      ? "carregado"
                      : "AUSENTE (crossfade cai de volta pro tema da cidade)")
              << ".\n";

    // M7-COSTURA fix BUG-1 (playtest ao vivo do lider: "a batalha so ativou vindo do
    // sul"): pick_fixed_enemy_position devolve so o ANCHOR (celula-alvo, AABB minusculo
    // do tamanho do jogador); enemy_sprite_footprint_aabb deriva o footprint VISUAL
    // (o quad desenhado) usando a MESMA formula que o marcador do androide usa - o
    // sprite desenhado NAO muda com esta mudanca (mesmo tamanho/posicao de sempre).
    //
    // SOLUCAO ARQUITETURAL (BUG-7 revisitado, decisao do lider: "ancorar as hitboxes de
    // ATIVACAO nos PES" - ver maestro_logic.hpp::feet_trigger_aabb): a hitbox que decide
    // "perto o bastante pra ENTRAR EM BATALHA" (should_trigger_battle, run() abaixo) NAO
    // e mais o footprint visual inteiro - e uma caixa PEQUENA ancorada na base do
    // footprint, DESACOPLADA do tamanho do sprite (mesma tecnica de Zelda/Stardew
    // Valley). enemy_trigger_aabb_ e derivada do footprint (enemy_aabb_) UMA vez aqui.
    const gus::core::spatial::Aabb enemy_anchor = pick_fixed_enemy_position(
        city_->grid(), city_->player_aabb(), kEnemyOffsetTilesX, kEnemyOffsetTilesY);
    enemy_aabb_ = enemy_sprite_footprint_aabb(
        enemy_anchor, city_->tuning().player_sprite_height_tiles,
        city_->grid().tile_size());
    enemy_trigger_aabb_ = feet_trigger_aabb(enemy_aabb_, city_->grid().tile_size());
    enemy_defeated_ = false;
    // M7-COSTURA Inc 2: o inimigo fixo agora e VISIVEL no mapa - o mesmo placeholder de
    // androide (retrato_inimigo.png) que a tela de BATALHA ja usa pros inimigos (ver
    // sdl_window.hpp/set_enemy_marker + overworld_sim.hpp). Sem isto, a colisao
    // continuava disparando a batalha "as cegas" (nada pro lider esbarrar de proposito).
    city_->set_enemy_marker(enemy_aabb_);
    std::cout << "Maestro: inimigo fixo (kFixedEnemy1) em (" << enemy_aabb_.x << ", "
              << enemy_aabb_.y << "); jogador em (" << city_->player_aabb().x << ", "
              << city_->player_aabb().y << ").\n";

    // M7-DIALOGO (NPC-MVP): MESMA tecnica de posicionamento do inimigo acima (offset
    // diferente - ver kNpcBertoldoOffsetTilesX/Y). O Bertoldo agora TEM sprite
    // proprio (south.png - a pose de frente pro jogador/camera; NPC parado, sem
    // walk/4-direcoes) num slot de marcador PROPRIO no SdlWindow/OverworldSim
    // (set_npc_bertoldo_marker, distinto de enemy_marker_* - nao reusa o placeholder
    // do androide, entao nao ha mais confusao "inimigo vs NPC amigavel"). Sem isto a
    // hitbox continuava disparando o dialogo "as cegas" (nada pro lider ver antes de
    // esbarrar).
    //
    // FIX BUG (playtest ao vivo do lider, "Bertoldo aparece menor que o Gus"):
    // escala PROPRIA (tuning().npc_bertoldo_sprite_height_tiles, ver overworld_
    // tuning.hpp) em vez de player_sprite_height_tiles - o retrato do Bertoldo
    // (south.png) tem uma margem transparente maior que o do Gus; reusar a MESMA
    // altura-de-CANVAS fazia o adulto renderizar visivelmente mais baixo que a
    // crianca. Isto e SO o footprint VISUAL (quad desenhado, set_npc_bertoldo_marker
    // abaixo) - INALTERADO por esta mudanca.
    //
    // SOLUCAO ARQUITETURAL (BUG-7 revisitado - ver o comentario espelho no inimigo
    // fixo acima): npc_bertoldo_trigger_aabb_ e a MESMA caixa-de-pes
    // (feet_trigger_aabb), reusada aqui SEM duplicar logica - substitui o remendo
    // anterior (sprite_width_fraction, que media o alpha-bbox do retrato pra
    // compensar o busto estreito do Bertoldo) por uma solucao generica, independente
    // do sprite: a caixa de ativacao do dialogo agora nunca precisa saber quanta
    // margem transparente cada retrato tem.
    const gus::core::spatial::Aabb npc_anchor = pick_fixed_enemy_position(
        city_->grid(), city_->player_aabb(), kNpcBertoldoOffsetTilesX,
        kNpcBertoldoOffsetTilesY);
    npc_bertoldo_aabb_ = enemy_sprite_footprint_aabb(
        npc_anchor, city_->tuning().npc_bertoldo_sprite_height_tiles,
        city_->grid().tile_size());
    npc_bertoldo_trigger_aabb_ =
        feet_trigger_aabb(npc_bertoldo_aabb_, city_->grid().tile_size());
    city_->set_npc_bertoldo_marker(npc_bertoldo_aabb_);
    std::cout << "Maestro: [dialogo] Bertoldo (NPC-MVP) em (" << npc_bertoldo_aabb_.x
              << ", " << npc_bertoldo_aabb_.y << ").\n";

    // Carrega o grafo de dialogo do Bertoldo (.dlg.txt real, I/O na fronteira app/ -
    // ver gus/app/dialogue/npc_dialogue_catalog.hpp). O parser e FAIL-FAST por design
    // (ADR-014: formato/estrutura malformada = erro de autoria) - capturado aqui pra
    // o BOOT nunca crashar por causa de um .dlg.txt quebrado (mesma degradacao segura
    // de asset ausente do resto do init(): o esbarrao no Bertoldo vira no-op).
    const std::string dlg_path =
        gus::app::dialogue::resolve_npc_intro_bertoldo_dialogue_path();
    try {
        npc_bertoldo_graph_ =
            gus::app::dialogue::load_dialogue_graph_from_file(dlg_path);
    } catch (const std::exception& e) {
        std::cerr << "Maestro: [dialogo] grafo do Bertoldo malformado (" << dlg_path
                  << "): " << e.what()
                  << " - degradando (esbarrao no NPC vira no-op).\n";
        npc_bertoldo_graph_.reset();
    }
    std::cout << "Maestro: [dialogo] grafo do Bertoldo "
              << (npc_bertoldo_graph_.has_value() ? "carregado (" : "AUSENTE (")
              << dlg_path << ").\n";

    return true;
}

bool Maestro::open_pause_from_city() {
    std::cout << "Maestro: [costura] Esc na cidade -> abrindo MENU DE PAUSA (troca "
                 "escondida pro contexto GL, MENU-PAUSA-CONFIG-SOM).\n";

    // FUNDO REAL CONGELADO (decisao do lider): captura o frame ATUAL da cidade
    // ANTES de soltar o renderer (a captura exige o SDL_Renderer vivo - ver
    // SdlWindow::capture_frame_to_png) - o menu de pausa desenha essa cena REAL
    // como fundo estatico, no lugar da vinheta abstrata (mesmo padrao de Chrono
    // Trigger/Zelda/Stardew Valley). frozen_ok==false (asset/GPU indisponivel) cai
    // de volta pra vinheta de sempre (degradacao segura, ver run_system_menu_loop_
    // owning_gl - string vazia == "sem fundo capturado").
    const std::string frozen_bg_path = frozen_city_snapshot_path();
    const bool frozen_ok = city_->capture_frame_to_png(frozen_bg_path);

    // MESMA tecnica de to_battle() (comprovada empiricamente): solta o SDL_Renderer
    // da cidade pra deixar a janela livre pro contexto GL do menu.
    city_->release_renderer();

    const std::string settings_dir = gus::platform::fs::resolve_settings_dir();
    gus::app::screens::SystemMenuLoopOutcome outcome{};
    const bool ok = gus::app::screens::run_system_menu_loop_owning_gl(
        window_, audio_, translator_, settings_dir, &outcome,
        frozen_ok ? frozen_bg_path : std::string());
    if (!ok) {
        SDL_Log(
            "Maestro: falha ao abrir o menu de pausa (contexto GL/glad) - voltando "
            "pra cidade sem mostrar o menu (degradacao segura).");
    }

    // Reconstroi o SDL_Renderer da cidade INCONDICIONALMENTE (mesmo se ok==false) -
    // senao a cidade fica sem desenhar pro resto da sessao. Mesmo padrao de
    // to_battle()/reacquire_renderer.
    if (!city_->reacquire_renderer()) {
        SDL_Log(
            "Maestro: falha ao reconstruir o renderer da cidade apos o menu de pausa "
            "- a cidade segue rodando SEM desenhar (degradacao segura, sem crash).");
    }

    // Higiene (AC-E3): apaga o snapshot congelado ao fechar o menu - a proxima
    // abertura sobrescreve de qualquer forma (dentro do dir 0700 o risco de symlink
    // ja nao existe), mas nao ha motivo pra deixar o PNG do ultimo frame parado em
    // disco entre uma abertura e outra. Falha silenciosa (arquivo pode nao existir
    // se frozen_ok==false).
    if (frozen_ok) {
        std::error_code remove_ec;
        std::filesystem::remove(frozen_bg_path, remove_ec);
    }

    // outcome so e valido quando ok==true (run_system_menu_loop_owning_gl deixa
    // *out_outcome no default quit_app=false se a criacao do contexto GL falhar) -
    // o "&&" ja cobre isso sem precisar de um guard extra.
    return ok && outcome.quit_app;
}

void Maestro::run() {
    bool running = true;
    while (running) {
        if (!city_->step()) {
            running = false;
            break;
        }
        // MENU-PAUSA-CONFIG-SOM (INTEGRACAO FINAL): Esc na cidade abre o MENU DE
        // PAUSA (a cidade nao tem pilha de modais como a batalha - e o gancho UNICO
        // pra Esc aqui). O jogador fica parado (o overworld nao tem 'update' rodando
        // dentro do menu) - ao fechar (Continuar), a cidade retoma exatamente de onde
        // parou. Sair/fechar-a-janela-durante-o-menu encerra o programa (mesmo
        // contrato de should_stop_running_after_battle, ver to_battle()).
        if (city_->consume_escape_pressed()) {
            if (open_pause_from_city()) {
                running = false;
                break;
            }
        }
        // EDGE-TRIGGER (BUG-6, playtest ao vivo do lider: fugir/perder re-disparava a
        // batalha na hora, pois o jogador volta pra cidade AINDA sobre o inimigo, que
        // PERMANECE em fuga/derrota, e should_trigger_battle - LEVEL-triggered - voltava
        // true enquanto houvesse overlap). A batalha so dispara na TRANSICAO nao-overlap
        // -> overlap (rising edge): overlap AGORA e NAO no frame anterior.
        // FIX BUG-7 revisitado (feet_trigger_aabb, ver init()): a colisao/ativacao
        // usa enemy_trigger_aabb_ (caixa pequena ancorada nos pes), NAO mais o
        // footprint visual inteiro (enemy_aabb_, que continua servindo SO o quad
        // desenhado via set_enemy_marker em init()).
        const bool overlapping_now = should_trigger_battle(
            city_->player_aabb(), enemy_trigger_aabb_, enemy_defeated_);
        if (should_trigger_battle_on_edge(overlapping_now, was_overlapping_enemy_)) {
            // FIX BUG-3 (playtest ao vivo do lider: "cliquei no X pra fechar, a janela
            // reabre na dungeon e em poucos ms vira batalha de novo; precisei pkill"):
            // to_battle() devolve true SO quando o jogador pediu pra fechar a janela
            // DURANTE a batalha - um sinal DISTINTO de qualquer CombatOutcome. Antes,
            // esse quit era absorvido dentro de to_battle() (o SDL_EVENT_QUIT so parava
            // o loop da BATALHA) e a Maestro voltava pra cidade como se fosse Ongoing.
            // Agora o quit propaga: encerra o `while (running)` da CIDADE tambem.
            if (should_stop_running_after_battle(to_battle(EncounterId::kFixedEnemy1))) {
                running = false;
            }
        }
        // Atualiza o estado de overlap DEPOIS de (eventualmente) rodar a batalha, com a
        // posicao/estado ATUAIS: se o inimigo foi DERROTADO (Victory), enemy_defeated_
        // agora torna should_trigger_battle false -> was_overlapping=false (o jogador fica
        // onde esta em paz). Se foi FUGA/DERROTA (inimigo permanece e o jogador segue em
        // cima), fica true -> o proximo disparo exige SAIR e RE-ENTRAR na hitbox (senao
        // re-dispararia em loop, o BUG-6). Sem batalha, so acompanha o overlap do
        // movimento normal (o rising edge dispara no proximo frame que ENTRAR na hitbox).
        was_overlapping_enemy_ = should_trigger_battle(
            city_->player_aabb(), enemy_trigger_aabb_, enemy_defeated_);

        // M7-DIALOGO (NPC-MVP): MESMA tecnica de edge-trigger do inimigo acima
        // (aabb_overlaps + should_trigger_battle_on_edge, ambas ja PUBLICAS/genericas
        // em maestro_logic.hpp - sem duplicar logica). O Bertoldo nao tem conceito de
        // "derrotado": aabb_overlaps sozinho ja da o "overlapping_now". Sem isto, sair
        // da conversa AINDA sobre a hitbox reabriria o dialogo no mesmo frame (mesmo
        // BUG-6 do inimigo) - por isso o rising-edge.
        // FIX BUG-7 revisitado (feet_trigger_aabb, ver init()): idem ao inimigo acima -
        // npc_bertoldo_trigger_aabb_ (caixa pequena ancorada nos pes) decide "perto o
        // bastante pra abrir o dialogo", NAO mais npc_bertoldo_aabb_ (footprint visual
        // inteiro, que continua servindo SO o quad desenhado).
        const bool overlapping_npc_now =
            aabb_overlaps(city_->player_aabb(), npc_bertoldo_trigger_aabb_);
        if (should_trigger_battle_on_edge(overlapping_npc_now,
                                           was_overlapping_npc_bertoldo_)) {
            if (to_npc_dialogue()) {
                running = false;
            }
        }
        was_overlapping_npc_bertoldo_ =
            aabb_overlaps(city_->player_aabb(), npc_bertoldo_trigger_aabb_);
    }
}

bool Maestro::run_city_fade(gus::core::anim::FadeDirection direction,
                             float duration_seconds) {
    if (duration_seconds <= 0.0f) {
        return true;  // no-op: nenhum frame extra (simetrico a fade_overlay_alpha)
    }
    const unsigned long long start_ns = SDL_GetTicksNS();
    while (true) {
        const float elapsed =
            static_cast<float>(SDL_GetTicksNS() - start_ns) / 1.0e9f;
        const float alpha =
            gus::core::anim::fade_overlay_alpha(direction, elapsed, duration_seconds);
        // M7-COSTURA Inc 2c: `direction` tambem escolhe a PERNA do boot pixelizado
        // que a cidade desenha (ver gus/app/sdl_window.hpp::step_with_fade) - o MESMO
        // direction que ja escolhia kOut/kIn pro alpha acima.
        if (!city_->step_with_fade(alpha, direction)) {
            return false;  // janela fechada durante o fade - propaga quit (mesmo
                            // contrato de to_battle/run())
        }
        if (elapsed >= duration_seconds) {
            break;
        }
    }
    return true;
}

bool Maestro::to_battle(EncounterId id) {
    (void)id;  // so 1 valor nesta onda (kFixedEnemy1) - o parametro ja existe pro futuro

    std::cout << "Maestro: [costura] esbarrou no inimigo -> ENTRANDO na batalha "
                 "(fade preto + crossfade de musica, M7-COSTURA Inc 2)...\n";

    // FADE-OUT sobre a CIDADE (tela escurece, ~kTransitionFadeSeconds) - M7-COSTURA
    // Inc 2 (ADR-012 decisao 5). Se o jogador fechar a janela DURANTE o fade, propaga
    // quit igual ao resto do fluxo (nunca chega a entrar na batalha).
    if (!run_city_fade(gus::core::anim::FadeDirection::kOut, kTransitionFadeSeconds)) {
        return true;
    }

    // CROSSFADE DE MUSICA cronometrado com o escurinho (tela 100% preta aqui): para a
    // faixa corrente com fade-out e toca a PROXIMA com fade-in (M7-COSTURA Inc 2, paga
    // a divida do ADR-011 "fade entre telas"). Inc 3: cruza pro tema da ARENA de
    // verdade (battle_music_id_ via battle_crossfade_target, POCO testavel headless em
    // maestro_logic.hpp) - se o load falhou, cai de volta pro tema da cidade
    // (degradacao segura, nunca crasha; o mecanismo de crossfade continua provado
    // mesmo sem a 2a faixa).
    crossfade_music(&audio_, battle_crossfade_target(battle_music_id_, city_music_id_),
                     /*loop=*/true, kAudioCrossfadeSeconds);

    // TROCA ESCONDIDA ATRAS DO PRETO: libera o SDL_Renderer da cidade pra deixar a
    // janela livre pro contexto GL da batalha (a MESMA SDL_Window - decisao do lider,
    // viabilidade validada empiricamente na Onda 1). A tela ja esta 100% preta aqui.
    city_->release_renderer();

    gus::domain::combat::CombatOutcome outcome =
        gus::domain::combat::CombatOutcome::Ongoing;
    bool quit_requested = false;
    // M7-COSTURA Inc 2: passa o AudioEngine DELA (ponteiro nao-dono - a battle_preview
    // so usa pro SFX do hit + o fade visual PROPRIO da tela de batalha, nunca musica) +
    // os DOIS fades visuais da batalha (entrada clareando, saida escurecendo).
    const int rc = gus::app::screens::run_battle_preview_embedded(
        window_, &outcome, &quit_requested, &audio_, kTransitionFadeSeconds,
        kTransitionFadeSeconds);
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

    // CROSSFADE DE VOLTA + FADE-IN sobre a CIDADE (Inc 2): mesma receita, sentido
    // inverso - a batalha ja fez o SEU fade-out (kOut, dentro de
    // run_battle_preview_embedded, tela preta ao voltar aqui); o crossfade dispara
    // agora (ainda preto, cidade acabou de reconstruir o renderer) e a cidade clareia.
    // Inc 3: sempre mira city_music_id_ (o tema da cidade, existe desde sempre) -
    // mesma degradacao segura de antes se estiver invalido (no-op silencioso).
    crossfade_music(&audio_, city_music_id_, /*loop=*/true, kAudioCrossfadeSeconds);
    if (!run_city_fade(gus::core::anim::FadeDirection::kIn, kTransitionFadeSeconds)) {
        return true;  // fechou a janela durante o fade de volta - mesmo contrato
    }

    std::cout << "Maestro: [costura] VOLTANDO pra cidade no mesmo ponto ("
              << city_->player_aabb().x << ", " << city_->player_aabb().y
              << "); inimigo_derrotado=" << (enemy_defeated_ ? "sim" : "nao") << ".\n";
    return false;
}

bool Maestro::to_npc_dialogue() {
    if (!npc_bertoldo_graph_.has_value()) {
        std::cout << "Maestro: [dialogo] esbarrao no Bertoldo ignorado - grafo "
                     "AUSENTE/invalido (degradacao segura).\n";
        return false;
    }
    std::cout << "Maestro: [dialogo] esbarrou no Bertoldo -> abrindo conversa "
                 "(caixa quente com retrato real, DIALOGO-TERMINAL).\n";

    // DialogueRuntime opera sobre save_.flags POR REFERENCIA (domain/dialogue NAO
    // depende de domain/save - ver dialogue_runtime.hpp): a MESMA instancia de
    // SaveData em memoria que ja guarda a flag do inimigo derrotado (on_battle_
    // result acima) - persistencia REAL em disco fica a cargo de quem chamar
    // gus::platform::fs::save_game(save_, slot, dir) (M2-SAVE-IO, ja disponivel;
    // a integracao do MENU "Salvar" com esta instancia e item separado - aqui a
    // prova de round-trip vive no teste headless de integracao, ver TODO.md).
    gus::domain::dialogue::DialogueRuntime runtime(*npc_bertoldo_graph_, save_.flags);
    runtime.enter();

    // FUNDO REAL CONGELADO (decisao do lider): MESMA tecnica de open_pause_from_
    // city - captura o frame ATUAL da cidade ANTES de soltar o renderer; a caixa
    // de dialogo desenha essa cena REAL como fundo estatico, no lugar da vinheta
    // abstrata (mesmo padrao de Chrono Trigger/Zelda/Stardew Valley). frozen_ok==
    // false degrada pra vinheta de sempre (ver run_npc_dialogue_loop_gl).
    const std::string frozen_bg_path = frozen_city_snapshot_path();
    const bool frozen_ok = city_->capture_frame_to_png(frozen_bg_path);

    // DIALOGO-TERMINAL: MESMA tecnica de open_pause_from_city (contexto GL PROPRIO,
    // criado so aqui/destruido ao sair) - solta o SDL_Renderer da cidade ANTES (a
    // janela precisa ficar livre pro contexto GL) e reconstroi DEPOIS,
    // INCONDICIONALMENTE (mesmo se o contexto GL falhar - degradacao segura, a
    // cidade nunca fica sem desenhar pro resto da sessao).
    city_->release_renderer();
    const bool quit_requested = gus::app::screens::run_npc_dialogue_loop_gl(
        window_, *city_, runtime, translator_, audio_,
        frozen_ok ? frozen_bg_path : std::string());
    if (!city_->reacquire_renderer()) {
        SDL_Log(
            "Maestro: falha ao reconstruir o renderer da cidade apos o dialogo - a "
            "cidade segue rodando SEM desenhar (degradacao segura, sem crash).");
    }

    // Higiene (AC-E3): mesma limpeza pos-uso do menu de pausa (ver open_pause_from_
    // city) - o snapshot congelado nao precisa sobreviver alem do dialogo que o leu.
    if (frozen_ok) {
        std::error_code remove_ec;
        std::filesystem::remove(frozen_bg_path, remove_ec);
    }

    const auto it = save_.flags.find("npc_intro.met");
    std::cout << "Maestro: [dialogo] conversa encerrada (npc_intro.met="
              << (it != save_.flags.end() && it->second ? "true" : "false") << ").\n";
    return quit_requested;
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
