// gus/domain/deck/deck_transactions.hpp
//
// DECK-3: as transacoes atomicas do sistema de deck/mao (sell/upload/acquire/craft).
// POCO puro, ZERO SDL/glintfx. Funcoes livres (nao um agregado com estado proprio) que
// mutam DUAS coisas juntas de forma atomica - o container (CardCollection, DECK-1) e a
// wallet (int64_t credits, referencia do chamador) - com garantia FORTE:
//
//   valida TUDO primeiro, so entao muta (container + wallet); falha no meio = ZERO
//   efeito colateral (nenhum dos dois muda).
//
// Nao usa excecao pra fluxo esperado que pode falhar legitimamente (saldo insuficiente,
// deck cheio, id ja fora do ativo, material faltando) - isso e devolvido via
// TransactionError num Result por-operacao (padrao ja usado em save_serializer.hpp
// LoadResult). std::invalid_argument fica reservado pra violacao de invariante de
// PROGRAMACAO (preco negativo passado pelo proprio codigo-caller, nunca input direto de
// jogador).
//
// IDEMPOTENCIA (deck-mao-sistema.md secao 7 inv.5): sell()/upload() sobre um
// instance_id que ja NAO esta no deck ativo (vendido/uploadado/descartado antes)
// devolve TransactionError::InstanceNotInActive e NAO credita - nunca credita 2x pela
// mesma instancia. A checagem e feita por LEITURA publica (CardCollection::active(),
// TierLookup) ANTES de qualquer mutacao - nao depende de capturar excecao de
// CardCollection::remove_for_sale() como controle de fluxo (o guard de tier daquele
// metodo fica como defesa redundante; nunca deveria disparar aqui, ja que a mesma regra
// foi checada antes).
//
// Este arquivo NAO conhece HandLoadout (DECK-2) nem acopla lifetime com ela. Se a
// instancia removida por sell()/upload() estava selecionada na mao, ela vira um ID
// ORFAO (HandLoadout::find_orphan_instance_ids()/revalidate() - card_collection.hpp/
// hand_loadout.hpp) - o CHAMADOR (gameplay_engineer) e quem decide quando revalidar a
// mao apos uma transacao, este dominio so garante o container+wallet.
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 6.1 (aquisicao
// garantida)/6.2 (upload ao commons)/7 (atomicidade+idempotencia)/8c (precos
// //PLAYTEST); gus/domain/deck/card_collection.hpp (remove_for_sale/add_to_active -
// este arquivo NAO reimplementa essas mecanicas, so orquestra); gus/domain/deck/
// deck_constants.hpp (kUploadCreditMin/Max, kShopBuyPriceMin/Max, kNpcSellPriceMin/Max,
// kShopSellPriceMin/Max - o CALLER escolhe o preco dentro da faixa e passa aqui; este
// arquivo nao sorteia/decide preco).

#ifndef GUS_DOMAIN_DECK_DECK_TRANSACTIONS_HPP
#define GUS_DOMAIN_DECK_DECK_TRANSACTIONS_HPP

#include <cstdint>
#include <functional>
#include <string>

#include "gus/domain/deck/card_collection.hpp"
#include "gus/domain/deck/deck_records.hpp"

namespace gus::domain::deck {

// Motivo de falha de uma transacao (fluxo esperado, NAO excecao). Ok = sucesso; os
// demais valores identificam qual validacao rejeitou a operacao ANTES de qualquer
// mutacao (garantia forte - container e wallet ficam intocados).
enum class TransactionError {
    Ok = 0,
    // sell()/upload(): instance_id nao esta (mais) no deck ATIVO - idempotencia (ja
    // vendida/uploadada/descartada antes, ou id nunca existiu) OU id normal de
    // "carta ja saiu por outro caminho". Nunca credita nesse caso.
    InstanceNotInActive,
    // sell()/upload(): a instancia resolve pra CardTier::Especial/Super (classe
    // PROTEGIDA, secao 7 inv.9) - nao ha via de saida do ativo pra elas.
    ProtectedTier,
    // acquire(): credits (saldo do chamador) < price. Nao debita, nao adiciona carta.
    InsufficientCredits,
    // acquire()/craft(): deck ativo ja na capacidade maxima (deck_constants.hpp
    // kDeckCapacityTier1/2/3) - nao debita/nao consome material, nao adiciona carta.
    ActiveCapacityFull,
    // craft(): o MaterialConsumer devolveu false (F3-Alpha nao tinha material
    // suficiente) - nao adiciona carta. O consumer so e chamado DEPOIS de confirmar
    // que o ativo tem espaco (nunca "consome" material pra depois descobrir que nao
    // cabe a carta resultante).
    MaterialsUnavailable,
};

// Resultado de sell(): remove do ativo + credita um preco.
struct SellResult {
    TransactionError error = TransactionError::Ok;
    // Valida SO quando error == Ok: a instancia que saiu do ativo.
    CardInstance instance;
    // Valida SO quando error == Ok: quanto foi creditado (== price passado).
    int credited = 0;

    [[nodiscard]] bool ok() const noexcept { return error == TransactionError::Ok; }
};

// Resultado de upload(): mesma forma de SellResult (upload tambem so tira do ativo e
// credita um preco, secao 6.2/7.4) - tipo DISTINTO (nao alias) so pra o call-site nao
// confundir o resultado de uma venda com o de um upload.
struct UploadResult {
    TransactionError error = TransactionError::Ok;
    CardInstance instance;
    int credited = 0;

    [[nodiscard]] bool ok() const noexcept { return error == TransactionError::Ok; }
};

// Resultado de acquire(): adiciona ao ativo + debita um preco (0 = loot garantido/achado).
struct AcquireResult {
    TransactionError error = TransactionError::Ok;
    // Valida SO quando error == Ok: a instancia nova adicionada ao ativo.
    CardInstance instance;
    // Valida SO quando error == Ok: quanto foi debitado (== price passado; 0 pra
    // loot garantido/achado visivel).
    int debited = 0;

    [[nodiscard]] bool ok() const noexcept { return error == TransactionError::Ok; }
};

// Resultado de craft(): adiciona a carta resultante SE os materiais foram consumidos.
struct CraftResult {
    TransactionError error = TransactionError::Ok;
    // Valida SO quando error == Ok: a instancia da carta resultante.
    CardInstance instance;

    [[nodiscard]] bool ok() const noexcept { return error == TransactionError::Ok; }
};

// Vende UMA instancia do deck ATIVO por `price` creditos. Atomico: valida presenca no
// ativo + guard de tier (inv.9, herdado de CardCollection::remove_for_sale) ANTES de
// mutar; se qualquer checagem falhar, `collection` e `credits` ficam EXATAMENTE como
// estavam (Result com error != Ok, instance/credited default). Idempotente: 2a chamada
// com o mesmo instance_id (ja removido pela 1a) devolve InstanceNotInActive, nunca
// credita de novo.
//
// Fail-fast (std::invalid_argument) se price < 0 (violacao de invariante de
// PROGRAMACAO - o caller escolhe o preco de deck_constants.hpp/economia, nunca input
// direto de jogador).
[[nodiscard]] SellResult sell(CardCollection& collection, std::int64_t& credits,
                               std::uint64_t instance_id, int price,
                               const CardCollection::TierLookup& tier_of);

// Upload/reciclagem ao repositorio-commons (secao 6.2/7.4): mesma mecanica de sell()
// (sai do ativo, credita um preco), so com o preco de upload (kUploadCreditMin/Max) em
// vez do de venda. MVP (secao 9): sai DIRETO do ativo, sem passar pelo deck morto FIFO
// (onda 2). O dominio NAO faz I/O - devolve o Result com dados suficientes pro
// chamador (gameplay) emitir a mensagem diegetica ("codigo enviado ao
// repositorio-commons, sera reusado..." - secao 6.2); esta funcao nunca imprime/loga.
// Mesma atomicidade + idempotencia de sell() (ver doc acima).
//
// Fail-fast (std::invalid_argument) se upload_price < 0.
[[nodiscard]] UploadResult upload(CardCollection& collection, std::int64_t& credits,
                                   std::uint64_t instance_id, int upload_price,
                                   const CardCollection::TierLookup& tier_of);

// Adquire uma carta nova por `price` creditos - a MESMA primitiva serve loja (preco
// N), loot garantido (preco 0) e achado visivel (preco 0); este dominio nao modela
// estoque de loja/loot table/economia inteira (decisao "e" do spec), so a
// transacao-primitiva de "debita saldo (se houver) + adiciona ao ativo". Atomico:
// valida saldo (credits >= price) E capacidade do ativo (nao estourar) ANTES de mutar
// qualquer um dos dois; se qualquer checagem falhar, nada muda.
//
// Origem fisica (CARDS-HW-3A, cartas-hardware-pirataria-energia.md secao 2/3): todo
// canal desta funcao (loja legitima/loot de repositorio limpo/achado/entrega
// narrativa) e aquisicao LEGITIMA - a instancia nasce com CardPhysicalState{} default
// (CardOrigin::OriginalRom), sem passar initial_physical algum. Ainda NAO existe aqui
// um canal de mercado negro/pirata (CardOrigin::PirateClone) - onda futura
// (mercado-negro/loja, fora do escopo desta fatia) precisara de uma
// transacao-primitiva propria ou de um parametro de origem, a decidir quando aquele
// canal for desenhado (nao inventado aqui).
//
// Fail-fast (std::invalid_argument) se price < 0.
[[nodiscard]] AcquireResult acquire(CardCollection& collection, std::int64_t& credits,
                                     std::string card_id, int price);

// Callback do chamador que tenta consumir os materiais de craft (F3-Alpha) e devolve
// se conseguiu. O inventario de materiais e do GAMEPLAY, nao do domain de deck - este
// arquivo so orquestra a chamada (decisao "e" do spec: dominio nao conhece
// materiais/receitas).
using MaterialConsumer = std::function<bool()>;

// Compila (craft) uma carta nova SE os materiais foram consumidos. Atomico com
// garantia forte: a capacidade do ativo e checada ANTES de invocar `consumer` - se o
// ativo ja esta cheio, `consumer` NUNCA e chamado (nao "gasta" material pra descobrir
// depois que a carta nao cabe). Se `consumer()` devolver false (material insuficiente),
// nada e adicionado ao ativo.
//
// Origem fisica (CARDS-HW-3A, cartas-hardware-pirataria-energia.md secao 2/3): craft
// via F3-Alpha grava a carta numa EPROM de bancada - a instancia resultante SEMPRE
// nasce com CardOrigin::HomebrewEprom (nunca OriginalRom). NAO rola contaminacao
// aqui (fatia futura, "acquire-com-origem" do doc-fonte secao 11) - so fixa a origem
// pra hardware_class_of()/contaminacao futura classificarem certo.
[[nodiscard]] CraftResult craft(CardCollection& collection, std::string result_card_id,
                                 const MaterialConsumer& consumer);

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_DECK_TRANSACTIONS_HPP
