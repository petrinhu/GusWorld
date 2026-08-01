// SPDX-License-Identifier: Apache-2.0
// gus/domain/combat/urandom_algorithm.hpp
//
// Motor PURO da carta `urandom` (a carta-caos do Gus; CARDS-HW-2 fatia B, ideia + numero do
// backfire 1/3 exato do Gus Dragon, playtester): as tabelas de peso (original x pirata) +
// as 2 funcoes deterministicas (weighted_pick_urandom_faixa/classify_urandom_faixa) que a
// FSM usa pra sortear a faixa e classificar a coleçao do caster. POCO puro, ZERO Qt
// (invariante de domain/, engine-design.md secao 2). Header-only pras constantes;
// declaracoes de funcao aqui, implementacao em urandom_algorithm.cpp (nao inline: o loop de
// weighted_pick e o if-chain de classify sao logica de verdade, nao dado puro - mesmo
// racional de combat_state_machine.cpp preferir .cpp a header pra funcoes com corpo).
//
// TRIGGER (GENERALIZADO em 2026-08-01, decisao do lider, ADR-019 addendum "lei do atomo"):
// ate 2026-08-01, urandom nao tinha EffectKind proprio no executor techMagic - o gatilho do
// branch dedicado de resolve_use_card era um `if (card.id == kUrandomCardId)`, isto e, um
// desvio por ID LITERAL de UMA carta so (decisao de arquitetura da fatia original, reportada
// ao lider na epoca, ver historico do ADR-019). A auditoria da lei do atomo (2026-08-01)
// achou exatamente esse `if` como a violacao real: "cada carta e um atomo... nenhuma tem
// caminho especial no motor". O lider decidiu GENERALIZAR (nao manter como excecao): o
// gatilho agora e `EffectKind::RandomRedirect` (card_enums.hpp) declarado em `card.effects`
// (urandom = [OnCast -> RandomRedirect] no catalogo, master_cards.cpp) - o MESMO vocabulario
// compartilhado que toda outra especial usa pra dizer "o que eu faco", checado por
// combat_state_machine.cpp::card_declares_effect. Uma 2a carta caotica futura que precisar
// do mesmo redirecionamento entra no branch de graca, sem novo `if`.
//
// O CORPO do branch continua IDENTICO (byte-a-byte) ao de antes da generalizacao: o
// redirecionamento AINDA precisa da COLEÇAO INTEIRA do caster (TechMagicContext ganhou 2
// campos aditivos pra isso - collection_snapshot/card_registry, techmagic.hpp - mas
// resolve_urandom continua sendo METODO PRIVADO da CombatStateMachine, acessando
// card_registry_/collection_snapshot_/rng_/queue_/log_ DIRETO como membro, nao via ctx;
// nenhum handler consome os 2 campos novos ainda) e precisa re-executar o resolvedor de
// OUTRA carta (record-base OU techMagic::execute conforme o tier sorteado) - por isso a
// ORQUESTRACAO continua vivendo em CombatStateMachine::resolve_urandom/
// resolve_redirected_card_effect (combat_state_machine.cpp), so o GATILHO que a alcanca
// mudou de id pra EffectKind. `kUrandomCardId` (a constante abaixo) SOBREVIVE, mas so como
// FILTRO ANTI-RECURSAO dentro do pool de candidatos do redirecionamento (exclui a propria
// urandom de se sortear a si mesma) - NAO E MAIS o gatilho de resolucao. So as 2 funcoes
// PURAS abaixo (sorteio de faixa + classificacao) moram aqui, fora da FSM, pra serem
// testaveis em isolamento (distribuicao estatistica sob N milhares de trials sem precisar
// montar um combate inteiro por trial).
//
// Cross-ref: gus/domain/combat/card_collection_snapshot.hpp (CardCollectionEntry, o outro
//            snapshot desta fatia); gus/domain/combat/combat_state_machine.cpp
//            (resolve_urandom/resolve_redirected_card_effect); docs/design/mecanicas/
//            cartas-spec-logica.md secao 7; docs/design/mecanicas/cartas-numeros-proposta.md
//            secao 4.

#ifndef GUS_DOMAIN_COMBAT_URANDOM_ALGORITHM_HPP
#define GUS_DOMAIN_COMBAT_URANDOM_ALGORITHM_HPP

#include <cstddef>
#include <optional>

#include "gus/domain/cards/card_enums.hpp"
#include "gus/domain/cards/card_records.hpp"
#include "gus/domain/combat/random_source.hpp"

