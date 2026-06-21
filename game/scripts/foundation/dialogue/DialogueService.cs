// DialogueService.cs
//
// Servico de ORQUESTRACAO de dialogo (F2-E.6, fundacao). Camada fina sobre o plugin
// DialogueManager (v3.10.4, MIT, ADR-003). NAO desenha balao/UI: traversa o grafo de
// dialogo, emite os signals de UIBus e faz a ponte de FLAGS com o estado de mundo.
//
// Escopo desta task (F2-E.6):
//   - Iniciar um dialogo (.dialogue + titulo de no de entrada).
//   - Emitir UIBus.DialogueShown(dialogueId) ao iniciar.
//   - Emitir UIBus.DialogueChoiceMade(dialogueId, choiceIndex) quando o player escolhe.
//   - Espelhar SaveDataV1.Flags num state-provider que o .dialogue le (gates por flag).
//   - Capturar mutations (set npc_intro.* = ...) de volta para o store de flags.
//   FORA de escopo (sprint seguinte): o balao de fala visual, o display inline do texto,
//   a renderizacao das responses. Aqui so a maquina de estado de quem-fala-o-que.
//
// API canonica (architecture engine-modules.md §3.5): StartDialogue / Continue / Choose /
// GetVariable / SetVariable + singleton Instance. O modulo e Godot-acoplado de proposito
// (ADR-003): vive em game/scripts/foundation/dialogue/, NAO em engine/ (que e POCO puro).
//
// PONTE COM PERSISTENCIA (lacuna nomeada): nao existe hoje um holder de world-state vivo
// em runtime — SaveDataV1 e record imutavel (init-only) e SaveManager so faz I/O. Por isso
// este servico mantem o store de flags EM MEMORIA (FlagStore) e expoe LoadFlagsFromSave /
// WriteFlagsToSave como o bridge a ser ligado quando o wiring de save da sessao existir.
// NAO fabricamos SaveManager.CurrentData (seria arquitetura nao-aprovada).
//
// AOT (ADR-002): o wrapper do plugin usa reflection (resolve mutations/metodos C# por
// Assembly.GetTypes). Isto e RISCO sob NativeAOT em release e precisa ser validado antes de
// depender de mutations C#-side. Esta fundacao roda em Debug/JIT; ver decisoes abertas.
//
// Cross-ref: docs/tech/adr/ADR-003-dialogue-library.md;
//            docs/design/narrativa/dialogue-tree-npc-intro.md (§4 contrato de flags);
//            docs/tech/architecture.md §4.4; docs/tech/engine-modules.md §3.5;
//            game/scripts/foundation/buses/UIBus.cs; engine SaveDataV1.Flags.

using Godot;
using DialogueManagerRuntime;
using GusDragon.Engine.Foundation.SaveSystem;
using GusWorld.Game.Foundation.Buses;

namespace GusWorld.Game.Foundation.Dialogue;

/// <summary>
/// Orquestrador de dialogo sobre o plugin DialogueManager. Singleton acessivel via
/// <see cref="Instance"/>. Instanciado sob demanda no slice (AutoLoad no futuro).
/// </summary>
public partial class DialogueService : Node
{
    /// <summary>Instancia ativa (set em _Ready, limpa em _ExitTree).</summary>
    public static DialogueService? Instance { get; private set; }

    /// <summary>
    /// Nome do AutoLoad singleton que o plugin registra (plugin.gd: add_autoload_singleton
    /// "DialogueManager"). Usado para checar presenca defensiva antes de chamar a lib.
    /// </summary>
    private const string DialogueManagerAutoload = "DialogueManager";

    /// <summary>
    /// Chave do objeto de estado injetado em extra_game_states. O .dialogue referencia as
    /// flags como "npc_intro.met", "npc_intro.choice", etc.; o DialogueManager resolve
    /// "npc_intro" para este objeto e le suas propriedades [Export].
    /// </summary>
    private const string StateKey = "npc_intro";

    /// <summary>
    /// Store de flags do dialogo EM MEMORIA (a fonte de verdade da sessao enquanto nao ha
    /// holder de world-state ligado ao SaveSystem). Espelhado em <see cref="_state"/> para o
    /// plugin ler. Chave = nome simples da flag (ex.: "met"); valor = bool.
    /// </summary>
    private readonly Dictionary<string, bool> _flagStore = new();

