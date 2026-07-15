// gus/domain/combat/master_cards.hpp
//
// Catalogo data-driven das cartas ESPECIAIS dos 20 mestres do Codex SUPORTADAS pelo
// executor techMagic (ADR-016) hoje. POCO puro, ZERO Qt (invariante de domain/,
// engine-design.md secao 2). Item TECHMAGIC-EXECUTOR (MVP steps 4-5).
//
// ESCOPO: as 12 cartas cujos efeitos escolhidos (docs/design/roster-analogos/
// _EFEITOS-ESCOLHIDOS.md) cabem nos 8 EffectKind ja implementados por
// gus/domain/combat/techmagic.cpp (ApplyStatus, Leech, Reflect, HypotenuseCombo,
// RepeatLastAction, ChainDamage, DelayAction) OU sao posse-only (fora-de-combate/
// passiva-flag, sem programa ainda). von Neumann e Giordano Bruno exigem
// EffectKind::CloneAlly (sem handler - lancaria std::logic_error): os DOIS FICAM DE FORA
// desta leva de proposito; entram quando o executor ganhar o handler correspondente.
//
// Mana das ativas/hibridas (Volta, Newton, Mandelbrot) e PROVISORIA (6, marcada
// //PLAYTEST no .cpp) - balanceamento real e trabalho futuro. As 4 fora-de-combate
// (Faraday/Euler/Turing/Menger) e as passivas Godel/Ada tem mana 0 (nunca jogadas via
// UseCard: possessao ou equipadas/sempre-ativas).
//
// ZERO logica de gameplay NOVA aqui: so dados (Card + EffectSpec), mesmo padrao de
// gus/domain/combat/placeholder_cards.hpp. NAO mexe no resolvedor (resolve_use_card),
// NAO cria EffectKind novo, NAO cria handler novo.
//
// Cross-ref: gus/domain/combat/placeholder_cards.hpp (padrao do registry); techmagic.hpp
//            (EffectKind/TriggerHook suportados); docs/design/roster-analogos/
//            _EFEITOS-ESCOLHIDOS.md (efeitos + nomes + frases pedagogicas dos 20
//            mestres); ADR-016.

#ifndef GUS_DOMAIN_COMBAT_MASTER_CARDS_HPP
#define GUS_DOMAIN_COMBAT_MASTER_CARDS_HPP

#include <string>
#include <unordered_map>

#include "gus/domain/combat/combat_records.hpp"

namespace gus::domain::combat::MasterCards {

// Monta e devolve o registry id->Card das 12 cartas ESPECIAIS suportadas (volta, newton,
// pythagoras, mandelbrot, ada, godel, faraday, euler, turing, menger, tesla, einstein). Por
// VALOR (nao cacheado num static const&, ao contrario de PlaceholderCards::all()) - o
// chamador decide se funde este
// registry com PlaceholderCards::all() num unico card_registry da CombatStateMachine.
// Fail-fast (std::logic_error) se algum id duplicar internamente (mesmo padrao emplace do
// placeholder).
[[nodiscard]] std::unordered_map<std::string, Card> build_registry();

}  // namespace gus::domain::combat::MasterCards

#endif  // GUS_DOMAIN_COMBAT_MASTER_CARDS_HPP
