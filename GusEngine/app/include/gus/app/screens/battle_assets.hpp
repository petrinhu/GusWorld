// gus/app/screens/battle_assets.hpp
//
// AC-E11 A3 (ADR-019, decomposicao atomica de battle_preview.cpp): resolucao de PATHS e
// LOADERS de asset do host da BattleScene. Reune o helper generico de junção de caminho
// (join), o dispatch pro porteiro de assets (resolve_asset_dir, ASSETS-VFS-F1/ADR-013),
// os resolvers especificos (retratos/status/intent/musica/SFX de hit e de UI) e o loader
// do sprite set de batalha do Gus. Extraido verbatim de battle_preview.cpp - ZERO mudanca
// de comportamento, so de arquivo.
//
// Este e o modulo "mais de baixo" da decomposicao (ADR-019 §"motor generico + conteudo
// como dado"): gus/app/screens/battle_cockpit_rml.hpp (montagem RML do cockpit) depende
// dele (resolve_asset_dir/join pra montar os caminhos de asset do stage), e
// battle_preview.cpp (a casca que resta) tambem - por isso este corte entrou ANTES do
// corte da RML na sequencia de commits real (decisao autonoma registrada no commit; a
// ordem descrita no brief da onda era RML->assets, mas RML DEPENDE de assets, entao a
// ordem natural de extracao e a inversa - conteudo de cada modulo intacto, so a ordem dos
// commits mudou).
//
// Cross-ref: gus/app/screens/battle_preview.hpp (casca dona do loop, chamador principal);
//            gus/app/screens/battle_sprite_anim.hpp (BattleClipId/ActorSpriteSet, o
//            contrato do sprite set que load_gus_sprite_set preenche);
//            gus/platform/assets/asset_source.hpp (FilesystemAssetSource, o porteiro
//            ASSETS-VFS-F1/ADR-013 que resolve_asset_dir/resolve_hit_sfx_path/
//            resolve_ui_sfx_path delegam); docs/tech/adr/ADR-019.

#ifndef GUS_APP_SCREENS_BATTLE_ASSETS_HPP
#define GUS_APP_SCREENS_BATTLE_ASSETS_HPP

#include <optional>
#include <string>
#include <string_view>

#include "gus/app/screens/battle_sprite_anim.hpp"  // ActorSpriteSet
#include "gus/core/asset_paths.hpp"                 // kCityThemeFile (default de resolve_music_path)
#include "gus/platform/render2d/i_renderer.hpp"     // IRenderer (load_gus_sprite_set)

namespace gus::app::screens {

// Junta dois segmentos de caminho com UMA barra (nao duplica se `a` ja termina em '/', nao
// perde a barra se faltar). Helper GENERICO usado por todo resolver de asset deste
// modulo E por battle_cockpit_rml.cpp (stage de assets do cockpit) - fonte unica.
[[nodiscard]] std::string join(const std::string& a, const std::string& b);

// Resolve um caminho RELATIVO de asset (do header central) - familia GENERICA (sprites/
// images/vfx sob resources/). ASSETS-VFS-F1 (ADR-013): CONSOLIDADO em
// FilesystemAssetSource::resolve_path (o `rel` JA E o id logico, sem prefixo de familia
// especifica - cai no dispatch generico). Assinatura INTOCADA.
[[nodiscard]] std::string resolve_asset_dir(std::string_view rel);

// Resolve o caminho do SFX de hit (M6 F3, ADR-011). ASSETS-VFS-F1: a cadeia `env
// GUSWORLD_SFX > macro GUSWORLD_SFX_DIR > CWD (kSfxDir)` foi CONSOLIDADA em
// FilesystemAssetSource (familia SFX, dispatch pelo prefixo "assets/sfx/"). O A/B
// GUSWORLD_HIT_SFX=alt continua decidido AQUI (e escolha de QUAL arquivo/id, nao de
// resolucao de raiz) - qualquer outro valor (ou ausente) usa o principal.
[[nodiscard]] std::string resolve_hit_sfx_path();

// COCKPIT-SFX-HOVER-CLIQUE: resolve o caminho de um SFX de UI (hover/click) - MESMA
// familia SFX de resolve_hit_sfx_path (ambos delegam pro MESMO porteiro, ASSETS-VFS-F1),
// so que o NOME do arquivo vem por parametro (os 2 blips do menu: kMenuHoverSfxFile/
// kMenuClickSfxFile). REUSA os MESMOS 2 arquivos do menu de sistema (mesma identidade
// sonora de UI - pedido do lider ao vivo, nao gerar novos).
[[nodiscard]] std::string resolve_ui_sfx_path(std::string_view file);

// Carrega o SPRITE SET de batalha do GUS (W3): pra cada clip conhecido
// (battle_sprite_anim::clip_dir_name), varre <resources>/<kGusBattleAnimsDir>/<clip>/
// f0.png..fN.png em ordem e resolve cada frame em TextureId. Devolve nullopt se NENHUM
// frame existir no disco (headless/CI sem assets) - a cena entao degrada pro retrato
// placeholder.
[[nodiscard]] std::optional<ActorSpriteSet> load_gus_sprite_set(
    gus::platform::render2d::IRenderer& renderer);

// Mapeia o id de ator de DEMO -> arquivo de retrato 48px. Os inimigos (inimigoN)
// compartilham retrato_inimigo. Ponto unico; quando os retratos forem por-personagem
// reais, troca-se aqui (ou vira data-driven).
[[nodiscard]] std::string retrato_file_for(const std::string& actor_id);

// Resolve a pasta dos retratos 48px (resources/sprites/icons-m5/retratos), na MESMA
// ordem do resolver de sprites do Gus: env GUSWORLD_ASSETS > macro embutido > relativo
// ao CWD. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_retratos_dir();

// Resolve a pasta dos icones de status (resources/sprites/icons-m5/status), na mesma
// ordem do resolver de retratos. So monta a STRING (nao abre arquivo).
[[nodiscard]] std::string resolve_status_icons_dir();

// M7-COSTURA Inc 2/3: resolve o caminho de uma faixa de MUSICA (M6 F4, ADR-011).
// ASSETS-VFS-F1 (ADR-013): a cadeia `env GUSWORLD_MUSIC > macro GUSWORLD_MUSIC_DIR >
// CWD (kMusicDir)` foi CONSOLIDADA em FilesystemAssetSource (familia MUSICA, dispatch
// pelo prefixo "assets/music/") + o NOME do arquivo dado em `file` (default
// kCityThemeFile, o comportamento de sempre - todo call-site existente sem argumento
// continua identico). A Maestro chama com `file=kBattleThemeFile` pra resolver a faixa
// da ARENA tambem (mesma pasta kMusicDir, so o nome do arquivo muda).
[[nodiscard]] std::string resolve_music_path(
    std::string_view file = gus::core::assets::kCityThemeFile);

// Resolve a pasta dos icones de INTENT (telegraph, incremento 5). So monta a STRING.
[[nodiscard]] std::string resolve_intent_icons_dir();

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_BATTLE_ASSETS_HPP
