// CombatScene.cs
//
// Control raiz da cena de combate jogável (F2-G.5). Liga o motor POCO
// (CombatStateMachine, via AutoLoad CombatManager) ao jogador: HUD de HP/AP/Mana,
// menu de ações, mão de cartas placeholder, log de feedback e overlay de resultado.
//
// FRONTEIRA (gameplay puro, não-backend): esta classe NÃO cria domínio nem
// persistência. Consome CharacterRepository/CanonicalTemplates/PlaceholderCards
// (engine) para montar o encontro de referência, inicia o combate no CombatManager
// e reage aos signals do CombatBus. Toda mecânica de turno vive na FSM; aqui só
// traduzimos estado em pixels e input em CombatAction.
//
// Localização: o projeto usa o AutoLoad custom Localization (TrMd + res://translations/*.md),
// NÃO o tr() nativo do Godot. Strings user-facing passam por Tr(...) (wrapper local
// defensivo). Chaves COMBAT_* definidas em translations/pt_br.md (§5).
//
// Seleção de alvo (F2-G.5): ações ofensivas e cartas não-self abrem uma lista de inimigos
// vivos (PainelAlvo construído sob demanda); o clique alimenta a CombatAction com o id
// escolhido. Defender/Passar/Fugir e cartas self-target não têm seleção (resolvem direto).
//
// Cross-ref: docs/design/mecanicas/combat.md §16 (event bus), §17 (escopo do slice);
//            game/scripts/foundation/turn_combat/CombatManager.cs;
//            engine/foundation/turn_combat/PlaceholderCards.cs / CanonicalTemplates.cs.

using Godot;
using GusDragon.Engine.Foundation.Data;
using GusDragon.Engine.Foundation.TurnCombat;
using GusWorld.Game.Foundation.Buses;

namespace GusWorld.Game.Foundation.TurnCombat;

/// <summary>
/// Cena de combate do vertical slice. Monta o encontro canônico, inicia o combate no
/// <see cref="CombatManager"/> e reflete o estado da FSM via signals do <see cref="CombatBus"/>.
/// </summary>
public partial class CombatScene : Control
{
    // Limite de linhas mantidas no log de feedback (evita crescer sem fim numa sessão longa).
    private const int MaxLinhasLog = 200;

    // Painéis de ator construídos dinamicamente em _Ready, indexados por id do ator. Agrupa
    // as refs de nodes que precisam atualizar a cada evento (barra de HP + rótulo de recursos).
    private readonly Dictionary<string, AtorPainel> _paineis = new();

    // true enquanto o jogador está escolhendo uma carta (PainelMaoCartas aberto). Impede o
    // polling de _Process de reabrir o PainelAcoes por cima do PainelMaoCartas.
    private bool _selecionandoCarta;

    // true enquanto o jogador está escolhendo o alvo de uma ação ofensiva (PainelAlvo aberto).
    // Mesmo papel de _selecionandoCarta para o seletor de alvo: trava o polling de _Process de
    // reabrir o PainelAcoes por cima da lista de alvos. _callbackAlvo guarda o que fazer com o
    // id escolhido (ex: Submeter(CombatAction.Attack(id))); _painelAlvo é construído sob demanda
    // na primeira seleção e reusado (lista de inimigos vivos reconstruída a cada abertura).
    private bool _selecionandoAlvo;
    private Action<string>? _callbackAlvo;
    private VBoxContainer? _painelAlvo;

    // -------------------------------------------------------------------------
    // Refs de nodes (resolvidas em _Ready via GetNode; paths casam com o .tscn).
    // -------------------------------------------------------------------------
    private HBoxContainer _listaInimigos = null!;
    private HBoxContainer _barraIniciativa = null!;
    private VBoxContainer _painelAcoes = null!;
    private VBoxContainer _painelMaoCartas = null!;
    private HBoxContainer _listaCartas = null!;
    private HBoxContainer _painelParty = null!;
    private RichTextLabel _logFeedback = null!;
    private Control _overlayResultado = null!;
    private Label _labelResultado = null!;

