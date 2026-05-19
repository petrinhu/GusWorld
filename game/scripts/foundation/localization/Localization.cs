// Localization.cs
//
// AutoLoad wrapper de i18n com formato MD custom (não-CSV).
//
// Registrar em Project Settings > AutoLoad como "Localization".
//
// Uso:
//   Localization.Instance.TrMd("MENU_START_GAME")        // "Iniciar jogo" (pt_br atual)
//   Localization.Instance.SetLocale("en_intl")
//   Localization.Instance.TrMd("MENU_START_GAME")        // "Start game"
//
// Princípios:
// - Locale default: pt_br (canon, dev sem fallback).
// - Fallback chain: pt_br → en_intl → key literal.
// - Recarrega arquivos MD em runtime (debug builds) via Reload().
// - NÃO substitui Godot tr() nativo; convive com ele se necessidade futura.
//
// Cross-ref: docs/tech/architecture.md §6 + CONTRACT.md §6 (a11y) + project_i18n_canonico memo.

using System.Collections.Generic;
using Godot;
using GusDragon.Engine.Foundation.Localization;

namespace GusWorld.Game.Foundation.Localization;

/// <summary>
/// AutoLoad wrapper de i18n custom MD format (canon GusWorld pós-ADR-002).
/// </summary>
public partial class Localization : Node
{
    public static Localization? Instance { get; private set; }

    private const string TranslationsDir = "res://translations";
    private const string DefaultLocale = "pt_br";
    private const string FallbackLocale = "en_intl";

    /// <summary>
    /// Locales suportados hardcoded (Godot DirAccess não vê .md não-importados em res://).
    /// Adicionar novo locale: criar arquivo + adicionar string aqui.
    /// </summary>
    private static readonly string[] SupportedLocales = { "pt_br", "en_intl" };

    private readonly Dictionary<string, Dictionary<string, string>> _strings = new();
    private string _currentLocale = DefaultLocale;

    public override void _Ready()
    {
        Instance = this;
        LoadAllLocales();
        _currentLocale = DefaultLocale;
    }

    public override void _ExitTree()
    {
        if (Instance == this) Instance = null;
    }

    /// <summary>
    /// Carrega arquivos MD pra cada locale em SupportedLocales.
    /// </summary>
    public void LoadAllLocales()
    {
        _strings.Clear();
        foreach (var locale in SupportedLocales)
        {
            var fullPath = $"{TranslationsDir}/{locale}.md";
            if (Godot.FileAccess.FileExists(fullPath))
                _strings[locale] = MdTranslationLoader.LoadFromFile(fullPath);
            else
                GD.PushWarning($"Localization: arquivo locale não encontrado: {fullPath}");
        }
    }

    /// <summary>
    /// Recarrega arquivos MD (útil em dev/playtest pra iterar sem reiniciar jogo).
    /// </summary>
    public void Reload() => LoadAllLocales();

    /// <summary>
    /// Define locale corrente. Se locale não estiver carregado, faz nothing + warning.
    /// </summary>
    public void SetLocale(string locale)
    {
        if (!_strings.ContainsKey(locale))
        {
            GD.PushWarning($"Localization: locale '{locale}' não carregado, mantendo '{_currentLocale}'");
            return;
        }
        _currentLocale = locale;
    }

    /// <summary>
    /// Retorna locale corrente.
    /// </summary>
    public string GetLocale() => _currentLocale;

    /// <summary>
    /// Retorna lista de locales disponíveis (arquivos MD encontrados).
    /// </summary>
    public IReadOnlyList<string> GetAvailableLocales() => new List<string>(_strings.Keys);

    /// <summary>
    /// Traduz chave usando locale corrente + fallback chain.
    /// </summary>
    public string TrMd(string key, Variant[]? args = null)
    {
        string value;

        if (_strings.TryGetValue(_currentLocale, out var current) && current.TryGetValue(key, out var v1))
            value = v1;
        else if (_strings.TryGetValue(FallbackLocale, out var fallback) && fallback.TryGetValue(key, out var v2))
            value = v2;
        else if (_strings.TryGetValue(DefaultLocale, out var def) && def.TryGetValue(key, out var v3))
            value = v3;
        else
        {
            if (OS.IsDebugBuild())
                GD.PushWarning($"Localization: chave '{key}' não encontrada em nenhum locale");
            return key;
        }

        // Interpolação simples {0}, {1}, etc.
        if (args != null && args.Length > 0)
        {
            for (var i = 0; i < args.Length; i++)
                value = value.Replace($"{{{i}}}", args[i].AsString());
        }

        return value;
    }

    /// <summary>
    /// Checa se key existe no locale atual (sem fallback).
    /// </summary>
    public bool HasKey(string key) => _strings.TryGetValue(_currentLocale, out var current) && current.ContainsKey(key);

    /// <summary>
    /// Lista todas chaves do locale atual (debug).
    /// </summary>
    public IReadOnlyList<string> ListKeys() =>
        _strings.TryGetValue(_currentLocale, out var current)
            ? new List<string>(current.Keys)
            : new List<string>();
}
