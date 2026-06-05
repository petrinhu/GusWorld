// SaveManager.cs
//
// AutoLoad global pra save/load orchestration.
//
// Estrutura disco (user://saves/):
//   slot_autosave.json        (autosave canon)
//   slot_autosave.backup1.json
//   slot_autosave.backup2.json
//   slot_autosave.backup3.json
//   slot_1.json               (manual slot 1)
//   slot_1.backup1.json
//   ... (backup chain N=3 por slot)
//   slot_4.json
//   meta.json                 (meta-save: hard mode flag, total playtime, config)
//
// API public:
//   - Task SaveAsync(int slot, SaveDataV1 data)
//   - Task<SaveDataV1?> LoadAsync(int slot)
//   - Task<SaveDataV1?> LoadBackupAsync(int slot, int backupIndex)
//   - bool HasSave(int slot)
//   - SaveSlotInfo[] ListSlots()
//
// Cross-ref: docs/tech/architecture.md §4.6 + ADR-002 batch 5.

using Godot;
using GusDragon.Engine.Foundation.SaveSystem;
using GusWorld.Game.Foundation.Buses;

namespace GusWorld.Game.Foundation.SaveSystem;

/// <summary>
/// Manager AutoLoad de save/load. Async API. Signals próprios + GameStateBus relay.
/// </summary>
public partial class SaveManager : Node
{
    public static SaveManager? Instance { get; private set; }

    /// <summary>Slot canonico autosave (índice 0 reservado).</summary>
    public const int AutosaveSlot = 0;

    /// <summary>Quantidade slots manuais (canon: 4).</summary>
    public const int ManualSlotsCount = 4;

    /// <summary>Backup chain depth N=3.</summary>
    public const int BackupChainDepth = 3;

    /// <summary>Directory base saves.</summary>
    private const string SavesDir = "user://saves";

    // Signals próprios + GameStateBus relay (ADR-002 batch 5 canon).
    [Signal] public delegate void SaveStartedEventHandler(int slot);
    [Signal] public delegate void SaveCompletedEventHandler(int slot);
    [Signal] public delegate void SaveFailedEventHandler(int slot, string errorMessage);
    [Signal] public delegate void LoadStartedEventHandler(int slot);
    [Signal] public delegate void LoadCompletedEventHandler(int slot);
    [Signal] public delegate void LoadIntegrityFailedEventHandler(int slot);
    [Signal] public delegate void LoadCorruptedEventHandler(int slot);
    // Save de versão FUTURA (forward-only, CONTRACT §7): jogo mais antigo não lê save mais novo.
    // UI deve orientar atualizar o jogo. saveVersion = versão do save; currentVersion = suportada.
    [Signal] public delegate void LoadVersionTooNewEventHandler(int slot, int saveVersion, int currentVersion);

    public override void _Ready()
    {
        Instance = this;
        EnsureSavesDirectoryExists();
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }

    private static void EnsureSavesDirectoryExists()
    {
        if (!DirAccess.DirExistsAbsolute(SavesDir))
        {
            var err = DirAccess.MakeDirRecursiveAbsolute(SavesDir);
            if (err != Error.Ok)
                GD.PushError($"SaveManager: falha criar {SavesDir}: {err}");
        }
    }

    /// <summary>
    /// Save async em slot (0=autosave, 1-4=manuais). Rotaciona backup chain N=3.
    /// </summary>
    public async Task SaveAsync(int slot, SaveDataV1 data)
    {
        ValidateSlot(slot);
        EmitSignal(SignalName.SaveStarted, slot);

        try
        {
            var primaryPath = GetSlotPath(slot);

            // Rotaciona backups: backup2 → backup3, backup1 → backup2, primary → backup1
            await Task.Run(() => RotateBackups(slot));

            // Serializa + HMAC + write
            var json = await Task.Run(() => SaveSerializer.SerializeWithHmac(data));
            await Task.Run(() =>
            {
                using var file = Godot.FileAccess.Open(primaryPath, Godot.FileAccess.ModeFlags.Write);
                if (file == null)
                    throw new IOException($"Falha abrir {primaryPath} para write: {Godot.FileAccess.GetOpenError()}");
                file.StoreString(json);
            });

            EmitSignal(SignalName.SaveCompleted, slot);
            GameStateBus.Instance?.EmitSignal(GameStateBus.SignalName.GameSaved, slot);
        }
        catch (Exception ex)
        {
            GD.PushError($"SaveManager.SaveAsync slot {slot} failed: {ex.Message}");
            EmitSignal(SignalName.SaveFailed, slot, ex.Message);
        }
    }

    /// <summary>
    /// Load async slot principal. Returns null se HMAC mismatch ou corrupt.
    /// Em mismatch, emit LoadIntegrityFailed + UI deve oferecer LoadBackupAsync.
    /// </summary>
    public async Task<SaveDataV1?> LoadAsync(int slot)
    {
        ValidateSlot(slot);
        return await LoadFromPathAsync(slot, GetSlotPath(slot), isBackup: false);
    }