    public override void _Ready()
    {
        ResolverNodes();
        AplicarTextosEstaticos();
        AplicarTooltipsAcoes();
        ConectarBotoes();
        ConectarSignals();

        // Estado inicial dos painéis sobrepostos: escondidos até o fluxo abri-los.
        _painelAcoes.Visible = false;
        _painelMaoCartas.Visible = false;
        _overlayResultado.Visible = false;

        IniciarCombate();
        PopularMaoDeCartas();
    }

    public override void _ExitTree() => DesconectarSignals();

    // Polling de visibilidade: CombatManager seta WaitingForPlayerAction = true em TryAdvance
    // (chamado todo _Process) sem emitir TurnStarted, então a UI não recebe signal p/ abrir o
    // painel. Poll aqui garante que o menu aparece no frame seguinte ao turno da party.
    public override void _Process(double delta) => AtualizarVisibilidadeAcoes();

    // -------------------------------------------------------------------------
    // Montagem
    // -------------------------------------------------------------------------

    private void ResolverNodes()
    {
        _listaInimigos = GetNode<HBoxContainer>("PainelInimigos/ListaInimigos");
        _barraIniciativa = GetNode<HBoxContainer>("BarraIniciativa");
        _painelAcoes = GetNode<VBoxContainer>("PainelAcoes");
        _painelMaoCartas = GetNode<VBoxContainer>("PainelMaoCartas");
        _listaCartas = GetNode<HBoxContainer>("PainelMaoCartas/ListaCartas");
        _painelParty = GetNode<HBoxContainer>("PainelParty");
        _logFeedback = GetNode<RichTextLabel>("LogFeedback");
        _overlayResultado = GetNode<Control>("OverlayResultado");
        _labelResultado = GetNode<Label>("OverlayResultado/LabelResultado");

        // Nodes de exibição pura não devem interceptar cliques (evita bloquear botões
        // que ficam sob eles no layout flat). RichTextLabel default = MOUSE_FILTER_STOP.
        _logFeedback.MouseFilter = Control.MouseFilterEnum.Ignore;
        _barraIniciativa.MouseFilter = Control.MouseFilterEnum.Ignore;
        _painelParty.MouseFilter = Control.MouseFilterEnum.Ignore;
    }

    private void AplicarTextosEstaticos()
    {
        GetNode<Label>("PainelInimigos/LabelInimigos").Text = Tr("COMBAT_PANEL_ENEMIES");
        GetNode<Label>("PainelAcoes/LabelAcoes").Text = Tr("COMBAT_PANEL_ACTIONS");
        GetNode<Label>("PainelMaoCartas/LabelCartas").Text = Tr("COMBAT_PANEL_HAND");

        GetNode<Button>("PainelAcoes/BtnAtacar").Text = Tr("COMBAT_ACTION_ATTACK");
        GetNode<Button>("PainelAcoes/BtnDefender").Text = Tr("COMBAT_ACTION_DEFEND");
        GetNode<Button>("PainelAcoes/BtnScan").Text = Tr("COMBAT_ACTION_SCAN");
        GetNode<Button>("PainelAcoes/BtnGambitoPrever").Text = Tr("COMBAT_ACTION_GAMBIT_PREDICT");
        GetNode<Button>("PainelAcoes/BtnGambitoReordenar").Text = Tr("COMBAT_ACTION_GAMBIT_REORDER");
        GetNode<Button>("PainelAcoes/BtnUsarCarta").Text = Tr("COMBAT_ACTION_USE_CARD");
        GetNode<Button>("PainelAcoes/BtnFugir").Text = Tr("COMBAT_FLEE");
        GetNode<Button>("PainelAcoes/BtnPassar").Text = Tr("COMBAT_ACTION_PASS");
        GetNode<Button>("OverlayResultado/BtnReiniciar").Text = Tr("COMBAT_ACTION_RESTART");
    }

