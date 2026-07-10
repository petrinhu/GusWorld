// gus/app/screens/save_load_menu.hpp
//
// Logica PURA da TELA DE SALVAR/CARREGAR (SAVE-LOAD-UI, PI9 — a peca que
// desbloqueia o M7 paridade). POCO 100% testavel sem SDL_Init/janela/glintfx nem
// disco - mesmo espirito de gus/app/screens/system_menu.hpp (o estado/navegacao
// vive aqui; a RENDERIZACAO via glintfx vive em save_load_menu_rml.hpp/cpp; o I/O
// real de disco - gus::platform::fs::save_game/load_game, JA EXISTE, M2-SAVE-IO -
// fica com o CHAMADOR, que decide quando persistir/carregar de fato a partir da
// SaveLoadMenuAction devolvida por esta camada).
//
// ESCOPO DESTE ARQUIVO (mock docs/design/mockups/07-save-load.html, Telas 2 e 3 -
// SALVAR e CARREGAR): a MESMA tela serve os 2 modos (SaveLoadMode), diferindo so
// em quais slots sao SELECIONAVEIS e no que Enter faz. NAO cobre (etapas
// remanescentes do item SAVE-LOAD-UI, ver TODO.md): tela de TITULO (etapa 4),
// gatilhos de AUTOSAVE (etapa 5), wiring real Salvar/Carregar no Maestro/menu de
// pausa (etapa 6), nem os 2 AVISOS (versao-incompativel/corrompido e
// controles-diferentes-do-save) - esses dependem de INSPECIONAR o resultado real
// de gus::domain::save::load_save (LoadOutcome), que so existe DEPOIS de uma
// tentativa de leitura em disco (fora do escopo de estado PURO desta tela; o
// CHAMADOR monta esses avisos a partir do LoadOutcome, fluxo separado).
//
// POLITICA DE SLOTS (ADR-006, save_slots.hpp, JA EXISTENTE — reusada sem
// alteracao): kSlotCount slots no total (kAutosaveSlot=0 + kManualSlotCount
// manuais, 1..kManualSlotCount). NOTA DE DIVERGENCIA (sinalizada, nao decidida
// aqui): o mock 07-save-load.html e o texto do item SAVE-LOAD-UI no TODO.md
// citam "6 espacos de save + 1 Auto" (7 no total), mas o motor JA CONSTRUIDO
// (ADR-006, 2026-06-21) tem kManualSlotCount=5 (6 no total). Esta tela e
// GENERICA sobre kSlotCount (nao hardcoda "6" nem "7") - se o lider confirmar
// que o motor deve crescer para 6 manuais, e um bump aditivo trivial em
// save_slots.hpp (kManualSlotCount 5->6), sem mudar formato/serializer. Nao
// alterado aqui: SAVE-LOAD-UI e dispatch de UI, save_slots.hpp e dominio
// (fronteira de outro papel) - decisao final do lider.
//
// SELECIONABILIDADE por modo (ver slot_selectable):
//   Save: o autosave (slot 0) e SO-LEITURA (nao aparece "sel", nao pode ser
//     escolhido para gravar por cima - decisao do lider, mock Tela 2: "o espaco
//     Auto e so-leitura"); os manuais (ocupados OU vazios) sao selecionaveis.
//   Load: slots VAZIOS nao sao selecionaveis (mock Tela 3: "espaco vazio nao
//     selecionavel"); o autosave, quando OCUPADO, e selecionavel (carregar do
//     autosave e um uso legitimo, "Continuar" na tela de titulo se apoia nisso).
//
// CONFIRMACAO DE SOBRESCRITA (decisao (e) do lider, TODO.md): Enter num slot
// MANUAL OCUPADO em modo Save abre um mini-dialogo Sim/Nao (MESMA mecanica
// visual/de fluxo de controls_confirming_restore em system_menu.hpp) antes de
// devolver Confirmed; slot VAZIO em modo Save confirma DIRETO (nada a
// sobrescrever). Em modo Load, Enter sempre confirma DIRETO (o aviso de
// versao/corrompido, se houver, e um fluxo SEPARADO pos-tentativa de load, fora
// deste arquivo).
//
// Cross-ref: gus/app/screens/system_menu.hpp (mesmo espirito POCO + mini-dialogo
//            Sim/Nao); gus/domain/save/save_slots.hpp (kSlotCount/kAutosaveSlot,
//            REUSADO sem alteracao); gus/domain/save/save_data.hpp (SaveData,
//            fonte do preview); gus/platform/fs/save_file_store.hpp
//            (save_game/load_game reais, consumidos pelo CHAMADOR); ADR-006.

