// gus/domain/combat/combat_actor.hpp
//
// Combatente em campo. Estado MUTAVEL (HP/AP/Mana/status mudam a cada acao), modelado
// como classe de referencia para que a fila de iniciativa e a FSM guardem ponteiros
// estaveis (CombatActor*) e mutem in-place. Portado de
// engine/foundation/turn_combat/CombatActor.cs. POCO puro, ZERO Qt (invariante de
// domain/, engine-design.md secao 2).
//
// DECISAO DE DESIGN (herdada do C#, reportada ao criador): a spec secao 17 lista
// CombatActor entre os "records imutaveis". Modelamos como classe mutavel porque um
// combatente e uma entidade com identidade e ciclo de vida, nao um valor. Os dados
// PUROS (Card, StatusEffect, CombatAction, IntentPreview, ComboRecipe) permanecem
// imutaveis (combat_records.hpp).
//
// PORTE DE ACESSO: o C# marca varios metodos `internal` (RefreshResourcesForTurn,
// SpendAp, ApplyStatDelta, IndexOfStatus, ReplaceStatusAt, ExpireElapsedStatuses,
// DrainStatusChanges, RevertStatDelta) e propriedades `internal set` (IsScanned). C++
// nao tem `internal`; expomos como public/metodos publicos. A FSM (chunk 4) e os
// testes os usam diretamente, mesma superficie de chamada do C#.
//
// MAPEAMENTO de excecoes C# -> C++ (mesmo padrao do M3):
//   ArgumentException            -> std::invalid_argument
//   ArgumentOutOfRangeException  -> std::out_of_range
//   InvalidOperationException    -> std::logic_error
//
// Invariantes garantidos no construtor (fail-fast):
//  - Id nao vazio (nem so de espacos, IsNullOrWhiteSpace).
//  - MaxHp > 0; Atk/Def/Spd >= 0; KnowledgeKills >= 0.
//  - Hp inicia cheio (= MaxHp).
//
// Cross-ref: engine/foundation/turn_combat/CombatActor.cs;
//            docs/design/mecanicas/combat.md secao 5/6/9/16/17; ADR-006.

#ifndef GUS_DOMAIN_COMBAT_COMBAT_ACTOR_HPP
#define GUS_DOMAIN_COMBAT_COMBAT_ACTOR_HPP

#include <string>
#include <utility>
#include <vector>

#include "gus/domain/combat/combat_constants.hpp"
#include "gus/domain/combat/combat_enums.hpp"
#include "gus/domain/combat/combat_records.hpp"

namespace gus::domain::combat {

// Resultado de try_add_status (ADR-016 Balde B, Faraday/EM-Shield portao de imunidade).
// BlockedByImmunity: NAO inseriu o status (o alvo tem BlindagemEM ativa e o status entrante
// e um debuff de familia Eletrico) - o caller NUNCA deve logar "aplicado" nesse caso (ver
// techmagic.cpp::handle_apply_status e combat_state_machine.cpp).
enum class StatusApplyResult {
    Applied,
    BlockedByImmunity,
};

// Combatente (party ou inimigo). Identidade + estado mutavel de combate.
class CombatActor {
public:
    CombatActor(std::string id,
                std::string display_name,
                int max_hp,
                int atk,
                int def,
                int spd,
                CardFamily family,
                bool is_player_side,
                bool is_boss = false,
                int knowledge_kills = 0,
                bool is_universal_compiler = false);

    // ---- Identidade + stats imutaveis ----

    // Identidade estavel (chave de save/i18n/telemetria).
    [[nodiscard]] const std::string& id() const noexcept { return id_; }
    // Nome diegetico (via tr() na UI).
    [[nodiscard]] const std::string& display_name() const noexcept { return display_name_; }
    // HP maximo. Invariante: > 0.
    [[nodiscard]] int max_hp() const noexcept { return max_hp_; }
    // Ataque base (entra na formula de dano secao 11).
    [[nodiscard]] int atk() const noexcept { return atk_; }
    // Familia do ator (roda de fraqueza secao 6).
    [[nodiscard]] CardFamily family() const noexcept { return family_; }
    // true = membro da party; false = inimigo.
    [[nodiscard]] bool is_player_side() const noexcept { return is_player_side_; }
    // true = boss/mini-boss. Bloqueia Flee enquanto vivo (secao 14).
    [[nodiscard]] bool is_boss() const noexcept { return is_boss_; }
    // Kills do MESMO tipo de inimigo acumulados (Knowledge Progression secao 11). >= 0.
    [[nodiscard]] int knowledge_kills() const noexcept { return knowledge_kills_; }
    // true = compilador universal (Gus). Defesa neutra (mult 1.0) como alvo (secao 6.1).
    [[nodiscard]] bool is_universal_compiler() const noexcept { return is_universal_compiler_; }