    private void AplicarTooltipsAcoes()
    {
        GetNode<Button>("PainelAcoes/BtnAtacar").TooltipText =
            "1 AP  |  0 Mana\nDano = Atk − Def do alvo  (mín. 1)\nSem custo de Mana";
        GetNode<Button>("PainelAcoes/BtnDefender").TooltipText =
            "1 AP  |  0 Mana\nAplica Shield (absorve dano igual ao seu Def) por 1 turno";
        GetNode<Button>("PainelAcoes/BtnScan").TooltipText =
            "1 AP  |  0 Mana\nRevela HP e família do alvo\nHabilita cartas Null e Expose nele";
        GetNode<Button>("PainelAcoes/BtnGambitoPrever").TooltipText =
            "1 AP  |  0 Mana\nRevela a próxima ação do inimigo\n(alvo, tipo e dano previsto)";
        GetNode<Button>("PainelAcoes/BtnGambitoReordenar").TooltipText =
            "2 AP  |  0 Mana\nEmpurra o inimigo 1 posição para a frente\nna fila de iniciativa";
        GetNode<Button>("PainelAcoes/BtnUsarCarta").TooltipText =
            "1 AP + custo da carta  |  Mana da carta\nAbre a mão de cartas para escolher";
        GetNode<Button>("PainelAcoes/BtnFugir").TooltipText =
            "1 AP  |  0 Mana\nTenta fugir do combate\n(chance calculada pela SPD comparada ao inimigo)";
        GetNode<Button>("PainelAcoes/BtnPassar").TooltipText =
            "0 AP  |  0 Mana\nEncerra o turno imediatamente sem agir";
    }

    private void ConectarBotoes()
    {
        GetNode<Button>("PainelAcoes/BtnAtacar").Pressed += OnBtnAtacar;
        GetNode<Button>("PainelAcoes/BtnDefender").Pressed += OnBtnDefender;
        GetNode<Button>("PainelAcoes/BtnScan").Pressed += OnBtnScan;
        GetNode<Button>("PainelAcoes/BtnGambitoPrever").Pressed += OnBtnGambitoPrever;
        GetNode<Button>("PainelAcoes/BtnGambitoReordenar").Pressed += OnBtnGambitoReordenar;
        GetNode<Button>("PainelAcoes/BtnUsarCarta").Pressed += OnBtnUsarCarta;
        GetNode<Button>("PainelAcoes/BtnFugir").Pressed += OnBtnFugir;
        GetNode<Button>("PainelAcoes/BtnPassar").Pressed += OnBtnPassar;
        GetNode<Button>("OverlayResultado/BtnReiniciar").Pressed += OnBtnReiniciar;
    }

    private static void IniciarCombate()
    {
        var manager = CombatManager.Instance;
        if (manager is null)
        {
            GD.PushError("CombatScene: CombatManager.Instance é null (AutoLoad ausente?).");
            return;
        }

        // Party (combat.md §17). IsUniversalCompiler do Gus já vem propagado pelo template.
        var gus = CharacterRepository.ToActor(CanonicalTemplates.Gus(), isPlayerSide: true);
        var caua = CharacterRepository.ToActor(CanonicalTemplates.Caua(), isPlayerSide: true);
        var jaci = CharacterRepository.ToActor(CanonicalTemplates.Jaci(), isPlayerSide: true);

        // Inimigos do encontro de referência.
        var sentinela = CharacterRepository.ToActor(CanonicalTemplates.SentinelaBit());
        var daemon = CharacterRepository.ToActor(CanonicalTemplates.DaemonGuard());

        var cardRegistry = PlaceholderCards.All();
        var brainRegistry = new Dictionary<string, IEnemyBrain>
        {
            [sentinela.Id] = CharacterRepository.BrainFor(BrainKind.Scripted),
            [daemon.Id] = CharacterRepository.BrainFor(BrainKind.Scripted),
        };

        manager.StartCombat(
            new[] { gus, caua, jaci, sentinela, daemon },
            cardRegistry,
            brainRegistry);
    }

    // -------------------------------------------------------------------------
    // Construção dos painéis de ator (em OnCombatStarted, quando a fila já existe)
    // -------------------------------------------------------------------------

