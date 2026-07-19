// gus/domain/cards/master_cards.hpp
//
// Catalogo data-driven das cartas ESPECIAIS dos 20 mestres do Codex SUPORTADAS pelo
// executor techMagic (ADR-016) hoje. POCO puro, ZERO Qt (invariante de domain/,
// engine-design.md secao 2). Item TECHMAGIC-EXECUTOR (MVP steps 4-5 + manifesto item 5).
//
// ATOM-2 (decomposicao atomica ao nivel de modulo): movido de
// gus/domain/combat/master_cards.hpp pra gus/domain/cards/ (LAR do catalogo de
// cartas). Namespace gus::domain::cards::MasterCards (era gus::domain::combat::
// MasterCards). Sem mudanca de comportamento.
//
// ESCOPO: as 19 cartas cujos efeitos escolhidos (docs/design/roster-analogos/
// _EFEITOS-ESCOLHIDOS.md) cabem nos EffectKind ja implementados por
// gus/domain/combat/techmagic.cpp (ApplyStatus, Leech, Reflect, HypotenuseCombo,
// CloneAlly, RepeatLastAction, ChainDamage, DelayAction, DamageQuantize, RevealIntent,
// DiversityBonus, ApEfficiency, TokenRefund) OU sao posse-only (fora-de-combate/passiva-
// flag, sem programa ainda). von Neumann (Fork) e Giordano Bruno (Echo-Self) usam
// EffectKind::CloneAlly (CARD-ENGINE-MANIFESTO item 8, ultimo step do manifesto) - status-
// based (StatusId::Eco), NAO entidade nova na fila.
//
// Mana das ativas/hibridas (Volta, Newton, Mandelbrot, Faraday) e PROVISORIA (6, marcada
// //PLAYTEST no .cpp) - balanceamento real e trabalho futuro. As 3 fora-de-combate
// (Euler/Turing/Menger) e as passivas Godel/Ada/Planck tem mana 0 (nunca jogadas via
// UseCard: possessao ou equipadas/sempre-ativas). Faraday (ADR-016 Balde B) e Hibrida:
// ganhou uma face de combate castavel (mana kActiveManaCost) alem da face fora-de-combate
// (anti-PEM, posse-only, ainda feat futura). Planck (Quantum-Lock, manifesto item 5) NAO
// executa via techMagic::execute (marcador no-op) - pluga direto no resolvedor/preview, ver
// combat_state_machine.cpp::quantize_spec_of.
//
// ZERO logica de gameplay NOVA aqui: so dados (Card + EffectSpec), mesmo padrao de
// gus/domain/cards/placeholder_cards.hpp. NAO mexe no resolvedor (resolve_use_card),
// NAO cria EffectKind novo, NAO cria handler novo.
//
// Cross-ref: gus/domain/cards/placeholder_cards.hpp (padrao do registry); techmagic.hpp
//            (EffectKind/TriggerHook suportados); docs/design/roster-analogos/
//            _EFEITOS-ESCOLHIDOS.md (efeitos + nomes + frases pedagogicas dos 20
//            mestres); ADR-016.

#ifndef GUS_DOMAIN_CARDS_MASTER_CARDS_HPP
#define GUS_DOMAIN_CARDS_MASTER_CARDS_HPP

#include <string>
#include <unordered_map>

#include "gus/domain/cards/card_records.hpp"

namespace gus::domain::cards::MasterCards {

// Monta e devolve o registry id->Card das cartas ESPECIAIS suportadas. Por VALOR (nao
// cacheado num static const&, ao contrario de PlaceholderCards::all()) - o chamador decide
// se funde este registry com PlaceholderCards::all() num unico card_registry da
// CombatStateMachine. Fail-fast (std::logic_error) se algum id duplicar internamente
// (mesmo padrao emplace do placeholder).
// Ids atuais: volta, newton, pythagoras, mandelbrot, ada, godel, faraday, euler, turing,
// menger, tesla, einstein, planck, dee, maxwell, hayek, mises, vonneumann, bruno.
[[nodiscard]] std::unordered_map<std::string, Card> build_registry();

}  // namespace gus::domain::cards::MasterCards

#endif  // GUS_DOMAIN_CARDS_MASTER_CARDS_HPP
