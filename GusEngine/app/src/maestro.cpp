// gus/app/src/maestro.cpp
//
// Implementacao da Maestro (M7-COSTURA, ADR-012 Onda 1). Ver header.

#include "gus/app/maestro.hpp"

#include <array>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

#include "gus/app/app_icon.hpp"  // APP-ICON: set_window_icon_if_available
#include "gus/app/dialogue/npc_dialogue_catalog.hpp"  // M7-DIALOGO: I/O do .dlg.txt
#include "gus/app/screens/battle_preview.hpp"    // run_battle_preview_embedded
// DIALOGO-TERMINAL: loop GL real (caixa quente com retrato) - substitui o overlay
// funcional simples de texto (npc_dialogue_loop.hpp, aposentado - ver seu header).
#include "gus/app/screens/npc_dialogue_loop_gl.hpp"
#include "gus/app/screens/save_load_menu.hpp"  // MODOS-MORTE Fase 0: most_recent_occupied_slot
#include "gus/app/screens/system_menu_loop.hpp"  // MENU-PAUSA-CONFIG-SOM: Esc na cidade
#include "gus/app/screens/title_menu_loop.hpp"  // SAVE-LOAD-UI etapa 4: TELA DE TITULO no boot
#include "gus/domain/dialogue/dialogue_runtime.hpp"
#include "gus/domain/input/controls_name.hpp"  // kDefaultProfile (M2: liga a tela Controles ao input real)
#include "gus/domain/save/save_policy.hpp"  // SAVE-LOAD-UI etapa 5: autosave_allowed_at
#include "gus/domain/save/save_slots.hpp"  // kAutosaveSlot
#include "gus/domain/settings/system_settings.hpp"
#include "gus/platform/fs/controls_file_store.hpp"  // load_controls (M2)
#include "gus/platform/fs/save_file_store.hpp"  // SAVE-LOAD-UI etapa 6: resolve_saves_dir
#include "gus/platform/fs/settings_file_store.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"  // FLASH-CTX (A1): gl3_load_functions do contexto GL unico

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
    // FLASH-CTX (A1): city_.reset() destroi o Render2dGl3 (libera GL: texturas/
    // programa/VAO/VBO) - precisa do contexto CORRENTE pra isso (glDelete* com
    // nenhum contexto corrente e comportamento indefinido). O contexto da Maestro
    // (gl_context_) so e destruido DEPOIS, na sequencia certa (mesma ordem de
    // gus/app/screens/battle_preview.cpp: Render2dGl3 destruido antes do
    // SDL_GL_DestroyContext). A janela e destruida por ultimo.
    city_.reset();
    if (gl_context_ != nullptr) {
        SDL_GL_DestroyContext(gl_context_);
    }
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

    // A JANELA UNICA do app: criada com SDL_WINDOW_OPENGL desde o inicio.
    // SDL_WINDOW_RESIZABLE preserva o feel de sempre.
    window_ = SDL_CreateWindow("GusWorld", kWindowW, kWindowH,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window_ == nullptr) {
        SDL_Log("Maestro: SDL_CreateWindow falhou: %s", SDL_GetError());
        return false;
    }

    // APP-ICON: a Maestro e a DONA da UNICA janela do modo normal (ver o comentario
    // do header) - este e o ponto real que o jogador roda (main.cpp -> Maestro::init,
    // nao a SdlWindow standalone). Degradacao segura embutida na propria funcao (asset
    // ausente/decode falhou/SDL recusou -> loga e segue sem icone, nunca crasha).
    set_window_icon_if_available(window_);

    // FLASH-CTX (A1, passo 2 do plano - docs/tech/pivot/menu-flash-contexto-unico-plano.md):
    // O CONTEXTO GL UNICO do processo - criado UMA vez aqui, corrente do boot ao
    // shutdown (destruido so no dtor). Atributos 3.3 core/doublebuffer/stencil 8 (MESMA
    // receita que run_system_menu_loop_owning_gl/run_battle_preview_embedded ja usavam
    // POR ENTRADA - agora e so uma vez, no processo inteiro). SDL_GL_SetSwapInterval(1)
    // substitui o SDL_SetRenderVSync(renderer, 1) que a cidade usava em SDL_Renderer -
    // com contexto unico o vsync e uma propriedade do CONTEXTO, setada uma vez so.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);  // o GL3 do RmlUi usa stencil (clip mask)
    gl_context_ = SDL_GL_CreateContext(window_);
    if (gl_context_ == nullptr) {
        SDL_Log("Maestro: SDL_GL_CreateContext (contexto unico) falhou: %s",
                SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(window_, gl_context_);
    SDL_GL_SetSwapInterval(1);  // 1 = sincroniza com o refresh (era SDL_SetRenderVSync)
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        SDL_Log("Maestro: falha ao carregar funcoes OpenGL (glad) pro contexto unico.");
        return false;
    }

    city_ = std::make_unique<SdlWindow>();
    if (!city_->init_attached(window_)) {
        SDL_Log("Maestro: falha ao inicializar o renderer/cidade.");
        return false;
    }

    // MENU-INICIAL (ACHADO 1): captura o SPAWN real do jogador AQUI - init_attached()
    // ja carregou a cidade (o ctor de SdlWindow monta o sim_ via load_city_or_fallback,
    // player_start = spawn_player_aabb(map)) mas nenhum step_fixed rodou ainda, entao
    // city_->player_aabb() e EXATAMENTE a posicao de nascimento. reset_to_new_game()
    // (chamado pelo gameplay_engineer quando "Novo Jogo" e escolhido FORA do boot, ver
    // o contrato no header) teleporta o jogador de volta pra cá.
    player_spawn_aabb_ = city_->player_aabb();

    // SAVE-LOAD-UI etapa 6: ancora do relogio de playtime (ver o comentario do
    // campo em maestro.hpp) - comeca no boot do processo, base 0 (nenhum save
    // carregado ainda).
    playtime_anchor_ns_ = SDL_GetTicksNS();

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

    // M2 (GAP FINAL: liga a tela Controles ao input REAL) - carrega o remap
    // persistido em "<perfil>_controls.json" (ou default_controls() se ausente/
    // corrompido - load_controls degrada com seguranca, NUNCA lanca) e alimenta o
    // SdlInput VIVO dentro da cidade. Antes deste fix, city_ SEMPRE nascia com
    // gus::domain::input::default_controls() hardcoded (SdlInput::SdlInput()) -
    // remapear na tela Controles gravava o disco certinho, mas o WASD real nunca
    // lia esse arquivo. Perfil UNICO "default" nesta onda (MESMO perfil que
    // system_menu_loop.cpp::persist_controls usa pra salvar).
    const gus::domain::input::InputRemapConfig loaded_controls =
        gus::platform::fs::load_controls(settings_dir,
                                          std::string(gus::domain::input::kDefaultProfile));
    city_->set_controls(loaded_controls);
    std::cout << "Maestro: [controles] carregado de " << settings_dir
              << " (perfil '" << gus::domain::input::kDefaultProfile << "').\n";

    // Traducao (i18n) do MENU DE PAUSA/CONFIG - carregada 1 vez aqui, reusada em toda
    // abertura do menu pela CIDADE (open_pause_from_city). Ausencia => fallback (o
    // Translator devolve a propria chave), mesma degradacao do resto do app/. FIX
    // (FEDORA-GCC16-NODISCARD): o retorno era descartado silenciosamente - a
    // degradacao (catalogo ausente/ilegivel -> UI mostra as CHAVES cruas) nao tinha
    // NENHUM log, quebrando a regra do projeto "todo efeito loga" e o padrao dos 2
    // loads vizinhos (settings/controls, ambos logam load ok/fallback acima).
    const std::string tr_path = gus::app::i18n::resolve_translations_path();
    const bool tr_loaded = translator_.load_from_file(tr_path);
    std::cout << "Maestro: [i18n] catalogo de " << tr_path << " - "
              << (tr_loaded ? "carregado" : "AUSENTE/ILEGIVEL (fallback: UI mostra as chaves)")
              << ".\n";

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
    // FIX BUG-8 revisitado (regressao "esbarrar nao aciona mais a batalha" - ver
    // maestro_logic.hpp::feet_trigger_aabb): solid_box_tiles PRECISA ser o MESMO
    // valor usado pela colisao FISICA real (OverworldSim::solid_obstacle_from_
    // footprint via tuning_.npc_solid_box_tiles) - nunca um numero duplicado aqui,
    // senao os dois sistemas podem divergir de novo.
    enemy_trigger_aabb_ = feet_trigger_aabb(enemy_aabb_, city_->grid().tile_size(),
                                             city_->tuning().npc_solid_box_tiles);
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
    // FIX BUG-8 revisitado (idem ao inimigo fixo acima - MESMA regra: solid_box_tiles
    // vem SEMPRE do tuning real, nunca duplicado aqui).
    npc_bertoldo_trigger_aabb_ = feet_trigger_aabb(
        npc_bertoldo_aabb_, city_->grid().tile_size(),
        city_->tuning().npc_solid_box_tiles);
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

gus::domain::save::SaveData Maestro::build_current_save_data() const {
    gus::domain::save::SaveData data = save_;  // flags ja acumuladas na sessao
    const gus::core::spatial::Aabb& player = city_->player_aabb();
    data.player_position = gus::domain::save::Vec3{
        static_cast<double>(player.x), static_cast<double>(player.y), 0.0};
    // scene_path SEM extensao (mesma convencao de location_key_for_scene em
    // save_load_menu_rml.cpp) - UNICO mapa desta vertical slice (M4).
    data.current_scene_path = "distritos_inferiores";
    data.timestamp_ms = static_cast<std::int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    data.playtime_seconds = playtime_base_seconds_ +
                             static_cast<double>(SDL_GetTicksNS() - playtime_anchor_ns_) /
                                 1.0e9;
    return data;
}

void Maestro::apply_loaded_save_data(const gus::domain::save::SaveData& data) {
    save_ = data;
    // Reposiciona o jogador (SAVE-LOAD-UI etapa 6: o "ponto de risco" que o
    // lider sinalizou - resolvido via OverworldSim::set_player_position,
    // ADITIVO/novo, ver overworld_sim.hpp). Preserva w/h ATUAIS (o sprite
    // nao muda de tamanho; so x/y vem do save).
    gus::core::spatial::Aabb pos = city_->player_aabb();
    pos.x = static_cast<float>(data.player_position.x);
    pos.y = static_cast<float>(data.player_position.y);
    city_->set_player_position(pos);

    // Re-deriva enemy_defeated_/marcador visual a partir da flag carregada
    // (MESMA chave que on_battle_result grava) - um save de ANTES do
    // inimigo ser derrotado deve TRAZER o inimigo de volta visualmente
    // (ex.: o jogador carregou um slot antigo apos ja te-lo vencido nesta
    // sessao).
    const auto it = data.flags.find(std::string(kEnemy1DefeatedFlag));
    enemy_defeated_ = (it != data.flags.end() && it->second);
    if (enemy_defeated_) {
        city_->clear_enemy_marker();
    } else {
        city_->set_enemy_marker(enemy_aabb_);
    }
    // EDGE-TRIGGER (mesma cautela de on_battle_result abaixo): forca SAIR
    // e RE-ENTRAR nas hitboxes antes de re-disparar batalha/dialogo -
    // o jogador pode ter sido teleportado PRA CIMA do inimigo/NPC.
    was_overlapping_enemy_ = false;
    was_overlapping_npc_bertoldo_ = false;

    // SAVE-LOAD-UI etapa 6 (playtime REAL): re-ancora o relogio da SESSAO
    // ATUAL na base do playtime do save carregado - dali em diante o
    // acumulo (build_current_save_data acima) soma o tempo REAL desta
    // sessao por cima do que o save trazia.
    playtime_base_seconds_ = data.playtime_seconds;
    playtime_anchor_ns_ = SDL_GetTicksNS();

    std::cout << "Maestro: [save-load] Load aplicado - posicao=(" << pos.x << ", "
              << pos.y << ") enemy_defeated=" << enemy_defeated_
              << " playtime_seconds=" << playtime_base_seconds_ << "\n";
}

void Maestro::reset_to_new_game(gus::domain::save::DifficultyLevel difficulty) {
    // MENU-INICIAL (ACHADO 1): mesma ESTRUTURA de apply_loaded_save_data() acima
    // (posicao -> inimigo/marcador -> edge-triggers -> playtime), so que a FONTE
    // do estado nao e um SaveData carregado do disco - e o SHAPE de um jogo NOVO
    // (domain::save::fresh_new_game_save_data, POCO, ver gus/domain/save/
    // new_game.hpp) + o spawn REAL capturado em init() (player_spawn_aabb_, ver o
    // comentario do campo em maestro.hpp).
    save_ = gus::domain::save::fresh_new_game_save_data(difficulty);

    // Posicao do jogador: TELEPORTE pro spawn (mesmo set_player_position sem
    // interpolacao fantasma que apply_loaded_save_data usa acima - nunca um
    // "deslize" visual do ponto antigo pro novo).
    city_->set_player_position(player_spawn_aabb_);

    // Inimigo fixo: volta a existir - uma Victory da sessao ANTERIOR (que
    // sobreviveria em memoria se so trocassemos de tela) nao deve persistir num
    // jogo novo. MESMO par set_enemy_marker/enemy_defeated_ que init() usa no
    // boot.
    enemy_defeated_ = false;
    city_->set_enemy_marker(enemy_aabb_);

    // EDGE-TRIGGER (mesma cautela de apply_loaded_save_data acima): o jogador
    // "acaba de nascer" no spawn - nunca deve contar como "ja estava sobre a
    // hitbox" (evitaria false-negative se o spawn ficasse perto de um trigger).
    was_overlapping_enemy_ = false;
    was_overlapping_npc_bertoldo_ = false;

    // Playtime: re-ancora do ZERO (um jogo novo comeca em playtime_seconds=0, ao
    // contrario de apply_loaded_save_data - que herda o playtime do save
    // carregado - a base aqui e sempre 0.0, nunca save_.playtime_seconds).
    playtime_base_seconds_ = 0.0;
    playtime_anchor_ns_ = SDL_GetTicksNS();

    std::cout << "Maestro: [reset] Novo Jogo (fora do boot, MENU-INICIAL achado 1) "
                 "aplicado - posicao=("
              << player_spawn_aabb_.x << ", " << player_spawn_aabb_.y
              << ") dificuldade=" << static_cast<unsigned>(difficulty) << ".\n";
}

void Maestro::maybe_autosave(const char* trigger_label) {
    // SAVE-LOAD-UI etapa 5 (AUTOSAVE): hook de politica por local (ver
    // gus/domain/save/save_policy.hpp, memoria project_save_dungeon_pem_faraday).
    // Nesta fatia (vertical slice, M4) SO EXISTE CIDADE - os 2 overrides ficam
    // hardcoded false POR ORA (o predicado ja esta pronto/testado pro dia em que
    // a 1a dungeon existir; SO a chamada muda entao, nunca o hook em si).
    const bool allowed = gus::domain::save::autosave_allowed_at(
        gus::domain::save::LocationKind::City, /*has_pem_discovered=*/false,
        /*has_faraday_card=*/false);
    if (!allowed) {
        std::cout << "Maestro: [autosave][" << trigger_label
                  << "] bloqueado pela politica de local (dungeon sem PEM/carta "
                     "Faraday).\n";
        return;
    }

    gus::domain::save::SaveData data = build_current_save_data();
    data.slot_id = gus::domain::save::kAutosaveSlot;
    const std::string saves_dir = gus::platform::fs::resolve_saves_dir();
    const bool ok = gus::platform::fs::save_game(data, gus::domain::save::kAutosaveSlot,
                                                  saves_dir);
    std::cout << "Maestro: [autosave][" << trigger_label << "] "
              << (ok ? "OK" : "FALHOU (I/O - disco cheio/permissao?)")
              << " - slot Auto (" << gus::domain::save::kAutosaveSlot << ") em "
              << saves_dir << ".\n";
}

void Maestro::handle_new_game_selected(
    bool reached_via_pause, gus::domain::save::DifficultyLevel difficulty) {
    if (reached_via_pause) {
        // MENU-INICIAL (wiring final): esta tela de titulo NAO foi alcancada pelo
        // boot - havia uma partida rodando em memoria (Pause -> Menu Inicial ->
        // Sim). reset_to_new_game() zera posicao/save/inimigo/playtime pro
        // equivalente exato de um jogo novo do zero (ver o comentario do metodo no
        // header) ANTES de devolver false (o que faz run() seguir rodando NA
        // CIDADE, agora com estado fresco).
        std::cout << "Maestro: [costura] Novo Jogo via Menu Inicial (pos-pausa) - "
                     "chamando reset_to_new_game().\n";
        reset_to_new_game(difficulty);
        return;
    }
    std::cout << "Maestro: [costura] Novo Jogo - estado FRESCO ja pronto desde "
                 "init() (sem retrabalho).\n";
    // MODOS-MORTE Fase 0 (§3.2): grava a dificuldade ESCOLHIDA na tela de selecao
    // (fixa por save, nunca reescrita depois - a UNICA escrita legitima e aqui,
    // antes do 1o save_game deste save novo).
    save_.difficulty = difficulty;
    std::cout << "Maestro: [modos-morte] dificuldade escolhida = "
              << static_cast<unsigned>(save_.difficulty)
              << " (0=Facil 1=Medio 2=Dificil 3=Hardcore).\n";
}

bool Maestro::show_title_screen(bool reached_via_pause) {
    std::cout << "Maestro: [costura] boot -> TELA DE TITULO (SAVE-LOAD-UI etapa "
                 "4, boot MUDOU - nao entra mais direto na cidade).\n";

    // MESMA tecnica de open_pause_from_city/to_battle: captura o frame ATUAL da
    // cidade (o boot ja deixou sim_ pronto em init(), mesmo sem nenhum step()
    // ainda - capture_frame_to_png redesenha o estado CORRENTE, nao exige um
    // frame anterior) - fundo congelado real (paridade visual estrita, decisao
    // do A1 no passo 4 do plano, INTOCADA aqui).
    const std::string frozen_bg_path = frozen_city_snapshot_path();
    const bool frozen_ok = city_->capture_frame_to_png(frozen_bg_path);

    gus::app::screens::TitleLoopExit exit = gus::app::screens::TitleLoopExit::QuitApp;
    gus::domain::save::SaveData loaded{};
    // MODOS-MORTE Fase 0: default Medio (§2.1) - so e sobrescrito de fato quando
    // exit==NewGame (a tela de selecao de dificuldade sempre roda nesse caminho,
    // ver title_menu_loop.cpp).
    gus::domain::save::DifficultyLevel new_game_difficulty =
        gus::domain::save::DifficultyLevel::Medio;
    // FLASH-CTX (A3, passo 5 do plano): o contexto GL UNICO da Maestro (gl_context_)
    // JA E o corrente do boot ao shutdown - a tela de titulo desenha DIRETO nele
    // (run_title_menu_loop_gl_current, o nucleo que o A2 extraiu), sem criar/destruir
    // contexto GL algum. Substitui run_title_menu_loop_owning_gl + city_->
    // release_renderer()/reacquire_renderer() + o SDL_GL_MakeCurrent de restauracao
    // (a PONTE TEMPORARIA do A1 nao existe mais - nao ha mais NENHUMA troca de
    // contexto pra mascarar: a cidade segue desenhada por baixo o tempo todo). Sem
    // retorno bool: a criacao do contexto ja foi resolvida em init() (se tivesse
    // falhado, o app nunca teria chegado aqui) - o unico modo de degradacao que resta
    // e interno ao proprio loop (glintfx::UiLayer::ok()==false), ja tratado dentro
    // dele (out_exit cai pra NewGame, mesmo padrao de sempre).
    gus::app::screens::run_title_menu_loop_gl_current(
        window_, audio_, translator_, gus::platform::fs::resolve_saves_dir(), &exit,
        &loaded, &new_game_difficulty, frozen_ok ? frozen_bg_path : std::string());

    if (frozen_ok) {
        std::error_code remove_ec;
        std::filesystem::remove(frozen_bg_path, remove_ec);
    }

    switch (exit) {
        case gus::app::screens::TitleLoopExit::QuitApp:
            std::cout << "Maestro: [costura] Sair na tela de titulo (ou janela "
                         "fechada) - encerrando SEM jogar (nenhum autosave).\n";
            return true;
        case gus::app::screens::TitleLoopExit::NewGame:
            handle_new_game_selected(reached_via_pause, new_game_difficulty);
            return false;
        case gus::app::screens::TitleLoopExit::ContinueGame:
            std::cout << "Maestro: [costura] Continuar - aplicando o save mais "
                         "recente entre todos os slots ocupados.\n";
            apply_loaded_save_data(loaded);
            return false;
    }
    return false;
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

    const std::string settings_dir = gus::platform::fs::resolve_settings_dir();

    // SAVE-LOAD-UI etapa 6 (wiring REAL): os 2 callbacks que dao a tela de save/
    // load acesso ao SaveData VIVO - so a Maestro conhece flags/posicao/tempo de
    // jogo de verdade, entao SO ela pode montar/aplicar um SaveData de fato (ver
    // o header de system_menu_loop.hpp e save_load_menu_loop.hpp pro contrato).
    // EXTRAIDOS pra metodos (build_current_save_data/apply_loaded_save_data,
    // SAVE-LOAD-UI etapa 4/5): maybe_autosave() e show_title_screen() ("Continuar"
    // na tela de titulo) reusam a MESMA logica, sem duplicar.
    const auto build_current_save_data = [this] { return this->build_current_save_data(); };
    const auto apply_loaded_save_data = [this](const gus::domain::save::SaveData& data) {
        this->apply_loaded_save_data(data);
    };

    // FLASH-CTX (A3, passo 5 do plano): contexto GL UNICO ja corrente - o menu (e a
    // tela de Save/Load ANINHADA dentro dele, mesmo contexto) desenha DIRETO nele
    // (run_system_menu_loop_gl_current), sem criar/destruir contexto GL. Substitui
    // run_system_menu_loop_owning_gl + o SDL_GL_MakeCurrent de restauracao (a PONTE
    // TEMPORARIA do A1 nao existe mais). O MENU-PAUSA-FLASH-FIX (hold_frozen_frame)
    // tambem sai: aquele metodo MASCARAVA o pisca do SDL_Renderer recriado por
    // reacquire_renderer() - sem essa recriacao (a cidade nunca para de desenhar no
    // MESMO contexto), nao ha mais nada pra mascarar. O flash morreu na RAIZ (Opcao
    // C do plano), nao no sintoma.
    const gus::app::screens::SystemMenuLoopOutcome outcome =
        gus::app::screens::run_system_menu_loop_gl_current(
            window_, audio_, translator_, settings_dir,
            gus::platform::fs::resolve_saves_dir(), build_current_save_data,
            apply_loaded_save_data, frozen_ok ? frozen_bg_path : std::string());

    // M2 (GAP FINAL) -> M2 STAGED CHANGES: RELE o controls.json e realimenta o
    // SdlInput da cidade - aplica o remap SEM exigir restart. O jogador pode
    // ter passado por Controles, confirmado "Aplicar" (persist_controls, so
    // ESSE botao escreve em controls.json agora - ver system_menu_loop.cpp) e
    // voltado direto pra Continuar; INCONDICIONAL (mesmo se nada mudou/nada
    // foi aplicado) porque load_controls e barato e sempre degrada com
    // seguranca (arquivo ausente/corrompido -> default_controls(), nunca
    // lanca) - reler de mais nunca corrompe nada, so confirma o estado do
    // disco (que so mudou se o jogador de fato aplicou).
    city_->set_controls(gus::platform::fs::load_controls(
        settings_dir, std::string(gus::domain::input::kDefaultProfile)));

    // Higiene (AC-E3): apaga o snapshot congelado ao fechar o menu - a proxima
    // abertura sobrescreve de qualquer forma (dentro do dir 0700 o risco de symlink
    // ja nao existe), mas nao ha motivo pra deixar o PNG do ultimo frame parado em
    // disco entre uma abertura e outra. Falha silenciosa (arquivo pode nao existir
    // se frozen_ok==false).
    if (frozen_ok) {
        std::error_code remove_ec;
        std::filesystem::remove(frozen_bg_path, remove_ec);
    }

    if (outcome.to_title) {
        // MENU-INICIAL: "Sim" confirmado no mini-dialogo "voltar ao menu
        // inicial?" - volta pra TELA DE TITULO (Novo Jogo/Continuar/Sair, a
        // MESMA tela do boot, ver show_title_screen()) SEM encerrar o processo -
        // diferente de PauseItem::Quit acima (RequestQuit -> outcome.quit_app,
        // fecha o jogo). show_title_screen() ja desenha DIRETO no MESMO contexto
        // GL UNICO da Maestro (FLASH-CTX) - nenhuma criacao/destruicao de
        // contexto acontece aqui, MESMA tecnica de qualquer outra transicao de
        // cena da Maestro. Devolve o que show_title_screen() devolver: true SO
        // se o jogador escolheu Sair OU fechou a janela DENTRO da tela de
        // titulo (o chamador de run() encerra o programa, MESMO contrato de
        // sempre); false (Novo Jogo/Continuar escolhidos) faz o loop de run()
        // seguir rodando NA CIDADE ATUAL. `reached_via_pause=true` (MENU-INICIAL,
        // wiring final): este caminho JA tinha uma partida rodando em memoria -
        // "Novo Jogo" aqui chama reset_to_new_game() por dentro de
        // show_title_screen() antes de devolver, pra nao herdar posicao/
        // progresso/inimigo/playtime da sessao anterior (ver o comentario do
        // parametro no header).
        return show_title_screen(/*reached_via_pause=*/true);
    }

    return outcome.quit_app;
}

void Maestro::run() {
    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 6, prova visual headless Xvfb :99):
    // GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir> abre o MENU DE PAUSA diretamente no
    // boot (sem esperar um Esc real do jogador - Xvfb nao tem jogador nenhum) -
    // o proprio open_pause_from_city() carrega o self-test de screenshot mais
    // fundo (ver o comentario grande em system_menu_loop.cpp), que gera os 2 PNGs
    // (save_load_save.png/save_load_load.png) e retorna. Sai do processo logo em
    // seguida (bypassa por completo o loop interativo normal).
    if (const char* saveload_screenshot_dir =
            std::getenv("GUSWORLD_SAVELOAD_SCREENSHOT_DIR");
        saveload_screenshot_dir != nullptr && saveload_screenshot_dir[0] != '\0') {
        (void)open_pause_from_city();
        return;
    }

    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 5, AUTOSAVE): GUSWORLD_AUTOSAVE_
    // SELFTEST=1 exercita maybe_autosave() DIRETO (bypassa a tela de titulo e o
    // loop de jogo por completo) - prova via log + has_save() ANTES/DEPOIS que
    // um gatilho de fato cria o arquivo do slot Auto (headless, sem input real).
    // Rodar com GUSWORLD_HOME apontando pra um dir de SCRATCH (nao os saves reais
    // do jogador).
    if (const char* autosave_selftest = std::getenv("GUSWORLD_AUTOSAVE_SELFTEST");
        autosave_selftest != nullptr && autosave_selftest[0] != '\0') {
        const std::string saves_dir = gus::platform::fs::resolve_saves_dir();
        std::cout << "Maestro: [autosave][selftest] ANTES: has_save(Auto)="
                  << gus::platform::fs::has_save(gus::domain::save::kAutosaveSlot,
                                                  saves_dir)
                  << " em " << saves_dir << "\n";
        maybe_autosave("selftest");
        std::cout << "Maestro: [autosave][selftest] DEPOIS: has_save(Auto)="
                  << gus::platform::fs::has_save(gus::domain::save::kAutosaveSlot,
                                                  saves_dir)
                  << "\n";
        return;
    }

    // DIAGNOSTICO/PROVA (MODOS-MORTE Fase 0, Facil ponta-a-ponta): GUSWORLD_
    // FACIL_RELOAD_SELFTEST=1 exercita on_battle_result(Defeat) DIRETO (bypassa a
    // tela de titulo/dificuldade e o loop de jogo por completo) - prova, via
    // log + posicao do jogador ANTES/DEPOIS, que uma Defeat em modo Facil
    // reload o save mais recente (gravado com uma posicao DISTINTA da posicao
    // "vivida" na hora da derrota simulada). Headless, sem GL/input real. Rodar
    // com GUSWORLD_HOME apontando pra um dir de SCRATCH.
    if (const char* facil_reload_selftest = std::getenv("GUSWORLD_FACIL_RELOAD_SELFTEST");
        facil_reload_selftest != nullptr && facil_reload_selftest[0] != '\0') {
        const std::string saves_dir = gus::platform::fs::resolve_saves_dir();

        // Simula um save PRE-EXISTENTE (posicao (111,222)) - o "ultimo save" que
        // o reload do Facil deve restaurar.
        gus::domain::save::SaveData pre_existing = build_current_save_data();
        pre_existing.difficulty = gus::domain::save::DifficultyLevel::Facil;
        pre_existing.player_position = gus::domain::save::Vec3{111.0, 222.0, 0.0};
        pre_existing.slot_id = gus::domain::save::kAutosaveSlot;
        (void)gus::platform::fs::save_game(pre_existing, gus::domain::save::kAutosaveSlot,
                                            saves_dir);

        // Estado VIVO diverge do save (o jogador "andou" ate (999,999) depois de
        // salvar, e entao MORREU ali) - simula a sessao real entre o save e a
        // derrota.
        save_.difficulty = gus::domain::save::DifficultyLevel::Facil;
        gus::core::spatial::Aabb moved = city_->player_aabb();
        moved.x = 999.0f;
        moved.y = 999.0f;
        city_->set_player_position(moved);

        std::cout << "Maestro: [modos-morte][selftest] ANTES (posicao vivida no "
                     "momento da derrota): player=("
                  << city_->player_aabb().x << ", " << city_->player_aabb().y << ")\n";
        on_battle_result(gus::domain::combat::CombatOutcome::Defeat);
        std::cout << "Maestro: [modos-morte][selftest] DEPOIS (reload do ultimo "
                     "save aplicado): player=("
                  << city_->player_aabb().x << ", " << city_->player_aabb().y
                  << ") esperado=(111, 222)\n";
        return;
    }

    // DIAGNOSTICO/PROVA (MENU-INICIAL, ACHADO 1): GUSWORLD_NEWGAME_RESET_SELFTEST=1
    // exercita reset_to_new_game() DIRETO (bypassa a tela de titulo/pausa e o loop de
    // jogo por completo) - prova, via log + posicao/save/inimigo/playtime ANTES/
    // DEPOIS, que o metodo devolve o estado de partida ao equivalente do BOOT MESMO
    // depois de "sujar" o estado em memoria (posicao andada, dificuldade/credits/flag
    // setados, inimigo derrotado, playtime acumulado, edge-triggers ligados) - o
    // cenario EXATO do bug relatado (Pause -> Menu Inicial -> titulo -> Novo Jogo NAO
    // resetava nada). Headless (so mexe em campos POCO/city_->set_player_position,
    // MESMA tecnica dos 2 selftests acima). Rodar com GUSWORLD_HOME apontando pra um
    // dir de SCRATCH.
    if (const char* newgame_reset_selftest =
            std::getenv("GUSWORLD_NEWGAME_RESET_SELFTEST");
        newgame_reset_selftest != nullptr && newgame_reset_selftest[0] != '\0') {
        // "Suja" o estado em memoria - simula uma sessao real jogada ANTES do reset.
        gus::core::spatial::Aabb moved = city_->player_aabb();
        moved.x = player_spawn_aabb_.x + 500.0f;
        moved.y = player_spawn_aabb_.y + 500.0f;
        city_->set_player_position(moved);
        save_.difficulty = gus::domain::save::DifficultyLevel::Dificil;
        save_.credits = 999;
        save_.flags["chave_teste_sujando_estado"] = true;
        enemy_defeated_ = true;
        city_->clear_enemy_marker();
        was_overlapping_enemy_ = true;
        was_overlapping_npc_bertoldo_ = true;
        playtime_base_seconds_ = 12345.0;
        playtime_anchor_ns_ = SDL_GetTicksNS();

        std::cout << "Maestro: [newgame-reset][selftest] ANTES (estado 'sujo', "
                     "simulando sessao jogada): player=("
                  << city_->player_aabb().x << ", " << city_->player_aabb().y
                  << ") credits=" << save_.credits
                  << " enemy_defeated=" << enemy_defeated_
                  << " playtime_seconds=" << playtime_base_seconds_ << "\n";

        reset_to_new_game(gus::domain::save::DifficultyLevel::Facil);

        const bool position_ok = city_->player_aabb().x == player_spawn_aabb_.x &&
                                  city_->player_aabb().y == player_spawn_aabb_.y;
        const bool save_ok =
            save_ == gus::domain::save::fresh_new_game_save_data(
                         gus::domain::save::DifficultyLevel::Facil);
        std::cout << "Maestro: [newgame-reset][selftest] DEPOIS (reset_to_new_game "
                     "aplicado): player=("
                  << city_->player_aabb().x << ", " << city_->player_aabb().y
                  << ") esperado=(" << player_spawn_aabb_.x << ", "
                  << player_spawn_aabb_.y << ") posicao_ok=" << position_ok
                  << " save_ok=" << save_ok << " enemy_defeated=" << enemy_defeated_
                  << " was_overlapping_enemy=" << was_overlapping_enemy_
                  << " playtime_seconds=" << playtime_base_seconds_
                  << " (esperado 0)\n";
        return;
    }

    // DIAGNOSTICO/PROVA (MENU-INICIAL, wiring final): GUSWORLD_NEWGAME_VIA_TITLE_
    // SELFTEST=1 exercita handle_new_game_selected() DIRETO (bypassa a tela de
    // titulo/pausa e o loop de jogo por completo, MESMA tecnica do selftest acima -
    // headless, so mexe em campos POCO/city_->set_player_position) - prova as DUAS
    // metades do wiring que o selftest acima NAO cobre (aquele exercita
    // reset_to_new_game() isolado; este exercita o METODO QUE show_title_screen()
    // REALMENTE CHAMA no case NewGame, com os 2 valores de reached_via_pause):
    //   1) reached_via_pause=false (BOOT) NAO reseta - so grava a dificuldade -
    //      estado "sujo" simulado permanece sujo (posicao/credits/flag intactos);
    //   2) reached_via_pause=true (POS-PAUSA, Menu Inicial) RESETA por completo -
    //      equivalente ao GUSWORLD_NEWGAME_RESET_SELFTEST acima, so que passando
    //      pelo mesmo metodo que o wiring real usa.
    if (const char* newgame_via_title_selftest =
            std::getenv("GUSWORLD_NEWGAME_VIA_TITLE_SELFTEST");
        newgame_via_title_selftest != nullptr && newgame_via_title_selftest[0] != '\0') {
        const auto dirty_state = [this] {
            gus::core::spatial::Aabb moved = city_->player_aabb();
            moved.x = player_spawn_aabb_.x + 500.0f;
            moved.y = player_spawn_aabb_.y + 500.0f;
            city_->set_player_position(moved);
            save_.difficulty = gus::domain::save::DifficultyLevel::Dificil;
            save_.credits = 999;
            save_.flags["chave_teste_sujando_estado"] = true;
            enemy_defeated_ = true;
            city_->clear_enemy_marker();
            was_overlapping_enemy_ = true;
            was_overlapping_npc_bertoldo_ = true;
            playtime_base_seconds_ = 12345.0;
            playtime_anchor_ns_ = SDL_GetTicksNS();
        };

        // PROVA VISUAL (opcional, MESMA tecnica PASSIVA de captura de frame que
        // frozen_city_snapshot_path/capture_frame_to_png ja usam em qualquer
        // outro lugar da Maestro - glReadPixels sobre o framebuffer, ZERO
        // automacao de foco/teclado/mouse de janela - ver a memoria global
        // feedback_nao_automacao_gui_ambiente_medico: agentes SO headless/
        // offscreen, nunca xdotool/wmctrl/ydotool). GUSWORLD_NEWGAME_VIA_TITLE_
        // SCREENSHOT_DIR=<dir> (opcional) salva 1 PNG do estado "sujo" (posicao
        // longe do spawn) e 1 do estado pos-reset (posicao NO spawn), pra
        // conferencia visual sem depender de injecao de input real.
        const char* screenshot_dir =
            std::getenv("GUSWORLD_NEWGAME_VIA_TITLE_SCREENSHOT_DIR");
        const bool want_screenshots = screenshot_dir != nullptr && screenshot_dir[0] != '\0';
        const auto snapshot = [this, screenshot_dir](const char* suffix) {
            const std::string path =
                std::string(screenshot_dir) + "/newgame_via_title_" + suffix + ".png";
            const bool ok = city_->capture_frame_to_png(path);
            std::cout << "Maestro: [newgame-via-title][selftest][screenshot] " << path
                      << " (" << (ok ? "OK" : "FALHOU") << ")\n";
        };

        // 1) reached_via_pause=false (BOOT) - "suja" o estado e chama o handler
        // como show_title_screen(/*reached_via_pause=*/false) chamaria - o estado
        // deve permanecer SUJO (so a dificuldade muda).
        dirty_state();
        if (want_screenshots) snapshot("1_boot_before_dirty");
        handle_new_game_selected(/*reached_via_pause=*/false,
                                  gus::domain::save::DifficultyLevel::Facil);
        if (want_screenshots) snapshot("2_boot_after_no_reset_still_dirty");
        const bool boot_position_untouched =
            city_->player_aabb().x == player_spawn_aabb_.x + 500.0f &&
            city_->player_aabb().y == player_spawn_aabb_.y + 500.0f;
        const bool boot_credits_untouched = save_.credits == 999;
        const bool boot_enemy_untouched = enemy_defeated_ == true;
        std::cout << "Maestro: [newgame-via-title][selftest] (1) reached_via_pause="
                     "false (BOOT): posicao_intacta=" << boot_position_untouched
                  << " credits_intacto=" << boot_credits_untouched
                  << " enemy_defeated_intacto=" << boot_enemy_untouched
                  << " dificuldade_gravada=" << static_cast<unsigned>(save_.difficulty)
                  << " (esperado TODOS true/0=Facil).\n";

        // 2) reached_via_pause=true (POS-PAUSA, Menu Inicial) - "suja" de novo e
        // chama o handler como show_title_screen(/*reached_via_pause=*/true)
        // chamaria (via open_pause_from_city()) - o estado deve voltar ao
        // equivalente de um jogo novo do zero.
        dirty_state();
        if (want_screenshots) snapshot("3_pause_before_dirty");
        handle_new_game_selected(/*reached_via_pause=*/true,
                                  gus::domain::save::DifficultyLevel::Facil);
        if (want_screenshots) snapshot("4_pause_after_reset_at_spawn");
        const bool pause_position_ok = city_->player_aabb().x == player_spawn_aabb_.x &&
                                        city_->player_aabb().y == player_spawn_aabb_.y;
        const bool pause_save_ok =
            save_ == gus::domain::save::fresh_new_game_save_data(
                         gus::domain::save::DifficultyLevel::Facil);
        std::cout << "Maestro: [newgame-via-title][selftest] (2) reached_via_pause="
                     "true (POS-PAUSA): posicao_ok=" << pause_position_ok
                  << " save_ok=" << pause_save_ok
                  << " enemy_defeated=" << enemy_defeated_
                  << " playtime_seconds=" << playtime_base_seconds_
                  << " (esperado TODOS true/false/0).\n";
        return;
    }

    // DIAGNOSTICO/PROVA (SAVE-LOAD-UI etapa 4, prova visual headless Xvfb :99):
    // GUSWORLD_TITLE_SCREENSHOT_DIR=<dir> mostra a TELA DE TITULO diretamente no
    // boot (o proprio show_title_screen()/run_title_menu_loop_owning_gl carrega o
    // self-test de screenshot, ver title_menu_loop.cpp) e retorna - bypassa por
    // completo o loop de jogo. Rodar 2x com 2 GUSWORLD_HOME diferentes (1 com
    // save gravado, 1 vazio) produz os 2 PNGs pedidos (Continuar habilitado vs
    // desabilitado), cada um com nome PROPRIO (nao colidem).
    if (const char* title_screenshot_dir = std::getenv("GUSWORLD_TITLE_SCREENSHOT_DIR");
        title_screenshot_dir != nullptr && title_screenshot_dir[0] != '\0') {
        (void)show_title_screen();
        return;
    }

    // DIAGNOSTICO/PROVA (MODOS-MORTE Fase 0, prova visual headless Xvfb :99):
    // GUSWORLD_DIFFICULTY_SCREENSHOT_DIR=<dir> mostra a TELA DE SELECAO DE
    // DIFICULDADE diretamente no boot - show_title_screen() cria o contexto GL
    // owning normal (run_title_menu_loop_owning_gl), que detecta a MESMA
    // variavel e pula reto pra tela de dificuldade (ANINHADA, ver
    // title_menu_loop.cpp) ANTES de sequer montar a tela de titulo - bypassa por
    // completo o loop de jogo, MESMO espirito de GUSWORLD_TITLE_SCREENSHOT_DIR.
    if (const char* difficulty_screenshot_dir =
            std::getenv("GUSWORLD_DIFFICULTY_SCREENSHOT_DIR");
        difficulty_screenshot_dir != nullptr && difficulty_screenshot_dir[0] != '\0') {
        (void)show_title_screen();
        return;
    }

    // SAVE-LOAD-UI etapa 4: o boot MUDOU - a TELA DE TITULO mostra ANTES do loop
    // cidade<->batalha (nao entra mais direto na cidade). "Sair" na tela de
    // titulo (ou fechar a janela durante ela) encerra o programa NA HORA, SEM
    // entrar no loop de jogo - nada foi jogado ainda, nenhum autosave faz
    // sentido nesse caso (ver o comentario de show_title_screen()).
    if (show_title_screen()) {
        return;
    }

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

    // SAVE-LOAD-UI etapa 5 (AUTOSAVE), gatilho (c) "ao sair pra tela de titulo /
    // fechar o jogo": chegar AQUI so acontece depois que show_title_screen() ja
    // devolveu false acima (o jogador de fato ENTROU a jogar) - cobre TODAS as
    // formas de encerrar (Sair no menu de pausa, fechar a janela na cidade/
    // batalha/dialogo, etc.), unificadas num UNICO ponto (nao precisa duplicar
    // em cada `running = false` acima).
    maybe_autosave("saindo_do_jogo");
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

    // SAVE-LOAD-UI etapa 5 (AUTOSAVE), gatilho (b) "ao trocar de area
    // entrar/sair": nesta fatia a UNICA fronteira de area e cidade<->batalha -
    // aqui e o lado "sair da cidade". Disparado ANTES de qualquer fade/troca de
    // renderer (city_ ainda 100% intacto) pra capturar o estado mais recente
    // possivel mesmo se o jogador fechar a janela no meio da transicao abaixo.
    maybe_autosave("entrando_em_batalha");

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

    // TROCA ESCONDIDA ATRAS DO PRETO (FLASH-CTX, A3, passo 5 do plano): a cidade E a
    // batalha compartilham o MESMO contexto GL UNICO/persistente da Maestro (nunca
    // muda de dono - substitui "PAUSA a cidade pra deixar a janela livre pro contexto
    // GL PROPRIO da batalha" da era da PONTE TEMPORARIA, ver historico git). A arena
    // desenha DIRETO nesse contexto via run_battle_preview_embedded_gl_current (o
    // nucleo que o A2 extraiu), sem criar/destruir contexto GL algum. A tela ja esta
    // 100% preta aqui (fade acima), entao a troca de TELA (cidade->arena) segue
    // invisivel, so que agora sem NENHUMA troca de CONTEXTO por baixo.
    gus::domain::combat::CombatOutcome outcome =
        gus::domain::combat::CombatOutcome::Ongoing;
    bool quit_requested = false;
    // M7-COSTURA Inc 2: passa o AudioEngine DELA (ponteiro nao-dono - a battle_preview
    // so usa pro SFX do hit + o fade visual PROPRIO da tela de batalha, nunca musica) +
    // os DOIS fades visuais da batalha (entrada clareando, saida escurecendo). Sem
    // retorno de erro: a criacao do contexto ja foi resolvida em init() (ver o
    // comentario de show_title_screen() acima pro mesmo racional).
    gus::app::screens::run_battle_preview_embedded_gl_current(
        window_, &outcome, &quit_requested, &audio_, kTransitionFadeSeconds,
        kTransitionFadeSeconds);

    if (quit_requested) {
        // FIX BUG-3: o jogador fechou a janela DENTRO da batalha. NAO volta pra cidade
        // (on_battle_result e PULADO de proposito - a janela vai ser destruida pelo
        // dtor da Maestro de qualquer jeito, aplicar o outcome na cidade agora so pra
        // descarta-la em seguida seria trabalho inutil). O outcome fica descartado
        // (nao importa mais: o programa esta encerrando).
        std::cout << "Maestro: [costura] janela fechada DURANTE a batalha -> "
                     "encerrando o programa (sem voltar pra cidade).\n";
        return true;
    }

    on_battle_result(outcome);

    // SAVE-LOAD-UI etapa 5 (AUTOSAVE), gatilho (a) "apos cada batalha VENCIDA":
    // AJUSTE do lider (pos-entrega, via AskUserQuestion) - o predicado PURO/
    // testavel should_autosave_after_battle (maestro_logic.hpp) decide: SO
    // Victory autosava no retorno; Defeat/Fled/Ongoing (janela fechada) NAO. O
    // gatilho de ENTRADA (entrando_em_batalha, acima) continua incondicional -
    // ja cobre o "trocar de area" (b) pro lado cidade->batalha independente do
    // desfecho, entao NAO ha lacuna de cobertura em (b) ao restringir este aqui
    // a Victory. Ver o comentario do predicado pro racional completo (sistema
    // de morte canonico ainda nao implementado).
    if (should_autosave_after_battle(outcome)) {
        maybe_autosave("retornando_da_batalha_vitoria");
    }

    // CROSSFADE DE VOLTA + FADE-IN sobre a CIDADE (Inc 2): mesma receita, sentido
    // inverso - a batalha ja fez o SEU fade-out (kOut, dentro de
    // run_battle_preview_embedded_gl_current, tela preta ao voltar aqui); o crossfade
    // dispara agora (ainda preto - FLASH-CTX: a cidade nunca parou de desenhar no
    // MESMO contexto, entao nao ha mais "reconstruir o renderer" antes disto) e a
    // cidade clareia. Inc 3: sempre mira city_music_id_ (o tema da cidade, existe
    // desde sempre) - mesma degradacao segura de antes se estiver invalido (no-op
    // silencioso).
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
    // city - captura o frame ATUAL da cidade; a caixa de dialogo desenha essa cena
    // REAL como fundo estatico, no lugar da vinheta abstrata (mesmo padrao de
    // Chrono Trigger/Zelda/Stardew Valley). frozen_ok==false degrada pra vinheta de
    // sempre (ver run_npc_dialogue_loop_gl_current).
    const std::string frozen_bg_path = frozen_city_snapshot_path();
    const bool frozen_ok = city_->capture_frame_to_png(frozen_bg_path);

    // DIALOGO-TERMINAL (FLASH-CTX, A3, passo 5 do plano): contexto GL UNICO ja
    // corrente - o dialogo desenha DIRETO nele (run_npc_dialogue_loop_gl_current, o
    // nucleo que o A2 extraiu), sem criar/destruir contexto GL. Substitui
    // run_npc_dialogue_loop_gl (a casca owning) + o SDL_GL_MakeCurrent de
    // restauracao (a PONTE TEMPORARIA do A1 nao existe mais). O nucleo _gl_current
    // NAO chama SdlWindow::clear_input() sozinho (fica no CHAMADOR que possui o
    // contexto, ver o header) - clear_input() ao ENTRAR e ao SAIR continua aqui,
    // MESMO fix de sempre (BUG "Gus anda sozinho apos fechar o dialogo").
    city_->clear_input();
    const bool quit_requested = gus::app::screens::run_npc_dialogue_loop_gl_current(
        window_, runtime, translator_, audio_,
        frozen_ok ? frozen_bg_path : std::string());
    city_->clear_input();

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
    } else if (outcome == gus::domain::combat::CombatOutcome::Defeat &&
               should_reload_last_save_on_defeat(save_.difficulty)) {
        // MODOS-MORTE Fase 0 (docs/design/mecanicas/modos-morte.md §3.3): dispatcher
        // central de morte, ramo Facil - substitui o placeholder do M7 SO pra este
        // modo. Medio/Dificil/Hardcore (should_reload_last_save_on_defeat==false)
        // caem no comentario abaixo (placeholder inalterado).
        reload_most_recent_save_on_defeat();
    }
    // Fled/Ongoing (janela fechada no meio)/Defeat em Medio-Dificil-Hardcore: NAO
    // marca - o inimigo continua no mapa, o jogador volta pro MESMO ponto (o
    // OverworldSim nunca foi destruido/recarregado - ficou vivo e pausado durante a
    // batalha) e pode tentar de novo. O flavor da derrota (reboot/bark/tela-xadrez) e
    // o Incremento 3; os modos de morte proprios de Medio/Dificil/Hardcore sao fases
    // futuras (modos-morte.md §6).
}

