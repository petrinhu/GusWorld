// SPDX-License-Identifier: Apache-2.0
// gus/domain/tests/enum_wire_contract_test.cpp
//
// CONTRATO BINARIO DE SAVE - NAO REORDENAR (CARDS-HW-ENUMFREEZE, TODO.md).
//
// domain/src/save/save_serializer.cpp grava estes enums como ORDINAL u32 CRU no
// payload do save (GDS3, ver build_payload_current/read_card_physical_state/
// read_difficulty_fields). Um reorder ou insert-no-meio de qualquer membro muda o
// valor numerico correspondente e CORROMPE SILENCIOSAMENTE todo save ja gravado em
// disco (o ordinal antigo passa a apontar pra outro membro - a suite de testes
// normal NAO pega isso, porque ela sempre recompila enum+serializer JUNTOS; so um
// save real, gravado com o binario ANTIGO e lido pelo binario NOVO, revelaria o
// problema - exatamente a classe de bug achada pelo qa-engineer na onda ATOM,
// AUD-ATOM-2026, que sobrevivia a suite inteira).
//
// Estes static_assert travam o valor numerico EXATO de cada membro na COMPILACAO:
// trocar dois ordinais (ex.: o mutante classico "inverte 2 linhas do enum") quebra
// o BUILD na hora, em vez de silenciosamente corromper saves em producao.
//
// Inventario GDS3 (grep em domain/src/save/save_serializer.cpp por static_cast<...>
// (r.read_u32())/put_u32(out, static_cast<uint32_t>(...)) - 2026-07-20):
//   - CardOrigin      (card_provenance.hpp)  - CardPhysicalState::origin
//   - VirusKind       (integrity_state.hpp)  - CardPhysicalState::virus_kind
//   - DifficultyLevel (save_data.hpp)        - SaveData::difficulty
//
// Inventario GDT1 (contrato binario IRMAO, mas DISTINTO - envelope GDS2/HMAC de
// templates, o comentario "templates continuam GDS2/HMAC, fora de escopo" no topo
// de save_serializer.cpp refere-se ao ENVELOPE, nao aos enums de payload; os enums
// abaixo SAO cobertos aqui desde 2026-07-21, grep em
// domain/src/templates/template_serializer.cpp):
//   - CardFamily (card_family.hpp, alias de gus::domain::combat::CardFamily, que por
//     sua vez reexporta gus::domain::cards::CardFamily) - CharacterTemplate::family
//     (serialize_character ~230) e EnemyTemplate::family (serialize_enemy ~267)
//   - BrainKind  (enemy_template.hpp) - EnemyTemplate::brain (serialize_enemy ~268)
//   - EnemyKind  (enemy_template.hpp) - EnemyTemplate::kind (serialize_enemy ~274)
//   - EnemyTier  (enemy_template.hpp) - EnemyTemplate::tier (serialize_enemy ~283,
//     DIFICULDADE-TABELA-DADO, campo aditivo apos central_command)
//
// Se este arquivo PARAR de compilar depois de voce mexer num destes enums: NAO
// mude os numeros aqui pra "consertar o erro" - isso e EXATAMENTE o alarme
// disparando. Adicionar um membro NOVO no FIM (nunca no meio) e seguro; só então
// atualize o assert do count (kCardOriginCount/kVirusKindCount/
// kDifficultyLevelCount/kCardFamilyCount/kBrainKindCount/kEnemyKindCount) e
// adicione o static_assert do membro novo aqui.
//
// ---------------------------------------------------------------------------
// TESTES-MUTANTES-W1: e QUEM avisa se a segunda metade da instrucao acima for
// esquecida?
// ---------------------------------------------------------------------------
// Ate 2026-08-06, ninguem. A auditoria da onda W1 plantou o mutante M2 -
// "acrescentar um membro no FIM de um enum de wire SEM atualizar o k*Count" - e
// ele COMPILOU LIMPO e passou a suite inteira nos 6 enums. O estrago e mudo e
// tardio: o k*Count e o teto da validacao de ordinal do LOAD (save_data.cpp:109,
// enemy_template.cpp:53-60, card_provenance.cpp/integrity_state validate()), entao
// um save gravado com o membro novo seria REJEITADO como ordinal invalido - sem
// nenhum sinal na hora de escrever o codigo. O paragrafo acima e uma instrucao pro
// humano; instrucao nao e gate.
//
// O gate esta no bloco "TRIPWIRE" no fim deste arquivo: um predicado is_named_*
// por enum, com switch EXAUSTIVO e SEM `default`. Ele fecha as DUAS metades:
//
//   (a) apendar membro sem mais nada  -> o switch deixa de ser exaustivo e o
//       -Werror=switch (gus_warnings, CMakeLists.txt:768-774; linkado tambem em
//       gusengine_domain_tests) QUEBRA O BUILD aqui;
//   (b) apendar membro E adicionar o case, mas esquecer o k*Count -> o
//       static_assert !is_named_*(static_cast<E>(kECount)) passa a ser FALSO (o
//       ordinal N virou membro nomeado) e QUEBRA O BUILD com a mensagem que diz o
//       que fazer.
//
// Nao ha terceira saida: qualquer append passa por (a) ou por (b).
//
// POR QUE AQUI, E NAO UMA SENTINELA `Count` NO PROPRIO ENUM (a forma classica):
// dois dos seis enums (CardFamily e VirusKind) tem switch exaustivo sem `default`
// em domain/src/combat/combat_state_machine.cpp, arquivo que estava sob edicao de
// outra fatia quando esta foi escrita - uma sentinela obrigaria a mexer nele. O
// tripwire abaixo da a MESMA garantia de compilacao para os 6 de forma uniforme,
// com ZERO mudanca em header de producao e ZERO membro novo que pudesse, por
// descuido de algum call site futuro, ser serializado.

