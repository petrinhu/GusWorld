// deck_invariants_property_test.cpp
//
// DECK-5 (revisao ADVERSARIAL INDEPENDENTE, qa-engineer): property-based testing "a
// mao" (property_gen.hpp, mesmo padrao de initiative_queue_property_test.cpp) sobre os
// invariantes anti-exploit do sistema de deck/mao (DECK-1/2/3/4,
// docs/design/mecanicas/deck-mao-sistema.md secao 7):
//
//   (a) toda instancia esta em EXATAMENTE 1 container (ativo XOR morto) apos qualquer
//       sequencia arbitraria de operacoes (inv.1).
//   (b) nenhum instance_id duplicado nunca, nem dentro de um container nem entre eles.
//   (c) a mao nunca referencia id fora do ativo APOS revalidate() (inv.6 - "mao so puxa
//       do ativo").
//   (d) credits (carteira UNICA da party) nunca fica negativo.
//   (e) carta Especial/Super (classe PROTEGIDA, inv.9) NUNCA aparece no deck morto.
//   (f) a soma de creditos e conservada exatamente pelas primitivas: sell()/upload()
//       creditam o preco passado, acquire() debita o preco passado, craft() NUNCA mexe
//       em credits - nenhuma outra via muta a carteira (sem "criar"/"destruir" dinheiro
//       fora das 3 primitivas que tocam credits).
//   (g) roundtrip do save (serialize_save/deserialize_save, SAVE V6/DECK-4) preserva
//       card_collection + hand_selection + credits para qualquer estado alcancavel pela
//       sequencia aleatoria (apos revalidate(), unico jeito de o estado satisfazer
//       CharacterSaveState::validate() - "mao so referencia instance_id do ativo").
//
// Sequencia aleatoria varre as 4 transacoes atomicas (deck_transactions.hpp:
// sell/upload/acquire/craft) + as mutacoes diretas de CardCollection
// (add_to_active/discard_to_dead) + as mutacoes de HandLoadout
// (add_card/remove_card/swap_card/set_selection/revalidate) - exercitando os caminhos
// de FALHA esperada (capacidade cheia, tier protegido, id ausente, saldo insuficiente,
// duplicata) tanto quanto os de sucesso; excecoes de fluxo esperado (std::invalid_argument/
// std::logic_error) sao capturadas e ignoradas - o teste verifica que o ESTADO
// permanece coerente independente de qual ramo foi tomado.
//
// Determinismo TOTAL (LCG semeado por indice de caso, property_gen.hpp): zero
// Date/random real. Reproducao garantida com a mesma seed.
//
// NAO altera codigo de producao (isto e o oraculo; a fase de mutation testing roda
// FORA deste arquivo, revertendo cada mutante). POCO puro, ZERO Qt.
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 7; card_collection.hpp/
// hand_loadout.hpp/deck_transactions.hpp; gus/domain/save/save_data.hpp (V6).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_records.hpp"
#include "gus/domain/deck/deck_transactions.hpp"
#include "gus/domain/deck/hand_loadout.hpp"
#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"
#include "property_gen.hpp"

using namespace gus::domain::deck;
using gus::domain::combat::CardTier;
using gus::domain::tests::Lcg;

