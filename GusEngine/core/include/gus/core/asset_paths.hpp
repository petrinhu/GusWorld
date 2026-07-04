// gus/core/asset_paths.hpp
//
// CAMINHOS DE ASSET CENTRALIZADOS (fonte unica da verdade). Toda referencia a um
// sub-caminho de asset (relativo a raiz resources/ ou ao repo) vive AQUI, como constante
// `constexpr std::string_view` - NUNCA hardcoded espalhado pelo codigo. Objetivo: a
// PROXIMA reorganizacao de pastas edita 1 lugar so (este header), sem cacar literais pelo
// codigo (decisao do criador 2026-06-25, apos a movida de sprites/gus ->
// sprites/personagens_inspirados/gus).
//
// POR QUE EM core/: e o nivel mais BAIXO, acessivel a TODAS as camadas (platform/, app/,
// domain/) respeitando a direcao de dependencia (app -> platform -> core). Sao so STRINGS
// (zero I/O, zero SDL/Qt): nao viola o GATE (core/ permanece POCO puro) nem a invariante
// das 4 camadas. A LOGICA de resolucao (env GUSWORLD_ASSETS + macro embutido + fallback
// CWD) permanece em quem usa (anim_catalog/battle_preview/font_atlas/...); aqui so mora a
// FONTE dos sub-caminhos.
//
// CONVENCAO: os caminhos sao RELATIVOS (sem a raiz, sem barra inicial nem final). Quem
// resolve junta a raiz (env ou macro de compilacao ou CWD). "<X>Dir" = pasta; "<X>File" =
// nome de arquivo dentro dela.
//
// Cross-ref: anim_catalog.cpp / player_sprites_loader.cpp / battle_preview.cpp /
//            font_atlas.cpp / translator.cpp (consumidores).

#ifndef GUS_CORE_ASSET_PATHS_HPP
#define GUS_CORE_ASSET_PATHS_HPP

#include <string_view>