    private void MontarPaineisAtores()
    {
        var manager = CombatManager.Instance;
        if (manager is null) return;

        LimparFilhos(_listaInimigos);
        LimparFilhos(_painelParty);
        _paineis.Clear();

        foreach (var ator in manager.Actors)
        {
            var painel = CriarPainelAtor(ator);
            _paineis[ator.Id] = painel;
            (ator.IsPlayerSide ? _painelParty : _listaInimigos).AddChild(painel.Raiz);
        }
    }

    private static AtorPainel CriarPainelAtor(CombatActor ator)
    {
        var raiz = new VBoxContainer { CustomMinimumSize = new Vector2(140, 0) };

        var nomeTexto = NomeAtor(ator);
        raiz.AddChild(new Label { Text = nomeTexto });

        // Indicador de status (menor, vazio até o primeiro status ativo).
        var labelStatus = new Label { Text = string.Empty };
        labelStatus.AddThemeFontSizeOverride("font_size", 10);
        labelStatus.AddThemeColorOverride("font_color", new Color(1f, 0.85f, 0.2f));
        raiz.AddChild(labelStatus);

        // Barra de HP vermelha.
        var barraHp = new ProgressBar
        {
            MinValue = 0, MaxValue = ator.MaxHp, Value = ator.Hp,
            CustomMinimumSize = new Vector2(0, 14),
            ShowPercentage = false,
        };
        barraHp.AddThemeStyleboxOverride("fill", new StyleBoxFlat { BgColor = new Color(0.80f, 0.22f, 0.22f) });
        raiz.AddChild(barraHp);

        var hpTexto = new Label { Text = TextoHp(ator) };
        raiz.AddChild(hpTexto);

        // Party: barra de Mana azul + label de AP.
        ProgressBar? barraMana = null;
        Label? recursos = null;
        if (ator.IsPlayerSide)
        {
            barraMana = new ProgressBar
            {
                MinValue = 0, MaxValue = ator.MaxMana, Value = ator.Mana,
                CustomMinimumSize = new Vector2(0, 10),
                ShowPercentage = false,
            };
            barraMana.AddThemeStyleboxOverride("fill", new StyleBoxFlat { BgColor = new Color(0.20f, 0.45f, 0.90f) });
            raiz.AddChild(barraMana);

            recursos = new Label { Text = TextoAp(ator) };
            recursos.AddThemeFontSizeOverride("font_size", 10);
            raiz.AddChild(recursos);
        }

        return new AtorPainel(raiz, barraHp, hpTexto, labelStatus, barraMana, recursos);
    }

    private void PopularMaoDeCartas()
    {
        var manager = CombatManager.Instance;
        if (manager is null) return;

        LimparFilhos(_listaCartas);
        foreach (var (id, carta) in manager.ActiveCardRegistry)
        {
            var botao = new Button { Text = NomeCarta(carta), TooltipText = TooltipCarta(carta) };
            var cardId = id; // captura local estável para o lambda
            botao.Pressed += () => OnCartaEscolhida(cardId);
            _listaCartas.AddChild(botao);
        }
    }

    // -------------------------------------------------------------------------
    // Handlers de botões de ação. Ações ofensivas abrem a seleção de alvo manual
    // (F2-G.5 bugfix); o id escolhido alimenta a CombatAction via callback.
    // -------------------------------------------------------------------------

    private void OnBtnAtacar() =>
        IniciarSelecaoDeAlvo(id => Submeter(CombatAction.Attack(id)));

    private void OnBtnDefender() => Submeter(CombatAction.Defend());

    private void OnBtnScan() =>
        IniciarSelecaoDeAlvo(id => Submeter(CombatAction.Scan(id)));

    private void OnBtnGambitoPrever() =>
        IniciarSelecaoDeAlvo(id => Submeter(CombatAction.GambitPredict(id)));

    private void OnBtnGambitoReordenar() =>
        IniciarSelecaoDeAlvo(id => Submeter(CombatAction.GambitReorder(id, delta: 1)));

    private void OnBtnFugir() => Submeter(CombatAction.Flee());

    private void OnBtnPassar() => Submeter(CombatAction.Pass());