    /// <summary>
    /// Objeto de estado que o plugin enxerga (propriedades [Export]). Reflete o
    /// <see cref="_flagStore"/> + a ultima escolha. Recriado/atualizado a cada StartDialogue.
    /// </summary>
    private DialogueState? _state;

    /// <summary>Dicionario Godot passado como extra_game_states ({ "npc_intro" = _state }).</summary>
    private Godot.Collections.Array<Variant> _extraGameStates = new();

    /// <summary>Id (caminho res://) do dialogo atualmente em execucao. Vazio = nenhum.</summary>
    private string _currentDialogueId = string.Empty;

    /// <summary>Recurso .dialogue carregado do dialogo atual.</summary>
    private Resource? _currentResource;

    /// <summary>Linha de dialogo atual (do plugin). Null = nenhuma / dialogo encerrado.</summary>
    private DialogueLine? _currentLine;

    /// <summary>True enquanto um dialogo esta em andamento (entre StartDialogue e o END).</summary>
    public bool IsRunning => _currentLine != null;

    /// <summary>Id do dialogo em execucao (caminho res://), ou string vazia se nenhum.</summary>
    public string CurrentDialogueId => _currentDialogueId;

    // Handler do signal estatico Mutated do plugin (capturado para refletir 'set' no store).
    // Guardado em campo para poder desinscrever em _ExitTree (evita leak entre cenas).
    private DialogueManager.MutatedEventHandler? _mutatedHandler;

    public override void _Ready()
    {
        Instance = this;

        // Inscreve no signal estatico de mutation do plugin: toda vez que o .dialogue roda
        // um 'set obj.prop = x', refletimos no store de flags (fonte de verdade da sessao).
        _mutatedHandler = OnDialogueMutated;
        DialogueManager.Mutated += _mutatedHandler;
    }

    public override void _ExitTree()
    {
        if (_mutatedHandler != null)
        {
            DialogueManager.Mutated -= _mutatedHandler;
            _mutatedHandler = null;
        }

        if (Instance == this) Instance = null;
    }

    /// <summary>
    /// Inicia um dialogo a partir de um arquivo .dialogue e um titulo de no de entrada.
    /// Emite UIBus.DialogueShown(dialogueId). Traversa ate a primeira linha printavel
    /// (rodando gates/mutations no caminho). Defensivo: se o plugin nao estiver presente ou
    /// o recurso nao carregar, loga e aborta sem crash.
    /// </summary>
    /// <param name="dialogueResourcePath">Caminho res:// do .dialogue (ex.: "res://dialogues/npc_intro_bertoldo.dialogue").</param>
    /// <param name="entryTitle">Titulo do no de entrada (default "start").</param>
    public async void StartDialogue(string dialogueResourcePath, string entryTitle = "start")
    {
        if (string.IsNullOrWhiteSpace(dialogueResourcePath))
        {
            GD.PushError("DialogueService.StartDialogue: dialogueResourcePath vazio.");
            return;
        }

        if (!EnsureDialogueManagerAvailable())
            return;

        if (IsRunning)
        {
            // Reentrancia: nao empilhar dialogos. Loga e ignora (a UI nao deveria permitir).
            GD.PushWarning(
                $"DialogueService.StartDialogue: dialogo '{_currentDialogueId}' ja em " +
                $"andamento; ignorando pedido para '{dialogueResourcePath}'.");
            return;
        }

        var resource = GD.Load<Resource>(dialogueResourcePath);
        if (resource == null)
        {
            GD.PushError($"DialogueService.StartDialogue: falha carregar '{dialogueResourcePath}'.");
            return;
        }

        _currentDialogueId = dialogueResourcePath;
        _currentResource = resource;

        // (Re)constroi o objeto de estado que o plugin le e o array extra_game_states.
        RebuildStateProvider();

        // Sinaliza ABERTURA antes de traversar (a UI/balao assina isto no sprint seguinte).
        UIBus.Instance?.EmitSignal(UIBus.SignalName.DialogueShown, _currentDialogueId);

        try
        {
            _currentLine = await DialogueManager.GetNextDialogueLine(
                _currentResource, entryTitle, _extraGameStates);
        }
        catch (Exception ex)
        {
            GD.PushError(
                $"DialogueService.StartDialogue: erro ao traversar '{dialogueResourcePath}' " +
                $"a partir de '{entryTitle}': {ex.Message}");
            ResetConversation();
            return;
        }

        if (_currentLine == null)
        {
            // Dialogo terminou imediatamente (ex.: so gates + END). Estado ja foi mutado.
            ResetConversation();
        }
    }