namespace gus::domain::combat {

// Id canonico da carta urandom no card_registry (master_cards.hpp). Consultado pelo branch
// dedicado de CombatStateMachine::resolve_use_card.
inline constexpr const char* kUrandomCardId = "urandom";

// Faixa sorteada pelo 1o draw de RNG (cartas-spec-logica.md secao 7.1). Backfire SO existe
// na tabela PIRATA (cartas-numeros-proposta.md secao 4) - a tabela ORIGINAL nunca a produz.
enum class UrandomFaixa {
    Fraco = 0,
    Medio = 1,
    Forte = 2,
    Jackpot = 3,
    Backfire = 4,
};

struct UrandomWeightEntry {
    UrandomFaixa faixa;
    int weight;
};

// Original (premio da RunaDex, ponderaçao generosa - cartas-numeros-proposta.md secao 4):
// fraco 21/34/21/8 -> 25,0%/40,5%/25,0%/9,5%. NUNCA Backfire (fora da tabela).
inline constexpr UrandomWeightEntry kUrandomOriginalWeights[] = {
    {UrandomFaixa::Fraco, 21},
    {UrandomFaixa::Medio, 34},
    {UrandomFaixa::Forte, 21},
    {UrandomFaixa::Jackpot, 8},
};
inline constexpr std::size_t kUrandomOriginalWeightsCount = 4;

// Pirata (mercado negro, REPONDERADA pro backfire = 1/3 exato - decisao do Gus Dragon,
// cartas-numeros-proposta.md secao 4): fraco 7/medio 2/forte 1/jackpot 0 (NUNCA puxa uma
// especial de verdade)/backfire 5, total 15 -> 46,7%/13,3%/6,7%/0%/33,3% (5/15 exato).
inline constexpr UrandomWeightEntry kUrandomPirataWeights[] = {
    {UrandomFaixa::Fraco, 7},
    {UrandomFaixa::Medio, 2},
    {UrandomFaixa::Forte, 1},
    {UrandomFaixa::Jackpot, 0},
    {UrandomFaixa::Backfire, 5},
};
inline constexpr std::size_t kUrandomPirataWeightsCount = 5;

// Classificacao fraco/medio/forte por ManaCost da carta COMUM candidata (secao 7.1: "fraco
// -> ManaCost 1 (Jab-tier) ... forte -> ManaCost 3 (Assinatura)"). Jackpot classifica por
// CardTier (qualquer tier != Comum), fora desta tabela - ver classify_urandom_faixa abaixo.
inline constexpr int kUrandomFaixaFracoManaCost = 1;
inline constexpr int kUrandomFaixaMedioManaCost = 2;
inline constexpr int kUrandomFaixaForteManaCost = 3;

// Pool do backfire pirata (AMB-08, proposta aprovada): reusa status negativos LEVES ja
// catalogados, sorteado uniformemente (2o draw, SO no ramo Backfire) - nunca inventa efeito
// novo. Stun = perde a proxima acao (1 turno); Poison = DoT leve e curto. Aplicado SEMPRE no
// proprio caster (backfire fixo, nao randomizado - cartas-numeros-proposta.md secao 4).
inline constexpr cards::StatusId kUrandomBackfirePool[] = {
    cards::StatusId::Stun,
    cards::StatusId::Poison,
};
inline constexpr std::size_t kUrandomBackfirePoolCount = 2;

inline constexpr int kUrandomBackfireStunDuration = 1;      //PLAYTEST AMB-08 ("Stun 1 turno").
inline constexpr int kUrandomBackfirePoisonMagnitude = 3;   //PLAYTEST AMB-08 (DoT leve).
inline constexpr int kUrandomBackfirePoisonDuration = 2;    //PLAYTEST AMB-08 (curto).

// Sorteia 1 UrandomFaixa por peso relativo (secao 7.1: "faixa = weighted_pick(weights,
// ctx.rng)"). 1 UNICO consumo de rng.next(total_dos_pesos) - determinismo canonico (secao
// 11). `table`/`count` = uma das 2 tabelas acima (NUNCA uma tabela vazia/soma-zero - as
// tabelas canonicas sempre somam >0; fallback defensivo devolve table[0].faixa/
// table[count-1].faixa se isso acontecer mesmo assim, nunca um comportamento indefinido).
[[nodiscard]] UrandomFaixa weighted_pick_urandom_faixa(const UrandomWeightEntry* table,
                                                       std::size_t count,
                                                       IRandomSource& rng);

// Classifica `card` numa faixa fraco/medio/forte (SO Comum, por ManaCost 1/2/3) ou jackpot
// (qualquer tier != Comum - "qualquer das 20 ESPECIAIS ja possuidas pelo jogador", secao
// 7.1). nullopt = nao classificavel (Comum com ManaCost fora de {1,2,3} - nao existe hoje no
// catalogo mas a funcao e defensiva).
[[nodiscard]] std::optional<UrandomFaixa> classify_urandom_faixa(const cards::Card& card);

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_URANDOM_ALGORITHM_HPP