    // Abre a mão de cartas; esconde o menu de ações para não sobrepor os botões de carta.
    private void OnBtnUsarCarta()
    {
        _selecionandoCarta = true;
        _painelAcoes.Visible = false;
        _painelMaoCartas.Visible = true;
    }

    private void OnCartaEscolhida(string cardId)
    {
        var manager = CombatManager.Instance;
        if (manager is null) return;

        if (!manager.ActiveCardRegistry.TryGetValue(cardId, out var carta))
        {
            AnexarLog(Tr("COMBAT_LOG_COMPILE_ERROR"));
            return;
        }

        // Self-target (cura/buff) mira o próprio ator do turno (resolve já). Demais cartas
        // abrem a seleção de alvo manual, igual às ações ofensivas (F2-G.5 bugfix).
        if (carta.TargetShape == TargetShape.Self)
        {
            _selecionandoCarta = false;
            _painelMaoCartas.Visible = false;
            Submeter(CombatAction.UseCard(cardId, manager.ActiveActorId!));
        }
        else
        {
            _selecionandoCarta = false;
            _painelMaoCartas.Visible = false;
            IniciarSelecaoDeAlvo(id => Submeter(CombatAction.UseCard(cardId, id)));
        }
    }

    // -------------------------------------------------------------------------
    // Seleção de alvo manual (F2-G.5 bugfix). Ações ofensivas abrem uma lista de
    // inimigos vivos; o clique dispara o callback capturado com o id escolhido.
    // -------------------------------------------------------------------------

    // Abre o seletor de alvo: esconde ações + mão de cartas e lista um botão por inimigo vivo.
    // O callback (ex: id => Submeter(CombatAction.Attack(id))) roda quando o jogador escolhe.
    private void IniciarSelecaoDeAlvo(Action<string> callback)
    {
        var manager = CombatManager.Instance;
        if (manager is null) return;

        // Se só restar um inimigo vivo, dispara o callback direto (sem abrir o painel).
        CombatActor? unicoInimigo = null;
        var totalInimigos = 0;
        foreach (var a in manager.QueueOrder)
        {
            if (!a.IsPlayerSide && a.IsAlive) { unicoInimigo = a; totalInimigos++; }
        }
        if (totalInimigos == 1) { callback(unicoInimigo!.Id); return; }

        _selecionandoAlvo = true;
        _painelAcoes.Visible = false;
        _painelMaoCartas.Visible = false;

        // Construção tardia: o painel só nasce na primeira seleção e é reusado depois.
        if (_painelAlvo is null)
        {
            _painelAlvo = new VBoxContainer();
            _painelAlvo.AddChild(new Label { Text = Tr("COMBAT_SELECT_TARGET") });
            AddChild(_painelAlvo);
        }

        // Mantém só o Label de cabeçalho (índice 0); remove botões da abertura anterior.
        var filhos = _painelAlvo.GetChildren();
        for (var i = filhos.Count - 1; i >= 1; i--)
            filhos[i].QueueFree();

        foreach (var ator in manager.QueueOrder)
        {
            if (ator.IsPlayerSide || !ator.IsAlive) continue;

            var botao = new Button { Text = NomeAtor(ator) };
            var id = ator.Id; // captura local estável para o lambda
            botao.Pressed += () => OnAlvoEscolhido(id);
            _painelAlvo.AddChild(botao);
        }

        _painelAlvo.Visible = true;
        _callbackAlvo = callback;
    }

    // Fecha o seletor e dispara o callback com o alvo escolhido (uma submissão por seleção).
    private void OnAlvoEscolhido(string alvoId)
    {
        _selecionandoAlvo = false;
        if (_painelAlvo is not null) _painelAlvo.Visible = false;
        _callbackAlvo?.Invoke(alvoId);
        _callbackAlvo = null;
    }

    private void OnBtnReiniciar() => GetTree().ReloadCurrentScene();

    // Submete a ação e recolhe o menu; ele reabre no próximo TurnStarted de party.
    private void Submeter(CombatAction acao)
    {
        var manager = CombatManager.Instance;
        if (manager is null || !manager.WaitingForPlayerAction) return;

        _painelAcoes.Visible = false;
        _painelMaoCartas.Visible = false;
        manager.SubmitPlayerAction(acao);
    }

