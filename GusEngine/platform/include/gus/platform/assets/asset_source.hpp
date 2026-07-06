// gus/platform/assets/asset_source.hpp
//
// AssetSource (ADR-013, ASSETS-VFS-F1): o "porteiro" unico de leitura de asset. Fase 1 =
// SO indirecao (opcao D do ADR): nenhum asset e empacotado ainda, tudo continua solto no
// disco - a novidade e que os ~15 call-sites de I/O direto (stbi_load/fopen/ifstream/
// ma_sound_init_from_file) passam a poder falar com UMA interface em vez de reimplementar
// a cadeia de resolucao `env > macro de compilacao > relativo ao CWD` cada um a sua moda
// (motivador central do ADR: essa cadeia estava DUPLICADA em 6+ lugares, com nomes de env
// e raizes proprios por familia de asset).
//
// FASE 2 (ASSETS-SELO-F2, pre-release): troca o BACKEND (uma nova implementacao de
// AssetSource sobre um pacote selado/cifrado) SEM mexer nos consumidores - e exatamente o
// ganho arquitetural desta peca: hoje selar significaria mexer em ~15 lugares; depois
// deste ADR, significa trocar 1 implementacao de interface.
//
// CAMADA: platform/ (nao core/ - core/ e zero-I/O por invariante do GATE de arquitetura,
// auditado por grep no CI/tools/check.sh; AssetSource REPRESENTA uma capacidade de I/O,
// mesmo precedente do IRenderer). core/asset_paths.hpp continua a fonte dos CAMINHOS
// (strings); este header e o COMO abrir.
//
// CONVENCAO DO id: a MESMA string relativa ja centralizada em core/asset_paths.hpp (ex.:
// "assets/fonts/PixelOperatorMono.ttf", "game/translations/pt_br.md",
// "sprites/caua_volt/walk/south/0.png"). Zero tabela de remapeamento nova - o
// asset_paths.hpp ja E a camada logica de nomes.
//
// Cross-ref: docs/tech/adr/ADR-013-asset-source-vfs-fase1.md (decisao completa, 4 pontos
// cravados pelo lider via AskUserQuestion); core/asset_paths.hpp (fonte dos ids);
// platform/tests/asset_source_test.cpp (prova de paridade por familia).

#ifndef GUS_PLATFORM_ASSETS_ASSET_SOURCE_HPP
#define GUS_PLATFORM_ASSETS_ASSET_SOURCE_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace gus::platform::assets {

// Metadados de um asset (Fase 1: derivados do filesystem). Superficie ALEM do
// estritamente necessario hoje (nenhum consumidor atual le size/mtime) - decisao
// CONSCIENTE do lider (ADR-013 secao 2, nota de transparencia), com vistas a
// hot-reload/barra de progresso futuros. Custo trivial na impl filesystem.
struct AssetInfo {
    std::uint64_t size = 0;  // bytes
    std::int64_t mtime = 0;  // epoch SECONDS (unidade cravada aqui); 0 = desconhecido
};

// Interface do porteiro. So-leitura por decisao (ADR-013): staging de fonte pro glintfx
// e save (envelope GDS2 proprio) NAO passam por aqui.
class AssetSource {
public:
    virtual ~AssetSource() = default;

    // Le o asset INTEIRO. nullopt = ausente/ilegivel - preserva a semantica de
    // "degrada graciosamente" que ja existia em cada resolver (font_atlas::read_file
    // devolvia vetor vazio; translator::load_from_file devolvia false). NUNCA lanca.
    [[nodiscard]] virtual std::optional<std::vector<std::byte>> read(
        std::string_view id) const = 0;

    // Metadados sem ler o conteudo. nullopt = ausente. Separado de read() pra permitir
    // consultar tamanho/mtime sem carregar o conteudo inteiro.
    [[nodiscard]] virtual std::optional<AssetInfo> stat(
        std::string_view id) const = 0;
};

