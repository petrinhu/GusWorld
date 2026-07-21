// cards_hw_i18n_registry_test.cpp
//
// CARDS-HW-QA1 fatia D (TODO.md): duas verificacoes cruzadas decision-free da onda
// cartas-hardware que os unitarios por-servico nao cobrem:
//
//   Parte 1 - registro global de chaves i18n outcome->chave da onda cartas-hardware
//   (turing_service::translation_key_for + contamination_service::translation_key_for).
//   Racional: cada servico ja testa sua PROPRIA tabela outcome->chave em isolamento
//   (turing_service_test.cpp, contamination_service_test.cpp) - o gap CRUZADO e garantir
//   que NENHUMA chave se repete ENTRE servicos. Chave duplicada entre servicos = dois
//   efeitos DIFERENTES emitindo o MESMO log diegetico pro jogador - violacao silenciosa
//   da regra "todo efeito loga" (feedback_todo_efeito_loga_terminal): o jogador veria a
//   mesma frase pra duas causas distintas e nao teria como distinguir o que aconteceu.
//
//   EXCECAO deliberada (NAO se aplica aqui, documentada por transparencia): a chave
//   ContaminationRollOutcome::Infected e AMBIGUA DE PROPOSITO NO CONTEUDO SEMANTICO (nao
//   confirma infeccao, so avisa "checksum estranho" - a certeza fica reservada ao
//   diagnostico do Turing; ja coberto por contamination_service_test.cpp). Essa
//   ambiguidade e DENTRO do servico (o TEXTO final que o jogador le e deliberadamente
//   vago) - a STRING da chave em si ("CONTAMINATION_ACQUIRE_SUSPICIOUS_CHECKSUM")
//   continua UNICA e nao colide com nenhuma chave de outro servico. A exigencia de
//   unicidade GLOBAL continua valendo sem excecao; se um dia uma chave nova colidir
//   ENTRE servicos, isso E um achado de QA, nao uma variacao aceitavel.
//
//   Parte 2 - bordas diretas de card_integrity_ledger.hpp (o vetor CardIntegrityRef
//   injetado na CombatStateMachine, "o ledger"). Lido backdoor_signal_test.cpp e
//   card_virus_combat_test.cpp antes de escrever: os dois cobrem ledger NULO (fail-safe),
//   MULTIPLAS entradas com owner_actor_id distinto/igual (dedup do leaked_intel,
//   propagacao do worm por direcao) e o caminho normal do lookup por instance_id
//   presente. Tres bordas NAO estao cobertas la, todas envolvendo o vetor NAO-NULO:
//     (a) ledger vazio (vetor existe, size()==0) - branch distinto de ledger==nullptr
//         dentro de find_ledger_entry (o loop sobre um range vazio nunca e exercitado
//         separado do ponteiro nulo em nenhum teste existente);
//     (b) consulta de instance_id AUSENTE num ledger NAO-vazio (a varredura linear
//         percorre entradas reais e nao acha match - so testado hoje via nullptr, nunca
//         via "ledger tem coisa, so nao tem ESSA instancia");
//     (c) MESMA instance_id registrada 2x - contrato implicito nunca documentado em
//         teste: find_ledger_entry faz varredura LINEAR e para no PRIMEIRO match (nao ha
//         checagem de unicidade nem "ultimo vence"). Instance_id duplicado nao deveria
//         acontecer em jogo normal (card_collection.hpp gera ids unicos), mas o
//         comportamento do lookup diante disso e um contrato observavel do motor que
//         merece ficar travado em teste - se um dia find_ledger_entry virar um map ou
//         mudar a ordem de varredura, este teste denuncia a mudanca de contrato.
//   CardIntegrityRef nao declara operator== (struct agregado simples) - nao ha
//   copia/igualdade extra pra testar aqui.
//
// Cross-ref: gus/domain/deck/turing_service.hpp; gus/domain/deck/contamination_service.hpp;
//            gus/domain/combat/card_integrity_ledger.hpp;
//            gus/domain/combat/combat_state_machine.cpp (find_ledger_entry, linha ~1966);
//            domain/tests/turing_service_test.cpp; domain/tests/contamination_service_test.cpp;
//            domain/tests/backdoor_signal_test.cpp; domain/tests/card_virus_combat_test.cpp;
//            TODO.md CARDS-HW-QA1.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "fixed_random.hpp"
#include "gus/domain/combat/card_integrity_ledger.hpp"
#include "gus/domain/combat/combat_actor.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"
#include "gus/domain/combat/combat_state.hpp"
#include "gus/domain/combat/combat_state_machine.hpp"
#include "gus/domain/combat/random_source.hpp"
#include "gus/domain/deck/card_hardware.hpp"
#include "gus/domain/deck/contamination_service.hpp"
#include "gus/domain/deck/turing_service.hpp"
#include "gus/domain/infection/integrity_state.hpp"