    // -------------------------------------------------------------------------
    // Reação aos signals do CombatBus
    // -------------------------------------------------------------------------

    private void OnCombatStarted(Godot.Collections.Array combatants)
    {
        MontarPaineisAtores();
        AtualizarBarraIniciativa();
        AtualizarVisibilidadeAcoes();
    }

    private void OnTurnStarted(string combatantName, int roundIndex)
    {
        AtualizarBarraIniciativa();
        RefreshActor(combatantName); // Mana reseta/rampa a cada turno (§5); atualiza a barra.

        var rotuloLado = EhDaParty(combatantName)
            ? Tr("COMBAT_PLAYER_TURN")
            : Tr("COMBAT_ENEMY_TURN");
        AnexarLog(Tr("COMBAT_TURN_ROUND", roundIndex + 1, rotuloLado));

        AtualizarVisibilidadeAcoes();
    }

    private void OnTurnEnded(string combatantName) => AtualizarVisibilidadeAcoes();

    private void OnActionResolved(string actor, string target, string actionType, int value)
    {
        RefreshActor(actor);
        if (!string.IsNullOrEmpty(target)) RefreshActor(target);

        // UseCard resolvido com efeito é reportado como "compilação" bem-sucedida (linguagem
        // de jogo, Pillar 1: magia = software). Demais ações usam o log genérico.
        if (actionType == CombatActionType.UseCard.ToString())
            AnexarLog(Tr("COMBAT_LOG_COMPILED", NomeAtorPorId(actor)));

        if (string.IsNullOrEmpty(target))
            AnexarLog(Tr("COMBAT_LOG_ACTION_NOTARGET", NomeAtorPorId(actor), actionType));
        else
            AnexarLog(Tr("COMBAT_LOG_ACTION", NomeAtorPorId(actor), actionType, NomeAtorPorId(target), value));
    }

    private void OnStatusApplied(string actorId, string statusId, int magnitude, int duration)
    {
        RefreshActor(actorId);
        AnexarLog(Tr("COMBAT_LOG_STATUS_APPLIED", NomeAtorPorId(actorId), statusId, magnitude, duration));
    }

    private void OnStatusExpired(string actorId, string statusId)
    {
        RefreshActor(actorId);
        AnexarLog(Tr("COMBAT_LOG_STATUS_EXPIRED", NomeAtorPorId(actorId), statusId));
    }

    private void OnActorDefeated(string actorId)
    {
        RefreshActor(actorId);
        AnexarLog(Tr("COMBAT_LOG_DEFEATED", NomeAtorPorId(actorId)));
    }

    private void OnActorIncapacitated(string actorId)
    {
        RefreshActor(actorId);
        AnexarLog(Tr("COMBAT_LOG_INCAPACITATED", NomeAtorPorId(actorId)));
    }

    private void OnCombatEnded(string winner)
    {
        _painelAcoes.Visible = false;
        _painelMaoCartas.Visible = false;

        _labelResultado.Text = TextoResultado(winner);
        _overlayResultado.Visible = true;
        AnexarLog(TextoResultado(winner));
    }

    // -------------------------------------------------------------------------
    // Atualização de painéis
    // -------------------------------------------------------------------------

    private void RefreshActor(string actorId)
    {
        var manager = CombatManager.Instance;
        if (manager is null) return;
        if (!_paineis.TryGetValue(actorId, out var painel)) return;

        var ator = manager.GetActor(actorId);
        if (ator is null) return;

        painel.BarraHp.Value = ator.Hp;
        painel.HpTexto.Text = TextoHp(ator);
        AtualizarLabelStatus(painel.LabelStatus, ator);

        if (painel.BarraMana is not null)
        {
            painel.BarraMana.MaxValue = ator.MaxMana;
            painel.BarraMana.Value = ator.Mana;
        }
        if (painel.Recursos is not null)
            painel.Recursos.Text = TextoAp(ator);
    }

