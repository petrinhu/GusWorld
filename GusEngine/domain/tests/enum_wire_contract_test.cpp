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
//
// Se este arquivo PARAR de compilar depois de voce mexer num destes enums: NAO
// mude os numeros aqui pra "consertar o erro" - isso e EXATAMENTE o alarme
// disparando. Adicionar um membro NOVO no FIM (nunca no meio) e seguro; só então
// atualize o assert do count (kCardOriginCount/kVirusKindCount/
// kDifficultyLevelCount/kCardFamilyCount/kBrainKindCount/kEnemyKindCount) e
// adicione o static_assert do membro novo aqui.

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
using gus::domain::templates::kBrainKindCount;
using gus::domain::templates::kCardFamilyCount;
using gus::domain::templates::kEnemyKindCount;

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

}  // namespace

// Sem TEST_CASE de runtime: os static_assert acima ja travam o contrato NA
// COMPILACAO (mais forte que qualquer assercao em runtime - um mutante que troque
// 2 ordinais nao chega nem a linkar). catch_discover_tests exige >=1 TEST_CASE por
// executavel-alvo linkado; este arquivo e compilado direto no alvo
// gusengine_domain_tests (que ja tem dezenas de outros TEST_CASE), entao nao
// precisa de nenhum aqui so pra "existir" no ctest.