namespace {

constexpr int kCasesPerProperty = 1500;
constexpr int kStepsPerCase = 50;
constexpr int kRoundtripCases = 300;
constexpr int kRoundtripSteps = 30;

// Catalogo-fake mapeado por PREFIXO (mesma convencao de card_collection_test.cpp, so
// que por prefixo em vez de sufixo - sem ambiguidade com "card_N"): "esp_*" = Especial,
// "sup_*" = Super, qualquer outro = Comum.
CardTier tier_of_pool(const std::string& card_id) {
    if (card_id.rfind("esp_", 0) == 0) return CardTier::Especial;
    if (card_id.rfind("sup_", 0) == 0) return CardTier::Super;
    return CardTier::Comum;
}

// card_id aleatorio: ~8% Especial, ~6% Super, resto Comum (pool pequeno de 10 IDs
// comuns + 3 especiais + 3 supers - repeticao proposital, testa dup/instancias
// multiplas do MESMO card_id, que e legitimo - card_id != instance_id).
std::string random_card_id(Lcg& g) {
    const int roll = g.in_range(0, 99);
    if (roll < 8) return "esp_" + std::to_string(g.in_range(0, 2));
    if (roll < 14) return "sup_" + std::to_string(g.in_range(0, 2));
    return "card_" + std::to_string(g.in_range(0, 9));
}

// ---- (a)/(b)/(e): invariantes ESTRUTURAIS do container, checaveis a qualquer momento.

void check_container_invariants(const CardCollection& deck, const CardCollection::TierLookup& tier_of) {
    std::set<std::uint64_t> seen;
    for (const auto& c : deck.active()) {
        REQUIRE(c.instance_id != 0);
        // (b) sem duplicata DENTRO do ativo, e (a) via a insercao unica abaixo somada
        // com o loop do morto - qualquer id repetido entre os dois containers falha
        // aqui (segundo insert() no MESMO set).
        REQUIRE(seen.insert(c.instance_id).second);
    }
    for (const auto& c : deck.dead()) {
        REQUIRE(c.instance_id != 0);
        // (a) XOR: id do morto NUNCA tambem esta no ativo (nem repetido dentro do
        // proprio morto) - mesmo set, mesma checagem.
        REQUIRE(seen.insert(c.instance_id).second);
        // (e) classe PROTEGIDA (Especial/Super) NUNCA aparece no deck morto (inv.9).
        const CardTier tier = tier_of(c.card_id);
        REQUIRE(tier != CardTier::Especial);
        REQUIRE(tier != CardTier::Super);
    }
}

// ---- (d)/(f): carteira nunca negativa E exatamente igual ao esperado (conservacao).

void check_wallet_invariant(std::int64_t credits, std::int64_t expected_credits) {
    REQUIRE(credits >= 0);
    REQUIRE(credits == expected_credits);
}

// Candidato de instance_id PARA A MAO: ~55% id LEGITIMO do ativo, ~30% id do deck
// MORTO (ataque direto ao caminho "morto->ativo indireto via mao" - inv.6), ~15% id
// BOGUS nunca emitido (ataque ao guard de "instancia inexistente"). Sem pool nao-vazio
// aplicavel, cai pro bogus. Este e o vetor de exploit que qualquer guard de
// HandLoadout::validate_candidate PRECISA recusar sempre.
std::uint64_t random_candidate_id(Lcg& g, const CardCollection& collection) {
    const int roll = g.in_range(0, 99);
    if (roll < 55 && !collection.active().empty()) {
        return collection
            .active()[static_cast<std::size_t>(g.in_range(0, static_cast<int>(collection.active().size()) - 1))]
            .instance_id;
    }
    if (roll < 85 && !collection.dead().empty()) {
        return collection
            .dead()[static_cast<std::size_t>(g.in_range(0, static_cast<int>(collection.dead().size()) - 1))]
            .instance_id;
    }
    return static_cast<std::uint64_t>(g.in_range(1000000, 2000000));  // nunca emitido
}

// ---- (c): apos revalidate(), toda a selecao da mao referencia o deck ATIVO.

void check_hand_only_active(const HandLoadout& hand, const CardCollection& deck) {
    for (const std::uint64_t id : hand.selection()) {
        const bool present = std::any_of(deck.active().begin(), deck.active().end(),
                                          [id](const CardInstance& c) { return c.instance_id == id; });
        REQUIRE(present);
    }
}

// Aplica UMA operacao aleatoria (op em [0,10]) sobre o estado do caso. Excecoes de
// fluxo esperado (guard de capacidade/tier/presenca/duplicata) sao capturadas - o
// objetivo e exercitar TANTO o caminho de sucesso quanto o de falha, sem deixar
// nenhuma excecao escapar do property loop. `expected_credits` e o oraculo de
// conservacao (f): so as 3 primitivas que tocam credits (sell/upload creditam,
// acquire debita) o atualizam; craft() nunca mexe em credits.
//
// Devolve TRUE quando, e SO quando, esta operacao GARANTE (c) - a selecao inteira da
// mao acabou de ser revalidada contra o deck ativo com sucesso (add_card/swap_card/
// set_selection que NAO lancaram, ou revalidate() - que por definicao SEMPRE remove
// todo orfao). Devolve FALSE nos demais casos: ops de container puro (0-5, podem
// legitimamente orfanizar a mao ate o proximo revalidate - inv.6 e uma garantia
// "apos revalidate", nao um invariante permanente), remove_card (so encolhe - NAO
// revalida o restante da selecao, entao um orfao ANTERIOR pode continuar la) e
// qualquer add_card/swap_card/set_selection que FALHOU (a selecao anterior - que pode
// ja conter um orfao transitorio de uma venda/descarte externo em um passo anterior -
// fica intocada). Chamar check_hand_only_active() fora dessas garantias e FALSO
// POSITIVO (acusaria uma orfandade transitoria LEGITIMA, nao um bug).
bool apply_random_step(Lcg& g, int op, CardCollection& collection, HandLoadout& hand,
                        std::int64_t& credits, std::int64_t& expected_credits) {
    switch (op) {
        case 0: {  // add_to_active direto (sem passar pela transacao acquire)
            try {
                collection.add_to_active(random_card_id(g));
            } catch (const std::logic_error&) {
                // deck ativo na capacidade maxima - esperado, sem efeito colateral.
            }
            return false;
        }
        case 1: {  // discard_to_dead direto
            if (!collection.active().empty()) {
                const auto id = collection
                                     .active()[static_cast<std::size_t>(
                                         g.in_range(0, static_cast<int>(collection.active().size()) - 1))]
                                     .instance_id;
                try {
                    collection.discard_to_dead(id, tier_of_pool);
                } catch (const std::invalid_argument&) {
                    // tier protegido (Especial/Super) - esperado, RECUSADO sem mutar.
                }
            }
            return false;
        }
        case 2: {  // sell (DECK-3) - id LEGITIMO (ativo) OU envenenado (morto/bogus, ataca
                   // a idempotencia inv.5 - "instancia ja fora do ativo NUNCA credita")
            const std::uint64_t id = random_candidate_id(g, collection);
            const int price = g.in_range(0, 20);
            const SellResult res = sell(collection, credits, id, price, tier_of_pool);
            if (res.ok()) expected_credits += price;
            return false;
        }
        case 3: {  // upload (DECK-3) - mesmo ataque de idempotencia que sell (id
                   // LEGITIMO ou envenenado morto/bogus)
            const std::uint64_t id = random_candidate_id(g, collection);
            const int price = g.in_range(0, 5);
            const UploadResult res = upload(collection, credits, id, price, tier_of_pool);
            if (res.ok()) expected_credits += price;
            return false;
        }
        case 4: {  // acquire (DECK-3) - preco pode exceder saldo (InsufficientCredits) de proposito
            const std::string card_id = random_card_id(g);
            const int price = g.in_range(0, 25);
            const AcquireResult res = acquire(collection, credits, card_id, price);
            if (res.ok()) expected_credits -= price;
            return false;
        }
        case 5: {  // craft (DECK-3) - NUNCA mexe em credits, so container
            const std::string card_id = random_card_id(g);
            const bool has_material = g.coin();
            const CraftResult res = craft(collection, card_id, [has_material] { return has_material; });
            (void)res;  // conservacao (f): expected_credits intocado de proposito.
            return false;
        }
        case 6: {  // hand.add_card - id LEGITIMO (ativo) OU envenenado (morto/bogus, de proposito)
            const std::uint64_t id = random_candidate_id(g, collection);
            try {
                hand.add_card(id, collection, tier_of_pool);
                return true;  // validate_candidate revalidou a selecao INTEIRA com sucesso
            } catch (const std::invalid_argument&) {
                // duplicata / fora do ativo (morto/bogus, inv.6) / teto de comum / slot
                // especial indevido - esperado; selecao anterior (pode ja ter orfao
                // transitorio de um passo anterior) fica intocada.
                return false;
            }
        }
        case 7: {  // hand.remove_card - so ENCOLHE, nunca revalida o RESTANTE da selecao
            if (!hand.selection().empty()) {
                const auto id = hand.selection()[static_cast<std::size_t>(
                    g.in_range(0, static_cast<int>(hand.selection().size()) - 1))];
                REQUIRE_NOTHROW(hand.remove_card(id));
            }
            return false;
        }
        case 8: {  // hand.swap_card - new_id LEGITIMO (ativo) OU envenenado (morto/bogus)
            if (!hand.selection().empty()) {
                const auto old_id = hand.selection()[static_cast<std::size_t>(
                    g.in_range(0, static_cast<int>(hand.selection().size()) - 1))];
                const std::uint64_t new_id = random_candidate_id(g, collection);
                try {
                    hand.swap_card(old_id, new_id, collection, tier_of_pool);
                    return true;  // validate_candidate revalidou a selecao INTEIRA
                } catch (const std::invalid_argument&) {
                    // new_id ja selecionado / fora do ativo (morto/bogus) / estoura teto /
                    // slot especial - esperado.
                }
            }
            return false;
        }
        case 9: {  // hand.set_selection - candidata mistura ids LEGITIMOS e ENVENENADOS
            std::vector<std::uint64_t> candidate;
            const int n = g.in_range(0, 5);
            for (int i = 0; i < n; ++i) {
                candidate.push_back(random_candidate_id(g, collection));
            }
            try {
                hand.set_selection(candidate, collection, tier_of_pool);
                return true;  // validate_candidate revalidou a NOVA selecao inteira
            } catch (const std::invalid_argument&) {
                // candidata invalida (dup/fora do ativo/teto/slot) - esperado, selecao
                // atual (pode ja ter orfao transitorio) intocada.
                return false;
            }
        }
        default: {  // 10: hand.revalidate - SEMPRE remove todo orfao, por definicao
            hand.revalidate(collection);
            return true;
        }
    }
}

}  // namespace