    private static void AtualizarLabelStatus(Label label, CombatActor ator)
    {
        if (ator.StatusEffects.Count == 0) { label.Text = string.Empty; return; }

        var partes = new System.Collections.Generic.List<string>(ator.StatusEffects.Count);
        foreach (var s in ator.StatusEffects)
            partes.Add(s.Id.ToString().ToUpperInvariant());
        label.Text = string.Join("  ", partes);
    }

    private void AtualizarBarraIniciativa()
    {
        var manager = CombatManager.Instance;
        if (manager is null) return;

        LimparFilhos(_barraIniciativa);
        var ativo = manager.ActiveActorId;
        foreach (var ator in manager.QueueOrder)
        {
            var marcador = ator.Id == ativo ? "> " : string.Empty;
            _barraIniciativa.AddChild(new Label { Text = $"{marcador}{NomeAtor(ator)}" });
        }
    }

    private void AtualizarVisibilidadeAcoes()
    {
        var manager = CombatManager.Instance;
        var ehTurnoDaParty = manager is not null && manager.WaitingForPlayerAction;

        // Menu de ações: visível só quando é turno da party E não está selecionando carta/alvo.
        _painelAcoes.Visible = ehTurnoDaParty && !_selecionandoCarta && !_selecionandoAlvo;

        // Quando o turno da party termina, fecha tudo e reseta os modos de carta/alvo.
        if (!ehTurnoDaParty)
        {
            _selecionandoCarta = false;
            _selecionandoAlvo = false;
            _callbackAlvo = null;
            _painelMaoCartas.Visible = false;
            if (_painelAlvo is not null) _painelAlvo.Visible = false;
        }
    }

    // -------------------------------------------------------------------------
    // Log de feedback
    // -------------------------------------------------------------------------

    private void AnexarLog(string mensagem)
    {
        // AddText (não AppendText): trata a mensagem como texto literal, sem parse de BBCode.
        // As chaves de log usam colchetes ([ator]); com AppendText eles seriam lidos como tags.
        _logFeedback.AddText(mensagem + "\n");

        // Apara o topo se passar do teto de linhas (mantém a sessão enxuta).
        var excedente = _logFeedback.GetLineCount() - MaxLinhasLog;
        if (excedente > 0)
            _logFeedback.RemoveParagraph(0);

        _logFeedback.ScrollToLine(_logFeedback.GetLineCount());
    }

    // -------------------------------------------------------------------------
    // Helpers de alvo / consulta
    // -------------------------------------------------------------------------

    private static bool EhDaParty(string actorId)
    {
        var ator = CombatManager.Instance?.GetActor(actorId);
        return ator is not null && ator.IsPlayerSide;
    }

    private static string NomeAtorPorId(string actorId)
    {
        var ator = CombatManager.Instance?.GetActor(actorId);
        return ator is null ? actorId : NomeAtor(ator);
    }

    // -------------------------------------------------------------------------
    // Texto i18n (wrapper sobre o AutoLoad Localization; defensivo se ausente)
    // -------------------------------------------------------------------------

    // DisplayName do ator é o id (combat.md §16); a chave de tradução do nome diegético é
    // "ACTOR_<ID_UPPER>". Sem essa chave, TrMd devolve a própria chave; no slice cai no id
    // legível, que é suficiente como placeholder.
    private static string NomeAtor(CombatActor ator) =>
        TrEstatico($"ACTOR_{ator.Id.ToUpperInvariant()}", ator.DisplayName);

    private static string NomeCarta(Card carta) =>
        TrEstatico(carta.DisplayName, carta.Id);

    private static string TooltipCarta(Card carta)
    {
        var partes = new System.Collections.Generic.List<string>
        {
            $"Custo: {carta.ApCost} AP  |  {carta.ManaCost} Mana",
        };
        if (carta.Power > 0)
            partes.Add($"Dano: {carta.Power}");
        if (carta.StatusApplied is { } st)
            partes.Add($"Aplica: {st.Id}  (mag {st.Magnitude}, {st.Duration} turno{(st.Duration > 1 ? "s" : "")})");
        partes.Add($"Alvo: {(carta.TargetShape == TargetShape.Self ? "próprio personagem" : "inimigo único")}");
        return string.Join("\n", partes);
    }