    /// <summary>
    /// Load async de backup específico (1-3).
    /// </summary>
    public async Task<SaveDataV1?> LoadBackupAsync(int slot, int backupIndex)
    {
        ValidateSlot(slot);
        if (backupIndex < 1 || backupIndex > BackupChainDepth)
            throw new ArgumentOutOfRangeException(nameof(backupIndex), $"backupIndex MUST 1-{BackupChainDepth}");
        return await LoadFromPathAsync(slot, GetBackupPath(slot, backupIndex), isBackup: true);
    }

    private async Task<SaveDataV1?> LoadFromPathAsync(int slot, string path, bool isBackup)
    {
        if (!isBackup)
            EmitSignal(SignalName.LoadStarted, slot);

        if (!Godot.FileAccess.FileExists(path))
            return null;

        try
        {
            var json = await Task.Run(() =>
            {
                using var file = Godot.FileAccess.Open(path, Godot.FileAccess.ModeFlags.Read);
                return file?.GetAsText() ?? string.Empty;
            });

            if (string.IsNullOrEmpty(json))
                return null;

            // Load version-aware (CONTRACT §7, forward-only): valida HMAC, rejeita versão futura,
            // roda a chain de migradores em saves antigos ANTES de materializar. Antes de
            // F2-E.3.MIG-WIRE este caminho ignorava SaveVersion (migradores eram dead code).
            var data = await Task.Run(() => SaveSerializer.DeserializeWithMigration(json));

            if (!isBackup)
            {
                EmitSignal(SignalName.LoadCompleted, slot);
                GameStateBus.Instance?.EmitSignal(GameStateBus.SignalName.GameLoaded, slot);
            }

            return data;
        }
        catch (SaveVersionTooNewException ex)
        {
            // Save de versão futura: forward-only não lê o futuro. Não tratar como corrupção;
            // UI deve orientar o jogador a atualizar o jogo.
            GD.PushWarning(
                $"SaveManager: save versão futura em {path}: {ex.Message}");
            if (!isBackup)
                EmitSignal(SignalName.LoadVersionTooNew, slot, ex.SaveVersion, ex.CurrentVersion);
            return null;
        }
        catch (SaveIntegrityException ex)
        {
            GD.PushWarning($"SaveManager: HMAC mismatch em {path}: {ex.Message}");
            if (!isBackup) EmitSignal(SignalName.LoadIntegrityFailed, slot);
            return null;
        }
        catch (SaveCorruptException ex)
        {
            GD.PushError($"SaveManager: save corrupt em {path}: {ex.Message}");
            if (!isBackup) EmitSignal(SignalName.LoadCorrupted, slot);
            return null;
        }
        catch (Exception ex)
        {
            GD.PushError($"SaveManager: erro inesperado em {path}: {ex.Message}");
            return null;
        }
    }

    /// <summary>Verifica se slot principal existe.</summary>
    public static bool HasSave(int slot)
    {
        ValidateSlot(slot);
        return Godot.FileAccess.FileExists(GetSlotPath(slot));
    }

    /// <summary>Deleta slot e backup chain.</summary>
    public static void DeleteSlot(int slot)
    {
        ValidateSlot(slot);
        TryDeleteFile(GetSlotPath(slot));
        for (var i = 1; i <= BackupChainDepth; i++)
            TryDeleteFile(GetBackupPath(slot, i));
    }

    private static void ValidateSlot(int slot)
    {
        if (slot < AutosaveSlot || slot > ManualSlotsCount)
            throw new ArgumentOutOfRangeException(nameof(slot),
                $"slot MUST {AutosaveSlot}-{ManualSlotsCount} (0=autosave, 1-{ManualSlotsCount}=manuais)");
    }

    private static string GetSlotPath(int slot) =>
        slot == AutosaveSlot ? $"{SavesDir}/slot_autosave.json" : $"{SavesDir}/slot_{slot}.json";

    private static string GetBackupPath(int slot, int backupIndex) =>
        slot == AutosaveSlot
            ? $"{SavesDir}/slot_autosave.backup{backupIndex}.json"
            : $"{SavesDir}/slot_{slot}.backup{backupIndex}.json";

    private static void RotateBackups(int slot)
    {
        var primary = GetSlotPath(slot);
        var b1 = GetBackupPath(slot, 1);
        var b2 = GetBackupPath(slot, 2);
        var b3 = GetBackupPath(slot, 3);

        // b2 → b3, b1 → b2, primary → b1
        TryDeleteFile(b3);
        if (Godot.FileAccess.FileExists(b2)) TryRenameFile(b2, b3);
        if (Godot.FileAccess.FileExists(b1)) TryRenameFile(b1, b2);
        if (Godot.FileAccess.FileExists(primary)) TryRenameFile(primary, b1);
    }

    private static void TryDeleteFile(string path)
    {
        if (Godot.FileAccess.FileExists(path))
        {
            var err = DirAccess.RemoveAbsolute(path);
            if (err != Error.Ok)
                GD.PushWarning($"SaveManager: falha deletar {path}: {err}");
        }
    }

    private static void TryRenameFile(string from, string to)
    {
        var err = DirAccess.RenameAbsolute(from, to);
        if (err != Error.Ok)
            GD.PushWarning($"SaveManager: falha renomear {from} → {to}: {err}");
    }
}
