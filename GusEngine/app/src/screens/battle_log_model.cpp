// gus/app/src/screens/battle_log_model.cpp
//
// Implementacao do modelo PURO do log (ver header). Classifica CombatLogEntry e
// StatusEffectChange em LogLine (categoria + texto cru), e seleciona os NOTAVEIS (D7).
// Sem SDL. As heuristicas seguem as mensagens que a FSM ja produz (combat_state_machine).

#include "gus/app/screens/battle_log_model.hpp"


#include "gus/domain/combat/combat_enums.hpp"

namespace gus::app::screens {

namespace {

using gus::domain::combat::CombatActionType;
using gus::domain::combat::CombatLogEntry;
using gus::domain::combat::StatusChangeKind;
using gus::domain::combat::StatusEffectChange;

// Piso de "dano grande" pra entrar no log mesmo sem ser critico (D7: nao todo hit, mas
// hits pesados sim). Ajustavel; conservador no slice.
constexpr int kBigDamageFloor = 20;

// true se a message carrega um marcador de canal NOTAVEL (critico/falha de compilacao),
// que o motor sufixa (combat.md par.11; ver resolve_use_card).
bool has_channel_marker(const std::string& m) noexcept {
    return m.find("[CRITICO]") != std::string::npos ||
           m.find("FALHA DE COMPILACAO") != std::string::npos;
}

// true se a message e uma anuncio de combo/sistema (COMPILADO:, ERRO, ANALISE).
bool is_system_message(const std::string& m) noexcept {
    return m.rfind("COMPILADO:", 0) == 0 ||
           m.find("ERRO DE COMPILACAO") != std::string::npos ||
           m.find("ANALISE PREDITIVA") != std::string::npos;
}

}  // namespace

bool is_notable(const CombatLogEntry& e) noexcept {
    switch (e.action) {
        case CombatActionType::Attack:
        case CombatActionType::UseCard:
            // Dano: notavel se grande OU marcado (critico/falha) OU anuncio de sistema
            // (COMPILADO/ERRO). Hit comum pequeno NAO entra (numero flutuante cobre).
            return is_system_message(e.message) || has_channel_marker(e.message) ||
                   e.value >= kBigDamageFloor;
        case CombatActionType::Scan:
        case CombatActionType::ScanEnvironment:
        case CombatActionType::GambitPredict:
        case CombatActionType::GambitReorder:
        case CombatActionType::Flee:
            return true;  // acoes de sistema/informacao sempre ecoam (D7)
        case CombatActionType::Defend:
            return true;  // ganhar Shield e notavel (muda a leitura do proximo hit)
        case CombatActionType::StatusTick:
            // Efeito de sistema fora de uma CombatAction de jogador (ex.: Knockback adiando
            // o turno, secao 9): regra canonica do lider (COMBATE-FILA-CURSOR-FIX,
            // 2026-07-15) - "todo efeito loga uma mensagem diegetica pro player entender o
            // que aconteceu". Sempre ecoa, mesmo racional do Scan/Gambito/Flee.
            return true;
        case CombatActionType::Pass:
            // Pass so e notavel se carrega message de sistema (ex.: ANALISE PREDITIVA).
            return !e.message.empty() && is_system_message(e.message);
    }
    return false;
}

LogLine classify(const CombatLogEntry& e) {
    LogLineKind kind = LogLineKind::System;
    switch (e.action) {
        case CombatActionType::Attack:
        case CombatActionType::UseCard:
            // Anuncio de combo/sistema vence (COMPILADO:); senao e dano.
            kind = is_system_message(e.message) ? LogLineKind::System
                                                : LogLineKind::Damage;
            break;
        case CombatActionType::Defend:
            kind = LogLineKind::Status;  // Shield aplicado
            break;
        case CombatActionType::Scan:
        case CombatActionType::ScanEnvironment:
        case CombatActionType::GambitPredict:
        case CombatActionType::GambitReorder:
        case CombatActionType::Flee:
        case CombatActionType::Pass:
            kind = LogLineKind::System;
            break;
        case CombatActionType::StatusTick:
            kind = LogLineKind::Status;  // efeito de status (Knockback adiando o turno).
            break;
    }
    return LogLine{kind, e.message};
}

LogLine classify(const StatusEffectChange& c) {
    // Toda mudanca de status e categoria Status (cura/Regen poderia ser Heal, mas o
    // tick de cura nao vem por StatusEffectChange; mantemos Status pra Applied/Expired/
    // Absorbed). O texto cru e sintetico (o motor nao da message aqui).
    std::string txt = "status ";
    switch (c.kind) {
        case StatusChangeKind::Applied:  txt += "aplicado"; break;
        case StatusChangeKind::Expired:  txt += "expirou"; break;
        case StatusChangeKind::Absorbed: txt += "absorvido"; break;
    }
    txt += " em " + c.actor_id;
    return LogLine{LogLineKind::Status, std::move(txt)};
}

std::vector<LogLine> build_log_lines(
    const std::vector<CombatLogEntry>& log,
    const std::vector<StatusEffectChange>& status_changes, int max_lines) {
    // BUG-FIX (criador testou no display, 2026-06-25): o log nao mostrava as acoes de
    // ataque. CAUSA-RAIZ confirmada por instrumentacao: status_changes (Applied) eram
    // empilhados DEPOIS de TODAS as acoes; com o corte das ULTIMAS N linhas, a janela
    // ficava dominada pelos "status aplicado" do INICIO da batalha (Regen/Haste/Poison
    // do demo), empurrando a narracao de ataque pra fora da caixa.
    //
    // FIX: o log e a NARRACAO DAS ACOES (combat.md/battle-screen D7-LOG: "gus ataca por
    // X"). As acoes mandam: o corte mantem as ULTIMAS N ACOES (o que o criador quer ver).
    // O status nao polui mais o historico - o status do ator ja aparece como ICONE sob
    // ele na arena/painel (incremento 2); o log nao precisa repetir, e o dump inicial de
    // status nao pode esconder o combate. (status_changes fica disponivel pra um painel
    // de status dedicado no futuro, mas FORA do rolling de narracao.)
    std::vector<LogLine> lines;
    lines.reserve(log.size());
    for (const CombatLogEntry& e : log) {
        lines.push_back(classify(e));
    }
    (void)status_changes;  // intencionalmente fora do rolling de narracao (ver acima)

    // A caixa rola: mantem so as ULTIMAS max_lines acoes (mostra o combate recente).
    if (max_lines > 0 && static_cast<int>(lines.size()) > max_lines) {
        lines.erase(lines.begin(), lines.end() - max_lines);
    }
    return lines;
}

std::string_view status_name_key(gus::domain::combat::StatusId id) noexcept {
    using gus::domain::combat::StatusId;
    switch (id) {
        case StatusId::Stun:      return "STATUS_STUN_NAME";
        case StatusId::Poison:    return "STATUS_POISON_NAME";
        case StatusId::Corrode:   return "STATUS_CORRODE_NAME";
        case StatusId::Disrupt:   return "STATUS_DISRUPT_NAME";
        case StatusId::Silence:   return "STATUS_SILENCE_NAME";
        case StatusId::Knockback: return "STATUS_KNOCKBACK_NAME";
        case StatusId::Break:     return "STATUS_BREAK_NAME";
        case StatusId::Expose:    return "STATUS_EXPOSE_NAME";
        case StatusId::Decrypt:   return "STATUS_DECRYPT_NAME";
        case StatusId::Shield:    return "STATUS_SHIELD_NAME";
        case StatusId::Regen:     return "STATUS_REGEN_NAME";
        case StatusId::Haste:     return "STATUS_HASTE_NAME";
        case StatusId::Slow:      return "STATUS_SLOW_NAME";
        case StatusId::SobrecargaTermica: return "STATUS_SOBRECARGATERMICA_NAME";
        case StatusId::Resfriamento:      return "STATUS_RESFRIAMENTO_NAME";
        case StatusId::Reflect:           return "STATUS_REFLECT_NAME";
        case StatusId::BlindagemEM:       return "STATUS_BLINDAGEMEM_NAME";
        case StatusId::NullProof:         return "STATUS_NULLPROOF_NAME";
    }
    return "STATUS_STUN_NAME";  // guarda (enum coberto)
}

std::string consequence_suffix(
    const std::string& target_id,
    const std::vector<StatusEffectChange>& changes) {
    std::string out;
    for (const StatusEffectChange& c : changes) {
        // So status APLICADO no ALVO do golpe entra na consequencia (D12). Applied so.
        if (c.kind != StatusChangeKind::Applied || c.actor_id != target_id) {
            continue;
        }
        // "; <alvo> ficou com <STATUS_KEY>". A STATUS_KEY fica literal; a casca resolve
        // por tr() antes de exibir (o POCO nao depende de i18n).
        out += "; " + target_id + " ficou com " +
               std::string(status_name_key(c.id));
    }
    return out;
}

}  // namespace gus::app::screens
