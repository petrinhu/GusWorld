// gus/domain/combat/techmagic.cpp
//
// Implementacao do executor techMagic (ADR-016, MVP steps 2-7). Handlers: ApplyStatus,
// Leech, Reflect (step 2) + HypotenuseCombo (step 3, ledger cross-ator/OnRoundEnd) +
// RepeatLastAction (step 5, Mandelbrot/Fractal-Echo + Ada/Re-Run, decisoes Q1-Q4 do
// lider 2026-07-14) + ChainDamage (step 6, Tesla) + DelayAction (step 7, Einstein/
// Time-Dilate, decisao do lider 2026-07-15 + addendum "modo-aliado assimetrico"
// 2026-07-15: alvo inimigo atrasa pro fim da fila, alvo aliado ADIANTA pro slot logo apos
// o ator atual - espelho beneficio). EffectKind sem handler (CloneAlly) lanca
// std::logic_error - fail-fast, nunca no-op silencioso (ver techmagic.hpp).
//
// Cross-ref: gus/domain/combat/techmagic.hpp; combat_state_machine.cpp (pontos de hook);
//            docs/design/mecanicas/cartas-technomagik.md; ADR-016.

#include "gus/domain/combat/techmagic.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "gus/domain/combat/initiative_queue.hpp"

namespace gus::domain::combat::techMagic {

namespace {

// Empurra uma CombatLogEntry pro log da FSM (secao 16), se `ctx.log` estiver setado.
// actor_id = ctx.caster (dono do programa); `target` opcional vira o TargetId da entrada.
void log_entry(TechMagicContext& ctx, CombatActor* target, int value, std::string message) {
    if (ctx.log == nullptr) return;
    ctx.log->push_back(CombatLogEntry{
        ctx.caster != nullptr ? ctx.caster->id() : std::string{}, CombatActionType::UseCard,
        target != nullptr ? std::optional<std::string>(target->id()) : std::nullopt, value,
        std::move(message)});
}

// ApplyStatus: monta um StatusEffect a partir do EffectSpec + a familia da CARTA (nao do
// caster - mesmo racional de card.status_applied no resolvedor base, secao 9) e aplica.
// Destinatario depende do hook: OnCast -> ctx.counterpart (alvo da carta); Always ->
// ctx.caster (o proprio dono, reforco no TurnStart enquanto a especial estiver equipada).
//
// side_filter (ADR-016 Balde B, Faraday/EM-Shield): quando o EffectSpec declara
// EnemyOnly/AllyOnly e o `recipient` resolvido esta do lado que o filtro EXCLUI, a carta
// DISSIPA - no-op + log, NAO lanca (estado NORMAL, mesmo racional de handle_delay_action).
// Any (default) nunca dissipa aqui.
//
// Portao de imunidade (try_add_status, CombatActor): o resultado real (Applied vs
// BlockedByImmunity) decide a MENSAGEM do log - nunca afirma "aplicado" quando o alvo
// bloqueou via BlindagemEM.
void handle_apply_status(const EffectSpec& spec, const Card& card, TriggerHook hook,
                         TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::ApplyStatus: ctx.caster nao pode ser nulo.");

    CombatActor* recipient = hook == TriggerHook::Always ? ctx.caster : ctx.counterpart;
    if (recipient == nullptr)
        throw std::logic_error(
            "techMagic::ApplyStatus: alvo ausente pro hook (ctx.counterpart nulo).");

    if (spec.side_filter != SideFilter::Any) {
        const bool ally = recipient->is_player_side() == ctx.caster->is_player_side();
        const bool dissipates = (spec.side_filter == SideFilter::AllyOnly && !ally) ||
                                (spec.side_filter == SideFilter::EnemyOnly && ally);
        if (dissipates) {
            log_entry(ctx, recipient, 0,
                     ctx.caster->id() + " tavus-executa " + card.id + " em " +
                         recipient->id() + ": nao adere a " + (ally ? "aliados" : "hostis") +
                         ", a carta se dissipa.");
            return;
        }
    }

    const StatusEffect status{spec.status, spec.magnitude, spec.duration, spec.stack_rule,
                              card.family};
    const StatusApplyResult result = recipient->try_add_status(status);

    if (result == StatusApplyResult::Applied) {
        log_entry(ctx, recipient, spec.magnitude,
                 ctx.caster->id() + " tavus-executa " + card.id + " em " + recipient->id() +
                     ": status aplicado (mag " + std::to_string(spec.magnitude) + ", dur " +
                     std::to_string(spec.duration) + ").");
    } else {
        log_entry(ctx, recipient, 0,
                 ctx.caster->id() + " tavus-executa " + card.id + " em " + recipient->id() +
                     ": bloqueado pela blindagem EM.");
    }
}

// Leech (OnDamageDealt): caster recupera lround(ctx.damage * percent/100) em HP E mana
// (split igual = placeholder de engine; o split real do Volta e o dado da carta, pendente
// VOLTA-LEECH-%). Sem efeito se ctx.damage <= 0 (canal FALHA/imunidade nao dispara).
void handle_leech(const EffectSpec& spec, const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::Leech: ctx.caster nao pode ser nulo.");
    if (ctx.damage <= 0) return;

    const int amount = static_cast<int>(
        std::lround(static_cast<double>(ctx.damage) * static_cast<double>(spec.percent) / 100.0));
    if (amount <= 0) return;

    ctx.caster->heal(amount);          // clamp em max_hp (CombatActor::heal).
    ctx.caster->restore_mana(amount);  // clamp em max_mana (CombatActor::restore_mana).

    log_entry(ctx, ctx.counterpart, amount,
             ctx.caster->id() + " drena " + std::to_string(amount) + " HP/mana via " + card.id +
                 " (Leech).");
}

// Reflect (OnDamageReceived): ctx.counterpart (o atacante original) sofre
// lround(ctx.damage * percent/100) de volta. take_damage PURO - NAO reentra pelo helper
// compartilhado de dano da FSM, entao nunca redispara OnDamageDealt/OnDamageReceived
// (guarda anti-recursao, secao 20).
void handle_reflect(const EffectSpec& spec, const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::Reflect: ctx.caster nao pode ser nulo.");
    if (ctx.counterpart == nullptr)
        throw std::logic_error(
            "techMagic::Reflect: ctx.counterpart (atacante) nao pode ser nulo.");
    if (ctx.damage <= 0) return;

    const int amount = static_cast<int>(
        std::lround(static_cast<double>(ctx.damage) * static_cast<double>(spec.percent) / 100.0));
    if (amount <= 0) return;

    ctx.counterpart->take_damage(amount);

    log_entry(ctx, ctx.counterpart, amount,
             ctx.caster->id() + " reflete " + std::to_string(amount) + " de volta em " +
                 ctx.counterpart->id() + " via " + card.id + " (Reflect).");
}

// HypotenuseCombo (OnRoundEnd, ADR-016 secao 20 item 4 / decisoes Q1-Q4 do lider,
// 2026-07-14): quando >=2 ALIADOS DISTINTOS do dono bateram no MESMO alvo nesta rodada, E
// o proprio dono esta entre eles (Q4), aplica um GOLPE-BONUS ADICIONAL de
// round(sqrt(soma dos quadrados dos danos POR ATACANTE DISTINTO)) por cima dos hits normais
// (Q1: soma, nao substitui). Mesmo aliado que bateu 2x conta como UM componente = soma dos
// danos dele (Q3). Formula estende a N atacantes (Q2). Dedup 1x/alvo/rodada via
// ctx.bonused_targets (>1 dono da passiva no mesmo alvo nao dobra o bonus).
void handle_hypotenuse_combo(const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::HypotenuseCombo: ctx.caster nao pode ser nulo.");
    if (ctx.round_hits == nullptr)
        throw std::logic_error(
            "techMagic::HypotenuseCombo: ctx.round_hits nao pode ser nulo (OnRoundEnd sem "
            "ledger injetado no contexto - bug de call site, nao efeito vazio).");

    // Agrega os hits do ledger por ALVO e, dentro de cada alvo, por ATACANTE DISTINTO do
    // MESMO lado do caster (combo e so entre aliados; hit do lado oposto no mesmo alvo nao
    // combina). Vetores em vez de mapas indexados por ponteiro: a ordem de agregacao segue
    // a ordem de insercao no ledger (deterministica, ver combat_state_machine.cpp) em vez de
    // depender de hash de endereco - mesmo racional de determinismo do RNG injetado
    // (secao 11).
    struct TargetAgg {
        CombatActor* target = nullptr;
        std::vector<std::pair<CombatActor*, int>> by_attacker;
    };
    std::vector<TargetAgg> targets;

    for (const RoundHitEntry& hit : *ctx.round_hits) {
        if (hit.attacker == nullptr || hit.target == nullptr) continue;
        if (hit.attacker->is_player_side() != ctx.caster->is_player_side())
            continue;  // so combina hits do MESMO lado do dono da passiva.

        auto target_it = std::find_if(targets.begin(), targets.end(),
                                      [&](const TargetAgg& t) { return t.target == hit.target; });
        if (target_it == targets.end()) {
            targets.push_back(TargetAgg{hit.target, {}});
            target_it = targets.end() - 1;
        }

        auto attacker_it =
            std::find_if(target_it->by_attacker.begin(), target_it->by_attacker.end(),
                        [&](const auto& p) { return p.first == hit.attacker; });
        if (attacker_it == target_it->by_attacker.end())
            target_it->by_attacker.emplace_back(hit.attacker, hit.damage);
        else
            attacker_it->second += hit.damage;  // Q3: mesmo atacante 2x = soma, 1 componente.
    }

    for (const TargetAgg& agg : targets) {
        if (!agg.target->is_alive()) continue;    // cadaver antes do fecho: sem bonus postumo.
        if (agg.by_attacker.size() < 2) continue;  // Q1/Q2: precisa de >=2 atacantes distintos.

        const bool caster_in_combo =
            std::any_of(agg.by_attacker.begin(), agg.by_attacker.end(),
                       [&](const auto& p) { return p.first == ctx.caster; });
        if (!caster_in_combo) continue;  // Q4: o DONO precisa estar entre os atacantes.

        if (ctx.bonused_targets != nullptr &&
            !ctx.bonused_targets->insert(agg.target).second)
            continue;  // dedup 1x/alvo/rodada (>1 dono da passiva no mesmo alvo).

        double sum_of_squares = 0.0;
        for (const auto& [attacker, dmg] : agg.by_attacker)
            sum_of_squares += static_cast<double>(dmg) * static_cast<double>(dmg);
        const int bonus = static_cast<int>(std::lround(std::sqrt(sum_of_squares)));
        if (bonus <= 0) continue;

        agg.target->take_damage(bonus);  // PURO - anti-recursao (nao redispara hooks de dano).

        log_entry(ctx, agg.target, bonus,
                 ctx.caster->id() + " fecha Hipotenuse combo em " + agg.target->id() +
                     " via " + card.id + ": +" + std::to_string(bonus) + " (combo de " +
                     std::to_string(agg.by_attacker.size()) + " atacantes).");
    }
}

// RepeatLastAction (step 5, Mandelbrot/Fractal-Echo + Ada/Re-Run; decisoes Q1-Q4 do
// lider, 2026-07-14): reaplica o dano>0 de CADA ALVO da ULTIMA ACAO de dano de um ALIADO
// do ctx.caster NESTA RODADA (ctx.last_action), escalado por spec.percent (Q1: eco do
// RESULTADO, sem novo sorteio/critico/mana/status; Q3: Ada usa percent=100 - o freio dela
// e a CHANCE, nao a escala). spec.magnitude e a chance% de disparar o Re-Run: 0 = sempre
// dispara SEM tocar ctx.rng (Mandelbrot, sempre-repete, 0 consumo de RNG por construcao);
// >0 consome ctx.rng->next(100) EXATAMENTE 1 vez (Ada 34%), mas SO quando ha uma acao
// echoable (determinismo "1 consumo por dono so se ha acao repetivel" - sem acao pra
// ecoar, ZERO RNG tambem).
void handle_repeat_last_action(const EffectSpec& spec, const Card& card,
                               TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::RepeatLastAction: ctx.caster nao pode ser nulo.");
    if (ctx.last_action == nullptr)
        throw std::logic_error(
            "techMagic::RepeatLastAction: ctx.last_action nao pode ser nulo (bug de call "
            "site - a FSM sempre injeta &last_action_, nao efeito vazio).");

    const LastActionRecord& last = *ctx.last_action;
    // Q4: so ecoa acao de um ALIADO (mesmo lado do dono); registro vazio (nada aconteceu
    // ainda nesta rodada) ou de lado oposto => nada a ecoar. Estado NORMAL, nao erro.
    const bool echoable = last.actor != nullptr && !last.hits.empty() &&
                          last.actor->is_player_side() == ctx.caster->is_player_side();
    if (!echoable) {
        log_entry(ctx, nullptr, 0,
                 ctx.caster->id() + " tavus-executa " + card.id +
                     ": nada a ecoar nesta rodada.");
        return;
    }

    if (spec.magnitude > 0) {
        if (ctx.rng == nullptr)
            throw std::logic_error(
                "techMagic::RepeatLastAction: ctx.rng nao pode ser nulo com chance "
                "(magnitude > 0) declarada na carta '" +
                card.id + "'.");
        const int roll = ctx.rng->next(100);
        if (roll >= spec.magnitude) {
            log_entry(ctx, nullptr, 0,
                     ctx.caster->id() + " tavus-executa " + card.id +
                         ": chance do Re-Run falhou (" + std::to_string(roll) + " >= " +
                         std::to_string(spec.magnitude) + ").");
            return;
        }
    }

    for (const auto& [target, dmg] : last.hits) {
        if (target == nullptr || !target->is_alive()) continue;
        const int echo = static_cast<int>(
            std::lround(static_cast<double>(dmg) * static_cast<double>(spec.percent) / 100.0));
        if (echo <= 0) continue;

        target->take_damage(echo);  // PURO - anti-recursao (nao redispara hooks/ledger).

        log_entry(ctx, target, echo,
                 ctx.caster->id() + " tavus-executa " + card.id + ": a acao de " +
                     last.actor->id() + " ecoou (" + std::to_string(spec.percent) +
                     "%) em " + target->id() + " por " + std::to_string(echo) + ".");
    }
}

// ChainDamage (OnCast, Tesla; ADR-016 step 6): apos o dano-base atingir o alvo primario
// (ctx.counterpart, ja resolvido no loop base), a descarga SALTA pros proximos inimigos VIVOS
// na ordem da fila (ctx.combatants), do lado OPOSTO ao caster, EXCLUINDO o primario. Coleta
// ate spec.magnitude alvos-salto; para quando faltam alvos. Cada salto retem spec.percent% do
// dano-base de forma MULTIPLICATIVA: salto k (1-indexado) = lround(ctx.damage * (percent/100)^k).
// Para tambem quando o salto arredonda pra <=0 (os proximos so seriam menores). ctx.damage e o
// dano REALMENTE causado ao primario (pos-fraqueza/crit/variancia); primario imune (0) => no-op.
// take_damage PURO - anti-recursao (nao redispara OnDamageDealt/OnDamageReceived, nao entra no
// ledger). 0 consumo de RNG (determinismo): nao toca ctx.rng.
void handle_chain_damage(const EffectSpec& spec, const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::ChainDamage: ctx.caster nao pode ser nulo.");
    if (ctx.combatants == nullptr)
        throw std::logic_error(
            "techMagic::ChainDamage: ctx.combatants nao pode ser nulo (OnCast sem roster "
            "injetado no contexto - bug de call site, nao efeito vazio).");
    if (ctx.counterpart == nullptr)
        throw std::logic_error(
            "techMagic::ChainDamage: ctx.counterpart (alvo primario) nao pode ser nulo "
            "(OnCast sempre tem alvo).");

    // Primario imune / dano-base 0: a cadeia nao tem de onde escalar. No-op + log.
    if (ctx.damage <= 0) {
        log_entry(ctx, ctx.counterpart, 0,
                 ctx.caster->id() + " tavus-executa " + card.id +
                     ": alvo primario imune, cadeia nao propaga.");
        return;
    }

    const double factor = static_cast<double>(spec.percent) / 100.0;
    const std::size_t max_jumps = static_cast<std::size_t>(std::max(0, spec.magnitude));

    // Alvos-salto: proximos inimigos VIVOS do lado oposto, na ORDEM da fila, excluindo o
    // primario (que ja levou o dano-base). Ate max_jumps; se houver menos, salta em menos.
    std::vector<CombatActor*> jump_targets;
    for (CombatActor* a : *ctx.combatants) {
        if (jump_targets.size() >= max_jumps) break;
        if (a == nullptr || !a->is_alive()) continue;
        if (a->is_player_side() == ctx.caster->is_player_side()) continue;
        if (a == ctx.counterpart) continue;
        jump_targets.push_back(a);
    }

    int k = 1;
    for (CombatActor* target : jump_targets) {
        const int jump = static_cast<int>(
            std::lround(static_cast<double>(ctx.damage) * std::pow(factor, k)));
        if (jump <= 0) break;  // arredondou pra 0: os saltos seguintes so seriam menores.

        target->take_damage(jump);  // PURO - anti-recursao (nao redispara hooks/ledger).

        log_entry(ctx, target, jump,
                 ctx.caster->id() + " tavus-executa " + card.id + ": cadeia salta em " +
                     target->id() + " por " + std::to_string(jump) + " (salto " +
                     std::to_string(k) + ").");
        ++k;
    }
}

// DelayAction (OnCast, Einstein/Time-Dilate; ADR-016 step 7 + addendum "modo-aliado
// assimetrico", decisao do lider 2026-07-15): quando o alvo (ctx.counterpart) e do lado
// OPOSTO ao caster (inimigo), empurra a acao dele pro FIM da fila da rodada corrente
// (spec.magnitude == 0) ou N posicoes fixas atrasando (spec.magnitude > 0), via
// InitiativeQueue::reorder_pending (mesma primitiva do Gambito-Reordenar, migrada de
// reorder_actor em COMBATE-FILA-CURSOR-FIX 2026-07-15 - clamp em [cursor()+1, count()-1]
// pela PROPRIA primitiva, nunca cruza o cursor). Quando o alvo e do MESMO lado do caster
// (aliado), o efeito e o ESPELHO BENEFICO: o aliado avanca pra agir MAIS CEDO, indo pro
// PRIMEIRO SLOT AINDA-NAO-AGIDO logo apos o ator atual (spec.magnitude == 0, "ao extremo")
// ou N posicoes fixas adiantando (spec.magnitude > 0) - "dilata o tempo A FAVOR do
// aliado". Estados NORMAIS que dissipam a carta em no-op + log (NAO lancam), IDENTICOS nos
// dois lados: alvo morto ou fora da fila; alvo e o current() (em acao agora, nao ha "acao
// futura" a reordenar); indice do alvo em ctx.queue->order() < ctx.queue->cursor() (ja
// agiu nesta rodada - a dilatacao NAO banca pra proxima rodada, nem atrasando nem
// adiantando). Dano zero, ZERO consumo de RNG, NAO toca take_damage/round_hits/
// last_action (sem eco em RepeatLastAction). O empurrao/avanco e uma reordenacao
// PERSISTENTE ate a proxima recomputacao natural por SPD
// (InitiativeQueue::recompute_by_speed) - a mesma regra do Gambito.
//
// INVARIANTE ANTI TURNO-DUPLO no lado aliado: o alvo NUNCA pode ser reordenado pra um
// indice <= cursor() - isso desincronizaria current() do ator cuja resolucao esta
// realmente em andamento (mesma classe de bug do guard "alvo e o current()" acima; ver
// techmagic_delay_test.cpp caso 11 e InitiativeQueue::bring_to_current, que resolve um
// problema PARECIDO mas com semantica diferente - la o alvo VIRA current(), aqui o alvo so
// pode ir pro PROXIMO slot, current() precisa continuar apontando pro caster em resolucao).
// Como o alvo aliado ja passou pelo guard "indice < cursor" acima, ele sempre comeca em
// indice >= cursor()+1; DESDE COMBATE-FILA-CURSOR-FIX (2026-07-15) o clamp inferior
// cursor()+1 e IMPOSTO PELA PROPRIA reorder_pending (nunca cruza o cursor), entao o calculo
// de `delta` aqui embaixo nao precisa mais de um max() manual pra proteger o overshoot -
// so calcula o delta PEDIDO; a primitiva absorve o resto. O VALOR logado continua sendo o
// pedido (nao o efetivamente aplicado), paridade byte-a-byte com o comportamento anterior
// (techmagic_delay_test.cpp e o oraculo).
void handle_delay_action(const EffectSpec& spec, const Card& card, TechMagicContext& ctx) {
    if (ctx.caster == nullptr)
        throw std::logic_error("techMagic::DelayAction: ctx.caster nao pode ser nulo.");
    if (ctx.queue == nullptr)
        throw std::logic_error(
            "techMagic::DelayAction: ctx.queue nao pode ser nulo (OnCast sem fila injetada "
            "no contexto - bug de call site, nao efeito vazio).");
    if (ctx.counterpart == nullptr)
        throw std::logic_error(
            "techMagic::DelayAction: ctx.counterpart (alvo) nao pode ser nulo (OnCast "
            "sempre tem alvo).");

    CombatActor* target = ctx.counterpart;

    // Alvo morto ou fora da fila: nada a reordenar. Estado NORMAL, nao erro.
    if (!target->is_alive() || !ctx.queue->contains(target)) {
        log_entry(ctx, target, 0,
                 ctx.caster->id() + " tavus-executa " + card.id +
                     ": alvo indisponivel, a dilatacao nao se aplica.");
        return;
    }

    // O alvo esta EM ACAO agora (current()): nao ha turno futuro dele nesta rodada pra
    // reordenar - reordena-lo seria indefinido (ele ja esta sendo resolvido).
    if (target == ctx.queue->current()) {
        log_entry(ctx, target, 0,
                 ctx.caster->id() + " tavus-executa " + card.id +
                     ": o alvo ja esta em acao neste instante, a dilatacao nao se aplica.");
        return;
    }

    const std::vector<CombatActor*>& order = ctx.queue->order();
    const auto it = std::find(order.begin(), order.end(), target);
    const int index = static_cast<int>(std::distance(order.begin(), it));
    const int cursor = ctx.queue->cursor();

    // Ja agiu nesta rodada (slot < cursor): a dilatacao se dissipa, nao banca pra proxima
    // rodada (decisao do criador 2026-07-15, AMB registrada em _EFEITOS-ESCOLHIDOS.md).
    // Vale nos dois sentidos - atrasar um inimigo que ja agiu ou adiantar um aliado que ja
    // agiu sao igualmente sem efeito: nao ha "acao futura" pra reordenar.
    if (index < cursor) {
        log_entry(ctx, target, 0,
                 ctx.caster->id() + " tavus-executa " + card.id +
                     ": a dilatacao se dissipa, o alvo ja agiu neste ciclo.");
        return;
    }

    const int count = static_cast<int>(order.size());
    const bool ally = target->is_player_side() == ctx.caster->is_player_side();

    int delta;
    std::string message;
    if (ally) {
        // Espelho beneficio (decisao do lider 2026-07-15): o aliado AVANCA. magnitude==0
        // ("ao extremo") vai pro primeiro slot ainda-nao-agido apos o cursor (age a
        // seguir); magnitude>0 adianta N posicoes fixas. O overshoot pra ALEM do slot
        // cursor()+1 (invariante anti turno-duplo, ver comentario da funcao acima) e
        // absorvido pelo clamp da PROPRIA reorder_pending - nao precisa de max() manual aqui.
        const int min_index = cursor + 1;
        delta = spec.magnitude == 0 ? min_index - index : -spec.magnitude;
        message = ctx.caster->id() + " tavus-executa " + card.id + ": dilata o tempo A "
                 "FAVOR de " + target->id() + ", adiantado pra agir a seguir.";
    } else {
        delta = spec.magnitude == 0 ? (count - 1) - index : spec.magnitude;
        message = ctx.caster->id() + " tavus-executa " + card.id + ": dilata o tempo de " +
                 target->id() + ", empurrado pro fim da fila.";
    }

    // Descarta o valor efetivamente aplicado de proposito: o log usa `delta` (o valor
    // PEDIDO), paridade byte-a-byte com o comportamento anterior (techmagic_delay_test.cpp
    // caso 14 crava esse contrato pro ramo inimigo).
    [[maybe_unused]] const int applied = ctx.queue->reorder_pending(target, delta);

    log_entry(ctx, target, delta, std::move(message));
}

// DamageQuantize (Planck/Quantum-Lock, manifesto item 5): MARCADOR no-op deliberado. A
// quantizacao NAO passa por este dispatcher - o resolvedor (combat_state_machine.cpp::
// resolve_use_card) e o preview PURO (estimate_card_damage) leem o EffectSpec DIRETO via
// combat_state_machine.cpp::quantize_spec_of, plugando no sorteio de degrau do canal COMUM
// da secao 11. Nao ha "alvo"/"dano de evento" no sentido dos outros EffectKind (Leech/
// Reflect/etc.) - nada pra este handler fazer. Existe so pra satisfazer o invariante
// fail-fast "EffectKind sem handler = bug" (techmagic.hpp) sem lancar. Trigger da carta e
// OnCast, que execute_equipped NUNCA despacha (Planck e Passiva, nunca jogada; equipadas so
// recebem Always/OnDamageDealt/OnDamageReceived/OnRoundEnd/OnAllyTurnEnd) - na pratica este
// handler nunca roda; documentado do mesmo jeito que o wiring fora-do-dispatcher do trunfo
// ignores_weakness_wheel de Godel.
void handle_damage_quantize(const EffectSpec&, const Card&, TechMagicContext&) {
    // No-op deliberado: ver comentario acima.
}

}  // namespace

void execute(TriggerHook hook, const Card& card, TechMagicContext& ctx) {
    for (const EffectSpec& spec : card.effects) {
        if (spec.trigger != hook) continue;

        switch (spec.kind) {
            case EffectKind::ApplyStatus:
                handle_apply_status(spec, card, hook, ctx);
                break;
            case EffectKind::Leech:
                handle_leech(spec, card, ctx);
                break;
            case EffectKind::Reflect:
                handle_reflect(spec, card, ctx);
                break;
            case EffectKind::HypotenuseCombo:
                handle_hypotenuse_combo(card, ctx);
                break;
            case EffectKind::RepeatLastAction:
                handle_repeat_last_action(spec, card, ctx);
                break;
            case EffectKind::ChainDamage:
                handle_chain_damage(spec, card, ctx);
                break;
            case EffectKind::DelayAction:
                handle_delay_action(spec, card, ctx);
                break;
            case EffectKind::DamageQuantize:
                handle_damage_quantize(spec, card, ctx);
                break;
            case EffectKind::CloneAlly:
            default:
                throw std::logic_error(
                    "techMagic: EffectKind sem handler implementado na carta '" + card.id +
                    "' (steps 2-3-5-6-7 + manifesto5 cobrem ApplyStatus/Leech/Reflect/"
                    "HypotenuseCombo/RepeatLastAction/ChainDamage/DelayAction/"
                    "DamageQuantize; CloneAlly e step futuro, ADR-016).");
        }
    }
}

void execute_equipped(TriggerHook hook, CombatActor& owner,
                      const std::unordered_map<std::string, Card>* registry,
                      TechMagicContext& ctx) {
    ctx.caster = &owner;
    for (const std::string& id : owner.equipped_special_ids()) {
        if (registry == nullptr)
            throw std::out_of_range("techMagic: '" + owner.id() + "' tem especial equipada '" +
                                    id +
                                    "' mas nenhum card_registry foi injetado na "
                                    "CombatStateMachine.");
        const auto it = registry->find(id);
        if (it == registry->end())
            throw std::out_of_range("techMagic: especial equipada '" + id + "' de '" +
                                    owner.id() + "' nao esta no registry de combate.");
        execute(hook, it->second, ctx);
    }
}

}  // namespace gus::domain::combat::techMagic