void Maestro::reload_most_recent_save_on_defeat() {
    const std::string saves_dir = gus::platform::fs::resolve_saves_dir();

    // MESMA varredura de disco de show_title_screen()/"Continuar" (TitleDiskScan em
    // title_menu_loop.cpp, nao exportada) - reusa most_recent_occupied_slot em vez
    // de inventar uma 2a nocao de "ultimo save" (ex.: so o autosave). Um save
    // PRESENTE mas nao Ok degrada como vazio (MESMA politica da tela de titulo).
    std::array<gus::app::screens::SaveSlotPreview, gus::domain::save::kSlotCount>
        previews{};
    std::array<std::optional<gus::domain::save::SaveData>,
               gus::domain::save::kSlotCount>
        loaded{};
    for (int slot = 0; slot < gus::domain::save::kSlotCount; ++slot) {
        if (!gus::platform::fs::has_save(slot, saves_dir)) {
            previews[static_cast<std::size_t>(slot)] =
                gus::app::screens::empty_slot_preview(slot);
            continue;
        }
        const auto outcome = gus::platform::fs::load_game(slot, saves_dir);
        if (outcome.has_value() &&
            outcome->result == gus::domain::save::LoadResult::Ok) {
            previews[static_cast<std::size_t>(slot)] =
                gus::app::screens::build_slot_preview(outcome->data, slot);
            loaded[static_cast<std::size_t>(slot)] = outcome->data;
        } else {
            previews[static_cast<std::size_t>(slot)] =
                gus::app::screens::empty_slot_preview(slot);
        }
    }

    const int best_slot = gus::app::screens::most_recent_occupied_slot(previews);
    if (best_slot < 0 || !loaded[static_cast<std::size_t>(best_slot)].has_value()) {
        // Degradacao segura: jogo recem-comecado, nenhum save Ok gravado ainda -
        // nao ha "ultimo save" pra reload. Segue no placeholder atual (o inimigo
        // permanece, o jogador volta pro mesmo ponto - o Defeat ja rodou o flavor
        // da derrota antes de chegar aqui).
        std::cout << "Maestro: [modos-morte][Facil] Defeat sem nenhum save Ok "
                     "gravado ainda - sem 'ultimo save' pra reload, segue no "
                     "placeholder atual.\n";
        return;
    }

    std::cout << "Maestro: [modos-morte][Facil] Defeat -> reload do save mais "
                 "recente (slot "
              << best_slot << ").\n";
    apply_loaded_save_data(*loaded[static_cast<std::size_t>(best_slot)]);
}

}  // namespace gus::app