// ===== (a)(b)(d)(e)(f): invariantes estruturais + carteira sob sequencia arbitraria ==

TEST_CASE("property: deck/mao mantem XOR de container + sem duplicata + carteira "
          "conservada e nao-negativa + classe protegida fora do morto, sob qualquer "
          "sequencia de operacoes",
          "[domain][deck][property]") {
    for (int c = 0; c < kCasesPerProperty; ++c) {
        Lcg g(0xDEC00000u + static_cast<unsigned>(c));

        const int capacity = g.in_range(2, 6);  // pequeno: forca full/edge com frequencia
        CardCollection collection(capacity);
        const bool is_gus = g.coin();
        const int comum_capacity = g.in_range(0, 4);
        HandLoadout hand(comum_capacity, is_gus);
        std::int64_t credits = g.in_range(0, 50);
        std::int64_t expected_credits = credits;

        check_container_invariants(collection, tier_of_pool);
        check_wallet_invariant(credits, expected_credits);

        for (int step = 0; step < kStepsPerCase; ++step) {
            const int op = g.in_range(0, 10);
            const bool hand_fully_revalidated =
                apply_random_step(g, op, collection, hand, credits, expected_credits);

            check_container_invariants(collection, tier_of_pool);
            check_wallet_invariant(credits, expected_credits);
            // (c): SO quando apply_random_step devolve true - a selecao INTEIRA acabou
            // de ser revalidada com sucesso contra o deck ativo (add_card/swap_card/
            // set_selection que NAO lancaram, ou revalidate(), que sempre garante) -
            // afirmamos que NENHUM id da mao esta fora do ativo. Checar fora dessas
            // garantias seria falso-positivo: um orfao transitorio legitimo (deixado
            // por um sell/upload/discard EXTERNO num passo anterior, ainda nao
            // revalidado) pode ficar na selecao entre revalidacoes - a spec garante (c)
            // "apos revalidate", nao a cada passo.
            if (hand_fully_revalidated) {
                check_hand_only_active(hand, collection);
            }
        }
    }
}