#include <cstdint>

#include "gus/domain/hardware/card_provenance.hpp"
#include "gus/domain/infection/integrity_state.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/templates/card_family.hpp"
#include "gus/domain/templates/enemy_template.hpp"

namespace {

using gus::domain::hardware::CardOrigin;
using gus::domain::hardware::kCardOriginCount;
using gus::domain::infection::VirusKind;
using gus::domain::infection::kVirusKindCount;
using gus::domain::save::DifficultyLevel;
using gus::domain::save::kDifficultyLevelCount;
using gus::domain::templates::BrainKind;
using gus::domain::templates::CardFamily;
using gus::domain::templates::EnemyKind;
using gus::domain::templates::EnemyTier;
using gus::domain::templates::kBrainKindCount;
using gus::domain::templates::kCardFamilyCount;
using gus::domain::templates::kWheelFamilyCount;
using gus::domain::templates::kEnemyKindCount;
using gus::domain::templates::kEnemyTierCount;

// ---- CardOrigin (card_provenance.hpp) - CardPhysicalState::origin, u32 no wire --
static_assert(static_cast<std::uint32_t>(CardOrigin::OriginalRom) == 0);
static_assert(static_cast<std::uint32_t>(CardOrigin::HomebrewEprom) == 1);
static_assert(static_cast<std::uint32_t>(CardOrigin::PirateClone) == 2);
static_assert(kCardOriginCount == 3, "kCardOriginCount desalinhado do enum acima");

// ---- VirusKind (integrity_state.hpp) - CardPhysicalState::virus_kind, u32 no wire
static_assert(static_cast<std::uint32_t>(VirusKind::None) == 0);
static_assert(static_cast<std::uint32_t>(VirusKind::LogicBomb) == 1);
static_assert(static_cast<std::uint32_t>(VirusKind::Backdoor) == 2);
static_assert(static_cast<std::uint32_t>(VirusKind::Worm) == 3);
static_assert(static_cast<std::uint32_t>(VirusKind::FalseBenign) == 4);
static_assert(static_cast<std::uint32_t>(VirusKind::AdwareSterling) == 5);
static_assert(static_cast<std::uint32_t>(VirusKind::ZipBomb) == 6);
static_assert(static_cast<std::uint32_t>(VirusKind::IndustrialWeapon) == 7);
static_assert(kVirusKindCount == 8, "kVirusKindCount desalinhado do enum acima");

// ---- DifficultyLevel (save_data.hpp) - SaveData::difficulty, u32 no wire --------
static_assert(static_cast<std::uint32_t>(DifficultyLevel::Facil) == 0);
static_assert(static_cast<std::uint32_t>(DifficultyLevel::Medio) == 1);
static_assert(static_cast<std::uint32_t>(DifficultyLevel::Dificil) == 2);
static_assert(static_cast<std::uint32_t>(DifficultyLevel::Hardcore) == 3);
static_assert(kDifficultyLevelCount == 4,
              "kDifficultyLevelCount desalinhado do enum acima");

// ---- CardFamily (card_family.hpp) - CharacterTemplate::family (serialize_character)
// e EnemyTemplate::family (serialize_enemy), u32 no wire (GDT1) --------------------
static_assert(static_cast<std::uint32_t>(CardFamily::Eletrico) == 0);
static_assert(static_cast<std::uint32_t>(CardFamily::Bioquimico) == 1);
static_assert(static_cast<std::uint32_t>(CardFamily::Sonico) == 2);
static_assert(static_cast<std::uint32_t>(CardFamily::Cinetico) == 3);
static_assert(static_cast<std::uint32_t>(CardFamily::Criptografico) == 4);
static_assert(static_cast<std::uint32_t>(CardFamily::Universal) == 5);
static_assert(kCardFamilyCount == 6, "CONTRATO BINARIO GDT1 - NAO REORDENAR");

// ---- BrainKind (enemy_template.hpp) - EnemyTemplate::brain, u32 no wire (GDT1) ----
static_assert(static_cast<std::uint32_t>(BrainKind::Scripted) == 0);
static_assert(static_cast<std::uint32_t>(BrainKind::Utility) == 1);
static_assert(kBrainKindCount == 2, "CONTRATO BINARIO GDT1 - NAO REORDENAR");

// ---- EnemyKind (enemy_template.hpp) - EnemyTemplate::kind, u32 no wire (GDT1) -----
static_assert(static_cast<std::uint32_t>(EnemyKind::Creature) == 0);
static_assert(static_cast<std::uint32_t>(EnemyKind::Human) == 1);
static_assert(kEnemyKindCount == 2, "CONTRATO BINARIO GDT1 - NAO REORDENAR");

// ---- EnemyTier (enemy_template.hpp) - EnemyTemplate::tier, u32 no wire (GDT1) -----
// DIFICULDADE-TABELA-DADO: campo aditivo novo (apos central_command), MESMO contrato
// binario append-only dos enums acima.
static_assert(static_cast<std::uint32_t>(EnemyTier::Trash) == 0);
static_assert(static_cast<std::uint32_t>(EnemyTier::Elite) == 1);
static_assert(kEnemyTierCount == 2, "CONTRATO BINARIO GDT1 - NAO REORDENAR");

// ===========================================================================
// TRIPWIRE DE APPEND (TESTES-MUTANTES-W1) - ver o cabecalho deste arquivo
// ===========================================================================
//
// Um predicado por enum: "este ordinal e um membro NOMEADO?". Duas propriedades
// deliberadas, e o gate nasce da combinacao delas:
//
//   1. O switch e EXAUSTIVO e NAO tem `default:`. Apendar um membro sem tocar
//      mais nada dispara -Werror=switch (enumeration value ... not handled in
//      switch) e o build MORRE aqui.
//   2. O parametro pode ser um ordinal FORA do conjunto nomeado - legal e bem
//      definido, porque todo enum aqui tem underlying type fixo (std::uint32_t),
//      entao qualquer valor do underlying type e um valor valido do enum
//      ([dcl.enum]/8) e o switch simplesmente nao casa nenhum case. Isso e o que
//      permite perguntar, em tempo de COMPILACAO, se o ordinal k*Count ainda esta
//      livre - e ele so deixa de estar quando alguem apendou membro e nao mexeu
//      no count.
//
// Se um destes static_assert falhar: NAO relaxe o assert. Atualize o k*Count no
// header do enum (e o assert de count la em cima), que e exatamente o passo
// esquecido que este tripwire existe pra pegar.

// Macro (nao constante): a mensagem de static_assert tem que ser um LITERAL de string
// em C++20 - `const char*` so vale a partir de C++26.
#define GUS_ENUM_APPEND_MSG                                                            \
    "enum de wire cresceu mas o k*Count correspondente NAO foi atualizado: o ordinal " \
    "k*Count ja e um membro nomeado. Corrija o k*Count no header do enum (a validacao " \
    "de ordinal do load usa ele como TETO - deixar velho REJEITA silenciosamente todo " \
    "save que grave o membro novo)."

constexpr bool is_named_card_origin(CardOrigin v) {
    switch (v) {
        case CardOrigin::OriginalRom:
        case CardOrigin::HomebrewEprom:
        case CardOrigin::PirateClone:
            return true;
    }
    return false;
}
static_assert(!is_named_card_origin(static_cast<CardOrigin>(kCardOriginCount)),
              GUS_ENUM_APPEND_MSG);

constexpr bool is_named_virus_kind(VirusKind v) {
    switch (v) {
        case VirusKind::None:
        case VirusKind::LogicBomb:
        case VirusKind::Backdoor:
        case VirusKind::Worm:
        case VirusKind::FalseBenign:
        case VirusKind::AdwareSterling:
        case VirusKind::ZipBomb:
        case VirusKind::IndustrialWeapon:
            return true;
    }
    return false;
}
static_assert(!is_named_virus_kind(static_cast<VirusKind>(kVirusKindCount)), GUS_ENUM_APPEND_MSG);

constexpr bool is_named_difficulty_level(DifficultyLevel v) {
    switch (v) {
        case DifficultyLevel::Facil:
        case DifficultyLevel::Medio:
        case DifficultyLevel::Dificil:
        case DifficultyLevel::Hardcore:
            return true;
    }
    return false;
}
static_assert(!is_named_difficulty_level(static_cast<DifficultyLevel>(kDifficultyLevelCount)),
              GUS_ENUM_APPEND_MSG);

constexpr bool is_named_card_family(CardFamily v) {
    switch (v) {
        case CardFamily::Eletrico:
        case CardFamily::Bioquimico:
        case CardFamily::Sonico:
        case CardFamily::Cinetico:
        case CardFamily::Criptografico:
        case CardFamily::Universal:
            return true;
    }
    return false;
}
// CardFamily tem DOIS contadores (card_family.hpp): kWheelFamilyCount = 5 e o
// dominio TOTAL kCardFamilyCount = 6. O tripwire cobre o TOTAL (o que o serializer
// .gdt grava); kWheelFamilyCount e um subconjunto deliberado (a roda de 5, sem
// Universal) e por isso o ordinal 5 E nomeado de proposito - assercao explicita
// abaixo pra que ninguem "conserte" o numero errado se o de cima disparar.
static_assert(!is_named_card_family(static_cast<CardFamily>(kCardFamilyCount)),
              GUS_ENUM_APPEND_MSG);
static_assert(is_named_card_family(static_cast<CardFamily>(kWheelFamilyCount)),
              "kWheelFamilyCount e subconjunto PROPOSITAL (a roda de 5, sem Universal): o "
              "ordinal 5 e nomeado por design. Nao alinhe este numero com kCardFamilyCount.");

constexpr bool is_named_brain_kind(BrainKind v) {
    switch (v) {
        case BrainKind::Scripted:
        case BrainKind::Utility:
            return true;
    }
    return false;
}
static_assert(!is_named_brain_kind(static_cast<BrainKind>(kBrainKindCount)), GUS_ENUM_APPEND_MSG);

constexpr bool is_named_enemy_kind(EnemyKind v) {
    switch (v) {
        case EnemyKind::Creature:
        case EnemyKind::Human:
            return true;
    }
    return false;
}
static_assert(!is_named_enemy_kind(static_cast<EnemyKind>(kEnemyKindCount)), GUS_ENUM_APPEND_MSG);

constexpr bool is_named_enemy_tier(EnemyTier v) {
    switch (v) {
        case EnemyTier::Trash:
        case EnemyTier::Elite:
            return true;
    }
    return false;
}
static_assert(!is_named_enemy_tier(static_cast<EnemyTier>(kEnemyTierCount)), GUS_ENUM_APPEND_MSG);

}  // namespace

// Sem TEST_CASE de runtime: os static_assert acima ja travam o contrato NA
// COMPILACAO (mais forte que qualquer assercao em runtime - um mutante que troque
// 2 ordinais nao chega nem a linkar). catch_discover_tests exige >=1 TEST_CASE por
// executavel-alvo linkado; este arquivo e compilado direto no alvo
// gusengine_domain_tests (que ja tem dezenas de outros TEST_CASE), entao nao
// precisa de nenhum aqui so pra "existir" no ctest.