    private static string TextoHp(CombatActor ator) => $"{ator.Hp}/{ator.MaxHp}";

    private static string TextoAp(CombatActor ator) =>
        TrEstatico("COMBAT_ACTOR_AP", $"AP {ator.Ap}/{ator.MaxAp}", ator.Ap, ator.MaxAp);

    private static string TextoResultado(string outcome) => outcome switch
    {
        nameof(CombatOutcome.Victory) => TrEstatico("COMBAT_VICTORY", "Vitoria."),
        nameof(CombatOutcome.Defeat) => TrEstatico("COMBAT_DEFEAT", "Derrota."),
        nameof(CombatOutcome.Fled) => TrEstatico("COMBAT_FLEE_SUCCESS", "Fuga."),
        _ => outcome,
    };

    private static string Tr(string key, params Variant[] args) => TrComFallback(key, key, args);

    // Variante usada em contexto estático onde queremos um fallback explícito diferente da key
    // (ex: nome do ator cai no id legível, não na chave crua) caso o Localization não resolva.
    private static string TrEstatico(string key, string fallback, params Variant[] args) =>
        TrComFallback(key, fallback, args);

    private static string TrComFallback(string key, string fallback, Variant[] args)
    {
        var loc = Localization.Localization.Instance;
        if (loc is null) return fallback;

        // HasKey antes de TrMd: chaves intencionalmente ausentes (ex: ACTOR_<ID> de nome
        // diegético, fora do escopo do slice) caem no fallback amigável SEM disparar o
        // GD.PushWarning ruidoso que o TrMd emite ao não resolver. pt_br é o locale primário
        // e MUST estar completo, então HasKey no locale corrente é suficiente para as
        // chaves de UI reais (COMBAT_*).
        if (!loc.HasKey(key)) return fallback;

        var resolvido = args.Length > 0 ? loc.TrMd(key, args) : loc.TrMd(key);
        return resolvido == key ? fallback : resolvido;
    }

    // -------------------------------------------------------------------------
    // Signals: conectar/desconectar (simétrico, evita listeners órfãos em reload)
    // -------------------------------------------------------------------------

    private void ConectarSignals()
    {
        var bus = CombatBus.Instance;
        if (bus is null)
        {
            GD.PushError("CombatScene: CombatBus.Instance é null (AutoLoad ausente?).");
            return;
        }

        bus.CombatStarted += OnCombatStarted;
        bus.TurnStarted += OnTurnStarted;
        bus.TurnEnded += OnTurnEnded;
        bus.ActionResolved += OnActionResolved;
        bus.StatusApplied += OnStatusApplied;
        bus.StatusExpired += OnStatusExpired;
        bus.ActorDefeated += OnActorDefeated;
        bus.ActorIncapacitated += OnActorIncapacitated;
        bus.CombatEnded += OnCombatEnded;
    }

    private void DesconectarSignals()
    {
        var bus = CombatBus.Instance;
        if (bus is null) return;

        bus.CombatStarted -= OnCombatStarted;
        bus.TurnStarted -= OnTurnStarted;
        bus.TurnEnded -= OnTurnEnded;
        bus.ActionResolved -= OnActionResolved;
        bus.StatusApplied -= OnStatusApplied;
        bus.StatusExpired -= OnStatusExpired;
        bus.ActorDefeated -= OnActorDefeated;
        bus.ActorIncapacitated -= OnActorIncapacitated;
        bus.CombatEnded -= OnCombatEnded;
    }

    private static void LimparFilhos(Node container)
    {
        foreach (var filho in container.GetChildren())
            filho.QueueFree();
    }

    /// <summary>
    /// Refs de UI de um ator agrupadas (criadas dinamicamente).
    /// BarraMana e Recursos são null para inimigos (sem HUD de recursos no slice).
    /// </summary>
    private sealed record AtorPainel(
        VBoxContainer Raiz,
        ProgressBar BarraHp,
        Label HpTexto,
        Label LabelStatus,
        ProgressBar? BarraMana,
        Label? Recursos);
}