    // ---- Estado mutavel ----

    // HP atual. Clamp [0, MaxHp].
    [[nodiscard]] int hp() const noexcept { return hp_; }
    // Defesa base (reduzida por Break/Corrode).
    [[nodiscard]] int def() const noexcept { return def_; }
    // Velocidade: define posicao na fila de iniciativa secao 4.
    [[nodiscard]] int spd() const noexcept { return spd_; }
    // true se ja foi alvo de Scan (secao 12). Habilita Null/Expose (secao 8).
    [[nodiscard]] bool is_scanned() const noexcept { return is_scanned_; }
    void set_scanned(bool value) noexcept { is_scanned_ = value; }
    // AP disponivel no turno atual. Preparado em TurnStart pela FSM.
    [[nodiscard]] int ap() const noexcept { return ap_; }
    // AP maximo por turno (3 fixo no slice; parametrizavel depois). secao 5.
    [[nodiscard]] int max_ap() const noexcept { return max_ap_; }
    // Mana disponivel no turno atual.
    [[nodiscard]] int mana() const noexcept { return mana_; }
    // Mana maxima do turno atual (ramp 2 + turno_index, cap 8). secao 5.
    [[nodiscard]] int max_mana() const noexcept { return max_mana_; }
    // Vivo = HP > 0.
    [[nodiscard]] bool is_alive() const noexcept { return hp_ > 0; }
    // Status ativos no ator (read-only snapshot).
    [[nodiscard]] const std::vector<StatusEffect>& status_effects() const noexcept {
        return status_effects_;
    }

    // true se id e um buff (status benefico), pro dispel do Decrypt (secao 9). Definido
    // por exclusao: buff = NAO esta no conjunto de debuffs/neutros conhecidos. Cobre
    // buffs futuros automaticamente (decisao criador 2026-06-03).
    [[nodiscard]] static bool is_buff(StatusId id);

    // Aplica dano (clamp em 0). Dano negativo e erro de chamador (out_of_range).
    // Shield (secao 9, BUG-3) absorve TODO dano antes do HP; depleta e remove o Shield
    // quando o pool zera; registra Absorbed/Expired (secao 16).
    void take_damage(int amount);

    // Cura (clamp em MaxHp). Cura negativa e erro de chamador (out_of_range).
    void heal(int amount);

    // Reduz a defesa por amount (clamp em 0). Corrode (reducao PERMANENTE). secao 9.
    // amount negativo e erro de chamador (out_of_range).
    void reduce_def(int amount);

    // Aplica (one-shot) o delta de stat de um status de DURACAO (Break/Haste/Slow). A
    // FSM chama no 1o tick em que o status fica ativo. Idempotente por id. Retorna true
    // se aplicou agora. secao 9.
    bool apply_stat_delta(StatusId id, int def_delta, int spd_delta);

    // Reverte o delta de stat aplicado por apply_stat_delta no expire/dispel. No-op se
    // nunca foi aplicado. Retorna true se reverteu um delta de SPD (caller recomputa a
    // fila); false para Def (Break). secao 9.
    bool revert_stat_delta(StatusId id);

    // Prepara AP e Mana para o turno (FSM em TurnStart). AP reseta para MaxAp; Mana
    // recarrega ao maximo (sem carry-over): mana_max = clamp(2 + round_index, .., 8).
    // secao 5.
    void refresh_resources_for_turn(int round_index);

    // Consome AP. Falha (logic_error) se insuficiente; (out_of_range) se cost < 0.
    void spend_ap(int cost);

