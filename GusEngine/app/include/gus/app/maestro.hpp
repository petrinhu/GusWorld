// gus/app/maestro.hpp
//
// Maestro: orquestracao LEVE cidade<->batalha (M7-COSTURA, ADR-012 Onda 1). NAO e um
// gerenciador de cenas generico (anti over-engineering, decisao 4 do ADR-012) - so
// alterna entre as DUAS telas que existem hoje (city_scene via OverworldSim/SdlWindow,
// battle_scene via run_battle_preview_embedded) por um contrato pequeno: "contra quem"
// (to_battle) e "ganhou/perdeu" (on_battle_result).
//
// DONA de: a janela + o CONTEXTO GL UNICO (FLASH-CTX, A1+A3 - docs/tech/pivot/
// menu-flash-contexto-unico-plano.md: um SO contexto GL 3.3 core/stencil 8, criado UMA
// vez em init() e vivo ate o dtor, servindo cidade (Render2dGl3) E menus/dialogo/batalha
// (glintfx::UiLayer) DIRETO nele, via as variantes `_gl_current` - to_battle()/
// open_pause_from_city()/show_title_screen()/to_npc_dialogue() nao criam/destroem
// contexto GL algum, passo 5 do plano); SUPERA por completo a tecnica antiga "trocar por
// baixo entre SDL_Renderer da cidade e contexto GL da batalha/menu, NA MESMA SDL_Window"
// descrita no historico do M7-COSTURA Onda 1 - a troca de contexto era a CAUSA RAIZ do
// flash ao fechar o menu de pausa, ver o plano; a instancia VIVA de OverworldSim (dentro de SdlWindow, mantida viva e
// PAUSADA - sem step_fixed - durante a batalha, entao a posicao do Gus e de GRACA: nao
// ha reposicionamento/serializacao, a cidade nunca foi destruida); e o estado em memoria
// que sobrevive a troca (flags de encontro - o I/O real em disco e M2-SAVE-IO, Onda 2).
//
// ESCOPO DO INCREMENTO 1 (esqueleto do loop, NAO o milestone inteiro): 1 inimigo FIXO no
// mapa; colisao dispara a batalha; Victory some o inimigo (flag em memoria); Defeat/Fled
// voltam pra cidade com o inimigo intacto (pode tentar de novo). NAO incluiu (pago no
// Incremento 2, ver abaixo): fade preto + crossfade de musica (era corte seco); a posse
// do AudioEngine subindo pra ca. A regra GUS-CENTRIC de fim de combate (o Rei cai = fim
// IMEDIATO, mesmo com companions vivos) ja vivia no motor desde o BUG-4 (ver
// combat_state_machine.cpp::check_end). Incremento 3 (M7-COSTURA, FECHADO): o FLAVOR da
// derrota - overlay "reboot de sistema" (kernel panic + bark do companion + nota-xadrez,
// ver gus/app/screens/battle_scene.hpp::defeat_flavor_active) que segura a tela por um
// tempo ANTES do corte de volta pra cidade (a Maestro so ve o combate como encerrado
// quando battle_preview::run_battle_preview_embedded devolve, o que agora espera o
// overlay - ver o comentario do FIX BUG-2 em battle_preview.cpp).
//
// ESCOPO DO INCREMENTO 2 (M7-COSTURA, ADR-012 decisao 5 + paga a divida do ADR-011
// "AudioEngine e dono da battle_preview"): a Maestro agora e DONA de 1 unica instancia
// do AudioEngine (audio_ abaixo), viva pro loop inteiro (cidade + todas as entradas/
// saidas de batalha - o device nao e mais reaberto a cada entrada). Toca o tema da
// cidade em loop enquanto na cidade; na troca cidade<->batalha, desenha um fade preto
// curto (gus/core/anim/fade_transition.hpp) POR CIMA da tela ATUAL e, no pico da
// opacidade (tela 100% preta), dispara o CROSSFADE de musica (gus/app/maestro_logic.hpp
// ::crossfade_music) - fecha o criterio de saida do M6 ("fade entre telas"). Inc 3
// (M7-COSTURA, mesma onda): a Maestro carrega TAMBEM o tema da arena (kBattleThemeFile,
// gerado via Suno pelo lider) - o crossfade agora cruza pra uma faixa DE VERDADE
// diferente em cada sentido (antes cruzava pra ela mesma, por falta da 2a faixa). A
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
#include <optional>

