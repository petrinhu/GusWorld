// SPDX-License-Identifier: Apache-2.0
// gus/domain/deck/deck_transactions.cpp
//
// Implementacao das transacoes atomicas do sistema de deck/mao (DECK-3). Ver
// deck_transactions.hpp pra garantia forte + idempotencia + fail-fast reservado a
// invariante de programacao.

#include "gus/domain/deck/deck_transactions.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "gus/domain/deck/contamination_service.hpp"

namespace gus::domain::deck {

using gus::domain::combat::CardTier;

namespace {

// Acha a instancia no ativo por leitura publica (CardCollection::active()) - usado
// pra classificar a falha (InstanceNotInActive vs ProtectedTier) SEM depender de
// capturar excecao de CardCollection::remove_for_sale() como controle de fluxo.
const CardInstance* find_in_active(const CardCollection& collection, std::uint64_t instance_id) {
    const auto& active = collection.active();
    auto it = std::find_if(active.begin(), active.end(),
                            [instance_id](const CardInstance& c) { return c.instance_id == instance_id; });
    return it == active.end() ? nullptr : &*it;
}

// Nucleo comum de sell()/upload(): valida presenca+tier ANTES de mutar (garantia
// forte), so entao remove do ativo e credita. Devolve {error, instance, credited}
// generico - sell()/upload() so envolvem no Result tipado proprio.
struct RemovalOutcome {
    TransactionError error;
    CardInstance instance;
    int credited;
};

RemovalOutcome remove_and_credit(CardCollection& collection, std::int64_t& credits,
                                  std::uint64_t instance_id, int price,
                                  const CardCollection::TierLookup& tier_of,
                                  const char* caller_name) {
    if (price < 0) {
        throw std::invalid_argument(std::string("deck_transactions::") + caller_name +
                                     ": price/upload_price nao pode ser negativo (invariante de "
                                     "programacao - o caller escolhe o preco de deck_constants.hpp)");
    }

    const CardInstance* found = find_in_active(collection, instance_id);
    if (found == nullptr) {
        // Idempotencia (inv.5): instancia ja fora do ativo (vendida/uploadada/descartada
        // antes, ou nunca existiu) - rejeita, NAO credita.
        return RemovalOutcome{TransactionError::InstanceNotInActive, CardInstance{}, 0};
    }

    // COPIA o card_id antes do callback opaco tier_of (mesma raiz de
    // HandLoadout::validate_candidate): `found` aponta pro buffer de
    // collection.active(); se tier_of mutar o deck e realocar active_, `found->card_id`
    // penduraria (heap-use-after-free). tier_of recebe uma copia estavel.
    const std::string card_id = found->card_id;
    if (const CardTier tier = tier_of(card_id);
        tier == CardTier::Especial || tier == CardTier::Super) {
        // Classe PROTEGIDA (inv.9) - nenhuma via de saida do ativo. Nada mutou ainda.
        return RemovalOutcome{TransactionError::ProtectedTier, CardInstance{}, 0};
    }

    // Tudo validado - agora muta. remove_for_sale() reaplica o mesmo guard
    // internamente (defesa redundante; nunca deveria disparar aqui, ja que acabamos
    // de checar as duas condicoes por leitura publica).
    CardInstance removed = collection.remove_for_sale(instance_id, tier_of);
    credits += price;
    return RemovalOutcome{TransactionError::Ok, removed, price};
}

}  // namespace

SellResult sell(CardCollection& collection, std::int64_t& credits, std::uint64_t instance_id,
                 int price, const CardCollection::TierLookup& tier_of) {
    const RemovalOutcome outcome = remove_and_credit(collection, credits, instance_id, price,
                                                      tier_of, "sell");
    return SellResult{outcome.error, outcome.instance, outcome.credited};
}

UploadResult upload(CardCollection& collection, std::int64_t& credits, std::uint64_t instance_id,
                     int upload_price, const CardCollection::TierLookup& tier_of) {
    const RemovalOutcome outcome = remove_and_credit(collection, credits, instance_id, upload_price,
                                                      tier_of, "upload");
    return UploadResult{outcome.error, outcome.instance, outcome.credited};
}

AcquireResult acquire(CardCollection& collection, std::int64_t& credits, std::string card_id,
                      int price, const CardCollection::TierLookup& tier_of,
                      combat::IRandomSource& rng) {
    if (price < 0) {
        throw std::invalid_argument(
            "deck_transactions::acquire: price nao pode ser negativo (invariante de "
            "programacao - 0 e o valor valido pra loot garantido/achado, nao negativo)");
    }

    if (credits < price) {
        return AcquireResult{TransactionError::InsufficientCredits, CardInstance{}, 0};
    }
    if (collection.active_is_full()) {
        return AcquireResult{TransactionError::ActiveCapacityFull, CardInstance{}, 0};
    }

    // Origem fisica (CARDS-HW-3A): todo canal desta funcao e aquisicao LEGITIMA -
    // origin default (CardOrigin::OriginalRom).
    CardPhysicalState physical;

    // Contaminacao na aquisicao (CARDS-HW-3B, cartas-spec-logica.md secao 5.1): SO
    // DEPOIS dos guards de saldo/capacidade acima (nenhum draw de RNG numa transacao
    // que ja seria rejeitada) - card_id ainda intocado (nao movido ainda) pro
    // tier_of() ler. mimics_special SEMPRE false (canal clone-falso ainda nao existe).
    //
    // tier_of e OPACO (std::function do chamador) - se ele reentrar e mutar esta MESMA
    // collection (gemeo do CRAFT-TOCTOU-REENTRANTE achado em craft() abaixo, mesmo
    // padrao "checa capacidade -> chama callback opaco -> muta"), a capacidade checada
    // acima pode ter ficado stale por quando chegarmos no add_to_active real logo
    // abaixo. O try/catch que envolve aquela chamada e a UNICA defesa necessaria aqui
    // (nao ha um "consumer" intermediario como em craft() pra reverificar antes do
    // roll de RNG).
    const cards::CardTier catalog_tier = tier_of(card_id);
    const ContaminationRollOutcome contamination =
        roll_contamination_on_acquisition(physical, catalog_tier, /*mimics_special=*/false, rng);

    // Tudo validado + a instancia ja nasce com o estado fisico final - muta o
    // container PRIMEIRO (unica mutacao que pode, em teoria, lancar via o guard
    // interno de capacidade de add_to_active); so credita/debita depois que ela teve
    // sucesso, pra nao deixar a wallet debitada sem a carta correspondente em nenhum
    // cenario defensivo.
    //
    // Defesa em profundidade (TOCTOU via tier_of reentrante, achado gemeo do
    // CRAFT-TOCTOU-REENTRANTE): mesmo com o guard de capacidade acima, tier_of pode
    // ter mutado a collection entre aquele guard e este ponto. add_to_active() levanta
    // std::logic_error se a capacidade estourou nesse meio-tempo - essa e uma condicao
    // de FLUXO ESPERADO (capacidade cheia), NAO uma excecao de programacao, entao o
    // header promete que ela nunca escapa pro chamador. Traduzimos pro mesmo
    // TransactionError::ActiveCapacityFull ja usado pelo guard preventivo (mesmo
    // significado: "o ativo nao tinha espaco").
    CardInstance instance;
    try {
        instance = collection.add_to_active(std::move(card_id), std::nullopt, physical);
    } catch (const std::logic_error&) {
        return AcquireResult{TransactionError::ActiveCapacityFull, CardInstance{}, 0};
    }
    credits -= price;
    return AcquireResult{TransactionError::Ok, instance, price, contamination};
}

CraftResult craft(CardCollection& collection, std::string result_card_id,
                   const MaterialConsumer& consumer, const CardCollection::TierLookup& tier_of,
                   combat::IRandomSource& rng) {
    // Capacidade checada ANTES de invocar o consumer (garantia forte): se nao cabe,
    // NUNCA tenta consumir material a toa, nem rola RNG.
    if (collection.active_is_full()) {
        return CraftResult{TransactionError::ActiveCapacityFull, CardInstance{}};
    }

    if (!consumer()) {
        return CraftResult{TransactionError::MaterialsUnavailable, CardInstance{}};
    }

    // Guard TOCTOU - 1a camada (CRAFT-TOCTOU-REENTRANTE, achado de auditoria QA
    // adversarial): consumer() e um std::function OPACO do chamador - se ele capturou
    // e mutou esta MESMA collection (ex.: um caller adversarial/errado que reentra),
    // a capacidade checada antes de invoca-lo pode ter ficado stale. Recheca AQUI,
    // ANTES de rolar RNG (contaminacao) - fecha a janela mais larga sem gastar draw
    // de RNG numa transacao que sera rejeitada de qualquer forma (mesma garantia de
    // determinismo dos demais guards desta funcao, CARDS-HW-3B).
    if (collection.active_is_full()) {
        return CraftResult{TransactionError::ActiveCapacityFull, CardInstance{}};
    }

    // Origem fisica (CARDS-HW-3A, cartas-hardware-pirataria-energia.md secao 2/3):
    // toda carta craftada via F3-Alpha e GRAVADA numa EPROM de bancada (o "terminal
    // de bancada" do doc-fonte) - nunca sai como ROM de fabrica.
    CardPhysicalState physical;
    physical.origin = CardOrigin::HomebrewEprom;

    // Contaminacao na aquisicao (CARDS-HW-3B, secao 5.1): SO depois do material
    // confirmado consumido - result_card_id ainda intocado (nao movido ainda) pro
    // tier_of() ler. mimics_special SEMPRE false. F3-Alpha nunca crafta Especial/
    // Super por design (inv.9), mas o guard defensivo se aplica igual.
    //
    // tier_of TAMBEM e opaco - a mesma janela reabre entre este ponto e o
    // add_to_active real logo abaixo (a 2a camada, try/catch, fecha essa).
    const cards::CardTier catalog_tier = tier_of(result_card_id);
    const ContaminationRollOutcome contamination =
        roll_contamination_on_acquisition(physical, catalog_tier, /*mimics_special=*/false, rng);

    // Guard TOCTOU - 2a camada (defesa em profundidade): mesmo com o recheck acima,
    // tier_of() (chamado logo depois) e igualmente opaco e pode ter mutado a
    // collection nessa janela remanescente. add_to_active() levantaria
    // std::logic_error (capacidade cheia) - fluxo ESPERADO, nunca uma excecao de
    // programacao - e o header promete que ela nunca escapa pro chamador. Traduz pro
    // mesmo TransactionError::ActiveCapacityFull da 1a camada (mesmo significado: "o
    // ativo nao tinha espaco"); aqui o RNG ja foi consumido (efeito colateral aceito
    // e documentado deste ramo raro de reentrancia via tier_of, distinto da 1a
    // camada que e coberta com precisao de zero-draw por CountingRandom).
    CardInstance instance;
    try {
        instance = collection.add_to_active(std::move(result_card_id), std::nullopt, physical);
    } catch (const std::logic_error&) {
        return CraftResult{TransactionError::ActiveCapacityFull, CardInstance{}};
    }
    return CraftResult{TransactionError::Ok, instance, contamination};
}

}  // namespace gus::domain::deck