#ifndef GUS_APP_SCREENS_SAVE_LOAD_MENU_HPP
#define GUS_APP_SCREENS_SAVE_LOAD_MENU_HPP

#include <array>
#include <cstdint>
#include <map>
#include <string>

#include <SDL3/SDL.h>  // SDL_Keycode

#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_slots.hpp"

namespace gus::app::screens {

// Numero de capitulos narrativos (GDD §7: "Capitulos: 6" - alternando
// cidade<->Selve<->cidade<->Selve<->hibrido-set-piece<->boss final). Teto de
// clamp de chapter_from_quest_progress abaixo.
inline constexpr int kChapterCount = 6;

// Capitulo DERIVADO (NUNCA "nivel" - GusWorld nao tem nivel numerico, GDD §5.4/
// Meta loop: progressao e por modulos de hardware + cartas + capitulos, sem
// "Gus Lv 47"). Fonte: SaveData::quest_progress (TODO.md, decisao (d):
// "Capitulo derivado de flags/quest_progress").
//
// REGRA (decisao do lider, SAVE-LOAD-UI etapa 6 - fecha o SINALIZADO abaixo):
// HISTORIA MANDA, XP e so o ESTIMADOR quando NAO ha historia ainda.
//   - quest_progress["main_story"] PRESENTE: capitulo = 1 + esse valor (stage
//     index, 0-based), CLAMPADO em [1, kChapterCount]. `xp` e IGNORADO neste
//     caso (a historia e AUTORIDADE; XP nunca ultrapassa - respeita o pillar
//     "sem grind, nao farmavel" do GDD §7).
//   - quest_progress["main_story"] AUSENTE (o caso de TODO save existente
//     hoje - nenhuma quest real ainda escreve nesta chave, grep confirmou ZERO
//     consumidor em app/gameplay): capitulo = chapter_from_xp_fallback(xp)
//     abaixo - um ESTIMADOR COLINEAR (o jogo e LINEAR: XP acompanha a
//     historia) por FAIXAS de XP PROVISORIAS (ver kChapterXpThresholds - a
//     curva de XP ainda nao fechou, PROVISORIO ate o CARTAS-BALANCEAMENTO/
//     economia). xp=0 (save recem-criado, sem character_states) cai na 1a
//     faixa = Cap. 1 - EXATAMENTE o que o mock aprovado mostra em todo slot
//     hoje, sem inventar dado novo.
//
// Forward-compat: quando quests reais comecarem a gravar
// quest_progress["main_story"], este calculo passa a devolver capitulos
// derivados da HISTORIA (a fonte definitiva) automaticamente, sem retrabalho
// aqui - o estimador de XP so serve de PONTE ate la.
[[nodiscard]] int chapter_from_quest_progress(
    const std::map<std::string, int>& quest_progress, int xp) noexcept;

// Numero de LIMIARES de XP (kChapterCount-1 fronteiras entre kChapterCount
// faixas). PROVISORIO — afinar no CARTAS-BALANCEAMENTO/economia (a curva de
// XP real ainda nao fechou); valores monotonicamente crescentes, so servem de
// estimador ENQUANTO quest_progress["main_story"] nao esta gravado (ver
// chapter_from_quest_progress acima). Nomeados (nao magic numbers soltos) pra
// ficarem faceis de re-tunar num lugar so quando a economia fechar.
inline constexpr int kChapterXpThresholds[kChapterCount - 1] = {100, 300, 600,
                                                                  1000, 1500};

// Estimador de capitulo por XP (PROVISORIO, ver kChapterXpThresholds acima):
// conta quantos limiares `xp` ja cruzou, +1 (base = capitulo 1), clampado em
// [1, kChapterCount]. Usado SO por chapter_from_quest_progress quando
// quest_progress["main_story"] esta ausente - nunca chamado direto pela tela
// (exposto pra ser testavel isoladamente).
[[nodiscard]] int chapter_from_xp_fallback(int xp) noexcept;

// XP para EXIBICAO no slot (nunca "nivel"). Fonte: CharacterSaveState::xp do Gus
// (character_states.at("gus")) - Gus e sempre o protagonista/sempre presente na
// party (GDD/party.md); ausente (save antigo/forjado sem esse char) devolve 0
// defensivamente, NUNCA lanca.
[[nodiscard]] int save_xp_for_display(const gus::domain::save::SaveData& data) noexcept;

// Formata timestamp_ms (epoch, UTC - deterministico, sem depender de fuso local
// da maquina que roda o teste) como "DD/MM/AAAA HH:MM" (mesmo formato do mock,
// ex. "07/07/2026 20:14"). timestamp_ms < 0 e defensivamente tratado como 0
// (epoch).
[[nodiscard]] std::string format_timestamp_ms(std::int64_t timestamp_ms);

// Formata segundos de playtime como "XhYYm" (mesmo formato do mock, ex. "0h
// 42m", "1h 18m"). Minutos sempre 2 digitos (zero-padded); segundos negativos
// tratados como 0 defensivamente.
[[nodiscard]] std::string format_playtime_seconds(double playtime_seconds);

// Preview de UM slot - o que a tela precisa saber para desenhar 1 linha (mock:
// numero/letra do slot, local, timestamp, playtime, XP, capitulo, ocupado/vazio/
// autosave). Dado PURO; build_slot_preview/empty_slot_preview constroem a
// partir de gus::domain::save::SaveData (quando ocupado) ou so do slot_id
// (quando vazio) - NENHUM dos dois toca disco (o CHAMADOR ja tentou
// load_game/has_save antes de chamar).
struct SaveSlotPreview {
    int slot_id = 0;          // 0..kSlotCount-1 (save_slots.hpp)
    bool occupied = false;    // false = slot vazio (novo jogo ainda nao gravou aqui)
    bool is_autosave = false; // slot_id == kAutosaveSlot (cache de is_autosave(slot_id))
    std::string scene_path;   // SaveData::current_scene_path (so valido se occupied)
    std::int64_t timestamp_ms = 0;   // SaveData::timestamp_ms (so valido se occupied)
    double playtime_seconds = 0.0;   // SaveData::playtime_seconds (so valido se occupied)
    int xp = 0;      // save_xp_for_display(data) (so valido se occupied)
    int chapter = 1; // chapter_from_quest_progress(data.quest_progress) (so valido se occupied)
};

// Preview de um slot VAZIO (occupied=false; demais campos default/irrelevantes).
[[nodiscard]] SaveSlotPreview empty_slot_preview(int slot_id) noexcept;

// Preview de um slot OCUPADO a partir do SaveData ja carregado (occupied=true).
// slot_id deve ser o slot FISICO de onde `data` foi lido (nao necessariamente
// data.slot_id, que e o valor SELADO - a divergencia entre os dois e o caso
// WrongSlot do serializer, tratado pelo CHAMADOR antes de chegar aqui).
[[nodiscard]] SaveSlotPreview build_slot_preview(
    const gus::domain::save::SaveData& data, int slot_id);

// Modo da tela: Save (menu de pausa > Salvar) ou Load (titulo > Continuar, ou
// menu de pausa > Carregar). MESMO layout visual (mock Telas 2/3); so muda
// selecionabilidade (slot_selectable) e o efeito de Enter.
enum class SaveLoadMode {
    Save,
    Load,
};

// Estado completo da tela (a UNICA fonte de verdade que a RML le e que o
// chamador muta via as funcoes abaixo). slots[i] corresponde ao slot fisico i
// (0..kSlotCount-1, MESMA indexacao de save_slots.hpp).
struct SaveLoadMenuState {
    SaveLoadMode mode = SaveLoadMode::Save;
    std::array<SaveSlotPreview, gus::domain::save::kSlotCount> slots{};
    int selected = gus::domain::save::kAutosaveSlot; // sobrescrito por save_load_menu_open

