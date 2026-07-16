// gus/domain/deck/hand_loadout.hpp
//
// HandLoadout: a MAO (loadout de combate) do sistema deck/mao (DECK-2). POCO puro,
// ZERO SDL/glintfx.
//
// A mao NAO e um container de cartas - e uma SELECAO (lista de instance_id) que
// referencia o deck ATIVO (CardCollection::active(), DECK-1). Carregar/tirar da mao
// NUNCA move nem copia a instancia (deck-mao-sistema.md secao 4/7 inv.2): este tipo so
// guarda IDs e valida cada mutacao contra o deck ativo passado pelo chamador - NAO
// possui/guarda uma referencia a CardCollection (cada metodo recebe o deck+tier_of no
// momento da chamada, pra nao acoplar lifetime nem esconder mutacao externa vinda de
// venda/descarte, DECK-1/3).
//
// Regras garantidas (deck-mao-sistema.md secao 3/4/7/8c):
//   - so aceita instance_id presente no deck ATIVO no momento da validacao (rejeita ID
//     de carta morta/vendida/inexistente - secao 7 inv.6, "mao so puxa do ativo").
//   - capacidade de cartas CardTier::Comum <= comum_capacity() (hand_capacity(), teto
//     do stat mental).
//   - slot dedicado a CardTier::Especial/Super: SO quando is_universal_compiler=true
//     (flag "e o Gus", canon IsUniversalCompiler, combat.md secao 6/deck-mao-sistema.md
//     secao 8b) - no maximo kGusSpecialHandSlots (1); qualquer outro personagem fica
//     com ZERO slots de especial. A identidade "e o Gus" e um bool passado pelo
//     chamador - este arquivo nunca compara nome de personagem.
//   - revalidate()/find_orphan_instance_ids(): se uma carta referenciada sair do deck
//     ativo por fora (venda/descarte, DECK-1/3), a mao detecta e remove o ID orfao -
//     nunca fica apontando pra instancia fantasma.
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 3/4/7/8c;
//            gus/domain/deck/card_collection.hpp (o agregado dono do deck ATIVO que a
//            mao referencia, NUNCA muta);
//            gus/domain/deck/deck_constants.hpp (kHandSizeBase/kHandDeltaBento/
//            kHandSizeMentalStatCap/kGusSpecialHandSlots).

#ifndef GUS_DOMAIN_DECK_HAND_LOADOUT_HPP
#define GUS_DOMAIN_DECK_HAND_LOADOUT_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_constants.hpp"

namespace gus::domain::deck {

// Capacidade pura da mao (secao 8c). base_por_personagem JA inclui a identidade do
// personagem (kHandSizeBase, ou kHandSizeBase + kHandDeltaBento pro Bento - quem chama
// soma isso ANTES de invocar; este arquivo nao hardcoda "quem e o Bento").
// hardware_bonus cresce com upgrade de oculos/Tavus-Drive/matriz (pillar "loop
// acoplado ao hardware"); mental_stat_bonus cresce com o stat mental/foco. Teto duro
// em kHandSizeMentalStatCap (~6): sem ele a mao iguala o toolkit inteiro no late-game
// e mata a escolha de loadout (anti-snowball, secao 8c). Piso 0 (defensivo - nunca
// capacidade negativa mesmo com bonus negativo/debuff futuro).
[[nodiscard]] int hand_capacity(int base_por_personagem, int hardware_bonus,
                                 int mental_stat_bonus) noexcept;

class HandLoadout {
public:
    // Mesmo alias do agregado (CardCollection::TierLookup) - fonte unica de
    // assinatura, sem redeclarar o tipo de callback.
    using TierLookup = CardCollection::TierLookup;

    // comum_capacity: teto de cartas CardTier::Comum na mao (ver hand_capacity()
    // acima - o chamador calcula e passa o resultado).
    // is_universal_compiler: flag "e o Gus" (canon IsUniversalCompiler) - so quem tem
    // a flag ganha o slot dedicado (kGusSpecialHandSlots) pra CardTier::Especial/
    // Super; qualquer outro personagem fica com 0 slots de especial.
    //
    // Fail-fast (std::invalid_argument) se comum_capacity for negativo.
    explicit HandLoadout(int comum_capacity, bool is_universal_compiler = false);

    // Substitui a selecao inteira (fluxo BANCADA - secao 5, "monta fora de combate").
    // Atomico: valida a lista completa ANTES de mutar; se QUALQUER regra falhar
    // (duplicata, ID fora do deck ativo, estouro de capacidade comum, estouro/uso
    // indevido do slot especial), a selecao atual permanece intocada
    // (std::invalid_argument, zero efeito colateral).
    void set_selection(std::vector<std::uint64_t> instance_ids, const CardCollection& deck,
                        const TierLookup& tier_of);

    // Adiciona 1 instance_id a selecao atual (fluxo incremental - bancada ou swap de
    // emergencia em combate, secao 5). Mesma validacao atomica de set_selection.
    void add_card(std::uint64_t instance_id, const CardCollection& deck, const TierLookup& tier_of);

    // Remove 1 instance_id da selecao. Remover NUNCA viola capacidade/slot especial
    // (so encolhe a selecao) - nao precisa do deck/tier_of.
    //
    // Fail-fast (std::invalid_argument) se o ID nao esta selecionado (nada a remover).
    void remove_card(std::uint64_t instance_id);

    // Troca old_instance_id por new_instance_id numa unica operacao atomica (swap de
    // emergencia em combate - secao 5, "gastando 1 acao/turno"). Mesma validacao de
    // set_selection sobre o resultado; se falhar, a selecao atual fica intocada.
    //
    // Fail-fast (std::invalid_argument) se old_instance_id nao esta na selecao.
    void swap_card(std::uint64_t old_instance_id, std::uint64_t new_instance_id,
                   const CardCollection& deck, const TierLookup& tier_of);

    // IDs da selecao atual que NAO estao mais em deck.active() (a carta saiu por
    // venda/descarte - DECK-1/3 - por fora desta mao). Read-only, nao muta a selecao.
    [[nodiscard]] std::vector<std::uint64_t> find_orphan_instance_ids(const CardCollection& deck) const;

    // Remove da selecao os IDs orfaos (find_orphan_instance_ids acima) e devolve a
    // lista removida (vazia se nada estava orfao). O chamador decide QUANDO revalidar
    // (tipicamente logo apos uma venda/descarte que possa ter afetado a mao) - este
    // tipo nao observa o deck sozinho (sem referencia guardada, sem callback global).
    std::vector<std::uint64_t> revalidate(const CardCollection& deck);

    [[nodiscard]] const std::vector<std::uint64_t>& selection() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] int comum_capacity() const noexcept;
    [[nodiscard]] bool is_universal_compiler() const noexcept;

private:
    // Valida a lista candidata inteira (duplicata, presenca no deck ativo, teto de
    // Comum, teto/guard do slot especial) - lanca std::invalid_argument na 1a
    // violacao, SEM mutar selection_. Todo metodo mutante (set_selection/add_card/
    // swap_card) so monta o candidato e chama isto - fonte unica da regra.
    void validate_candidate(const std::vector<std::uint64_t>& candidate, const CardCollection& deck,
                             const TierLookup& tier_of) const;

    int comum_capacity_;
    bool is_universal_compiler_;
    std::vector<std::uint64_t> selection_;
};

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_HAND_LOADOUT_HPP
