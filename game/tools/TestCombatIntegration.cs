using Godot;
using GusDragon.Engine.Foundation.TurnCombat;
using GusWorld.Game.Foundation.Buses;
using GusWorld.Game.Foundation.TurnCombat;
using System.Collections.Generic;

namespace GusWorld.Game.Tools;

/// <summary>
/// Validação headless da integração CombatManager + CombatBus + PlayerBus.
/// Roda via: godot --headless -s tools/TestCombatIntegration.cs
/// </summary>
public partial class TestCombatIntegration : SceneTree
{
    public override void _Initialize()
    {
        GD.Print("=== TestCombatIntegration START ===");

        var passed = 0;
        var failed = 0;

        void Assert(bool condition, string label)
        {
            if (condition) { GD.Print($"  PASS: {label}"); passed++; }
            else { GD.PrintErr($"  FAIL: {label}"); failed++; }
        }

        // --- Setup ---
        // SceneTree -s scripts rodam _Initialize ANTES dos AutoLoads _Ready (ver
        // ValidateAutoloads.cs). Os nodes já estão em Root, mas o static Instance (setado
        // em _Ready) ainda é null. Buscar de Root e disparar _Ready manualmente prima os
        // singletons sem depender da ordem de boot. Em jogo real, _Ready roda normalmente.
        var manager = Root.GetNodeOrNull<CombatManager>("CombatManager");
        manager?._Ready();
        Assert(manager != null, "CombatManager AutoLoad registrado");

        var combatBus = Root.GetNodeOrNull<CombatBus>("CombatBus");
        combatBus?._Ready();
        Assert(combatBus != null, "CombatBus AutoLoad registrado");

        var playerBus = Root.GetNodeOrNull<PlayerBus>("PlayerBus");
        playerBus?._Ready();
        Assert(playerBus != null, "PlayerBus AutoLoad registrado");

        // --- Conectar signals para captura ---
        var combatStartedFired = false;
        var combatEndedOutcome = "";
        var turnStartedActors = new List<string>();
        var playerResultReceived = false;
        var enemyDefeated = false;
        var statusAppliedFired = new List<string>(); // "actorId:statusId"

        combatBus?.Connect(CombatBus.SignalName.CombatStarted, Callable.From<Godot.Collections.Array>((_) => combatStartedFired = true));
        combatBus?.Connect(CombatBus.SignalName.CombatEnded, Callable.From<string>((o) => combatEndedOutcome = o));
        combatBus?.Connect(CombatBus.SignalName.TurnStarted, Callable.From<string, int>((id, _) => turnStartedActors.Add(id)));
        combatBus?.Connect(CombatBus.SignalName.ActorDefeated, Callable.From<string>((_) => enemyDefeated = true));
        combatBus?.Connect(CombatBus.SignalName.StatusApplied, Callable.From<string, string, int, int>((aid, sid, _, __) => statusAppliedFired.Add($"{aid}:{sid}")));
        playerBus?.Connect(PlayerBus.SignalName.CombatResultReceived, Callable.From<string, int>((_, __) => playerResultReceived = true));

        // --- Montar combate: 1 party vs 1 inimigo, inimigo mais fraco ---
        var gus = new CombatActor("gus", "Gus", maxHp: 30, atk: 10, def: 3, spd: 8, CardFamily.Eletrico, isPlayerSide: true);
        var enemy = new CombatActor("enemy_trash", "Lata Velha", maxHp: 5, atk: 2, def: 0, spd: 4, CardFamily.Cinetico, isPlayerSide: false);

        var brain = new ScriptedBrain();
        var brainRegistry = new Dictionary<string, IEnemyBrain> { ["enemy_trash"] = brain };

        // Carta de teste com status aplicado (valida emissão de StatusApplied, §16).
        // AddStatus roda mesmo num golpe letal, então o StatusApplied dispara ainda que a
        // carta mate o inimigo (a UI quer feedback do efeito aplicado).
        var poison = new StatusEffect(StatusId.Poison, Magnitude: 2, Duration: 3, StackRule.Replace, CardFamily.Bioquimico);
        var card = new Card("pulso.eletrico", "Pulso Elétrico", CardFamily.Eletrico, CardBaseType.Pulso,
            ManaCost: 1, ApCost: 1, Power: 5, TargetShape.Single, StatusApplied: poison,
            Modifiers: new List<CardModifier>(), Mastery: 0);
        var cardRegistry = new Dictionary<string, Card> { [card.Id] = card };

        manager?.StartCombat(new[] { gus, enemy }, cardRegistry, brainRegistry);

        Assert(combatStartedFired, "CombatBus.CombatStarted disparado em StartCombat");

        // --- Simular turno do jogador: Gus vai primeiro (SPD 8 > 4) ---
        // Joga a carta (aplica Poison + dano). O dano mata a lata velha; o Poison ainda
        // é aplicado e deve emitir StatusApplied.
        if (manager != null && CombatManager.Instance?.ActiveActorId == "gus")
        {
            manager.SubmitPlayerAction(CombatAction.UseCard("pulso.eletrico", "enemy_trash"));
        }

        // Em headless, _Process não roda automaticamente: chamar diretamente.
        for (int i = 0; i < 20 && combatEndedOutcome == ""; i++)
        {
            manager?._Process(0.016);
        }

        Assert(combatEndedOutcome == "Victory", $"Combate termina em Victory (foi: '{combatEndedOutcome}')");
        Assert(playerResultReceived, "PlayerBus.CombatResultReceived disparado ao fim do combate");
        Assert(turnStartedActors.Count > 0, "TurnStarted disparado ao menos 1x");
        Assert(enemyDefeated, "CombatBus.ActorDefeated disparado para o inimigo derrotado");
        Assert(statusAppliedFired.Contains("enemy_trash:Poison"),
            $"CombatBus.StatusApplied disparado para Poison no inimigo (fired: [{string.Join(", ", statusAppliedFired)}])");

        // --- Report ---
        GD.Print($"=== RESULTADO: {passed} passou, {failed} falhou ===");
        if (failed > 0) GD.PrintErr("ALGUNS TESTES FALHARAM");

        Quit(failed > 0 ? 1 : 0);
    }
}