using gus::domain::tests::FixedRandom;

namespace {

// ---- helper de formato: UPPER_SNAKE_CASE, char a char, SEM <regex> ---------------------
bool is_upper_snake_case(std::string_view key) {
    if (key.empty()) return false;
    for (char c : key) {
        const bool is_upper = (c >= 'A' && c <= 'Z');
        const bool is_digit = (c >= '0' && c <= '9');
        const bool is_underscore = (c == '_');
        if (!is_upper && !is_digit && !is_underscore) return false;
    }
    return true;
}

}  // namespace

// ===== Parte 1: registro global de chaves i18n =========================================

TEST_CASE("cards_hw i18n registry: todas as chaves outcome->i18n da onda cartas-hardware "
         "sao nao-vazias, UPPER_SNAKE_CASE e GLOBALMENTE unicas entre servicos",
         "[domain][cards_hw][i18n]") {
    using gus::domain::deck::ContaminationRollOutcome;
    using gus::domain::deck::CureOutcome;
    using gus::domain::deck::DiagnoseOutcome;

    // Arrange: enumeracao EXAUSTIVA de cada enum de outcome que tem mapeamento i18n hoje
    // (gus/domain/deck/turing_service.hpp + gus/domain/deck/contamination_service.hpp -
    // os 2 UNICOS arquivos com translation_key_for() na onda cartas-hardware, confirmado
    // por grep em domain/include/ e domain/src/). Nenhum enumerador e omitido - se um
    // outcome novo for adicionado a qualquer um dos 3 enums e nao aparecer aqui, o teste
    // NAO pega esse caso (documentado: manutencao manual, mesmo padrao dos testes
    // per-servico que ja fazem essa enumeracao explicita).
    const std::vector<std::string_view> all_keys = {
        // DiagnoseOutcome (2 valores).
        gus::domain::deck::translation_key_for(DiagnoseOutcome::Diagnosed),
        gus::domain::deck::translation_key_for(DiagnoseOutcome::RejectedNotInfected),
        // CureOutcome (4 valores).
        gus::domain::deck::translation_key_for(CureOutcome::Cured),
        gus::domain::deck::translation_key_for(CureOutcome::Burned),
        gus::domain::deck::translation_key_for(CureOutcome::RejectedNotDiagnosed),
        gus::domain::deck::translation_key_for(CureOutcome::RejectedProtectedTier),
        // ContaminationRollOutcome (3 valores).
        gus::domain::deck::translation_key_for(ContaminationRollOutcome::Infected),
        gus::domain::deck::translation_key_for(ContaminationRollOutcome::Clean),
        gus::domain::deck::translation_key_for(ContaminationRollOutcome::SkippedProtectedTier),
    };

    // Sanity: documenta a contagem exaustiva (2+4+3=9) - se um enumerador for adicionado
    // sem atualizar este vetor, este assert nao pega isso (limitacao conhecida, ver
    // comentario acima); o que ELE pega e alguem remover uma linha por engano.
    REQUIRE(all_keys.size() == 9);

    // Act + Assert (a): nenhuma vazia, todas UPPER_SNAKE_CASE.
    for (std::string_view key : all_keys) {
        REQUIRE_FALSE(key.empty());
        REQUIRE(is_upper_snake_case(key));
    }

    // Assert (b): TODAS globalmente unicas entre servicos (std::set dedup - size igual).
    const std::set<std::string_view> unique_keys(all_keys.begin(), all_keys.end());
    REQUIRE(unique_keys.size() == all_keys.size());
}

// ===== Parte 2: bordas diretas do card_integrity_ledger =================================

