// gus/domain/deck/card_collection.hpp
//
// CardCollection: agregado UNICO dono do deck ATIVO e do deck MORTO de um personagem
// (DECK-1, fundacao do sistema deck/mao). POCO puro, ZERO SDL/glintfx.
//
// Os dois vetores (ativo/morto) sao membros PRIVADOS; a API publica e o UNICO caminho
// de mutacao e garante, por construcao, o invariante-mestre do spec (secao 7):
//
//   inv.1 - carta = instancia unica com ID, vive em EXATAMENTE UM container (ativo XOR
//           morto). instance_id e deterministico (contador sequencial interno, comeca
//           em 1, NUNCA reusa) - sem RNG, sem Date/timestamp.
//   inv.3 - deck morto e INERTE: nao conta na capacidade/contagem do deck ativo, nao e
//           jogavel, nao serve de "slot extra".
//   inv.4 - one-way por AUSENCIA de API: existe ativo->morto (discard_to_dead); NAO
//           existe (e nao deve existir) morto->ativo.
//   inv.9 - classe PROTEGIDA: cartas CardTier::Especial/Super NUNCA saem do deck ativo
//           (nem descarte nem venda) - sao unicas, so-narrativa, sem craft/drop/2a
//           chance; perde-las seria perda permanente de conteudo unico.
//
// Este arquivo cobre SO a mecanica de container do MVP (secao 9): adicionar/consultar/
// descartar/retirar-pra-venda. NAO implementa mao/loadout (DECK-2), preco/wallet/venda-
// de-fato (DECK-3), nem o FIFO do deck morto (onda 2, kDeadDeckLimit e so declarada).
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 4/6.2/7/9;
//            gus/domain/combat/combat_enums.hpp (CardTier);
//            gus/domain/combat/placeholder_cards.hpp / master_cards.hpp (catalogo -
//            fonte do TierLookup passado pelo chamador; este agregado NAO conhece o
//            registry, so recebe o resultado via callback).

#ifndef GUS_DOMAIN_DECK_CARD_COLLECTION_HPP
#define GUS_DOMAIN_DECK_CARD_COLLECTION_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/deck/deck_constants.hpp"
#include "gus/domain/deck/deck_records.hpp"

namespace gus::domain::deck {

class CardCollection {
public:
    // Callback fornecido pelo chamador pra resolver o tier de um card_id contra o
    // registry de catalogo (PlaceholderCards::all()/MasterCards). O agregado NAO
    // conhece o registry - so consulta o guard de tier (inv.9) por cima deste lookup.
    using TierLookup = std::function<gus::domain::combat::CardTier(const std::string&)>;

    // active_capacity: capacidade do deck ATIVO no patamar atual (kDeckCapacityTier1/2/3,
    // deck_constants.hpp). next_instance_id: valor inicial do contador sequencial
    // (default 1, spec secao 7 inv.1); um valor > 1 permite retomar a sequencia apos
    // restaurar de um save ja serializado (uso futuro, fora do MVP).
    explicit CardCollection(int active_capacity = kDeckCapacityTier1,
                             std::uint64_t next_instance_id = 1);

    // Adiciona uma NOVA instancia de card_id ao deck ATIVO.
    //
    // Sem instance_id_override (caso normal - aquisicao via loja/loot/craft/achado):
    // gera o proximo id do contador interno (sequencial, nunca reusa).
    //
    // Com instance_id_override (uso futuro - restaurar instancia ja existente de um
    // save serializado): o guard XOR (inv.1) RECUSA (std::invalid_argument) se aquele
    // instance_id JA esta presente no ativo OU no morto - nunca duplica, nunca cria em
    // 2 containers. O contador interno avanca pra nao colidir com o id restaurado.
    //
    // Fail-fast (std::logic_error) se o deck ativo ja esta na capacidade maxima.
    CardInstance add_to_active(std::string card_id,
                                std::optional<std::uint64_t> instance_id_override = std::nullopt);

    // Move a instancia do deck ATIVO pro deck MORTO (unica saida "descarte" - inv.4:
    // NAO existe o caminho inverso). Guard de tier (inv.9): RECUSA
    // CardTier::Especial/Super com std::invalid_argument, SEM mutar nenhum container
    // (checagem acontece antes de qualquer erase/push_back).
    //
    // Fail-fast (std::invalid_argument) se instance_id nao esta no deck ativo.
    void discard_to_dead(std::uint64_t instance_id, const TierLookup& tier_of);

    // Retira a instancia do deck ATIVO pra uma transacao de VENDA (DECK-3 monta
    // credito/wallet por cima; este metodo so cuida do container). Diferente de
    // discard_to_dead: a instancia removida NAO vai pro deck morto - o chamador que
    // decide o destino (tipicamente "some" numa venda). MESMO guard de tier (inv.9):
    // Especial/Super tambem RECUSAM sair por venda (nenhuma via de saida pra elas).
    //
    // Fail-fast (std::invalid_argument) se instance_id nao esta no deck ativo.
    [[nodiscard]] CardInstance remove_for_sale(std::uint64_t instance_id,
                                               const TierLookup& tier_of);

    // Views read-only (copia por referencia const; chamador nao muta por aqui).
    [[nodiscard]] const std::vector<CardInstance>& active() const noexcept;
    [[nodiscard]] const std::vector<CardInstance>& dead() const noexcept;

    // Contagem/capacidade do deck ATIVO. O deck morto NUNCA entra nestas contas
    // (inv.3 - inerte, nao ocupa slot do ativo).
    [[nodiscard]] std::size_t active_count() const noexcept;
    [[nodiscard]] int active_capacity() const noexcept;
    [[nodiscard]] bool active_is_full() const noexcept;

    // Proximo instance_id que seria gerado (uso futuro: persistir no save pra manter a
    // sequencia entre sessoes, DECK-2+).
    [[nodiscard]] std::uint64_t next_instance_id() const noexcept;

private:
    [[nodiscard]] bool present_in_active(std::uint64_t instance_id) const noexcept;
    [[nodiscard]] bool present_in_dead(std::uint64_t instance_id) const noexcept;
    // Lanca std::invalid_argument se card_id resolver pra CardTier::Especial/Super
    // (inv.9). Chamado ANTES de qualquer mutacao de container (guard atomico).
    void guard_protected_tier(const std::string& card_id, const TierLookup& tier_of) const;

    std::vector<CardInstance> active_;
    std::vector<CardInstance> dead_;
    int active_capacity_;
    std::uint64_t next_instance_id_;
};

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_CARD_COLLECTION_HPP