    /// <summary>
    /// Avanca para a proxima linha (continuacao linear, sem escolha). No-op se nao houver
    /// dialogo em andamento ou se a linha atual exigir escolha (use <see cref="Choose"/>).
    /// </summary>
    public async void Continue()
    {
        if (!IsRunning || _currentResource == null || _currentLine == null)
            return;

        if (_currentLine.Responses.Count > 0)
        {
            // Linha atual e um ponto de escolha: avancar linearmente seria ambiguo.
            GD.PushWarning(
                "DialogueService.Continue: linha atual exige escolha; use Choose(index).");
            return;
        }

        await AdvanceTo(_currentLine.NextId);
    }

    /// <summary>
    /// Escolhe uma das responses da linha atual pelo indice e avanca por aquele ramo.
    /// Emite UIBus.DialogueChoiceMade(dialogueId, choiceIndex). Defensivo contra indice fora
    /// de faixa e contra chamada sem escolha disponivel.
    /// </summary>
    /// <param name="choiceIndex">Indice 0-based na lista de responses da linha atual.</param>
    public async void Choose(int choiceIndex)
    {
        if (!IsRunning || _currentResource == null || _currentLine == null)
        {
            GD.PushWarning("DialogueService.Choose: nenhum dialogo/linha em andamento.");
            return;
        }

        var responses = _currentLine.Responses;
        if (responses.Count == 0)
        {
            GD.PushWarning("DialogueService.Choose: linha atual nao tem responses.");
            return;
        }

        if (choiceIndex < 0 || choiceIndex >= responses.Count)
        {
            GD.PushError(
                $"DialogueService.Choose: indice {choiceIndex} fora de faixa " +
                $"(0..{responses.Count - 1}).");
            return;
        }

        // Sinaliza a escolha ANTES de avancar (telemetria/UI; blueprint §4 DA-4).
        UIBus.Instance?.EmitSignal(
            UIBus.SignalName.DialogueChoiceMade, _currentDialogueId, choiceIndex);

        var chosen = responses[choiceIndex];
        await AdvanceTo(chosen.NextId);
    }

    /// <summary>
    /// Le uma variavel de flag do store da sessao (bridge GetVariable da §3.5). Chave =
    /// nome simples da flag (ex.: "met"). Retorna false se ausente.
    /// </summary>
    public bool GetVariable(string name) =>
        !string.IsNullOrEmpty(name) && _flagStore.TryGetValue(name, out var v) && v;

    /// <summary>
    /// Escreve uma variavel de flag no store da sessao (bridge SetVariable da §3.5) e
    /// reflete no state-provider para o proximo gate do dialogo enxergar. NAO persiste em
    /// disco — a persistencia e via <see cref="WriteFlagsToSave"/> + SaveManager.
    /// </summary>
    public void SetVariable(string name, bool value)
    {
        if (string.IsNullOrEmpty(name))
            return;

        _flagStore[name] = value;
        _state?.SetFlag(name, value);
    }

    /// <summary>
    /// Carrega as flags relevantes de um SaveDataV1 para o store da sessao. Bridge de
    /// LEITURA da persistencia: chamar ao entrar numa cena/carregar save, antes de iniciar
    /// dialogos que dependam de estado de progresso (combat/puzzle/lore cleared). Le apenas
    /// chaves conhecidas do contrato (blueprint §4) para nao poluir o store.
    /// </summary>
    public void LoadFlagsFromSave(SaveDataV1 save)
    {
        ArgumentNullException.ThrowIfNull(save);

        foreach (var key in DialogueFlagKeys.All)
        {
            if (save.Flags.TryGetValue(SaveFlagName(key), out var value))
                _flagStore[key] = value;
        }

        _state?.SyncFrom(_flagStore);
    }