namespace {

using namespace gus::domain::combat;
using gus::domain::infection::IntegrityState;
using gus::domain::infection::VirusKind;

CombatActor make_actor(const std::string& id, bool player_side, int spd) {
    return CombatActor(id, id, /*hp=*/100, /*atk=*/0, /*def=*/0, spd, CardFamily::Eletrico,
                       player_side);
}

// Carta COMUM local (nunca do catalogo de producao, mesma convencao de
// card_virus_combat_test.cpp/backdoor_signal_test.cpp): dano baixo, foco no ledger.
Card ledger_test_card(const std::string& id) {
    Card c;
    c.id = id;
    c.display_name = id;
    c.family = CardFamily::Cinetico;
    c.base_type = CardBaseType::Pulso;
    c.mana_cost = 0;
    c.ap_cost = 1;
    c.power = 0;
    c.target_shape = TargetShape::Single;
    c.tier = CardTier::Comum;
    return c;
}

std::unordered_map<std::string, Card> registry(std::initializer_list<Card> cards) {
    std::unordered_map<std::string, Card> d;
    for (const auto& c : cards) d.emplace(c.id, c);
    return d;
}

// Toca 1 acao quando e a vez do lado da party; dali em diante passa. Mesmo padrao de
// card_virus_combat_test.cpp::play_sequence.
CombatActionProvider play_once(CombatAction action) {
    auto acted = std::make_shared<bool>(false);
    return [acted, action](CombatActor& a, const CombatState&) -> CombatAction {
        if (!a.is_player_side() || *acted) return CombatAction::pass();
        *acted = true;
        return action;
    };
}

const StatusEffect* find_status(const CombatActor& a, StatusId id) {
    for (const auto& s : a.status_effects())
        if (s.id == id) return &s;
    return nullptr;
}

bool log_has(const CombatStateMachine& sm, const std::string& needle) {
    for (const auto& entry : sm.log())
        if (entry.message.find(needle) != std::string::npos) return true;
    return false;
}

}  // namespace

TEST_CASE("card_integrity_ledger: ledger VAZIO (vetor nao-nulo, size()==0) se comporta "
         "IDENTICO a ledger nulo - nenhum payload de virus dispara (branch do range vazio "
         "dentro de find_ledger_entry, distinto do early-return de ponteiro nulo, nunca "
         "exercitado nos testes de virus/backdoor existentes)",
         "[domain][combat][cards_hw][card_integrity_ledger]") {
    // Arrange.
    CombatActor caster = make_actor("h", true, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*spd=*/10);

    Card worm_card = ledger_test_card("ledger.empty.card");
    auto reg = registry({worm_card});

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 1;

    const std::vector<CardIntegrityRef> empty_ledger;  // NAO nullptr - so vazio.
    FixedRandom rng;  // default (0.5, 99): sem propagacao mesmo que o worm disparasse.
    auto provider = play_once(cast);

    // Act.
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &empty_ledger);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // Assert: nenhum efeito de worm (Slow) e nenhum log de virus.
    REQUIRE(find_status(caster, StatusId::Slow) == nullptr);
    REQUIRE_FALSE(log_has(sm, "desempenho degradado"));
}

TEST_CASE("card_integrity_ledger: consulta de instance_id AUSENTE num ledger NAO-vazio nao "
         "aplica o payload de OUTRA instancia infectada - find_ledger_entry so casa por "
         "instance_id EXATO, nunca 'ledger tem alguma infeccao' de forma generica",
         "[domain][combat][cards_hw][card_integrity_ledger]") {
    // Arrange: o ledger TEM uma entrada infectada, mas de outra instancia (999) - a carta
    // conjurada usa instance_id=1, que NAO existe no ledger.
    CombatActor caster = make_actor("h", true, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*spd=*/10);

    IntegrityState other_instance_infected;
    other_instance_infected.is_infected = true;
    other_instance_infected.virus_kind = VirusKind::Worm;
    const std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/999, "outra.carta.qualquer", /*owner_actor_id=*/1,
                         &other_instance_infected}};

    Card plain_card = ledger_test_card("ledger.absent.instance.card");
    auto reg = registry({plain_card});

    CombatAction cast = CombatAction::use_card(plain_card.id, filler.id());
    cast.card_instance_id = 1;  // NAO bate com nenhuma entrada do ledger (que so tem 999).

    FixedRandom rng;
    auto provider = play_once(cast);

    // Act.
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // Assert: a infeccao da instancia 999 nunca "vaza" pra instancia 1, sem match.
    REQUIRE(find_status(caster, StatusId::Slow) == nullptr);
    REQUIRE_FALSE(log_has(sm, "desempenho degradado"));
    REQUIRE(other_instance_infected.is_infected);  // instancia 999 continua intocada.
}