namespace gus::core::assets {

// ============================================================================
// SPRITES DE PERSONAGENS (sob resources/sprites/).
//
// Duas familias (estado do disco em 2026-06-25):
//   - "personagens_inspirados/" agrupa personagens baseados em PESSOAS REAIS:
//     gus, yakov, pyotor_vance, brunus_vetorial. (Reorg do criador 2026-06-25.)
//   - os COMPANIONS (caua_volt, jaci_proxy, bento_requiem, dante_grid, linda_siren,
//     iara_lumen) continuam na RAIZ de sprites/ (NAO foram movidos).
// ============================================================================

// Raiz dos sprites de personagens inspirados (pessoas reais).
inline constexpr std::string_view kPersonagensInspiradosDir =
    "sprites/personagens_inspirados";

// --- inspirados (pessoas reais) ---
// GUS (protagonista). MOVIDO 2026-06-25: era "sprites/gus" -> personagens_inspirados/gus.
inline constexpr std::string_view kGusSpritesDir =
    "sprites/personagens_inspirados/gus";
// Frames de animacao de BATALHA do Gus (W3, sprite na arena): 1 pasta por clip
// (battle_idle/, run/, attack_melee/, hurt_physical/, cast/, ko/, ...) com frames
// f0.png..fN.png (256x256 RGBA). Consumido por battle_preview (sprite set da arena);
// os NOMES das pastas sao contrato de battle_sprite_anim::clip_dir_name.
inline constexpr std::string_view kGusBattleAnimsDir =
    "sprites/personagens_inspirados/gus/anims";
inline constexpr std::string_view kYakovSpritesDir =
    "sprites/personagens_inspirados/yakov";
inline constexpr std::string_view kPyotorVanceSpritesDir =
    "sprites/personagens_inspirados/pyotor_vance";
inline constexpr std::string_view kBrunusVetorialSpritesDir =
    "sprites/personagens_inspirados/brunus_vetorial";

// --- companions (na RAIZ de sprites/, nao moveram) ---
// O sub-NOME (depois de "sprites/") e o usado pelo resolve_sprites_dir generico.
inline constexpr std::string_view kCauaSpritesDir = "sprites/caua_volt";
inline constexpr std::string_view kJaciSpritesDir = "sprites/jaci_proxy";
inline constexpr std::string_view kBentoSpritesDir = "sprites/bento_requiem";
inline constexpr std::string_view kDanteSpritesDir = "sprites/dante_grid";
inline constexpr std::string_view kLindaSpritesDir = "sprites/linda_siren";
inline constexpr std::string_view kIaraSpritesDir = "sprites/iara_lumen";

// ============================================================================
// ICONES DA TELA DE BATALHA (sob resources/sprites/icons-m5/). NAO mudou na reorg.
// ============================================================================

// Pasta dos icones de batalha (raiz de retratos/status/intent/molduras).
inline constexpr std::string_view kIconsM5Dir = "sprites/icons-m5";

// Retratos (48px / busto 3/4) usados na fila CTB. (retrato_gus_combate.png virou 3/4; o
// arquivo continua nesta pasta - so o conteudo mudou.)
inline constexpr std::string_view kRetratosDir = "sprites/icons-m5/retratos";
// Retrato GENERICO do androide-inimigo (busto ciber-gotico magenta/cinza): usado na tela
// de BATALHA pra qualquer inimigo sem retrato proprio (retrato_file_for em
// battle_preview.cpp) E, desde M7-COSTURA Inc 2, como o MESMO marcador visivel do
// inimigo FIXO no OVERWORLD (overworld_sim.cpp/sdl_window.cpp) - consistencia "o inimigo
// visto no mapa = o mesmo enfrentado na batalha" (pedido do lider). Em kRetratosDir.
inline constexpr std::string_view kRetratoInimigoFile = "retrato_inimigo.png";

// Icones de status effect (Stun/Poison/Shield/...), 14px.
inline constexpr std::string_view kStatusIconsDir = "sprites/icons-m5/status";

// Icones de intent (telegraph): atacar/defender/aplicar_status/ruido.
inline constexpr std::string_view kIntentIconsDir = "sprites/icons-m5/intent";

// --- COCKPIT "otimo" (variante C, redesign 2026-06-25) ---
// Moldura ornamental TCG (75x97, dourada/azul, com JANELA INTERNA TRANSPARENTE):
// desenhada POR CIMA do retrato_nobg do ator ativo no cockpit. Em sprites/icons-m5/.
inline constexpr std::string_view kMolduraCartaFrameFile = "moldura_carta_frame.png";
// Retrato 3/4 do Gus SEM fundo (128x128), pra emoldurar no cockpit (vai DENTRO da janela
// da moldura). Em sprites/icons-m5/retratos/.
inline constexpr std::string_view kRetratoGusCombateNobgFile =
    "retrato_gus_combate_nobg.png";

// --- Imagens gerais (resources/images/) ---
// Pasta das imagens de lore/UI (fundo por bucket, glifos, retratos formais).
inline constexpr std::string_view kImagesDir = "images";
// Glifo Vetor-Dragao (brasao Vance, 677x369, transparente): centro dos aneis do brasao da
// ABERTURA do cockpit, no lugar do monograma "V". Em resources/images/.
inline constexpr std::string_view kVanceDragonGlyphFile = "vance_dragon_glyph.png";

// ============================================================================
// FONTES (sob GusEngine/assets/fonts/, asset de ENGINE versionado no repo).
// ============================================================================

// Pasta das fontes da engine (relativa: <raiz_assets_engine>/fonts).
inline constexpr std::string_view kFontsDir = "assets/fonts";
// Pixel Operator Mono (CC0): regular + bold.
inline constexpr std::string_view kFontMonoRegularFile = "PixelOperatorMono.ttf";
inline constexpr std::string_view kFontMonoBoldFile = "PixelOperatorMono-Bold.ttf";

// ============================================================================
// MAPAS (.gmap selado, sob GusEngine/assets/maps/compiled/).
// ============================================================================

// Pasta dos mapas compilados (.gmap) versionados no repo.
inline constexpr std::string_view kMapsCompiledDir = "assets/maps/compiled";
// 1o mapa (Distritos Inferiores).
inline constexpr std::string_view kDistritosInferioresGmapFile =
    "distritos_inferiores.gmap";

// ============================================================================
// TRADUCOES (catalogos i18n, sob game/translations/).
// ============================================================================

// Pasta dos catalogos de traducao.
inline constexpr std::string_view kTranslationsDir = "game/translations";
// Catalogo do locale dev primario (pt-br).
inline constexpr std::string_view kTranslationPtBrFile = "pt_br.md";

// ============================================================================
// AUDIO (kit CC0 provisorio, sob <raiz_repo>/assets/sfx/ e assets/music/ - M6 F2/F3,
// ADR-011). Raiz DIFERENTE de kFontsDir/kMapsCompiledDir (essas ficam DENTRO de
// GusEngine/assets/, versionadas com a engine): sfx/music sao asset SOURCE do JOGO,
// na raiz do repo (irma de resources/ e GusEngine/), resolvida via env GUSWORLD_SFX >
// macro de compilacao (GUSWORLD_SFX_DIR) > relativo ao CWD (mesmo padrao dos demais
// resolvers - a LOGICA fica em quem consome, battle_preview.cpp).
// ============================================================================

// Pasta dos SFX curtos (WAV, decode-on-load).
inline constexpr std::string_view kSfxDir = "assets/sfx";
// Hit "digital/runico" (Pillar magia=software, filtro da curadoria F2): SFX de contato
// do golpe (F3) - melee E projetil, mesmo play_sfx nos dois call-sites.
inline constexpr std::string_view kHitSfxFile = "hit_digital_provisorio.wav";
// Variante A/B do hit (GUSWORLD_HIT_SFX=alt troca pro lider comparar ao vivo no
// playtest e escolher qual fica - F3, ADR-011).
inline constexpr std::string_view kHitSfxAltFile = "hit_digital_alt_provisorio.wav";
// Blips de UI do menu de sistema (MENU-PAUSA-CONFIG-SOM, M7-COSTURA): sintese pura
// (tools/gen_menu_ui_sfx.py, sem lib externa), formato alinhado ao hit acima (PCM16,
// 44100Hz, estereo). Hover = bip curto/agudo/discreto (nao cansa navegando o menu
// rapido); click = confirmacao 2-tons ascendente (mais presente que o hover).
inline constexpr std::string_view kMenuHoverSfxFile = "menu_hover_provisorio.wav";
inline constexpr std::string_view kMenuClickSfxFile = "menu_click_provisorio.wav";

// Pasta da musica (MP3, streaming - M6 F4, ADR-011). Raiz assets/music/ (irma de
// assets/sfx/, mesmo padrao/resolvedor - ver comentario de kSfxDir acima).
inline constexpr std::string_view kMusicDir = "assets/music";
// Tema provisorio "GusWorld City cyber-gotica" (curadoria F2, filtro = bíblia de
// leitmotivs R7). NOTA HISTORICA (M6 F4, superada pelo M7-COSTURA Inc 3 abaixo): quando
// so existia ESTA faixa, ela tocava tambem na tela de BATALHA (a UNICA do kit
// provisorio) so pra PROVAR loop+fade tecnicamente. Desde kBattleThemeFile existir, a
// batalha usa a faixa DELA - esta aqui volta a ser so o tema da CIDADE.
inline constexpr std::string_view kCityThemeFile = "cidade_tema_provisorio.mp3";
// Tema de ARENA/batalha (M7-COSTURA Inc 3, ADR-012): gerada via Suno IA pelo lider
// 2026-07-03, prompt ancorado na biblia de leitmotivs do time - fecha o "NOTA HONESTA"
// acima (kCityThemeFile parava de ser so tema de cidade por falta de 2a faixa). Ainda
// PROVISORIA (kit CC0/IA, nao a producao final) ate a onda de audio dedicada
// (audio-designer-composer) substituir. Ver assets/AUDIO_KIT_PROVISORIO.md.
inline constexpr std::string_view kBattleThemeFile = "Arena_GusWorld.mp3";

// ============================================================================
// VFX (efeitos visuais pre-renderizados, sob resources/vfx/). M7-COSTURA Inc 2c:
// substitui o glitch procedural (vetado pelo lider ao vivo - "pareceu bug") por uma
// sequencia de frames pre-renderizada aprovada (boot de sistema pixelizado, Pillar
// "magia = software" - o sistema runico do Gus REBOOTANDO pro combate).
// ============================================================================

// Pasta dos kBootPixelFrameCount (20) frames do boot pixelizado (frame_00.png..
// frame_19.png, 320x180, RGB opaco). Ver gus/app/boot_pixel_overlay.hpp (quem
// carrega/desenha, nos 2 backends) e gus/core/anim/boot_pixel_sequence.hpp
// (kBootPixelFrameCount + o indice POCO progresso->frame).
inline constexpr std::string_view kVfxBootPixelDir = "vfx/boot_pixel";

}  // namespace gus::core::assets

#endif  // GUS_CORE_ASSET_PATHS_HPP