    /// <summary>
    /// Aplica o store de flags da sessao sobre um dicionario de flags do save (mutacao por
    /// copia, sem efeito colateral no SaveDataV1 imutavel). Bridge de ESCRITA: o chamador
    /// monta o proximo SaveDataV1 com este dicionario e entrega ao SaveManager. So escreve
    /// as flags que ESTE dialogo possui (met/choice-derivadas), respeitando o contrato §4
    /// (o dialogo nao escreve flags de combate/puzzle/lore).
    /// </summary>
    /// <param name="existingFlags">Snapshot atual de SaveDataV1.Flags (nao e mutado).</param>
    /// <returns>Novo dicionario com as flags do dialogo aplicadas.</returns>
    public Dictionary<string, bool> WriteFlagsToSave(IReadOnlyDictionary<string, bool> existingFlags)
    {
        ArgumentNullException.ThrowIfNull(existingFlags);

        var merged = new Dictionary<string, bool>(existingFlags);
        foreach (var key in DialogueFlagKeys.Writable)
        {
            if (_flagStore.TryGetValue(key, out var value))
                merged[SaveFlagName(key)] = value;
        }

        return merged;
    }

    /// <summary>
    /// Ultima escolha registrada no dialogo introdutorio ("curioso"/"pragmatico"/"seco"),
    /// ou string vazia se ainda nao escolhida. Capturada via mutation 'set npc_intro.choice'.
    /// Reservado para callback narrativo pos-VS (blueprint DA-4).
    /// </summary>
    public string LastChoice => _state?.Choice ?? string.Empty;

    // --- internals ---

    private async Task AdvanceTo(string nextId)
    {
        if (_currentResource == null)
            return;

        try
        {
            _currentLine = await DialogueManager.GetNextDialogueLine(
                _currentResource, nextId, _extraGameStates);
        }
        catch (Exception ex)
        {
            GD.PushError($"DialogueService.AdvanceTo('{nextId}'): {ex.Message}");
            ResetConversation();
            return;
        }

        if (_currentLine == null)
            ResetConversation();
    }

    private void RebuildStateProvider()
    {
        _state = new DialogueState();
        _state.SyncFrom(_flagStore);

        _extraGameStates = new Godot.Collections.Array<Variant>
        {
            Variant.From(new Godot.Collections.Dictionary { { StateKey, _state } }),
        };
    }

    // Reflete um 'set obj.prop = x' do .dialogue de volta no store de flags. O plugin entrega
    // o dicionario de mutation; capturamos as chaves do nosso objeto de estado (StateKey).
    private void OnDialogueMutated(Godot.Collections.Dictionary mutation)
    {
        if (_state == null)
            return;

        // O store e a copia canonica da sessao; o _state ja foi mutado pelo plugin (e o
        // objeto que ele escreveu). Reespelhamos _state -> store para manter coerencia.
        _flagStore["met"] = _state.Met;
        // 'choice' nao e bool; vive em _state.Choice (lido via LastChoice). Nada a fazer aqui
        // alem de manter o store de bools coerente. 'mutation' fica disponivel para telemetria.
        _ = mutation;
    }

    private void ResetConversation()
    {
        _currentLine = null;
        _currentResource = null;
        _currentDialogueId = string.Empty;
    }

    private static bool EnsureDialogueManagerAvailable()
    {
        if (Engine.HasSingleton(DialogueManagerAutoload))
            return true;

        GD.PushError(
            $"DialogueService: AutoLoad '{DialogueManagerAutoload}' ausente. O plugin " +
            "DialogueManager precisa estar habilitado (project.godot [editor_plugins] + " +
            "[autoload]). Abra o editor uma vez ou confirme o registro do AutoLoad.");
        return false;
    }

    // Nome da flag como persiste em SaveDataV1.Flags. Prefixo de namespace para nao colidir
    // com outras flags de mundo. Mantido em um lugar so (espelha o blueprint §4).
    private static string SaveFlagName(string key) => $"npc_intro.{key}";
}

