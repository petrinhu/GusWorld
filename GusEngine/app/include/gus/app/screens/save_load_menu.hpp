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
// "Capitulo derivado de flags/quest_progress"). REGRA MINIMA-VIAVEL adotada
// AGORA (nenhuma quest real ainda escreve em quest_progress - grep confirmou
// ZERO consumidor em app/gameplay nesta dispatch; o campo existe so no schema,
// aguardando o sistema de quests futuro): chave "main_story" e o marco de
// progresso principal (stage index, 0-based); capitulo = 1 + esse valor,
// CLAMPADO em [1, kChapterCount]. Chave ausente (o caso de TODO save existente
// hoje) devolve 1 - EXATAMENTE o que o mock aprovado mostra em todo slot ("Cap.
// 1"), sem inventar nenhum dado novo. Forward-compat: quando quests reais
// comecarem a gravar quest_progress["main_story"], este calculo passa a
// devolver capitulos maiores SEM precisar de nenhum retrabalho aqui.
//
// SINALIZADO AO LIDER (nao e a MESMA formula do texto da dispatch - "Capitulo
// DERIVADO do XP" - que citava XP como fonte; o TODO.md original pede derivar
// de flags/quest_progress. Adotada a leitura do TODO.md por ser a que exige
// ZERO invencao de limiar novo (quest_progress esta sempre vazio hoje = Cap. 1
// para todo save, fielmente igual ao mock). CONFIRMAR com o lider se a fonte
// certa e quest_progress (como aqui) ou um limiar de XP dedicado antes de
// content real de capitulos existir.
[[nodiscard]] int chapter_from_quest_progress(
    const std::map<std::string, int>& quest_progress) noexcept;

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
};

// true se o slot em `index` (0..kSlotCount-1) e SELECIONAVEL no modo ATUAL de
// `state` (ver a doc do header: Save exclui o autosave; Load exclui slots
// vazios). index fora do intervalo devolve false (defensivo).
[[nodiscard]] bool slot_selectable(const SaveLoadMenuState& state, int index) noexcept;

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
};

// Roteia UMA tecla pelo estado ATUAL (lista de slots OU mini-dialogo de
// sobrescrita aberto). Sobe/desce SO por slots selecionaveis (slot_selectable),
// com wrap-around; Enter delega para a regra de confirmacao documentada no
// header (slot vazio ou modo Load = SlotChosen direto; slot manual ocupado em
// modo Save = abre o mini-dialogo). Dentro do mini-dialogo, LEFT/RIGHT/UP/DOWN
// alternam confirm_selected (0/1) e ENTER confirma a escolha atual; ESC no
// mini-dialogo equivale a "Nao" (OverwriteCancelled, mesma seguranca de
// controls_confirming_discard). ESC/Voltar fora do mini-dialogo devolve Back.
[[nodiscard]] SaveLoadMenuAction save_load_menu_key_down(SaveLoadMenuState& state,
                                                          SDL_Keycode key) noexcept;

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SAVE_LOAD_MENU_HPP
