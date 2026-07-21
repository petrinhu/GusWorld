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
// Inventario (grep em domain/src/save/save_serializer.cpp por static_cast<...>
// (r.read_u32())/put_u32(out, static_cast<uint32_t>(...)) - 2026-07-20):
//   - CardOrigin      (card_provenance.hpp)  - CardPhysicalState::origin
//   - VirusKind       (integrity_state.hpp)  - CardPhysicalState::virus_kind
//   - DifficultyLevel (save_data.hpp)        - SaveData::difficulty
//
// FORA de escopo (contrato binario IRMAO, mas DISTINTO - envelope GDS2/HMAC de
// templates, explicitamente "fora de escopo" do save por comentario em
// save_serializer.cpp topo do arquivo: "templates continuam GDS2/HMAC, fora de
// escopo"): CardFamily/BrainKind em domain/src/templates/template_serializer.cpp.
// Mesma classe de risco, mas e outro arquivo/formato (GDT1, nao GDS3) - achado
// reportado a parte, nao congelado aqui.
//
// Se este arquivo PARAR de compilar depois de voce mexer num destes 3 enums: NAO
// mude os numeros aqui pra "consertar o erro" - isso e EXATAMENTE o alarme
// disparando. Adicionar um membro NOVO no FIM (nunca no meio) e seguro; só então
// atualize o assert do count (kCardOriginCount/kVirusKindCount/
// kDifficultyLevelCount) e adicione o static_assert do membro novo aqui.

#include <cstdint>

#include "gus/domain/hardware/card_provenance.hpp"
#include "gus/domain/infection/integrity_state.hpp"
#include "gus/domain/save/save_data.hpp"

namespace {

using gus::domain::hardware::CardOrigin;
using gus::domain::hardware::kCardOriginCount;
using gus::domain::infection::VirusKind;
using gus::domain::infection::kVirusKindCount;
using gus::domain::save::DifficultyLevel;
using gus::domain::save::kDifficultyLevelCount;

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

}  // namespace

// Sem TEST_CASE de runtime: os static_assert acima ja travam o contrato NA
// COMPILACAO (mais forte que qualquer assercao em runtime - um mutante que troque
// 2 ordinais nao chega nem a linkar). catch_discover_tests exige >=1 TEST_CASE por
// executavel-alvo linkado; este arquivo e compilado direto no alvo
// gusengine_domain_tests (que ja tem dezenas de outros TEST_CASE), entao nao
// precisa de nenhum aqui so pra "existir" no ctest.