TEST_CASE("card_integrity_ledger: MESMA instance_id registrada 2x - find_ledger_entry "
         "(varredura linear) resolve pra PRIMEIRA entrada do vetor quando ela ESTA "
         "infectada, a segunda entrada (mesma instance_id) nunca e consultada",
         "[domain][combat][cards_hw][card_integrity_ledger][regression]") {
    // Arrange: 1a entrada (instance_id=7) infectada com Worm; 2a entrada, MESMA instance_id,
    // limpa. Se o payload disparar, prova que o lookup achou a 1a (a unica infectada).
    CombatActor caster = make_actor("h", true, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*spd=*/10);

    IntegrityState first_infected;
    first_infected.is_infected = true;
    first_infected.virus_kind = VirusKind::Worm;
    IntegrityState second_clean;  // mesma instance_id, mas nunca deveria ser consultada.

    Card worm_card = ledger_test_card("ledger.dup.instance.first.wins.card");
    auto reg = registry({worm_card});
    const std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/7, worm_card.id, /*owner_actor_id=*/1, &first_infected},
        CardIntegrityRef{/*instance_id=*/7, worm_card.id, /*owner_actor_id=*/2, &second_clean}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 7;

    FixedRandom rng;
    auto provider = play_once(cast);

    // Act.
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // Assert: efeito disparou (achou a 1a entrada, infectada).
    REQUIRE(find_status(caster, StatusId::Slow) != nullptr);
    REQUIRE(log_has(sm, "desempenho degradado"));
}

TEST_CASE("card_integrity_ledger: MESMA instance_id registrada 2x - quando a PRIMEIRA "
         "entrada esta LIMPA, ela 'sombreia' a segunda entrada (mesma instance_id) mesmo "
         "que a segunda esteja infectada - confirma first-match-wins, nao 'qualquer match "
         "infectado vence'",
         "[domain][combat][cards_hw][card_integrity_ledger][regression]") {
    // Arrange: ordem invertida do teste anterior - 1a entrada (instance_id=7) LIMPA; 2a
    // entrada, MESMA instance_id, infectada com Worm. Se nao houver efeito, prova que o
    // lookup parou na 1a (limpa) e nunca alcancou a 2a.
    CombatActor caster = make_actor("h", true, /*spd=*/50);
    CombatActor filler = make_actor("e", false, /*spd=*/10);

    IntegrityState first_clean;
    IntegrityState second_infected;
    second_infected.is_infected = true;
    second_infected.virus_kind = VirusKind::Worm;

    Card worm_card = ledger_test_card("ledger.dup.instance.first.shadows.card");
    auto reg = registry({worm_card});
    const std::vector<CardIntegrityRef> ledger = {
        CardIntegrityRef{/*instance_id=*/7, worm_card.id, /*owner_actor_id=*/1, &first_clean},
        CardIntegrityRef{/*instance_id=*/7, worm_card.id, /*owner_actor_id=*/2,
                         &second_infected}};

    CombatAction cast = CombatAction::use_card(worm_card.id, filler.id());
    cast.card_instance_id = 7;

    FixedRandom rng;
    auto provider = play_once(cast);

    // Act.
    CombatStateMachine sm({&caster, &filler}, provider, &reg, nullptr, &rng, &ledger);
    sm.begin_turn();
    sm.run_active_turn_to_end();

    // Assert: nenhum efeito - a 1a entrada (limpa) "ganhou" o lookup, a 2a (infectada)
    // nunca foi consultada. A instancia infectada continua intocada (nao foi "curada" nem
    // "achada" - so nunca foi olhada).
    REQUIRE(find_status(caster, StatusId::Slow) == nullptr);
    REQUIRE_FALSE(log_has(sm, "desempenho degradado"));
    REQUIRE(second_infected.is_infected);
}