// Implementacao Fase 1: le do filesystem solto, reproduzindo EXATAMENTE a cadeia de
// resolucao que cada consumidor fazia antes (por FAMILIA de asset - env/macro/raiz
// DIFERENTES por familia, ver ADR-013 "achado da investigacao"):
//   - fontes    (prefixo "assets/fonts/"):      GUSWORLD_ASSETS+"/fonts" (com checagem
//                de exists, pra nao "sequestrar" a fonte) > macro GUSWORLD_FONTS_DIR >
//                relativo ao CWD (id como esta).
//   - traducoes (prefixo "game/translations/"): GUSWORLD_TRANSLATIONS = override
//                LITERAL completo (ignora o id, mesmo comportamento de hoje) > macro
//                GUSWORLD_TRANSLATIONS_DIR+nome-do-arquivo > CWD (id).
//   - sfx       (prefixo "assets/sfx/"):        GUSWORLD_SFX > macro GUSWORLD_SFX_DIR >
//                CWD (id).
//   - musica    (prefixo "assets/music/"):      GUSWORLD_MUSIC > macro
//                GUSWORLD_MUSIC_DIR > CWD (id).
//   - generico (sprites/images/vfx/...): GUSWORLD_ASSETS > macro GUSWORLD_ASSETS_DIR >
//                relativo a "resources/" (CWD).
// SO a familia de FONTES verifica existencia em cada candidato (paridade com o
// comportamento anterior de font_atlas::resolve_font_path); as demais usam o PRIMEIRO
// candidato com raiz nao-vazia, sem checar o disco (paridade com resolve_sprites_dir/
// resolve_hit_sfx_path/resolve_music_path/resolve_translations_path, que tambem nunca
// verificavam existencia - read()/stat() e quem acaba falhando se o caminho estiver
// errado, exatamente como o comportamento de hoje).
class FilesystemAssetSource final : public AssetSource {
public:
    [[nodiscard]] std::optional<std::vector<std::byte>> read(
        std::string_view id) const override;
    [[nodiscard]] std::optional<AssetInfo> stat(std::string_view id) const override;

    // Escape hatch da Fase 1 (NAO faz parte da interface abstrata AssetSource - a Fase 2
    // pode nao ter um caminho de disco real pra devolver de um pacote selado): resolve o
    // `id` logico pro caminho de disco concreto, sem ler o conteudo. Usado por
    // consumidores que ainda precisam de um PATH de verdade (nao bytes) nesta fase -
    // ex.: audio_engine.cpp (ma_sound_init_from_file le do arquivo; retrofit do ATO de
    // ler adiado, ver relato do dispatch ASSETS-VFS-F1) e o cache-por-caminho de
    // IRenderer::load_texture (stb_image ainda le via stbi_load(path) nesta Fase 1). E o
    // ponto que SUBSTITUI as 6+ copias divergentes de `env > macro > CWD` por-familia.
    [[nodiscard]] std::string resolve_path(std::string_view id) const;
};

// Le os bytes de um caminho de disco CONCRETO (ja resolvido ou literal), sem nenhuma
// logica de familia/resolucao. nullopt = arquivo ausente/ilegivel/leitura incompleta.
// Primitivo de baixo nivel compartilhado: usado internamente por
// FilesystemAssetSource::read() (depois de resolver o id) E por consumidores cujo
// contrato publico precisa continuar aceitando caminhos ARBITRARIOS (nao so ids sob
// asset_paths.hpp) - ex.: font_atlas::bake_font_atlas e i18n::Translator::load_from_file,
// cujos testes ja travam esse comportamento (aceitam qualquer path, inclusive
// inexistente, sem passar por resolucao de familia).
[[nodiscard]] std::optional<std::vector<std::byte>> read_raw_file(
    std::string_view path);

}  // namespace gus::platform::assets

#endif  // GUS_PLATFORM_ASSETS_ASSET_SOURCE_HPP