// ===== (g): roundtrip do save preserva card_collection/hand_selection/credits ========

TEST_CASE("property: roundtrip do save (V6) preserva card_collection + hand_selection "
          "+ credits para qualquer estado alcancavel pela sequencia aleatoria",
          "[domain][deck][property][save]") {
    for (int c = 0; c < kRoundtripCases; ++c) {
        Lcg g(0xDEC10000u + static_cast<unsigned>(c));

        const int capacity = g.in_range(2, 6);
        CardCollection collection(capacity);
        const bool is_gus = g.coin();
        const int comum_capacity = g.in_range(0, 4);
        HandLoadout hand(comum_capacity, is_gus);
        std::int64_t credits = g.in_range(0, 50);
        std::int64_t expected_credits = credits;

        for (int step = 0; step < kRoundtripSteps; ++step) {
            const int op = g.in_range(0, 10);
            apply_random_step(g, op, collection, hand, credits, expected_credits);
        }

        // Espelha o fluxo real (gameplay chama revalidate() antes de salvar): so ai o
        // estado satisfaz CharacterSaveState::validate() (hand so referencia o ativo).
        hand.revalidate(collection);
        check_hand_only_active(hand, collection);
        check_container_invariants(collection, tier_of_pool);
        check_wallet_invariant(credits, expected_credits);

        gus::domain::save::CharacterSaveState char_state;
        char_state.current_hp = 34;
        char_state.xp = 55;
        char_state.card_collection.active = collection.active();
        char_state.card_collection.dead = collection.dead();
        char_state.card_collection.next_instance_id = collection.next_instance_id();
        char_state.hand_selection = hand.selection();

        gus::domain::save::SaveData save_data;
        save_data.timestamp_ms = 1721140000000LL + c;
        save_data.current_scene_path = "res://property/deck_invariants.tscn";
        save_data.character_states = {{"property_char", char_state}};
        save_data.credits = credits;

        const auto restored =
            gus::domain::save::deserialize_save(gus::domain::save::serialize_save(save_data));

        REQUIRE(restored == save_data);
        REQUIRE(restored.credits == credits);
        const auto& restored_state = restored.character_states.at("property_char");
        REQUIRE(restored_state.card_collection.active == collection.active());
        REQUIRE(restored_state.card_collection.dead == collection.dead());
        REQUIRE(restored_state.card_collection.next_instance_id == collection.next_instance_id());
        REQUIRE(restored_state.hand_selection == hand.selection());
    }
}

