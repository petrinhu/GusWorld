// gus/app/screens/battle_log_model.hpp
//
// MODELO PURO da caixa de LOG da BattleScreen (M5, incremento 3, D7): POCO 100%
// testavel SEM SDL. Classifica os eventos que o motor acumula (CombatLogEntry +
// StatusEffectChange, combat.md par.16) em LINHAS com COR por TIPO (dano / cura /
// status / sistema), e seleciona so os NOTAVEIS (D7: nao todo hit). O texto fica
// PLACEHOLDER ate a fonte entrar: cada linha carrega a categoria + a mensagem CRUA
// (string que o motor ja produz), pronta pra virar texto quando houver fonte; por ora
// o render desenha so a MARCA colorida.
//
// SEM FONTE (decisao do criador): nada de stb_truetype/fonte aqui. A linha guarda a
// message do motor pra o futuro; o incremento 3 desenha barra/marca, nao glifo.
//
// D7 (eventos NOTAVEIS): o log so ecoa o que importa - sistema (COMPILADO/ERRO/ANALISE,
// que vem como Pass/UseCard com message), dano CRITICO/grande, status aplicado, e morte
// de ator. Hits comuns NAO entram (o numero flutuante, incremento 5, cobre o hit). A
// regra de "notavel" e POCO testavel (is_notable).
//
// Cross-ref: docs/design/mecanicas/battle-screen.md par.5 (D7);
//            gus/domain/combat/combat_records.hpp (CombatLogEntry/StatusEffectChange);
//            gus/app/screens/battle_layout.hpp (zona do log).

#ifndef GUS_APP_SCREENS_BATTLE_LOG_MODEL_HPP
#define GUS_APP_SCREENS_BATTLE_LOG_MODEL_HPP

#include <string>
#include <string_view>
#include <vector>

#include "gus/domain/combat/combat_enums.hpp"  // StatusId
#include "gus/domain/combat/combat_records.hpp"

namespace gus::app::screens {

// Categoria visual de uma linha do log (cor por tipo, D7).
enum class LogLineKind : int {
    System = 0,   // mensagem de sistema (COMPILADO/ERRO/ANALISE PREDITIVA)
    Damage = 1,   // dano notavel (critico/grande) a um ator
    Heal = 2,     // cura/Regen
    Status = 3,   // status aplicado/expirado (Poison/Stun/Shield...)
    Defeat = 4,   // morte/incapacitacao de um ator
};

// Uma linha pronta pra exibir: categoria (define a cor) + texto CRU do motor
// (placeholder ate a fonte; o render do incremento 3 usa so a categoria).
struct LogLine {
    LogLineKind kind = LogLineKind::System;
    std::string text;  // message crua do motor (ou derivada); NAO renderizada ainda
};

// true se uma entrada de log e NOTAVEL (D7): vale ecoar na caixa. Sistema (tem message
// nao-vazia e nao e um hit comum), dano com value alto, etc. crit_threshold define o
// piso de "dano grande" (default deixa o chamador passar; ver .cpp).
[[nodiscard]] bool is_notable(const gus::domain::combat::CombatLogEntry& e) noexcept;

// Classifica uma entrada de log do motor numa LogLine (categoria + texto cru). NAO
// filtra: classifica. O filtro de "notavel" e is_notable (aplicado por build_log_lines).
[[nodiscard]] LogLine classify(const gus::domain::combat::CombatLogEntry& e);

// Classifica uma mudanca de status (Applied/Expired/Absorbed) numa LogLine de Status.
[[nodiscard]] LogLine classify(const gus::domain::combat::StatusEffectChange& c);

// Monta as linhas NOTAVEIS (D7) a partir dos logs + status changes acumulados pelo
// motor, na ordem de chegada, limitando as ULTIMAS max_lines (a caixa rola). O motor
// acumula tudo; aqui filtramos (is_notable) e cortamos pro tamanho da caixa.
[[nodiscard]] std::vector<LogLine> build_log_lines(
    const std::vector<gus::domain::combat::CombatLogEntry>& log,
    const std::vector<gus::domain::combat::StatusEffectChange>& status_changes,
    int max_lines);

// ---- D12: log narra a CONSEQUENCIA (incremento 6) ----
//
// Chave i18n do NOME de um StatusId (STATUS_<id>_NAME, ver resources/translations). A UI
// resolve via tr(); o POCO so devolve a chave (deterministico, testavel). Todo StatusId
// do enum mapeia (sem chave vazia).
[[nodiscard]] std::string_view status_name_key(
    gus::domain::combat::StatusId id) noexcept;

// Sufixo de CONSEQUENCIA pra a linha de um golpe (D12): "; <target> ficou com
// <STATUS_KEY>" pra cada status APLICADO no target_id neste evento. Vazio se nenhum
// status foi aplicado no alvo. As STATUS_KEY ficam LITERAIS (a casca substitui por tr()
// antes de exibir; o POCO nao depende de i18n). Status aplicado em OUTRO ator nao entra.
[[nodiscard]] std::string consequence_suffix(
    const std::string& target_id,
    const std::vector<gus::domain::combat::StatusEffectChange>& changes);

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_LOG_MODEL_HPP