/// <summary>
/// Chaves de flag do dialogo introdutorio (espelha o contrato do blueprint §4). Centraliza o
/// vocabulario para o store, o save-bridge e o state-provider nao divergirem.
/// </summary>
internal static class DialogueFlagKeys
{
    /// <summary>First-visit done (escrita pelo dialogo).</summary>
    public const string Met = "met";

    /// <summary>Sentinela derrotado no VS (LEITURA; escrita pelo FSM de combate).</summary>
    public const string CombatSentinelaCleared = "combat_sentinela_cleared";

    /// <summary>Puzzle da patrulha resolvido (LEITURA; escrita pelo sistema de puzzle).</summary>
    public const string PuzzlePatrolCleared = "puzzle_patrol_cleared";

    /// <summary>1o no de lore decifrado (LEITURA; escrita pelo sistema de lore).</summary>
    public const string LoreNode1Deciphered = "lore_node_1_deciphered";

    /// <summary>Todas as flags bool do contrato (para LoadFlagsFromSave).</summary>
    public static readonly string[] All =
    {
        Met, CombatSentinelaCleared, PuzzlePatrolCleared, LoreNode1Deciphered,
    };

    /// <summary>Flags que ESTE dialogo escreve (contrato §4: so 'met' entre as bool).</summary>
    public static readonly string[] Writable = { Met };
}

/// <summary>
/// State-provider que o DialogueManager le. As propriedades [Export] sao visiveis ao plugin
/// (CSharp.md "State": precisa de [Export]). O .dialogue referencia "npc_intro.met",
/// "npc_intro.choice", etc.; o plugin resolve este objeto e suas propriedades. Escreve via
/// 'set npc_intro.prop = x'. Espelha o store de bools do <see cref="DialogueService"/>.
/// </summary>
public partial class DialogueState : GodotObject
{
    /// <summary>First-visit done. Gate first-visit vs revisit (blueprint §3).</summary>
    [Export] public bool Met { get; set; }

    /// <summary>Escolha do first-visit ("curioso"/"pragmatico"/"seco"). Reservado (DA-4).</summary>
    [Export] public string Choice { get; set; } = string.Empty;

    /// <summary>Sentinela derrotado (LEITURA, revisit dispatch). Escrita pelo FSM de combate.</summary>
    [Export] public bool CombatSentinelaCleared { get; set; }

    /// <summary>Puzzle resolvido (LEITURA, revisit dispatch). Escrita pelo sistema de puzzle.</summary>
    [Export] public bool PuzzlePatrolCleared { get; set; }

    /// <summary>1o no de lore decifrado (LEITURA). Escrita pelo sistema de lore.</summary>
    [Export] public bool LoreNode1Deciphered { get; set; }

    /// <summary>Sincroniza as propriedades bool a partir do store da sessao.</summary>
    public void SyncFrom(IReadOnlyDictionary<string, bool> store)
    {
        ArgumentNullException.ThrowIfNull(store);
        Met = store.TryGetValue(DialogueFlagKeys.Met, out var m) && m;
        CombatSentinelaCleared =
            store.TryGetValue(DialogueFlagKeys.CombatSentinelaCleared, out var c) && c;
        PuzzlePatrolCleared =
            store.TryGetValue(DialogueFlagKeys.PuzzlePatrolCleared, out var p) && p;
        LoreNode1Deciphered =
            store.TryGetValue(DialogueFlagKeys.LoreNode1Deciphered, out var l) && l;
    }

    /// <summary>Seta uma flag bool por nome simples (espelho de SetVariable).</summary>
    public void SetFlag(string key, bool value)
    {
        switch (key)
        {
            case DialogueFlagKeys.Met: Met = value; break;
            case DialogueFlagKeys.CombatSentinelaCleared: CombatSentinelaCleared = value; break;
            case DialogueFlagKeys.PuzzlePatrolCleared: PuzzlePatrolCleared = value; break;
            case DialogueFlagKeys.LoreNode1Deciphered: LoreNode1Deciphered = value; break;
            default: break;
        }
    }
}