    // Consome mana. Falha (logic_error) se insuficiente; (out_of_range) se cost < 0.
    void spend_mana(int cost);

    // Aplica um status respeitando a StackRule contra um existente de mesmo Id
    // (Replace/Refresh/StackMagnitude/StackDuration). Sem existente, apenas adiciona.
    // secao 9. Registra Applied (secao 16). WRAPPER de try_add_status (abaixo) que ignora
    // o retorno - preserva os call sites de BUFF/defesa (resolve_defend/Shield etc) que
    // nunca precisam checar bloqueio.
    void add_status(const StatusEffect& status);

    // Portao de imunidade EM-Shield (Faraday, ADR-016 Balde B): se este ator tem
    // BlindagemEM ativa E `status` e um debuff (!is_buff) de familia Eletrico, o status NAO
    // e inserido e o metodo retorna BlockedByImmunity (SEM registrar Applied - secao 16).
    // Caso contrario, aplica normalmente (mesma logica de StackRule de add_status) e retorna
    // Applied. Efeito colateral extra quando o status inserido E o proprio BlindagemEM:
    // limpa (remove_status) todos os debuffs de familia Eletrico JA presentes no alvo -
    // "previne + limpa" (decisao do criador, EM-Shield). Choke point unico: TODO sitio de
    // status OFENSIVO do motor (techmagic.cpp::handle_apply_status, combat_state_machine.cpp
    // status_applied/result_status) deve chamar ISTO, nao add_status, pra respeitar o
    // portao (ver PR2/Newton, que reusa a mesma primitiva).
    StatusApplyResult try_add_status(const StatusEffect& status);

    // Remove status por id (dispel via carta utilitaria, Null hostil ou Decrypt).
    // Reverte primeiro qualquer delta de stat aplicado. Retorna true se removeu >= 1.
    // secao 9.
    bool remove_status(StatusId id);

    // Indice corrente do status com este id, ou -1 se ausente (FSM re-localiza no tick).
    [[nodiscard]] int index_of_status(StatusId id) const;

    // Substitui o status no indice index (uso da FSM no tick: decrementar Duration etc).
    void replace_status_at(int index, const StatusEffect& updated);

    // Remove todos os status com Duration <= 0 (expire no TurnEnd). Reverte deltas de
    // stat de duracao. Retorna true se algum status que afeta SPD expirou (FSM recomputa
    // a fila). Registra Expired (secao 16). secao 9.
    bool expire_elapsed_statuses();

    // Drena (le e limpa) o buffer de mudancas de status (secao 16). Consumo unico.
    [[nodiscard]] std::vector<StatusEffectChange> drain_status_changes();

    // Restaura mana (clamp em max_mana). Ex.: Leech do executor techMagic (ADR-016).
    // Negativo e erro de chamador (out_of_range), mesmo padrao de heal/take_damage.
    void restore_mana(int amount);

    // ---- Especiais equipadas (executor techMagic, ADR-016) ----

    // Ids de Card (Passiva/Hibrida) equipadas por este ator. Vazio = comportamento atual
    // (nenhuma passiva ativa). Resolvidas contra o card_registry ja existente na FSM - o
    // CombatActor NAO guarda os dados da carta, so a referencia por id.
    [[nodiscard]] const std::vector<std::string>& equipped_special_ids() const noexcept {
        return equipped_special_ids_;
    }
    void set_equipped_special_ids(std::vector<std::string> ids) {
        equipped_special_ids_ = std::move(ids);
    }

    // ---- Tag "comando central" (Mises/Calc-Edge, CARD-ENGINE-MANIFESTO item 9) ----

    // true = este ator esta marcado "comando central" (EffectKind::ApEfficiency face 2):
    // sofre atraso de fila + erro de mira quando algum VIVO do lado OPOSTO porta a carta
    // Mises equipada. Espelha EnemyTemplate::central_command (templates/enemy_template.hpp).
    // Default false preserva TODO ator/teste existente. Setter dedicado (MESMO padrao de
    // equipped_special_ids acima) - nenhum call site do construtor quebra.
    [[nodiscard]] bool central_command() const noexcept { return central_command_; }
    void set_central_command(bool value) noexcept { central_command_ = value; }