// ===== (g-adversarial): roundtrip com COLECAO ADULTERADA em memoria e SEMPRE rejeitado
// =====                  (instance_id duplicado / mao apontando pro vazio / credits
// =====                  negativo), sobre a MESMA diversidade de estados alcancaveis
// =====                  da sequencia aleatoria (nao so os fixtures fixos de
// =====                  save_v6_test.cpp - aqui a "forma" da colecao/mao varia por seed).

TEST_CASE("property: save rejeita QUALQUER adulteracao em memoria (dup instance_id, "
          "hand orfa, credits negativo) para qualquer estado base alcancavel",
          "[domain][deck][property][save][adversarial]") {
    for (int c = 0; c < kRoundtripCases; ++c) {
        Lcg g(0xDEC20000u + static_cast<unsigned>(c));

        const int capacity = g.in_range(2, 6);
        CardCollection collection(capacity);
        const bool is_gus = g.coin();
        const int comum_capacity = g.in_range(0, 4);
        HandLoadout hand(comum_capacity, is_gus);
        std::int64_t credits = g.in_range(0, 50);
        std::int64_t expected_credits = credits;

        for (int step = 0; step < kRoundtripSteps; ++step) {
            const int op = g.in_range(0, 10);
            apply_random_step(g, op, collection, hand, credits, expected_credits);
        }
        hand.revalidate(collection);

        gus::domain::save::CharacterSaveState char_state;
        char_state.current_hp = 34;
        char_state.xp = 55;
        char_state.card_collection.active = collection.active();
        char_state.card_collection.dead = collection.dead();
        char_state.card_collection.next_instance_id = collection.next_instance_id();
        char_state.hand_selection = hand.selection();

        gus::domain::save::SaveData save_data;
        save_data.timestamp_ms = 1721150000000LL + c;
        save_data.current_scene_path = "res://property/deck_invariants_adversarial.tscn";
        save_data.character_states = {{"property_char", char_state}};
        save_data.credits = credits;

        // Estado BASE (sem adulteracao) tem que continuar valido - confirma que a
        // adulteracao abaixo (nao o estado base) e a causa da rejeicao.
        REQUIRE_NOTHROW(gus::domain::save::serialize_save(save_data));

        // Adultera EM MEMORIA de 1 de 3 jeitos (rotaciona por seed) - cada um ataca um
        // invariante DIFERENTE do validate() fail-fast.
        auto adulterated = save_data;
        auto& state = adulterated.character_states.at("property_char");
        const int poison = g.in_range(0, 2);
        if (poison == 0) {
            // instance_id duplicado: clona a 1a entrada do ativo (se vazio, injeta uma
            // dupla direto no ativo) - viola inv.1 (XOR/unicidade).
            const std::uint64_t dup_id =
                state.card_collection.active.empty() ? 1 : state.card_collection.active.front().instance_id;
            if (state.card_collection.active.empty()) {
                state.card_collection.active.push_back(CardInstance{dup_id, "card_dup_seed"});
                if (state.card_collection.next_instance_id <= dup_id) {
                    state.card_collection.next_instance_id = dup_id + 1;
                }
            }
            state.card_collection.dead.push_back(CardInstance{dup_id, "card_dup"});
        } else if (poison == 1) {
            // mao apontando pro vazio: instance_id que nunca existiu em NENHUM container.
            state.hand_selection.push_back(9000000ULL + static_cast<std::uint64_t>(c));
        } else {
            // credits (carteira da party) negativo.
            adulterated.credits = -1;
        }

        REQUIRE_THROWS_AS(gus::domain::save::serialize_save(adulterated), std::invalid_argument);
    }
}
