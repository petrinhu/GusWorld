// gus/app/i18n/translator.hpp
//
// Translator de UI (M5, incremento 3.5): o tr() runtime da camada app/. Carrega o
// catalogo de traducao ativo (resources/translations/<locale>.md, M8 decommission
// moveu de game/translations/ via git mv) e resolve KEY -> string.
// O PARSER e o POCO do dominio (gus::domain::i18n::parse); o I/O de arquivo (ler o .md)
// vive AQUI na fronteira app/ (como o city_loader le o .gmap), respeitando a invariante
// 4 camadas (domain/ nao faz I/O). Fallback: chave ausente devolve a propria chave
// (visivel na UI, sinaliza traducao faltando sem crashar).
//
// i18n-ready (canon do projeto): nenhuma string user-facing e hardcoded em pt-br no
// codigo; tudo passa por tr(KEY). As mensagens-CODIGO do log de combate (COMPILADO,
// ERRO DE COMPILACAO) sao literais tecnicas NAO-traduziveis por design (combat-flavor.md)
// e NAO passam por aqui.
//
// Cross-ref: gus/domain/i18n/md_translation_loader.hpp (parser POCO);
//            resources/translations/pt_br.md (catalogo dev primario);
//            gus/app/screens/battle_menu.hpp (verbos -> chaves via verb_label_key).

#ifndef GUS_APP_I18N_TRANSLATOR_HPP
#define GUS_APP_I18N_TRANSLATOR_HPP

#include <string>
#include <string_view>

#include "gus/app/screens/battle_menu.hpp"  // BattleVerb
#include "gus/domain/i18n/md_translation_loader.hpp"  // TranslationCatalog

namespace gus::app::i18n {

// Resolve KEY -> string a partir de um catalogo carregado. Imutavel apos load.
class Translator {
public:
    Translator() = default;

    // Carrega o catalogo a partir do CONTEUDO de um .md (parser POCO do dominio). Usado
    // por testes (injecao direta) e pela carga real (que le o arquivo antes). last-wins
    // segue a regra do parser.
    void load_from_content(std::string_view md_content);

    // Le e carrega o catalogo do locale do arquivo .md (I/O na fronteira app/). Devolve
    // true se leu algo; false se o arquivo faltar (a UI cai no fallback = mostra a chave).
    [[nodiscard]] bool load_from_file(const std::string& path);

    // Resolve a chave. Fallback: a propria chave se ausente/vazia. NUNCA lanca.
    [[nodiscard]] std::string tr(const std::string& key) const;

    // Numero de chaves carregadas (diagnostico/teste).
    [[nodiscard]] std::size_t size() const noexcept { return catalog_.size(); }

private:
    gus::domain::i18n::TranslationCatalog catalog_;
};

// Resolve o caminho do catalogo pt_br.md (locale dev primario), como os outros assets de
// jogo: env GUSWORLD_TRANSLATIONS > macro embutido em compilacao > relativo ao CWD. So
// monta a STRING.
[[nodiscard]] std::string resolve_translations_path();

// Chave i18n do ROTULO curto de cada verbo do menu (battle_menu). Reusa as chaves de
// combate existentes onde o rotulo ja e curto; Gambito/Compilar usam COMBAT_VERB_*.
[[nodiscard]] std::string_view verb_label_key(
    gus::app::screens::BattleVerb verb) noexcept;

}  // namespace gus::app::i18n

#endif  // GUS_APP_I18N_TRANSLATOR_HPP