#include <SDL3/SDL.h>

#include "gus/app/i18n/translator.hpp"
#include "gus/app/maestro_logic.hpp"
#include "gus/app/sdl_window.hpp"
#include "gus/core/anim/fade_transition.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/dialogue/dialogue_graph.hpp"  // M7-DIALOGO (NPC-MVP)
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
    // batalha) + o CONTEXTO GL UNICO (FLASH-CTX, A1: SDL_GL_SetAttribute 3.3 core/
    // doublebuffer/stencil 8 -> SDL_GL_CreateContext -> SDL_GL_MakeCurrent -> gl3_load_
    // functions (glad) -> SDL_GL_SetSwapInterval(1), TUDO uma unica vez aqui - o
    // contexto vive ate o dtor) + a cidade (SdlWindow::init_attached, que agora ASSUME
    // este contexto ja corrente e desenha em Render2dGl3) + posiciona o inimigo fixo.
    // Devolve false se qualquer passo falhar (o main reporta e sai != 0).
    [[nodiscard]] bool init();

    // Loop principal: roda a cidade ate o jogador fechar a janela OU esbarrar no
    // inimigo (dispara to_battle); ao voltar da batalha, segue rodando a cidade no
    // MESMO ponto. Retorna quando a janela fecha (em qualquer tela).
    void run();