    // Concede AP bonus TEMPORARIO neste turno (Mises/Calc-Edge, CARD-ENGINE-MANIFESTO item
    // 9, EffectKind::ApEfficiency face 1). NAO muta max_ap_: o bonus NAO persiste, o
    // PROXIMO refresh_resources_for_turn reseta ap_ = max_ap_ de novo (secao 5). amount
    // negativo e erro de chamador (out_of_range), mesmo padrao de heal/restore_mana.
    void grant_bonus_ap(int amount);

    // ---- Overclock (CARTAS-COMUNS-ENGINE, Eletrico-utilidade "Tavus-Overclock",
    // 2026-07-16) ----

    // true = este ator JA recarregou recurso (Card::restore_ap/restore_mana) NESTE turno.
    // Trava 1x/turno (decisao do lider: sem ela vira loop infinito + farm de Mastery) - a
    // FSM le/marca em resolve_use_card; resetada por refresh_resources_for_turn (mesmo
    // lugar que zera AP/mana, secao 5).
    [[nodiscard]] bool overclock_used() const noexcept { return overclock_used_; }
    void set_overclock_used(bool value) noexcept { overclock_used_ = value; }

private:
    // Drena o pool de Shield contra amount e devolve o dano remanescente pro HP.
    int absorb_with_shield(int amount);
    void record_applied(const StatusEffect& status);
    // Corpo de insercao/stacking de add_status/try_add_status (StackRule contra um
    // existente de mesmo Id). Extraido pra try_add_status poder rodar o portao de
    // imunidade ANTES e o cleanse de BlindagemEM (F-2) DEPOIS, sem duplicar a logica de
    // StackRule.
    void insert_or_stack_status(const StatusEffect& status);
    // true se este ator tem BlindagemEM ativa E `status` e um debuff de familia Eletrico
    // (portao de imunidade EM-Shield, ADR-016 Balde B).
    [[nodiscard]] bool blocked_by_em_shield(const StatusEffect& status) const;
    // Remove todos os debuffs de familia Eletrico ja presentes (exceto o proprio
    // BlindagemEM), chamado apos inserir BlindagemEM (F-2, "previne + limpa").
    void clear_electric_debuffs();

    // Identidade + stats fixos.
    std::string id_;
    std::string display_name_;
    int max_hp_;
    int atk_;
    CardFamily family_;
    bool is_player_side_;
    bool is_boss_;
    int knowledge_kills_;
    bool is_universal_compiler_;

    // Estado mutavel.
    int hp_;
    int def_;
    int spd_;
    bool is_scanned_ = false;
    int ap_ = 0;
    int max_ap_ = combat_constants::kBaseApPerTurn;
    int mana_ = 0;
    int max_mana_ = 0;

    std::vector<StatusEffect> status_effects_;
    // Deltas de stat de DURACAO (Break/Haste/Slow) ja aplicados, por StatusId. Aplicacao
    // one-shot; guardamos o delta exato pra restaurar com fidelidade no expire/dispel.
    // Break: delta = -Magnitude em Def; Haste: +Magnitude em Spd; Slow: -Magnitude em Spd.
    std::vector<std::pair<StatusId, int>> applied_stat_deltas_;
    // Buffer de mudancas de status (secao 16), drenado pos-turno pela FSM.
    std::vector<StatusEffectChange> status_changes_;
    // Especiais equipadas (executor techMagic, ADR-016). Vazio por padrao (nenhuma
    // passiva); ver equipped_special_ids()/set_equipped_special_ids() acima.
    std::vector<std::string> equipped_special_ids_;
    // Tag "comando central" (Mises/Calc-Edge, CARD-ENGINE-MANIFESTO item 9). Default false;
    // ver central_command()/set_central_command() acima.
    bool central_command_ = false;
    // Trava 1x/turno do Overclock (CARTAS-COMUNS-ENGINE). Default false; ver
    // overclock_used()/set_overclock_used() acima.
    bool overclock_used_ = false;
};

}  // namespace gus::domain::combat

#endif  // GUS_DOMAIN_COMBAT_COMBAT_ACTOR_HPP