    // Mini-dialogo de confirmacao de sobrescrita (decisao (e) do lider) - MESMA
    // mecanica visual/de fluxo de controls_confirming_restore (system_menu.hpp).
    bool confirming_overwrite = false;
    int confirm_selected = 1; // 0=Sim (sobrescreve), 1=Nao (default seguro)

    // Mini-dialogo de confirmacao de EXCLUSAO (feature "Apagar", aprovada pelo
    // lider) - MESMA mecanica visual/de fluxo do overwrite acima, campos PROPRIOS
    // (os dois dialogos nunca abrem ao mesmo tempo - ver save_load_menu_request_
    // delete). delete_target_slot e o slot ALVO da confirmacao - PODE divergir de
    // `selected` (clique no icone de apagar de uma linha NAO focada tambem
    // funciona, ver save_load_menu_loop.cpp); -1 = nenhuma exclusao em curso.
    bool confirming_delete = false;
    int delete_confirm_selected = 1; // 0=Sim (apaga), 1=Nao (default seguro)
    int delete_target_slot = -1;
};

// true se o slot em `index` (0..kSlotCount-1) e SELECIONAVEL no modo ATUAL de
// `state` (ver a doc do header: Save exclui o autosave; Load exclui slots
// vazios). index fora do intervalo devolve false (defensivo).
[[nodiscard]] bool slot_selectable(const SaveLoadMenuState& state, int index) noexcept;

// SAVE-LOAD-UI etapa 4 (TELA DE TITULO): indice do slot OCUPADO com o MAIOR
// timestamp_ms entre `slots` (Auto E manuais concorrem igualmente - "Continuar"
// carrega o save mais recente da PARTIDA, nao um slot escolhido a dedo). Empate no
// timestamp: o PRIMEIRO indice (ordem crescente 0..kSlotCount-1) vence (defensivo,
// raro na pratica - timestamps sao milissegundos de wall clock). Devolve -1 se
// NENHUM slot estiver ocupado (a tela de titulo NAO deveria oferecer "Continuar"
// nesse caso).
[[nodiscard]] int most_recent_occupied_slot(
    const std::array<SaveSlotPreview, gus::domain::save::kSlotCount>& slots) noexcept;

// Abre a tela no modo dado, com os previews ja construidos pelo CHAMADOR (via
// build_slot_preview/empty_slot_preview - esta funcao NAO le disco). Selecao
// inicial = o PRIMEIRO slot selecionavel (ordem crescente de indice,
// comecando em kAutosaveSlot+1 se possivel - o mock mostra o slot 1 "sel", nao
// o Auto); se NENHUM slot for selecionavel (Load com save novo/vazio - caso de
// borda defensivo, a tela de titulo normalmente nao ofereceria "Continuar"
// aqui), selected fica em kAutosaveSlot sem quebrar (Enter vira no-op, ver
// save_load_menu_key_down). Reseta confirming_overwrite/confirm_selected.
void save_load_menu_open(
    SaveLoadMenuState& state, SaveLoadMode mode,
    const std::array<SaveSlotPreview, gus::domain::save::kSlotCount>& slots) noexcept;

// Resultado de um save_load_menu_key_down: o que o CHAMADOR deve fazer.
enum class SaveLoadMenuAction {
    None,               // tecla sem efeito
    Back,               // Voltar/Esc na lista (fora do mini-dialogo) - feche a tela
    SlotChosen,         // slot ESCOLHIDO sem precisar de confirmacao (Load sempre;
                        // Save num slot VAZIO) - state.selected e o slot; o CHAMADOR
                        // grava/carrega de fato (I/O real, gus::platform::fs::*)
    OverwriteConfirmed, // o mini-dialogo de sobrescrita fechou com "Sim" - state.selected
                        // e o slot; o CHAMADOR grava por cima de fato
    OverwriteCancelled, // o mini-dialogo fechou com "Nao"/Esc - permanece na lista,
                        // NENHUM I/O
    DeleteConfirmed,    // o mini-dialogo de EXCLUSAO fechou com "Sim" -
                        // state.delete_target_slot e o slot; o CHAMADOR apaga de fato
                        // (gus::platform::fs::delete_save) e atualiza o preview local
    DeleteCancelled,    // o mini-dialogo de exclusao fechou com "Nao"/Esc - permanece
                        // na lista, NENHUM I/O
};

// Roteia UMA tecla pelo estado ATUAL (lista de slots OU um dos 2 mini-dialogos
// abertos - sobrescrita OU exclusao, NUNCA os dois ao mesmo tempo). Sobe/desce SO
// por slots selecionaveis (slot_selectable), com wrap-around; Enter delega para a
// regra de confirmacao documentada no header (slot vazio ou modo Load =
// SlotChosen direto; slot manual ocupado em modo Save = abre o mini-dialogo de
// sobrescrita). Delete (tecla dedicada, SDLK_DELETE) sobre um slot OCUPADO abre o
// mini-dialogo de EXCLUSAO (ver save_load_menu_request_delete) - alvo = state.
// selected (o clique do MOUSE no icone por-linha usa save_load_menu_click_slot/
// save_load_menu_request_delete direto, targeting a linha clicada). Dentro de
// QUALQUER mini-dialogo, LEFT/RIGHT/UP/DOWN alternam a pill selecionada (0/1) e
// ENTER confirma a escolha atual; ESC equivale a "Nao" (mesma seguranca de
// controls_confirming_discard, system_menu.hpp). ESC/Voltar fora de ambos os
// mini-dialogos devolve Back.
[[nodiscard]] SaveLoadMenuAction save_load_menu_key_down(SaveLoadMenuState& state,
                                                          SDL_Keycode key) noexcept;

// Clique de MOUSE num slot da lista (fora de qualquer mini-dialogo, ver
// save_load_menu_loop.cpp): equivalente a "focar + Enter" (MESMA convencao de
// system_menu_click_option, system_menu.hpp). No-op (None) se `slot` fora do
// intervalo, se algum mini-dialogo ja estiver aberto (o clique na LISTA nao se
// aplica enquanto um dialogo cobre a tela), ou se o slot nao for selecionavel
// (readonly/vazio conforme o modo, ver slot_selectable) - caso contrario, foca o
// slot (state.selected = slot) e aplica a MESMA regra de confirmacao do Enter.
[[nodiscard]] SaveLoadMenuAction save_load_menu_click_slot(SaveLoadMenuState& state,
                                                            int slot) noexcept;

// Abre o mini-dialogo de confirmacao de EXCLUSAO tendo `slot` como ALVO - tecla
// dedicada (Delete, targeting state.selected) OU clique no icone de apagar
// por-linha (targeting a linha clicada, que pode divergir de state.selected).
// No-op defensivo (nao muta nada) se `slot` fora do intervalo, se `slot` estiver
// VAZIO (nada a apagar), ou se algum mini-dialogo (sobrescrita OU exclusao) ja
// estiver aberto (nunca 2 dialogos simultaneos).
void save_load_menu_request_delete(SaveLoadMenuState& state, int slot) noexcept;

// Se state.selected NAO for mais selecionavel (ex.: o slot focado acabou de ser
// APAGADO - ver SaveLoadMenuAction::DeleteConfirmed - e ficou vazio em modo Load),
// move para o PROXIMO slot selecionavel (mesma busca de save_load_menu_open,
// wrap-around). No-op se state.selected ainda for valido; se NENHUM slot for
// selecionavel, fica inalterado (mesmo caso de borda defensivo de next_selectable).
// Chamado pelo CHAMADOR logo apos um delete de fato em disco (a mutacao de
// state.slots[slot] pra vazio e responsabilidade do CHAMADOR, que sabe o I/O real
// - esta funcao so re-ancora a selecao).
void save_load_menu_reselect_if_needed(SaveLoadMenuState& state) noexcept;

// Clique de MOUSE numa das 2 pills do mini-dialogo de SOBRESCRITA (0=Sim/
// sobrescreve, 1=Nao) - equivalente a focar aquela pill + Enter. No-op (None) se
// o dialogo NAO estiver aberto (confirming_overwrite==false) ou `pill` fora de
// {0,1} (defensivo).
[[nodiscard]] SaveLoadMenuAction save_load_menu_click_overwrite_confirm(
    SaveLoadMenuState& state, int pill) noexcept;

// Idem para o mini-dialogo de EXCLUSAO (0=Sim/apaga, 1=Nao) - MESMO contrato de
// save_load_menu_click_overwrite_confirm acima, campos proprios
// (confirming_delete/delete_confirm_selected).
[[nodiscard]] SaveLoadMenuAction save_load_menu_click_delete_confirm(
    SaveLoadMenuState& state, int pill) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SAVE_LOAD_MENU_HPP