private:
    // Troca cidade->batalha (FLASH-CTX, A3, passo 5 do plano - docs/tech/pivot/
    // menu-flash-contexto-unico-plano.md): a cidade E a batalha compartilham o MESMO
    // contexto GL UNICO/persistente da Maestro (gl_context_, nunca muda de dono) - a
    // arena desenha DIRETO nele via run_battle_preview_embedded_gl_current, SEM
    // criar/destruir contexto GL (a PONTE TEMPORARIA do A1 - release_renderer/
    // reacquire_renderer/SDL_GL_MakeCurrent de restauracao - foi removida; ver o
    // historico git pra era anterior). FIX BUG-3 (playtest ao vivo do lider: fechar a
    // janela DURANTE a batalha reabria a cidade em LOOP INFINITO): devolve true se o
    // jogador pediu pra FECHAR A JANELA durante a batalha (sinal DISTINTO de qualquer
    // CombatOutcome) - o chamador (run()) DEVE encerrar o programa na hora, sem
    // aplicar o outcome na cidade (a janela esta fechando de qualquer jeito). false =
    // retorno normal pra cidade (Victory/Defeat/Fled/Ongoing).
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

    // MENU-PAUSA-CONFIG-SOM (M7-COSTURA, INTEGRACAO FINAL). FLASH-CTX (A3, passo 5 do
    // plano): Esc na CIDADE (fora de qualquer modal - a cidade nao tem pilha como a
    // batalha, ver SdlWindow::consume_escape_pressed) abre o MENU DE PAUSA DIRETO no
    // contexto GL UNICO/persistente da Maestro (run_system_menu_loop_gl_current) - SEM
    // criar/destruir contexto GL, SEM pausar a cidade (a PONTE TEMPORARIA do A1 -
    // release_renderer/reacquire_renderer/SDL_GL_MakeCurrent de restauracao - foi
    // removida; ver o historico git pra era anterior). Devolve true se o jogador
    // confirmou "Sair" no menu OU fechou a janela DURANTE ele - o chamador (run())
    // encerra o programa (mesmo contrato/nome de intencao de
    // should_stop_running_after_battle). MENU-INICIAL: "Menu Inicial" confirmado
    // (com "Sim" no mini-dialogo "voltar ao menu inicial?") chama
    // show_title_screen() e devolve O QUE ELA devolver - true SO se o jogador
    // escolher Sair/fechar a janela DENTRO da tela de titulo (o jogo NAO fecha
    // so por ter voltado ao titulo, diferente de "Sair" no menu de pausa).
    [[nodiscard]] bool open_pause_from_city();

    // M7-DIALOGO (NPC-MVP): esbarrou no Bertoldo (rising edge, MESMA tecnica de
    // to_battle() acima) -> abre o loop de dialogo (DIALOGO-TERMINAL, glintfx::UiLayer -
    // gus/app/screens/npc_dialogue_loop_gl.hpp). FLASH-CTX (A3): MESMO padrao de
    // open_pause_from_city() acima - desenha DIRETO no contexto GL UNICO da Maestro
    // (run_npc_dialogue_loop_gl_current), sem criar/destruir contexto. Devolve true SO
    // se o jogador fechou a JANELA durante a conversa (mesmo contrato de to_battle()/
    // open_pause_from_city() - o chamador encerra o programa). Grafo AUSENTE/
    // invalido (degradacao segura, ver init()) -> no-op, devolve false (o
    // esbarrão e ignorado, a cidade continua rodando normalmente).
    [[nodiscard]] bool to_npc_dialogue();

    // SAVE-LOAD-UI etapa 4 (TELA DE TITULO): chamada UMA vez no INICIO de run(),
    // ANTES do loop cidade<->batalha (o boot MUDOU: nao entra mais direto na
    // cidade). FLASH-CTX (A3): MESMO padrao de open_pause_from_city() acima - captura
    // o frame congelado (glReadPixels, ver sdl_window.hpp::capture_frame_to_png) e roda
    // o loop DIRETO no contexto GL UNICO/persistente da Maestro
    // (run_title_menu_loop_gl_current), sem criar/destruir contexto GL. "Continuar"
    // aplica o save mais recente via apply_loaded_save_data(); "Novo Jogo" e um no-op
    // (o estado FRESCO que init() ja deixou pronto ja serve). Devolve true SO se o
    // jogador escolheu "Sair" na tela de titulo OU fechou a JANELA durante ela - o
    // chamador (run()) encerra o programa IMEDIATAMENTE, SEM entrar no loop de jogo
    // (nada foi jogado ainda, nenhum autosave faz sentido nesse caso).
    [[nodiscard]] bool show_title_screen();

    // Monta o SaveData VIVO da sessao ATUAL (flags acumuladas + posicao real do
    // jogador + timestamp/playtime frescos) - EXTRAIDO da lambda local que
    // open_pause_from_city() usava (SAVE-LOAD-UI etapa 6), agora reusado TAMBEM
    // por maybe_autosave() (etapa 5): so a Maestro conhece o SaveData de verdade,
    // entao so ela pode montar um snapshot fiel. Sem efeito colateral (const).
    [[nodiscard]] gus::domain::save::SaveData build_current_save_data() const;

    // Aplica um SaveData JA CARREGADO no jogo VIVO (reposiciona o jogador,
    // re-deriva o marcador/estado do inimigo fixo, re-ancora o relogio de
    // playtime) - EXTRAIDO da lambda local que open_pause_from_city() usava
    // (SAVE-LOAD-UI etapa 6), agora reusado TAMBEM por show_title_screen()
    // ("Continuar" na tela de titulo, etapa 4).
    void apply_loaded_save_data(const gus::domain::save::SaveData& data);

    // SAVE-LOAD-UI etapa 5 (AUTOSAVE): consulta o hook de politica por local
    // (gus::domain::save::autosave_allowed_at, ver save_policy.hpp) e, se
    // permitido, grava build_current_save_data() no slot Auto
    // (gus::domain::save::kAutosaveSlot) via gus::platform::fs::save_game - o
    // MESMO save_game real da etapa 6, nao um caminho separado. Nesta fatia SO
    // existe cidade (LocationKind::City, sempre ON) - os 2 overrides (PEM
    // descoberto/carta Gaiola de Faraday) ficam hardcoded false ATE a 1a dungeon
    // real existir (ver o comentario do proprio hook). `trigger_label` e SO
    // diagnostico (aparece no log, ex. "entrando_em_batalha"/
    // "retornando_da_batalha"/"saindo_do_jogo") - nao afeta a decisao.
    void maybe_autosave(const char* trigger_label);

    // MODOS-MORTE Fase 0 (docs/design/mecanicas/modos-morte.md §3.3, dispatcher
    // central de morte): reload do save MAIS RECENTE entre TODOS os slots
    // ocupados - o MESMO conceito de "Continuar" na tela de titulo
    // (most_recent_occupied_slot, gus/app/screens/save_load_menu.hpp), reusado
    // aqui em vez de inventar uma 2a nocao de "ultimo save" (ex.: so o
    // autosave). Chamado por on_battle_result SO quando outcome==Defeat E
    // should_reload_last_save_on_defeat(save_.difficulty) (Facil, Fase 0
    // ponta-a-ponta). Degradacao segura: nenhum save Ok gravado ainda (jogo
    // recem-comecado) e no-op (loga e segue no placeholder atual do M7 - o
    // inimigo permanece, o jogador volta pro mesmo ponto).
    void reload_most_recent_save_on_defeat();

    SDL_Window* window_ = nullptr;         // dono (a UNICA janela do app)

    // FLASH-CTX (A1, contrato final consumado pelo A3 no passo 5 do plano): o
    // CONTEXTO GL UNICO do processo - criado UMA vez em init(), corrente do boot ao
    // shutdown (destruido no dtor), servindo cidade (Render2dGl3) E menus/dialogo/
    // batalha (glintfx::UiLayer), todos DIRETO nele via as variantes `_gl_current`
    // (to_battle()/open_pause_from_city()/show_title_screen()/to_npc_dialogue()) -
    // NUNCA muda de dono, NUNCA e recriado. Zero SDL_GL_MakeCurrent fora de init()/
    // dtor: a antiga PONTE TEMPORARIA (cascas owning criando/destruindo um contexto
    // PROPRIO por cima deste + o MakeCurrent de restauracao) foi removida - era ela a
    // causa raiz do flash ao fechar o menu (ver docs/tech/pivot/menu-flash-contexto-
    // unico-plano.md).
    SDL_GLContext gl_context_ = nullptr;

    std::unique_ptr<SdlWindow> city_;      // a cidade (OverworldSim vive aqui, sempre)

    // AUDIO (M7-COSTURA Inc 2): a Maestro e DONA - 1 instancia viva pro loop inteiro
    // (paga a divida do ADR-011 "AudioEngine e dono da battle_preview" - o device nao
    // e mais reaberto a cada entrada na batalha). device_active=true tenta o hardware
    // real; degrada com seguranca se indisponivel (mesma API no-op de sempre).
    gus::platform::audio::AudioEngine audio_{/*device_active=*/true};
    // Tema da cidade (kCityThemeFile) e tema da arena (kBattleThemeFile, M7-COSTURA
    // Inc 3), ambos carregados UMA vez em init(). O crossfade (maestro_logic.hpp::
    // crossfade_music) cruza cidade->battle_music_id_ em to_battle() e volta
    // battle->city_music_id_ em on_battle_result() - as DUAS faixas agora sao
    // DIFERENTES de verdade (antes, sem a 2a faixa, cruzava pra ela mesma). Se
    // battle_music_id_ ficar invalido (load falhou - arquivo ausente/corrompido), a
    // Maestro cai de volta pra city_music_id_ nos dois sentidos (mesma degradacao
    // segura que ja existia pra city_music_id_ invalido - crossfade_music/play_music
    // com kInvalidSound e no-op silencioso, nunca crasha).
    gus::platform::audio::SoundId city_music_id_ =
        gus::platform::audio::kInvalidSound;
    gus::platform::audio::SoundId battle_music_id_ =
        gus::platform::audio::kInvalidSound;

    // Inimigo FIXO (item 1 do escopo, ver maestro_logic.hpp): AABB + estado "derrotado"
    // em memoria. dado de app/ - NAO toca o formato .gmap/TileMap. enemy_aabb_ e o
    // FOOTPRINT VISUAL inteiro (usado SO pro quad desenhado, set_enemy_marker abaixo -
    // NAO muda de tamanho/posicao nesta mudanca); enemy_trigger_aabb_ (solucao
    // ARQUITETURAL do lider pos-playtest, ver maestro_logic.hpp::feet_trigger_aabb) e a
    // caixa PEQUENA ancorada nos PES, DESACOPLADA do footprint - e a que decide "perto o
    // bastante pra ativar a batalha" (should_trigger_battle em run(), abaixo).
    gus::core::spatial::Aabb enemy_aabb_{};
    gus::core::spatial::Aabb enemy_trigger_aabb_{};
    bool enemy_defeated_ = false;

    // EDGE-TRIGGER (M7-COSTURA BUG-6): estado "havia overlap jogador x inimigo no frame
    // ANTERIOR". A batalha so dispara na TRANSICAO nao-overlap -> overlap (rising edge,
    // ver should_trigger_battle_on_edge em maestro_logic.hpp) - senao, na FUGA/DERROTA (o
    // inimigo PERMANECE e o jogador volta pra cidade AINDA sobre ele) o overlap
    // continuo re-dispararia a batalha em loop. Comeca false (o jogador nasce longe do
    // inimigo, ver o offset em init()); apos uma batalha que NAO remove o inimigo, e
    // forcado a true (o jogador esta em cima) pra exigir SAIR e RE-ENTRAR na hitbox.
    bool was_overlapping_enemy_ = false;

    // NPC FIXO (M7-DIALOGO, NPC-MVP): Seu Bertoldo Caim (F2-N.1) - MESMA tecnica do
    // inimigo fixo acima (AABB hardcoded em app/, posicionada via pick_fixed_enemy_
    // position + enemy_sprite_footprint_aabb sobre o mapa REAL - NAO toca o formato
    // .gmap/TileMap, que nao tem nocao de NPC), com o MESMO edge-trigger
    // (should_trigger_battle_on_edge) - so que sem "derrotado": o Bertoldo nunca
    // some, esbarrar de novo (sair e re-entrar na hitbox) abre a conversa de novo.
    // npc_bertoldo_aabb_ e o FOOTPRINT VISUAL (quad desenhado, set_npc_bertoldo_marker
    // abaixo - INALTERADO); npc_bertoldo_trigger_aabb_ e a MESMA tecnica de caixa-de-pes
    // do inimigo acima (feet_trigger_aabb), reusada aqui SEM duplicar logica - e ela que
    // decide "perto o bastante pra abrir o dialogo" (aabb_overlaps em run(), abaixo).
    gus::core::spatial::Aabb npc_bertoldo_aabb_{};
    gus::core::spatial::Aabb npc_bertoldo_trigger_aabb_{};
    bool was_overlapping_npc_bertoldo_ = false;

    // Grafo de dialogo do Bertoldo, parseado 1 vez no boot (init(), a partir do
    // .dlg.txt real via gus::app::dialogue::load_dialogue_graph_from_file).
    // nullopt = arquivo ausente/malformado (degradacao segura, mesmo espirito de
    // battle_music_id_ invalido acima) - to_npc_dialogue() vira no-op nesse caso.
    std::optional<gus::domain::dialogue::DialogueGraph> npc_bertoldo_graph_{};

    // "Save" desta onda: instancia em MEMORIA (a persistencia REAL em disco e
    // SAVE-LOAD-UI etapa 6, ver open_pause_from_city/build_current_save_data/
    // apply_loaded_save_data em maestro.cpp). Usa SaveData::flags (ja existe,
    // domain/save) em vez de inventar um formato novo pra "inimigo1_derrotado".
    gus::domain::save::SaveData save_{};

    // SAVE-LOAD-UI etapa 6 (playtime REAL, nao fingido): playtime_seconds no
    // instante de um Salvar = playtime_base_seconds_ + (SDL_GetTicksNS() -
    // playtime_anchor_ns_)/1e9. playtime_anchor_ns_ e RESETADO pra "agora" (e
    // playtime_base_seconds_ pro playtime_seconds do save) toda vez que um Load
    // aplica com sucesso - assim o acumulo segue correto mesmo apos carregar um
    // save de uma sessao anterior (o relogio da SESSAO ATUAL sempre comeca do
    // zero, SDL_GetTicksNS() e desde o boot do processo, nao desde o save).
    unsigned long long playtime_anchor_ns_ = 0;
    double playtime_base_seconds_ = 0.0;

    // MENU-PAUSA-CONFIG-SOM (INTEGRACAO FINAL): Translator de UI carregado 1 VEZ no
    // boot (init()) e reusado em toda abertura do menu de pausa (cidade OU batalha
    // ja tem o SEU proprio translator local - este e SO da cidade, ver
    // open_pause_from_city). Mesma receita de carga de battle_preview.cpp (Translator
    // + resolve_translations_path), so que vivendo pelo loop INTEIRO em vez de por
    // entrada-de-batalha.
    gus::app::i18n::Translator translator_{};
};

}  // namespace gus::app

#endif  // GUS_APP_MAESTRO_HPP
