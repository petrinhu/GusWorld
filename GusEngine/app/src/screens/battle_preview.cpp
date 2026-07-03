// gus/app/src/screens/battle_preview.cpp
//
// Ver header. Casca SDL do viewer da BattleScene (esqueleto M5). Reusa Render2dSdl
// (atras de IRenderer) e o mesmo padrao de loop do anim_preview (poll -> render). A
// cena LE o motor de combate; aqui so abrimos janela, carregamos os retratos 48px e
// desenhamos o esqueleto a cada frame. Esc/fechar encerra.

#include "gus/app/screens/battle_preview.hpp"

#include <cstdlib>  // std::getenv
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "gus/app/screens/battle_cockpit_verb_ids.hpp"  // GLINTFX-CLICK: id->indice de verbo
#include "gus/app/screens/battle_hud_model.hpp"  // status_icon_file/index
#include "gus/app/screens/battle_layout.hpp"     // arena_layout (selftest de mouse A2)
#include "gus/app/screens/battle_scene.hpp"
#include "gus/core/asset_paths.hpp"             // caminhos de asset centralizados
#include "gus/domain/combat/combat_enums.hpp"  // StatusId
#include "gus/platform/audio/audio_engine.hpp"     // AudioEngine (M6 F3, ADR-011)
#include "gus/platform/render2d/render2d_gl3.hpp"  // ADR-009 GL3: backend OpenGL da arena
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load + read_backbuffer (captura)

// ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD - o backend RmlUi
// vendorizado (RmlUiHud) foi aposentado. Compilado e linkado SEMPRE (app/ linka
// glintfx::glintfx incondicional). A arena (Render2dGl3) e o gl3_loader continuam.
#include <filesystem>  // tempfile do RML (load() do glintfx exige um path)
#include <fstream>
#include <optional>
#include <glintfx/ui_event.hpp>
#include <glintfx/ui_layer.hpp>

// stb_image_write: captura de frame (PNG) para o SMOKE VISUAL do ADR-009 (comparar o
// jogo com o mock). IMPLEMENTACAO definida UMA vez aqui (camada app/, fora do hot path).
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Raiz resources/ do repo, embutida pelo CMake (mesma macro do resolver de sprites).
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
#endif

// Pasta das fontes (.ttf), embutida pelo CMake (ADR-010 F2a). So usada no caminho glintfx
// (cockpit BAKED): o @font-face do RCSS aponta pra ca. Fallback vazio se ausente.
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

// Pasta do kit CC0 de SFX (M6 F3, ADR-011), raiz DIFERENTE de GUSWORLD_ASSETS_DIR
// (repo_root/assets/sfx, nao resources/ - ver gus/core/asset_paths.hpp). Override em
// runtime via env GUSWORLD_SFX.
#ifndef GUSWORLD_SFX_DIR
#define GUSWORLD_SFX_DIR ""
#endif

// Pasta da musica CC0 (M6 F4, ADR-011), irma de GUSWORLD_SFX_DIR (repo_root/assets/
// music). Override em runtime via env GUSWORLD_MUSIC.
#ifndef GUSWORLD_MUSIC_DIR
#define GUSWORLD_MUSIC_DIR ""
#endif

namespace gus::app::screens {

namespace {

constexpr int kWindowW = 1920;  // 960x540 * 2 (escala inteira x2 = 1080p, D1)
constexpr int kWindowH = 1080;

std::string join(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// Resolve um caminho RELATIVO de asset (do header central) pela ordem padrao:
// env GUSWORLD_ASSETS > macro de compilacao (GUSWORLD_ASSETS_DIR) > relativo ao CWD.
// A FONTE do sub-caminho e a constante; aqui so a logica de resolucao.
std::string resolve_asset_dir(std::string_view rel) {
    const std::string sub(rel);
    if (const char* env = std::getenv("GUSWORLD_ASSETS")) {
        if (env[0] != '\0') {
            return join(env, sub);
        }
    }
    const std::string compiled = GUSWORLD_ASSETS_DIR;
    if (!compiled.empty()) {
        return join(compiled, sub);
    }
    return join("resources", sub);
}

// Resolve o caminho do SFX de hit (M6 F3, ADR-011): env GUSWORLD_SFX > macro embutida
// (GUSWORLD_SFX_DIR = repo_root/assets/sfx) > relativo ao CWD (kSfxDir). Raiz DIFERENTE
// de resolve_asset_dir (essa e resources/; sfx/music vivem em assets/ na raiz do repo -
// ver o comentario de kSfxDir em core/asset_paths.hpp). GUSWORLD_HIT_SFX=alt troca pro
// arquivo alternativo (A/B pro lider comparar e escolher ao vivo no playtest); qualquer
// outro valor (ou ausente) usa o principal.
std::string resolve_hit_sfx_path() {
    const bool alt = [] {
        const char* e = std::getenv("GUSWORLD_HIT_SFX");
        return e != nullptr && std::string(e) == "alt";
    }();
    const std::string file = alt ? std::string(gus::core::assets::kHitSfxAltFile)
                                  : std::string(gus::core::assets::kHitSfxFile);
    if (const char* env = std::getenv("GUSWORLD_SFX")) {
        if (env[0] != '\0') {
            return join(env, file);
        }
    }
    const std::string compiled = GUSWORLD_SFX_DIR;
    if (!compiled.empty()) {
        return join(compiled, file);
    }
    return join(std::string(gus::core::assets::kSfxDir), file);
}

// Resolve o caminho da MUSICA (M6 F4, ADR-011): env GUSWORLD_MUSIC > macro embutida
// (GUSWORLD_MUSIC_DIR = repo_root/assets/music) > relativo ao CWD (kMusicDir). Mesma
// receita de resolve_hit_sfx_path, sem variante A/B (so 1 faixa no kit provisorio).
std::string resolve_music_path() {
    const std::string file(gus::core::assets::kCityThemeFile);
    if (const char* env = std::getenv("GUSWORLD_MUSIC")) {
        if (env[0] != '\0') {
            return join(env, file);
        }
    }
    const std::string compiled = GUSWORLD_MUSIC_DIR;
    if (!compiled.empty()) {
        return join(compiled, file);
    }
    return join(std::string(gus::core::assets::kMusicDir), file);
}

// Carrega o SPRITE SET de batalha do GUS (W3): pra cada clip conhecido
// (battle_sprite_anim::clip_dir_name), varre <resources>/<kGusBattleAnimsDir>/<clip>/
// f0.png..fN.png em ordem e resolve cada frame em TextureId. fps/loop = defaults do
// modulo POCO; clip_frame_cap TRUNCA a carga (attack_melee_east: so f0..f5 entram -
// f6-f8 derivam e nunca chegam na memoria/render; decisao 2026-07-01, ver
// battle_sprite_anim.hpp). Devolve nullopt se NENHUM frame existir no disco
// (headless/CI sem assets) - a cena entao degrada pro retrato placeholder de hoje.
// So o Gus nesta onda; os demais atores ganham set quando as anims deles existirem
// (PixelLab). Clipes de perfil (run_east/run_west/attack_melee_east) entram pelo
// MESMO laco (o enum e a fonte); ausencia degrada via clip_fallback na cena.
std::optional<gus::app::screens::ActorSpriteSet> load_gus_sprite_set(
    gus::platform::render2d::IRenderer& renderer) {
    namespace fs = std::filesystem;
    using gus::app::screens::BattleClipId;
    const std::string base =
        resolve_asset_dir(gus::core::assets::kGusBattleAnimsDir);
    gus::app::screens::ActorSpriteSet set;
    bool any = false;
    for (int c = 0; c < gus::app::screens::kBattleClipCount; ++c) {
        const auto id = static_cast<BattleClipId>(c);
        const std::string dir =
            join(base, std::string(gus::app::screens::clip_dir_name(id)));
        auto& clip = set.clips[static_cast<std::size_t>(c)];
        clip.fps = gus::app::screens::default_clip_fps(id);
        clip.loop = gus::app::screens::default_clip_loop(id);
        const int cap = gus::app::screens::clip_frame_cap(id);
        for (int i = 0;; ++i) {
            if (cap > 0 && i >= cap) {
                break;  // frames alem do cap sao DERIVADOS: nao carregam (nem renderizam)
            }
            const std::string path = join(dir, "f" + std::to_string(i) + ".png");
            std::error_code ec;
            if (!fs::exists(path, ec)) {
                break;  // fim da sequencia (frames sao contiguos f0..fN)
            }
            const gus::platform::render2d::TextureId tex =
                renderer.load_texture(path.c_str());
            if (tex == gus::platform::render2d::kInvalidTexture) {
                break;  // backend sem textura (Null/headless): degrada
            }
            clip.frames.push_back(tex);
            any = true;
        }
    }
    if (!any) {
        return std::nullopt;
    }
    return set;
}

// Mapeia o id de ator de DEMO -> arquivo de retrato 48px. Os inimigos (inimigoN)
// compartilham retrato_inimigo. Ponto unico; quando os retratos forem por-personagem
// reais, troca-se aqui (ou vira data-driven).
std::string retrato_file_for(const std::string& actor_id) {
    // Gus na BATALHA usa o retrato de COMBATE (meio corpo do sprite de jogo: cabelo
    // revolto + oculos taticos + aparelho + antena + casaco tatico). O retrato_gus.png
    // (terno/formal) NAO entra na luta (vira quadro na casa dos pais / narracoes).
    if (actor_id == "gus") return "retrato_gus_combate.png";
    if (actor_id == "caua") return "retrato_caua.png";
    if (actor_id == "jaci") return "retrato_jaci.png";
    // inimigo1..4 e qualquer outro -> retrato generico de inimigo.
    return "retrato_inimigo.png";
}

// ============================================================================
// F1: COCKPIT RML/RCSS (conversao do mock scratchpad/cockpit_otimo/index.html).
// AGORA com efeitos NATIVOS do backend GL3: vertical-gradient/radial-gradient,
// box-shadow (glow), border-radius, @keyframes/animation (halo pulsante, brasao),
// transform (aneis girando), drop-shadow. Paleta canonica preservada. Os caminhos de
// asset (moldura TCG + retrato do Gus sem fundo) sao ABSOLUTOS (doc da memoria).
// ============================================================================
// Diretorio base dos assets do cockpit (icons-m5). Usado como source_url do doc RmlUi
// (com um nome de arquivo ficticio) pra resolver os image() relativos.
std::string cockpit_asset_base_dir() {
    return resolve_asset_dir(gus::core::assets::kIconsM5Dir);
}

std::string load_cockpit_rml() {
    // GLINTFX-CLICK (v0.2.5): cada pill de verbo (`.verb`, na div `.menu` abaixo) carrega um
    // `id="verb-*"` ESTAVEL (fonte unica gus/app/screens/battle_cockpit_verb_ids.hpp) que o
    // UiLayer::set_click_callback devolve no clique. ZERO geometria manual pra sincronizar
    // aqui - so os ids precisam bater com o mapeamento id->indice de battle_cockpit_verb_ids
    // .hpp (um teste dedicado prova isso; battle_cockpit_verb_ids_test.cpp). Historico: ate
    // a v0.2.4 o glintfx nao expunha hit-test de elemento, entao a geometria da regra `.verb`
    // (width/height/margin/padding/border) era ESPELHADA A MAO num modulo separado
    // (battle_cockpit_pills.hpp, aposentado neste incremento - ver historico git).
    // Caminhos de asset RELATIVOS a base (cockpit_asset_base_dir): o RmlUi resolve image()
    // contra o source_url do doc. Absoluto perderia a barra inicial na canonicalizacao.
    const std::string moldura(gus::core::assets::kMolduraCartaFrameFile);
    const std::string retrato = "retratos/retrato_gus_combate_nobg.png";

    // RCSS usa unidades 'dp' (= px logico do mock; o dp_ratio escala pro alvo real). As
    // cores e a estrutura espelham o mock. Os efeitos (gradiente/box-shadow/animation) sao
    // os que o backend GL3 implementa nativo (achado da F0: o SDL_Renderer nao fazia).
    std::string rml;
    rml += R"RML(<rml>
<head>
<style>
/* body transparente: o cockpit compoe SO a coluna por cima da arena. */
body { font-family: "Pixel Operator Mono"; background: transparent; }

/* NOTA DE EFEITOS (corrigido apos o FXTEST): o GL3 RENDERIZA gradiente/box-shadow/radial
   - o que faltava era CONTRASTE + SPREAD POSITIVO. box-shadow spread NEGATIVO (5o valor)
   encolhe o glow pra dentro (invisivel); spread POSITIVO expande (glow visivel). Gradientes
   precisam de cores distintas. Tudo abaixo usa spread positivo e degrades de contraste. */

/* ---- COCKPIT: coluna lateral esquerda 252dp, degrade vertical ---- */
#cockpit {
  position: absolute; top: 0dp; left: 0dp; bottom: 0dp; width: 252dp;
  decorator: vertical-gradient( #141a2c #0f1322 );  /* topo escurecido (some a barra clara em y=0) -> base escura */
  /* POLISH 1: padding compacto (a coluna inteira precisa caber em 540dp). */
  padding: 10dp 12dp 0dp 12dp;
}
/* AJUSTE (veredito do lider): o filete #edge da divisa direita foi REMOVIDO por completo.
   Mesmo suavizado (filete translucido uniforme) ele deixava um traco teal sutil no TOPO da
   tela. Topo agora 100% limpo, zero linha. A divisa cockpit/arena fica so pelo contraste do
   degrade da coluna contra a arena. */

/* ---- RETRATO emoldurado (moldura TCG asset) + HALO cyan pulsante ---- */
#actor { text-align: center; }
/* #portrait e o CONTEXTO de posicionamento (relative) dos filhos absolutos (pic/frame),
   senao eles ancoram no root (0,0). Centrado na coluna com margens laterais iguais. */
#portrait {
  position: relative;
  /* POLISH 1: retrato compactado (proporcao mantida 128:165 -> 104:134); margin-left
     recentraliza na coluna (inner 228dp). pic/frame escalam junto abaixo. */
  width: 104dp; height: 134dp; margin-left: 62dp; margin-top: 6dp;
  /* HALO neon do ator ATIVO: box-shadow cyan FORTE que PULSA (breathe). Spread POSITIVO. */
  box-shadow: #22D3EE 0dp 0dp 34dp 10dp;
  border-radius: 14dp;
  animation: breathe 2.4s cubic-in-out infinite-alternate;
}
@keyframes breathe {
  from { box-shadow: #22D3EEaa 0dp 0dp 22dp 4dp; }
  to   { box-shadow: #22D3EEff 0dp 0dp 44dp 14dp; }
}
/* janela interna da moldura (retrato vai DENTRO), atras da moldura */
#pic { position: absolute; top: 13dp; left: 13dp; width: 78dp; height: 106dp;
  background-color: #0c1322;
  decorator: image( )RML";
    rml += retrato;
    rml += R"RML( cover ); }
/* a moldura (asset) POR CIMA, cobrindo o quadro inteiro */
#frame { position: absolute; top: 0dp; left: 0dp; width: 104dp; height: 134dp;
  decorator: image( )RML";
    rml += moldura;
    rml += R"RML( ); }

/* POLISH 2 (veredito do lider: nome/role DESLOCADOS da moldura): o #name/#role tinham
   text-align:center MAS a caixa auto-width que o RmlUi centrava media a LARGURA DO PADDING-BOX
   (252dp) a partir do content-left (12dp) => centro em ~138dp, enquanto o retrato (largura
   EXPLICITA 104dp + margin-left 62dp) centra em 126dp (= centro da coluna). Medido na captura:
   moldura=126.6dp, nome="Drone"=136.9dp (~10dp a direita). A diferenca (~12dp) = o padding
   esquerdo, que a auto-width ignorava. FIX: dar ao nome/role LARGURA EXPLICITA (148dp, folga
   pra "VETOR DO GAMBITO") + margin-left 40dp, ancorada no mesmo content-left do retrato =>
   centro = 12 + 40 + 74 = 126dp, EXATAMENTE sob o eixo da moldura (mesmo truque de largura
   explicita do #portrait). text-align:center segue centrando o texto dentro dessa caixa. */
#name { text-align: center; margin-top: 6dp; margin-left: 40dp; width: 148dp;
  font-size: 14dp; color: #eaf6fb; }
#role { text-align: center; margin-left: 40dp; width: 148dp;
  font-size: 10dp; color: #E8A33D; }

/* ---- HP barra (degrade verde + glow) + numeros ---- */
#vitals { margin-top: 6dp; }
.hpbar {
  /* FIX largura (regressao do block-by-default da UA-sheet v0.2.4): sem 'width' o .hpbar
     (agora BLOCK nativo) esticava ate a largura de conteudo da coluna (228dp) + o glow,
     invadindo a arena (~260dp, alem da coluna de 252dp). CLAMP explicito em 110dp = o MESMO
     width dos pills de verbo (.verb), left-aligned => a barra coincide com a coluna de botoes
     abaixo e fica 100% DENTRO do cockpit (borda direita 110dp + glow ~18dp = ~128dp << 228dp
     inner). O fill escala proporcional ao HP dentro deste width quando virar track/fill. */
  width: 110dp; height: 12dp; border-radius: 6dp; margin-top: 3dp;
  decorator: vertical-gradient( #7dffbe #2e9c63 );  /* verde claro -> verde escuro */
  box-shadow: #3FB97a 0dp 0dp 16dp 2dp;            /* glow verde, spread POSITIVO */
}
.hpnum { font-size: 10dp; color: #8fa6b4; }
.hpnum .v { color: #5fe3a0; }

/* ---- pips AP (latao) / Mana (cyan), radiais com glow ---- */
.pips { margin-top: 5dp; font-size: 10dp; color: #8fa6b4; }
.pip { display: inline-block; width: 12dp; height: 12dp; border-radius: 6dp;
  margin-right: 4dp; border: 1dp #2a3450; background-color: #0c1322; }
.pip.ap.on { decorator: radial-gradient( circle closest-side, #ffe6a8, #E8A33D );
  box-shadow: #E8A33D 0dp 0dp 12dp 2dp; }
.pip.mana.on { decorator: radial-gradient( circle closest-side, #c2f6ff, #22D3EE );
  box-shadow: #22D3EE 0dp 0dp 12dp 2dp; }

/* ---- MENU de verbos: pill rococo com GLOW neon nos 3 estados ---- */
/* ESPACAMENTO (veredito do lider): +18dp de folga MANA->1o botao (margin-top do .menu) e
   +18dp FUGIR->log (margin-top do #log). Com a UA-stylesheet do RmlUi carregada em AMBOS os
   modos (vendorizado sempre; glintfx a partir do v0.2.4), .menu/#log sao block por padrao =>
   margin vertical aplica nativo. O RmlUi nao faz margin-collapsing, entao os 18dp sao exatos. */
.menu { margin-top: 18dp; }
.verb {
  /* AJUSTE (veredito do lider): pills NITIDAMENTE mais enxutos. O 25->21 anterior foi
     imperceptivel; aqui o corte e no PISO de legibilidade E no FOOTPRINT: altura 18dp (fonte
     11dp centrada via line-height => ~3.5dp de folga vertical em cima/embaixo = o "padding
     vertical ~4dp" pedido, sem estourar a caixa content-box), gap 4dp, padding-h 12dp, raio
     9dp. LARGURA travada em 160dp (nao mais full-column) => o menu fica visivelmente mais
     compacto/estreito, nao so mais baixo. Piso 18dp/11dp respeitado (nao ir abaixo). O
     SELECIONADO segue proeminente (glow/contraste, regras .sel). O rotulo mais largo
     (DEFENDER/COMPILAR, 8 chars a 11dp) mede ~82dp de conteudo; a caixa a 110dp deixa ~28dp
     de folga sem clipar. */
  width: 110dp; height: 18dp; margin-bottom: 4dp; padding: 0dp 12dp;
  border-radius: 9dp;
  decorator: vertical-gradient( #2a3658 #131a2e );  /* topo mais claro (3D sutil) */
  border: 1dp #38456e;
  color: #d6e6ef; font-size: 11dp;
}
.verb .lbl { display: inline-block; line-height: 18dp; }
/* NORMAL: cor do verbo no glifo lateral */
.verb .glyph { display: inline-block; width: 8dp; height: 8dp; margin-right: 10dp;
  border-radius: 2dp; background-color: #8fa6b4; }
.verb.cyan .glyph { background-color: #22D3EE; box-shadow: #22D3EE 0dp 0dp 8dp 1dp; }
.verb.latao .glyph { background-color: #E8A33D; box-shadow: #E8A33D 0dp 0dp 8dp 1dp; }
/* HOVER (ADITIVO, SO-VISUAL): o ponteiro sobre o pill CLAREIA o fundo + poe uma BORDA NEUTRA
   clara. De proposito DISCRETO: e so o feedback "o mouse esta aqui", que NAO deve competir com
   o glow cyan FORTE do .sel (a selecao REAL, do teclado/confirmada). Sem box-shadow (zero glow
   que dispute) e mantendo border 1dp (= .verb): NAO mexe na geometria nem no `id="verb-*"`
   (GLINTFX-CLICK) => o hit-test de clique segue valido. RmlUi 6.3 casa :hover NATIVO (o
   MouseMove ja chega via sdl_to_glintfx -> process_event -> Context::ProcessMouseMove).
   ORDEM PROPOSITAL: declarado ANTES de .verb.sel e .verb.fired. Como :hover conta como 1
   pseudo-classe, .verb:hover (2) EMPATA em especificidade com .verb.sel/.verb.fired (2 classes)
   => o empate vai pra ORDEM DE FONTE e a selecao/acionado (mais abaixo) VENCEM. Assim, num pill
   SELECIONADO E sob o mouse, o .sel segue DOMINANTE (glow cyan intacto) e o hover nao o ofusca.
   As cores neutras (fundo lift #384562/#171f38, borda #8a94a8) ficam na paleta canonica da
   coluna e ficam CLARAMENTE menos vivas que o azul-cyan do .sel. */
.verb:hover {
  decorator: vertical-gradient( #384562 #171f38 );
  border-color: #8a94a8;
}
/* SELECIONADO: borda + GLOW FORTE na cor do verbo (spread POSITIVO) + fundo mais vivo */
.verb.sel { border: 2dp #5fe6ff; color: #ffffff;
  decorator: vertical-gradient( #2f4a7a #16213e );
  box-shadow: #22D3EE 0dp 0dp 22dp 4dp; }
.verb.cyan.sel { border-color: #5fe6ff; box-shadow: #22D3EE 0dp 0dp 24dp 5dp; }
.verb.latao.sel { border-color: #f5c878;
  decorator: vertical-gradient( #4a3a1e #241a0c );
  box-shadow: #E8A33D 0dp 0dp 24dp 5dp; }
.verb.sel .glyph { box-shadow: #5fe6ff 0dp 0dp 12dp 2dp; }
/* ACIONADO: glow FORTE pulsante */
.verb.fired { border: 2dp #ffffff; color: #ffffff;
  animation: fire 0.9s cubic-in-out infinite-alternate; }
@keyframes fire {
  from { box-shadow: #22D3EE 0dp 0dp 16dp 2dp; }
  to   { box-shadow: #22D3EE 0dp 0dp 40dp 10dp; }
}

/* ---- LOG (terminal) ---- */
/* POLISH 2 (veredito do lider: HP bar OK, LOG reprovado): cada entrada em UMA linha SO.
   O problema anterior: entradas longas (ex. "inimigo3 ataca caua por 5.; caua ficou com
   Aceleracao") QUEBRAVAM em 2 linhas visuais cada; 2 entradas => 4 linhas + now-line = 5
   linhas, empurrando a now-line "> verbo -> alvo" pra FORA (sumia) e colando a ultima linha
   no rodape. FIX: white-space:nowrap trava cada .ln em 1 linha; overflow:hidden +
   text-overflow:ellipsis truncam com reticencias dentro da largura da coluna (inner ~228dp,
   caixa definida pelo #cockpit width 252dp). Agora e DETERMINISTICO: cap 2 entradas +
   now-line = EXATAS 3 linhas x line-height 12dp = 36dp de log. A coluna inteira
   (header+vitals+6 verbos+log+now-line) cabe em 540dp com FOLGA generosa do rodape (~70dp),
   a now-line SEMPRE visivel embaixo e nada cortado. */
/* ESPACAMENTO (veredito do lider): +18dp de folga FUGIR->log via margin-top (block nativo
   com a UA-stylesheet do RmlUi). O border-top/padding-top desenham o filete do log. */
#log { margin-top: 18dp; padding-top: 6dp; border-top: 1dp #2a3450; font-size: 10dp; }
/* Cada .ln e block nativo (UA) => as linhas do data-for empilham limpas, sem overlap inline
   (o falso strikethrough era fragmentos inline sobrepostos, resolvido de raiz pelo block).
   nowrap+overflow:hidden+ellipsis => 1 linha fixa por entrada (a now-line .ln now herda). */
#log .ln { color: #6f8593; line-height: 12dp;
  white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
#log .who { color: #22D3EE; }
#log .hit { color: #E11D74; }
#log .now { color: #cfe6ee; }

/* ---- ABERTURA: brasao GusWorld (monograma V + aneis girando + AGUARDANDO ORDEM) ---- */
#opening { text-align: center; margin-top: 90dp; }
#crest { position: relative; width: 148dp; height: 148dp;
  margin-left: 52dp; margin-bottom: 18dp; }
.ring { position: absolute; border-radius: 74dp; }
.ring.r1 { top: 0dp; left: 0dp; width: 148dp; height: 148dp;
  border: 1dp #8a94a8; box-shadow: #c9d2e044 0dp 0dp 22dp -6dp; }
/* anel aco-azulado GIRANDO (transform animado) */
.ring.r2 { top: 16dp; left: 16dp; width: 116dp; height: 116dp;
  border: 1dp #6f7c96; box-shadow: #c9d2e033 0dp 0dp 16dp -6dp;
  animation: spin 18s linear infinite; }
.ring.r3 { top: 34dp; left: 34dp; width: 80dp; height: 80dp; border: 1dp #5d6a86; }
@keyframes spin { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }
/* POLISH 2 (veredito do lider: o quadrado escuro do glifo quebrava os circulos): glifo
   Vetor-Dragao (brasao Vance) recortado em MEDALHAO circular no centro dos aneis. O PNG
   (677x369) e um plaque QUADRADO centrado num canvas landscape (padding lateral); logo
   'cover' numa caixa QUADRADA center-cropa exatamente o plaque -> o dragao fica enquadrado
   e o fundo de pedra vira o disco (medalhao aceito pelo lider). Caixa 64x64dp (quadrada,
   senao vira elipse) centrada no crest 148dp -> top=left=(148-64)/2=42, cai dentro do anel
   interno r3 (34..114, diametro 80dp) com ~8dp de folga. RmlUi NAO aceita % em border-radius
   (so comprimento): metade do box (64/2=32dp) fecha o circulo completo. IMPORTANTE: no RmlUi
   overflow:hidden recorta os FILHOS, nao o decorator do PROPRIO elemento; entao o .mono e a
   MASCARA circular (border-radius + overflow:hidden) e o .disc FILHO carrega a imagem, que
   fica recortada pela mascara. Nome flat casa a copia (write_live/baked_cockpit_rml). */
.mono { position: absolute; top: 42dp; left: 42dp; width: 64dp; height: 64dp;
  border-radius: 32dp; overflow: hidden; }
.mono .disc { width: 64dp; height: 64dp;
  decorator: image( vance_dragon_glyph.png cover ); }
#otitle { font-size: 16dp; color: #cfe6ee; }
#osub { font-size: 10dp; color: #6f8593; margin-top: 2dp; }
#ostatus { margin-top: 16dp; font-size: 11dp; color: #22D3EE; }
#ostatus .dot { display: inline-block; width: 7dp; height: 7dp; border-radius: 4dp;
  margin-right: 8dp; background-color: #22D3EE; box-shadow: #22D3EE 0dp 0dp 8dp;
  animation: blink 1.1s step-start infinite; }
@keyframes blink { from { opacity: 1; } 50% { opacity: 0.25; } to { opacity: 1; } }
</style>
</head>
<body>
  <div id="cockpit" data-model="hud">
    <!-- ABERTURA (C): brasao GusWorld. So aparece quando intro (data-if). -->
    <div id="opening" data-if="intro">
      <div id="crest">
        <div class="ring r1"></div>
        <div class="ring r2"></div>
        <div class="ring r3"></div>
        <div class="mono"><div class="disc"></div></div>
      </div>
      <div id="otitle">GUSWORLD</div>
      <div id="osub">PROTOCOLO DE COMBATE</div>
      <div id="ostatus"><span class="dot"></span>AGUARDANDO ORDEM</div>
    </div>

    <!-- COMBATE: cockpit de combate. So aparece depois de Encarar (data-if !intro). -->
    <div id="combat" data-if="!intro">
      <div id="actor">
        <div id="portrait">
          <div id="pic"></div>
          <div id="frame" class="framebox"></div>
        </div>
        <div id="name">{{nome}}</div>
        <div id="role">{{role}}</div>
      </div>

      <div id="vitals">
        <div class="hpnum">HP <span class="v">{{hp}}</span> / {{hp_max}}</div>
        <div class="hpbar"></div>
        <div class="pips">
          AP
          <span class="pip ap on"></span><span class="pip ap on"></span><span class="pip ap on"></span><span class="pip ap"></span>
        </div>
        <div class="pips">
          MANA
          <span class="pip mana on"></span><span class="pip mana on"></span><span class="pip mana"></span><span class="pip mana"></span><span class="pip mana"></span>
        </div>
      </div>

      <!-- +18dp de folga MANA->1o botao vem do margin-top do .menu (block nativo). -->
      <!-- GLINTFX-CLICK: `id` ESTAVEL por pill (gus/app/screens/battle_cockpit_verb_ids.hpp
           e a fonte unica do mapeamento id->indice; ordem = BattleVerb). SO ADITIVO: nao
           mexe em classe/estrutura/geometria - UiLayer::set_click_callback (v0.2.5) usa
           estes ids pro hit-test, substituindo a geometria manual aposentada. -->
      <div class="menu">
        <div class="verb" id="verb-scan"><span class="glyph"></span><span class="lbl">SCAN</span></div>
        <div class="verb" id="verb-gambito"><span class="glyph"></span><span class="lbl">GAMBITO</span></div>
        <div class="verb cyan sel" id="verb-atacar"><span class="glyph"></span><span class="lbl">ATACAR</span></div>
        <div class="verb" id="verb-defender"><span class="glyph"></span><span class="lbl">DEFENDER</span></div>
        <div class="verb latao" id="verb-compilar"><span class="glyph"></span><span class="lbl">COMPILAR</span></div>
        <div class="verb" id="verb-flee"><span class="glyph"></span><span class="lbl">FUGIR</span></div>
      </div>

      <!-- +18dp de folga FUGIR->log vem do margin-top do #log (block nativo). -->
      <div id="log">
        <div class="ln"><span class="who">Gus</span> compilou Vetor de Defesa.</div>
        <div class="ln">Daemon-Corrompido prepara <span class="hit">Overflow</span>.</div>
        <div class="ln now">&gt; Selecione uma acao.</div>
      </div>
    </div>
  </div>
</body>
</rml>
)RML";
    return rml;
}

}  // namespace

std::string resolve_retratos_dir() {
    return resolve_asset_dir(gus::core::assets::kRetratosDir);
}

std::string resolve_status_icons_dir() {
    return resolve_asset_dir(gus::core::assets::kStatusIconsDir);
}

std::string resolve_intent_icons_dir() {
    return resolve_asset_dir(gus::core::assets::kIntentIconsDir);
}

// ADR-010 F2b RETRATO-VIVO: nome FLAT do retrato da MOLDURA do cockpit para o ator ATIVO.
// O retrato segue o ator (motor = autoridade): inimigo -> retrato_inimigo.png (a MESMA
// cabeca generica que a fila CTB / coluna usa pros inimigos, via retrato_file_for); party
// -> o retrato do membro. Excecao do Gus: usa a versao NO-BG (recorte sem fundo) que ja
// encaixa limpa na moldura (a versao da fila tem fundo). O stage achata os caminhos, entao
// devolvemos so o nome do arquivo (resolvido contra o base-url do stage). NUNCA inventa um
// caminho novo: reusa retrato_file_for (fonte unica dos retratos da fila).
std::string cockpit_retrato_flat_for(const gus::domain::combat::CombatActor& actor) {
    if (!actor.is_player_side()) {
        return "retrato_inimigo.png";  // mesma fonte da coluna/CTB pros inimigos
    }
    if (actor.id() == "gus") {
        return "retrato_gus_combate_nobg.png";  // recorte sem fundo (encaixa na moldura)
    }
    return retrato_file_for(actor.id());  // caua/jaci/... = retrato da fila (mesma fonte)
}

// ADR-010 F1 SMOKE: escreve um RML TRIVIAL (sem assets externos, sem fonte) num tempfile e
// devolve o path. O glintfx v0.2.1 carrega por PATH (load()) e NAO resolve base-url nem
// carrega fonte default (so a v0.2.2) - por isso o smoke e um DIV com gradiente
// semitransparente + glow (decorator/box-shadow nativos), SEM texto. Prova so o compose do
// embed mode por cima da arena. Unidades em 'px' (deterministico; dp-ratio fica pra v0.2.2).
std::string write_smoke_glintfx_rml() {
    namespace fs = std::filesystem;
    const fs::path path = fs::temp_directory_path() / "gusworld_glintfx_smoke.rml";
    std::ofstream f(path);
    f << R"RML(<rml>
<head>
<style>
body { background: transparent; }
#smoke {
  position: absolute; top: 80px; left: 80px; width: 520px; height: 300px;
  decorator: vertical-gradient( #22D3EE #7C3AED );
  border-radius: 22px;
  box-shadow: #22D3EE 0px 0px 48px 8px;
  opacity: 0.80;
}
#smoke #bar {
  position: absolute; bottom: 0px; left: 0px; right: 0px; height: 64px;
  decorator: horizontal-gradient( #F59E0B #EF4444 );
  border-radius: 0px 0px 22px 22px;
}
</style>
</head>
<body>
<div id="smoke"><div id="bar"/></div>
</body>
</rml>)RML";
    return path.string();
}

// ADR-010 F2a: diretorio de STAGE dos assets do cockpit BAKED (tempfile). Por que stage:
// o glintfx carrega o doc por PATH e o RmlUi canonicaliza os caminhos RELATIVOS contra o
// DIR do documento ANTES de chamar o FileInterface (entao um base-url sozinho nao pega
// caminho ja-canonicalizado, e caminho ABSOLUTO arrisca perder a barra inicial). A receita
// robusta (= ao teste base_url_sanity do glintfx): juntar doc + assets num dir unico, com
// referencias RELATIVAS achatadas. As fontes (GusEngine/assets/) e os sprites
// (resources/sprites/icons-m5/) vivem em ARVORES diferentes; o stage os reune.
std::string glintfx_cockpit_stage_dir() {
    namespace fs = std::filesystem;
    return (fs::temp_directory_path() / "gusworld_glintfx_cockpit").string();
}

// ADR-010 F2a: produz a variante BAKED (valores ESTATICOS) do cockpit REAL pelo
// glintfx::UiLayer. REUSA o RML/RCSS autorado de load_cockpit_rml() (gradientes/glow/
// molduras/keyframes intactos) e so TRANSFORMA por string: (1) injeta @font-face (o
// UiLayer nao expoe Rml::LoadFontFace - so o doc carrega fonte); (2) remove o data-model;
// (3) troca {{bindings}} por literais de um encontro-exemplo (Gus 55/55, papel); (4) achata
// o caminho do retrato; (5) escolhe o estado (combate por padrao, ou intro/brasao). Copia
// os 4 assets (2 fontes + moldura + retrato) pro stage dir e escreve o .rml la. Devolve o
// path do .rml. NAO toca o caminho vendorizado (load_cockpit_rml fica intacto).
std::string write_baked_cockpit_rml(bool intro) {
    namespace fs = std::filesystem;
    const fs::path stage = glintfx_cockpit_stage_dir();
    std::error_code ec;
    fs::create_directories(stage, ec);

    // Copia os assets pro stage (flat). Fonte: GUSWORLD_FONTS_DIR (env GUSWORLD_FONTS tem
    // prioridade). Sprites: cockpit_asset_base_dir() (icons-m5) + retratos/.
    auto copy_into = [&](const std::string& src, const std::string& dst_name) {
        if (src.empty()) return;
        fs::copy_file(src, stage / dst_name, fs::copy_options::overwrite_existing, ec);
    };
    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) {
        if (envf[0] != '\0') fonts_dir = envf;
    }
    if (!fonts_dir.empty()) {
        copy_into(join(fonts_dir, "PixelOperatorMono.ttf"), "PixelOperatorMono.ttf");
        copy_into(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                  "PixelOperatorMono-Bold.ttf");
    }
    const std::string icons = cockpit_asset_base_dir();
    copy_into(join(icons, std::string(gus::core::assets::kMolduraCartaFrameFile)),
              "moldura_carta_frame.png");
    copy_into(join(icons, "retratos/retrato_gus_combate_nobg.png"),
              "retrato_gus_combate_nobg.png");
    // POLISH 1: glifo Vetor-Dragao (brasao da ABERTURA). Vive em resources/images/ (arvore
    // diferente dos sprites); o stage o achata pelo mesmo nome que o RCSS .mono referencia.
    copy_into(join(resolve_asset_dir(gus::core::assets::kImagesDir),
                   std::string(gus::core::assets::kVanceDragonGlyphFile)),
              "vance_dragon_glyph.png");

    // Pega o RML autorado e transforma.
    std::string rml = load_cockpit_rml();

    auto replace_all = [&rml](std::string_view from, std::string_view to) {
        std::size_t pos = 0;
        while ((pos = rml.find(from.data(), pos, from.size())) != std::string::npos) {
            rml.replace(pos, from.size(), to.data(), to.size());
            pos += to.size();
        }
    };

    // (1) @font-face logo apos <style> (o UiLayer nao expoe LoadFontFace; o doc registra a
    // familia). Caminhos RELATIVOS achatados (os .ttf foram copiados pro stage). 'src' usa
    // a sintaxe do RmlUi.
    replace_all(
        "<style>\n",
        "<style>\n"
        "@font-face { font-family: \"Pixel Operator Mono\"; "
        "src: \"PixelOperatorMono.ttf\"; }\n"
        "@font-face { font-family: \"Pixel Operator Mono\"; font-weight: bold; "
        "src: \"PixelOperatorMono-Bold.ttf\"; }\n");

    // (2) remove o data-model (nao ha binding no v0.2.2 - e por isso que bakamos).
    replace_all(" data-model=\"hud\"", "");

    // (3) {{bindings}} -> literais do encontro-exemplo (decisao do lider: Gus 55/55).
    replace_all("{{nome}}", "Gus");
    replace_all("{{role}}", "VETOR DO GAMBITO");
    replace_all("{{hp}}", "55");
    replace_all("{{hp_max}}", "55");

    // (4) achata o caminho do retrato (copiado flat pro stage).
    replace_all("retratos/retrato_gus_combate_nobg.png", "retrato_gus_combate_nobg.png");

    // (5) estado: o doc tem DOIS blocos mutuamente exclusivos via data-if (opening/combat).
    // Sem data-model nao ha data-if; entao REMOVEMOS o bloco do estado nao-desejado e
    // tiramos o atributo data-if do que fica (senao o RmlUi tenta resolver e some).
    const std::size_t open_a = rml.find("<div id=\"opening\"");
    const std::size_t comb_a = rml.find("<div id=\"combat\"");
    if (intro) {
        // INTRO (brasao): remove o bloco de combate (de #combat ate o fechamento do
        // #cockpit) e tira o data-if do #opening.
        const std::size_t cockpit_end = rml.find("\n  </div>\n</body>");
        if (comb_a != std::string::npos && cockpit_end != std::string::npos &&
            cockpit_end > comb_a) {
            rml.erase(comb_a, cockpit_end - comb_a);
        }
        replace_all(" data-if=\"intro\"", "");
    } else {
        // COMBATE (padrao): remove o bloco do brasao (de #opening ate #combat) e tira o
        // data-if do #combat.
        if (open_a != std::string::npos && comb_a != std::string::npos &&
            comb_a > open_a) {
            rml.erase(open_a, comb_a - open_a);
        }
        replace_all(" data-if=\"!intro\"", "");
    }

    const fs::path out = stage / "cockpit_baked.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// ADR-010 F2b: rotulos dos verbos do menu (ordem = BattleVerb: Scan/Gambito/Atacar/
// Defender/Compilar/Flee), em MAIUSCULA, espelhando os <span class="lbl"> do RML. Usado
// pra alimentar o binding {{verb}} (verbo SELECIONADO no motor) a cada frame. Self-contido
// na casca (o spike nao depende do Translator pra isto).
constexpr std::array<std::string_view, 6> kVerbLabels = {
    "SCAN", "GAMBITO", "ATACAR", "DEFENDER", "COMPILAR", "FUGIR"};

// ADR-010 F2b: variante LIVE (data-model) do cockpit REAL pelo glintfx::UiLayer. Diferente
// do BAKED (F2a, valores literais): aqui o RML MANTEM o data-model="hud" + os {{bindings}}
// + os data-if(intro/!intro), e a casca alimenta os valores VIVOS por frame (set_*). REUSA
// o RML/RCSS de load_cockpit_rml() (gradientes/glow/moldura/keyframes intactos) e so
// TRANSFORMA por string: (1) injeta @font-face (o UiLayer nao expoe LoadFontFace); (2)
// achata o caminho do retrato; (3) acrescenta tab-index/nav ao .verb (foco navegavel,
// secao 5 da doc de embed) + data-class-sel por verbo (a SELECAO do motor vira a classe
// .sel, motor = fonte de verdade); (4) troca o log hardcoded por data-for("line : log") +
// uma now-line com {{verb}}/{{alvo}} (verbo selecionado + alvo, vivos). Copia os 4 assets
// (2 fontes + moldura + retrato) pro stage e escreve o .rml la. Devolve o path. NAO toca
// load_cockpit_rml (o caminho vendorizado fica intacto).
std::string write_live_cockpit_rml() {
    namespace fs = std::filesystem;
    const fs::path stage = glintfx_cockpit_stage_dir();
    std::error_code ec;
    fs::create_directories(stage, ec);

    // Copia os assets pro stage (flat). Mesma receita do BAKED (stage = doc+fontes+sprites
    // num dir unico com refs RELATIVAS achatadas; ver write_baked_cockpit_rml).
    auto copy_into = [&](const std::string& src, const std::string& dst_name) {
        if (src.empty()) return;
        fs::copy_file(src, stage / dst_name, fs::copy_options::overwrite_existing, ec);
    };
    std::string fonts_dir = GUSWORLD_FONTS_DIR;
    if (const char* envf = std::getenv("GUSWORLD_FONTS")) {
        if (envf[0] != '\0') fonts_dir = envf;
    }
    if (!fonts_dir.empty()) {
        copy_into(join(fonts_dir, "PixelOperatorMono.ttf"), "PixelOperatorMono.ttf");
        copy_into(join(fonts_dir, "PixelOperatorMono-Bold.ttf"),
                  "PixelOperatorMono-Bold.ttf");
    }
    const std::string icons = cockpit_asset_base_dir();
    copy_into(join(icons, std::string(gus::core::assets::kMolduraCartaFrameFile)),
              "moldura_carta_frame.png");
    copy_into(join(icons, "retratos/retrato_gus_combate_nobg.png"),
              "retrato_gus_combate_nobg.png");
    // RETRATO-VIVO: alem do Gus (no-bg), o stage precisa dos retratos que o ator ATIVO pode
    // assumir, achatados. Inimigo = a MESMA cabeca generica da fila CTB (retrato_inimigo);
    // demais membros da party = seus retratos da fila. cockpit_retrato_flat_for() escolhe
    // qual; o data-style-decorator do #pic (abaixo) referencia o nome flat por frame.
    copy_into(join(icons, "retratos/retrato_inimigo.png"), "retrato_inimigo.png");
    copy_into(join(icons, "retratos/retrato_caua.png"), "retrato_caua.png");
    copy_into(join(icons, "retratos/retrato_jaci.png"), "retrato_jaci.png");
    // POLISH 1: glifo Vetor-Dragao (brasao da ABERTURA), de resources/images/ (arvore
    // separada). Achatado pro stage com o nome que o RCSS .mono referencia.
    copy_into(join(resolve_asset_dir(gus::core::assets::kImagesDir),
                   std::string(gus::core::assets::kVanceDragonGlyphFile)),
              "vance_dragon_glyph.png");

    std::string rml = load_cockpit_rml();

    auto replace_all = [&rml](std::string_view from, std::string_view to) {
        std::size_t pos = 0;
        while ((pos = rml.find(from.data(), pos, from.size())) != std::string::npos) {
            rml.replace(pos, from.size(), to.data(), to.size());
            pos += to.size();
        }
    };

    // (1) @font-face logo apos <style> (o UiLayer nao expoe LoadFontFace; o doc registra a
    // familia). Caminhos RELATIVOS achatados (os .ttf foram copiados pro stage).
    replace_all(
        "<style>\n",
        "<style>\n"
        "@font-face { font-family: \"Pixel Operator Mono\"; "
        "src: \"PixelOperatorMono.ttf\"; }\n"
        "@font-face { font-family: \"Pixel Operator Mono\"; font-weight: bold; "
        "src: \"PixelOperatorMono-Bold.ttf\"; }\n");

    // (2) achata o caminho do retrato (copiado flat pro stage).
    replace_all("retratos/retrato_gus_combate_nobg.png", "retrato_gus_combate_nobg.png");

    // (2b) RETRATO-VIVO: o retrato da moldura (#pic) vira DATA-DRIVEN e segue o ator ATIVO.
    // data-style-decorator monta a property 'decorator' por frame a partir do binding
    // {{retrato_src}} (nome flat do retrato, alimentado por cockpit_retrato_flat_for). Mantem
    // o fit 'cover' (encaixa sem distorcer/cropa o que sobra). O decorator ESTATICO do RCSS
    // (#pic { ... image( retrato_gus_combate_nobg.png cover ) }) fica como fallback do 1o
    // frame; o data-style o sobrescreve assim que o motor tem ator ativo. O '+' do interpretador
    // de expressao do RmlUi CONCATENA strings (AnyString -> Get<String>), provado no fonte.
    replace_all(
        "<div id=\"pic\"></div>",
        "<div id=\"pic\" data-style-decorator=\"'image( ' + retrato_src + ' cover )'\">"
        "</div>");

    // (3a) foco navegavel: o .verb vira focavel (tab-index) + navegavel por setas (nav:auto,
    // shorthand de nav-up/right/down/left). O glintfx ja roteia Key->foco; aqui o DOC declara
    // os elementos focaveis (secao 5 da doc de embed). SEM estilo :focus de proposito: a
    // selecao VISIVEL e a classe .sel dirigida pelo MOTOR (data-class-sel abaixo), pra NAO
    // criar uma 2a fonte de verdade (o foco do glintfx fica como navegacao inerte).
    replace_all("  color: #d6e6ef; font-size: 11dp;\n}",
                "  color: #d6e6ef; font-size: 11dp;\n  tab-index: auto; nav: auto;\n}");

    // (3b) menu de verbos data-driven: a classe .sel de cada verbo segue o indice SELECIONADO
    // no motor (binding 'sel'), via data-class-sel="sel == N". O motor (scene.menu_) e a
    // fonte de verdade da selecao; aqui so REFLETIMOS. (ordem N = BattleVerb).
    // GLINTFX-CLICK: a string de origem casa a saida ATUAL de load_cockpit_rml() (que ja
    // carrega o `id="verb-*"` estavel de cada pill, ver F1 acima); o destino PRESERVA esses
    // ids (o click_callback do UiLayer os usa pro hit-test) e SO ACRESCENTA data-class-sel.
    replace_all(
        "      <div class=\"menu\">\n"
        "        <div class=\"verb\" id=\"verb-scan\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">SCAN</span></div>\n"
        "        <div class=\"verb\" id=\"verb-gambito\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">GAMBITO</span></div>\n"
        "        <div class=\"verb cyan sel\" id=\"verb-atacar\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">ATACAR</span></div>\n"
        "        <div class=\"verb\" id=\"verb-defender\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">DEFENDER</span></div>\n"
        "        <div class=\"verb latao\" id=\"verb-compilar\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">COMPILAR</span></div>\n"
        "        <div class=\"verb\" id=\"verb-flee\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">FUGIR</span></div>\n"
        "      </div>",
        "      <div class=\"menu\">\n"
        "        <div class=\"verb\" id=\"verb-scan\" data-class-sel=\"sel == 0\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">SCAN</span></div>\n"
        "        <div class=\"verb\" id=\"verb-gambito\" data-class-sel=\"sel == 1\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">GAMBITO</span></div>\n"
        "        <div class=\"verb cyan\" id=\"verb-atacar\" data-class-sel=\"sel == 2\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">ATACAR</span></div>\n"
        "        <div class=\"verb\" id=\"verb-defender\" data-class-sel=\"sel == 3\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">DEFENDER</span></div>\n"
        "        <div class=\"verb latao\" id=\"verb-compilar\" data-class-sel=\"sel == "
        "4\"><span class=\"glyph\"></span><span class=\"lbl\">COMPILAR</span></div>\n"
        "        <div class=\"verb\" id=\"verb-flee\" data-class-sel=\"sel == 5\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">FUGIR</span></div>\n"
        "      </div>");

    // (4) log VIVO: troca as 3 linhas hardcoded por um data-for sobre a lista 'log'
    // (scene.log_lines, ultimas N) + uma now-line com o verbo SELECIONADO e o ALVO.
    replace_all(
        "      <div id=\"log\">\n"
        "        <div class=\"ln\"><span class=\"who\">Gus</span> compilou Vetor de "
        "Defesa.</div>\n"
        "        <div class=\"ln\">Daemon-Corrompido prepara <span "
        "class=\"hit\">Overflow</span>.</div>\n"
        "        <div class=\"ln now\">&gt; Selecione uma acao.</div>\n"
        "      </div>",
        "      <div id=\"log\">\n"
        "        <div class=\"ln\" data-for=\"line : log\">{{line}}</div>\n"
        "        <div class=\"ln now\">&gt; {{verb}} -&gt; {{alvo}}</div>\n"
        "      </div>");

    const fs::path out = stage / "cockpit_live.rml";
    std::ofstream f(out);
    f << rml;
    return out.string();
}

// ADR-010 F1 SMOKE: ponte SDL_Event -> glintfx::UiEvent (mapa de design da doc de prep).
// Retorna false p/ eventos sem mapeamento (nao injeta ruido). Cobre mouse-move/button,
// teclas de navegacao, texto e resize (pixels reais via SDL_GetWindowSizeInPixels). 'Type'
// e enum class (scoped) na v0.2.1 -> UiEvent::Type::*.
bool sdl_to_glintfx(const SDL_Event& ev, SDL_Window* window, glintfx::UiEvent* out) {
    using K = glintfx::Key;
    using T = glintfx::UiEvent::Type;
    glintfx::UiEvent e{};
    switch (ev.type) {
        case SDL_EVENT_MOUSE_MOTION:
            e.type = T::MouseMove;
            e.x = ev.motion.x;
            e.y = ev.motion.y;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            e.type = T::MouseButton;
            e.x = ev.button.x;
            e.y = ev.button.y;
            e.pressed = (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
            e.button = ev.button.button == SDL_BUTTON_RIGHT    ? 1
                       : ev.button.button == SDL_BUTTON_MIDDLE ? 2
                                                               : 0;
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            e.type = T::Key;
            e.pressed = (ev.type == SDL_EVENT_KEY_DOWN);
            e.modifiers = ((ev.key.mod & SDL_KMOD_SHIFT) ? glintfx::Mod_Shift : 0) |
                          ((ev.key.mod & SDL_KMOD_CTRL) ? glintfx::Mod_Ctrl : 0) |
                          ((ev.key.mod & SDL_KMOD_ALT) ? glintfx::Mod_Alt : 0);
            switch (ev.key.key) {
                case SDLK_UP: e.key = K::Up; break;
                case SDLK_DOWN: e.key = K::Down; break;
                case SDLK_LEFT: e.key = K::Left; break;
                case SDLK_RIGHT: e.key = K::Right; break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER: e.key = K::Enter; break;
                case SDLK_ESCAPE: e.key = K::Escape; break;
                case SDLK_TAB: e.key = K::Tab; break;
                case SDLK_SPACE: e.key = K::Space; break;
                case SDLK_BACKSPACE: e.key = K::Backspace; break;
                default: e.key = K::None; break;
            }
            break;
        case SDL_EVENT_TEXT_INPUT:
            e.type = T::Text;
            e.text = ev.text.text;  // valido so no escopo do evento; process_event copia
            break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            int pw = 0, ph = 0;
            SDL_GetWindowSizeInPixels(window, &pw, &ph);
            e.type = T::Resize;
            e.width = pw;
            e.height = ph;
            break;
        }
        default:
            return false;  // sem mapeamento -> nao injeta
    }
    *out = e;
    return true;
}

// GLINTFX-CLICK (v0.2.5): aciona o verbo do PILL cujo `id` RCSS (fonte unica em
// gus/app/screens/battle_cockpit_verb_ids.hpp) bate. Wired como o callback de
// glintfx::UiLayer::set_click_callback (ver setup do loop abaixo) -- o GLINTFX faz o
// hit-test ele mesmo (o MESMO que ja move o :hover nativo) e devolve so o `id`; aqui so
// traduzimos esse id pra acao do motor (BattleScene). Extraida em funcao-livre (nao inline
// no lambda do callback) pra ser chamavel DIRETO pelo self-test sintetico
// (GUSWORLD_BATTLE_MOUSE_SELFTEST) sem precisar simular pixel/evento SDL algum -- o
// glintfx ja resolveu o pixel; o self-test so precisa saber que id->verbo bate.
//
// GUARDA: replica, byte a byte, a MESMA ordem de prioridade que battle_mouse_click usa pro
// resto do cockpit (escolha-de-ator > mira > menu). O motivo: os pills SEGUEM no DOM
// (RmlUi data-if="!intro") mesmo durante a escolha de ator ou a mira -- um clique sobre a
// coluna do cockpit nesses estados dispararia este callback EM PARALELO ao hit-test de
// mundo/arena de battle_mouse_click (sao dois listeners INDEPENDENTES, nao mutuamente
// exclusivos como a cadeia if/else antiga). Sem esta guarda, clicar sobre o cockpit
// enquanto mira/escolhe ator selecionaria um verbo por baixo -- regressao. Na ABERTURA
// (is_intro()) o bloco #combat inteiro (pills inclusos) nem existe no DOM -- o id nunca
// bateria mesmo sem a guarda, mas ela fica explicita por clareza/defesa-em-profundidade.
void battle_cockpit_verb_click(BattleScene& scene, const char* element_id) {
    if (scene.is_choosing_actor() || scene.is_aiming() || scene.is_intro()) {
        return;
    }
    const int idx = gus::app::screens::cockpit_verb_index_for_click_id(element_id);
    if (idx < 0) {
        return;  // id de outro elemento do cockpit (#combat/#vitals/#log/...) ou "" -> NO-OP
    }
    // Clique = SELECIONA e CONFIRMA. menu_move (delta ate o indice) + menu_confirm; ambos
    // ja sao NO-OP fora do turno do jogador (mesma guarda do teclado) -> seguro em turno
    // de inimigo/combate acabado. menu_move faz WRAP, mas o delta idx-sel (ambos 0..5) cai
    // exato no indice. menu_confirm respeita 'enabled' (verbo sem AP: seleciona, nao aciona).
    scene.menu_move(idx - scene.menu().selected_index());
    scene.menu_confirm();
}

// ADR-010 / Incremento A2 (MOUSE), revisado GLINTFX-CLICK: hit-tests de MUNDO/ARENA
// (escolha de ator + mira de alvo) resolvidos AQUI, no host, em coordenadas de MUNDO
// (a projecao ESTICA 960x540 pra pw x ph, NAO-uniforme -- ver viewport_transform
// world_to_screen -> world_x = px/pw*960, world_y = px/ph*540, Y por ph!). O hit-test vem
// do motor de cena (actor_pick_index_at_arena / aim_index_at_arena, casam
// arena_rect_for_actor). O clique nos PILLS DE VERBO NAO passa mais por aqui -- resolvido
// pelo callback do glintfx (battle_cockpit_verb_click acima), que roda em paralelo a esta
// funcao pro MESMO evento SDL (ver o loop de eventos: ambos os caminhos recebem o clique).
// Pressuposto: mouse em px de janela == px do viewport (sem HiDPI neste alvo; MESMO pressuposto
// do forward glintfx). Se houver escala HiDPI no futuro, converter mouse(pontos)->px antes.
void battle_mouse_click(BattleScene& scene, float mx, float my, int pw, int ph) {
    if (pw < 1 || ph < 1) {
        return;
    }
    if (scene.is_choosing_actor()) {
        // ESCOLHA DE ATOR (§4.1): clique num SLOT da party. MESMA conversao MUNDO/arena do modo-
        // mira (estica 960x540; Y por ph) -> reusa o hit-test do motor (actor_pick_index_at_arena,
        // que casa arena_rect_for_actor dos slots da party). Clique unico = ESCOLHE e CONFIRMA o
        // membro (inicia o turno dele), a mesma filosofia "aciona na hora" do A2.
        const float wx = mx / static_cast<float>(pw) * 960.0f;
        const float wy = my / static_cast<float>(ph) * 540.0f;
        const int idx = scene.actor_pick_index_at_arena(wx, wy);
        if (idx >= 0) {
            scene.actor_picker_select(idx);   // poe o cursor no membro clicado
            scene.actor_picker_confirm();      // clique unico = escolhe E inicia o turno dele
        }
        // Fora de qualquer membro elegivel: NO-OP (o picker precede o menu; nao ha o que cancelar).
        return;
    }
    if (scene.is_aiming()) {
        // Clique num INIMIGO (mira): coordenadas de MUNDO da arena (estica; Y por ph).
        const float wx = mx / static_cast<float>(pw) * 960.0f;
        const float wy = my / static_cast<float>(ph) * 540.0f;
        const int idx = scene.aim_index_at_arena(wx, wy);
        if (idx >= 0) {
            scene.aim_select(idx);  // pousa a mira no alvo clicado
            scene.aim_confirm();    // clique unico = mira E confirma (aciona), igual ao verbo
        }
        // Fora de qualquer inimigo: NO-OP (nao cancela, nao "erra" o alvo) -- escopo A2.
        return;
    }
    // Fora de escolha-de-ator/mira: nao ha mais hit-test de MUNDO a fazer aqui (a ABERTURA
    // tambem nao tem nada clicavel em coordenadas de mundo). O clique nos pills de verbo e'
    // resolvido pelo callback do glintfx (battle_cockpit_verb_click), nao por esta funcao.
}

// Incremento A2 (HOVER, nice-to-have): SO o inimigo. Durante a mira, mover o mouse sobre um
// inimigo PRE-SELECIONA ele (reusa o realce multimodal do alvo do Incremento A, dirigido por
// aim_target()). ZERO risco: nao toca RCSS nem o estado do glintfx. Pill NAO tem hover: o
// realce .sel do RCSS e dirigido pelo MOTOR (data-class-sel); um :hover exigiria mexer na RCSS
// aprovada do cockpit -> fora do escopo A2 (documentado; o CLIQUE no pill ja funciona).
void battle_mouse_hover(BattleScene& scene, float mx, float my, int pw, int ph) {
    if (pw < 1 || ph < 1) {
        return;
    }
    // ESCOLHA DE ATOR (§4.1): passar o mouse sobre um SLOT elegivel da party PRE-SELECIONA ele
    // (move o cursor do picker, SEM confirmar) - o analogo do hover de mira. Checado ANTES da
    // mira (quando is_choosing_actor(), a mira nem existe). MESMA conversao MUNDO/arena.
    if (scene.is_choosing_actor()) {
        const float wx = mx / static_cast<float>(pw) * 960.0f;
        const float wy = my / static_cast<float>(ph) * 540.0f;
        const int idx = scene.actor_pick_index_at_arena(wx, wy);
        if (idx >= 0) {
            scene.actor_picker_select(idx);  // hover destaca (move o cursor, SEM confirmar)
        }
        return;
    }
    if (!scene.is_aiming()) {
        return;
    }
    const float wx = mx / static_cast<float>(pw) * 960.0f;
    const float wy = my / static_cast<float>(ph) * 540.0f;
    const int idx = scene.aim_index_at_arena(wx, wy);
    if (idx >= 0) {
        scene.aim_select(idx);  // hover destaca (move o cursor de mira, SEM confirmar)
    }
}

// Roteamento de TECLADO do host, EXTRAIDO do loop de eventos pra ser CHAMAVEL pelo self-test
// sintetico (espelha battle_mouse_click, que ja e uma funcao-livre testavel). MESMA ordem e
// semantica de antes; ADITIVO: a ESCOLHA DE ATOR (§4.1) ganha PRIORIDADE MAXIMA sobre mira/menu,
// porque quando is_choosing_actor() o menu de verbos nem existe (begin_turn deferido). `running`
// so vira false no Esc de TOPO (fora de qualquer sub-modo).
// Digito 1-9 de uma tecla numerica (fileira OU numpad); 0 se nao for numerica 1-9. Fonte
// unica do mapeamento tecla->N pros atalhos numericos (mira e escolha de ator).
int battle_digit_for_key(SDL_Keycode key) noexcept {
    switch (key) {
        case SDLK_1: case SDLK_KP_1: return 1;
        case SDLK_2: case SDLK_KP_2: return 2;
        case SDLK_3: case SDLK_KP_3: return 3;
        case SDLK_4: case SDLK_KP_4: return 4;
        case SDLK_5: case SDLK_KP_5: return 5;
        case SDLK_6: case SDLK_KP_6: return 6;
        case SDLK_7: case SDLK_KP_7: return 7;
        case SDLK_8: case SDLK_KP_8: return 8;
        case SDLK_9: case SDLK_KP_9: return 9;
        default: return 0;
    }
}

void battle_key_down(BattleScene& scene, SDL_Keycode key, bool& running) {
    // TECLAS-ATALHO NUMERICAS (1-9, fileira + numpad). PRIORIDADE: MODO-MIRA (§3.5) > ESCOLHA
    // DE ATOR (§4.1). Os dois modos nunca sao simultaneos (a mira so abre no menu de verbos,
    // ja fora do picker), mas a ordem deixa explicito: mirando, N mira+confirma o N-esimo
    // inimigo (aim_hotkey); escolhendo ator, N escolhe+confirma o N-esimo membro
    // (actor_picker_hotkey). Ambos ja sao NO-OP fora do seu modo / fora de faixa (guarda
    // interna) -> mapeamento seguro. Consumidas aqui (nao caem no switch abaixo).
    if (const int nth = battle_digit_for_key(key); nth != 0) {
        if (scene.is_aiming()) {
            scene.aim_hotkey(nth);
        } else if (scene.is_choosing_actor()) {
            scene.actor_picker_hotkey(nth);
        }
        return;
    }
    switch (key) {
        case SDLK_ESCAPE:
            // MODO-MIRA (§3.5): Esc CANCELA a mira (volta ao menu sem consumir o turno). Na
            // ESCOLHA DE ATOR (§4.1) NAO ha cancel (o picker precede o menu: sem AP gasto, sem
            // verbo a desfazer) -> Esc cai no generico (sai do preview), igual ao menu de verbos.
            // Fora de tudo: Esc sai do preview.
            if (scene.is_aiming()) {
                scene.aim_cancel();
            } else {
                running = false;
            }
            break;
        // Navegacao vertical. PRIORIDADE: escolha de ator (§4.1) > mira (§3.5) > menu de verbos.
        // A party e uma COLUNA vertical na arena -> UP/DOWN move o cursor do picker (com WRAP),
        // a mesma linguagem da mira/menu.
        case SDLK_UP:
        case SDLK_W:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(-1);
            } else if (scene.is_aiming()) {
                scene.aim_move(-1);
            } else {
                scene.menu_move(-1);
            }
            break;
        case SDLK_DOWN:
        case SDLK_S:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(+1);
            } else if (scene.is_aiming()) {
                scene.aim_move(+1);
            } else {
                scene.menu_move(+1);
            }
            break;
        // LEFT/RIGHT: aliases horizontais de navegacao na escolha de ator (§4.1) e na mira
        // (§3.5). Fora desses modos, sem efeito (o menu de verbos e vertical).
        case SDLK_LEFT:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(-1);
            } else if (scene.is_aiming()) {
                scene.aim_move(-1);
            }
            break;
        case SDLK_RIGHT:
            if (scene.is_choosing_actor()) {
                scene.actor_picker_move(+1);
            } else if (scene.is_aiming()) {
                scene.aim_move(+1);
            }
            break;
        // (As teclas-atalho numericas 1-9 sao tratadas no TOPO desta funcao, com prioridade
        // mira > escolha-de-ator, antes deste switch.)
        case SDLK_RETURN:
        case SDLK_KP_ENTER:  // Enter do numpad tambem confirma
        case SDLK_SPACE:
            // ABERTURA (lider 2026-06-25): na tela "BATALHA!" parada, Enter ENCARA. Depois, por
            // PRIORIDADE: ESCOLHA DE ATOR (§4.1) confirma o membro sob o cursor -> inicia o turno
            // dele; MODO-MIRA (§3.5) confirma o ALVO (resolve); vez do jogador (menu) confirma o
            // verbo (Atacar/Scan ENTRAM na mira). Fora disso, ACELERA o ritmo.
            if (scene.is_intro()) {
                scene.start_combat();  // Encarar
            } else if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();  // confirma o membro escolhido -> comeca o turno
            } else if (scene.is_aiming()) {
                scene.aim_confirm();  // confirma o alvo mirado
            } else if (scene.waiting_player_input()) {
                scene.menu_confirm();  // Atacar/Scan -> abre a mira
            } else {
                scene.skip();
            }
            break;
        case SDLK_Q:
            // "[Q] Resolver sem encarar" (verbo OPT-IN, so TRASH na abertura). Placeholder
            // neste incremento: a cena loga "[auto-resolve: a implementar]".
            scene.request_auto_resolve();
            break;
        default:
            break;
    }
}

int run_battle_preview() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "BattlePreview: SDL_Init falhou: " << SDL_GetError() << "\n";
        return 1;
    }

    // ADR-009 adendo GL3: a janela usa contexto OpenGL 3.3 core (nao SDL_Renderer), pois o
    // HUD RmlUi-GL3 precisa de shaders (gradiente/box-shadow/blur). A arena (Render2dGl3) e
    // o HUD compartilham o MESMO contexto GL; swap unico (SDL_GL_SwapWindow).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);  // o GL3 do RmlUi usa stencil (clip mask)

    // FIX W1 item 2 (lider: "maximizada, a base da cena desliza pra tras da barra de
    // tarefas"): a janela INICIAL deve caber na AREA UTIL do desktop (que desconta a barra
    // de tarefas/paineis via struts), senao a base (log/rodape do cockpit) nasce escondida.
    // SDL_GetDisplayUsableBounds ja desconta os paineis. Preservamos a proporcao 16:9 (a
    // arena estica 960x540 -> janela; 16:9 = sem distorcao) escolhendo a MAIOR janela 16:9
    // que cabe na area util (com margem p/ a decoracao da janela), limitada ao alvo
    // 1920x1080. Sob Xvfb/headless (sem barra) os usable bounds = display inteiro -> escala
    // 1.0 -> janela 1920x1080 como antes (self-tests de mouse/hover intactos: as coordenadas
    // derivam de pw0/ph0 REAIS da janela, nao de constantes). NAO ha offset de letterbox
    // aqui -> os hit-tests de mouse (A2/picker) seguem validos sem desconto.
    int win_w = kWindowW, win_h = kWindowH;
    {
        SDL_Rect usable{};
        const SDL_DisplayID disp = SDL_GetPrimaryDisplay();
        if (disp != 0 && SDL_GetDisplayUsableBounds(disp, &usable) && usable.w > 0 &&
            usable.h > 0) {
            constexpr float kMargin = 0.95f;  // folga p/ bordas/titlebar da janela
            const float avail_w = static_cast<float>(usable.w) * kMargin;
            const float avail_h = static_cast<float>(usable.h) * kMargin;
            float scale = 1.0f;
            scale = std::min(scale, avail_w / static_cast<float>(kWindowW));
            scale = std::min(scale, avail_h / static_cast<float>(kWindowH));
            if (scale < 1.0f) {
                win_w = static_cast<int>(static_cast<float>(kWindowW) * scale);
                win_h = static_cast<int>(static_cast<float>(kWindowH) * scale);
            }
            std::cout << "BattlePreview: [win] area util=" << usable.w << "x" << usable.h
                      << " -> janela inicial=" << win_w << "x" << win_h << " (16:9)\n";
        }
    }

    SDL_Window* window =
        SDL_CreateWindow("GusWorld BattlePreview (M5, GL3)", win_w, win_h,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        std::cerr << "BattlePreview: SDL_CreateWindow falhou: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (gl == nullptr) {
        std::cerr << "BattlePreview: SDL_GL_CreateContext falhou: " << SDL_GetError()
                  << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);  // VSync

    // Carrega os ponteiros de funcao GL (glad, via o backend GL3 do RmlUi). PRECISA vir
    // depois do contexto corrente e ANTES de qualquer chamada GL (Render2dGl3/RmlUiHud).
    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        std::cerr << "BattlePreview: falha ao carregar funcoes OpenGL (glad)\n";
        SDL_GL_DestroyContext(gl);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // DIAGNOSTICO DE EFEITOS (ADR-009 #1): GUSWORLD_RMLUI_FXTEST=1 carrega um doc com
    // efeitos MAXIMAMENTE OBVIOS (gradiente vermelho->azul, box-shadow magenta forte) e
    // renderiza SO o HUD (sem arena, fundo preto), pra provar SE os shaders de
    // gradiente/box-shadow chegam a tela neste setup GL. Se aparecerem aqui, o problema
    // sao os VALORES (sutis) ou a composicao com a arena; se NAO, e o pipeline GL3.
    const bool fxtest = [] {
        const char* e = std::getenv("GUSWORLD_RMLUI_FXTEST");
        return e != nullptr && e[0] == '1';
    }();

    {
        gus::platform::render2d::Render2dGl3 renderer(/*gl_active=*/true);

        // ====================================================================
        // ADR-010 F3: glintfx::UiLayer (embed mode) e o UNICO motor de UI/HUD, compondo
        // POR CIMA da arena com efeitos nativos (gradiente/box-shadow/glow). A arena desenha
        // primeiro (backbuffer), o glintfx compoe por cima (layer -> backbuffer), o swap e
        // unico (SDL_GL_SwapWindow). Opt-out (debug so-arena): GUSWORLD_RMLUI_OFF=1.
        // ====================================================================
        const bool rmlui_opt_out = [] {
            const char* e = std::getenv("GUSWORLD_RMLUI_OFF");
            return e != nullptr && e[0] == '1';
        }();
        int pw0 = kWindowW, ph0 = kWindowH;
        SDL_GetWindowSizeInPixels(window, &pw0, &ph0);
        {
            int ww = 0, wh = 0;
            SDL_GetWindowSize(window, &ww, &wh);
            std::cout << "BattlePreview: [scale] janela logica=" << ww << "x" << wh
                      << " pixels=" << pw0 << "x" << ph0
                      << " dp_ratio=" << (static_cast<float>(pw0) / 960.0f) << "\n";
        }

        // ====================================================================
        // ADR-010 F3: glintfx::UiLayer (embed mode) - UNICO motor de UI/HUD.
        // Anexa ao contexto GL JA corrente (SDL_GL_MakeCurrent acima); compose-only no
        // loop (sem clear, sem swap), no slot do hud.compose(). load_gl=true: o glintfx usa
        // gl3w (tabela de ponteiros PROPRIA, independente do glad que o GusEngine carregou
        // em gl3_load_functions); por isso PRECISA carregar a sua (false deixaria os
        // ponteiros gl3w NULL -> crash). Ver R-glad-owner no relatorio.
        // ====================================================================
        bool glintfx_on = false;
        std::optional<glintfx::UiLayer> ui;
        bool glintfx_live = false;  // F2b: cockpit dirigido por data-model (valores vivos)
        // F2b DEBUG: GUSWORLD_GLINTFX_DP=<float> FORCA o dp_ratio (em vez de pw/960). Util
        // pra INSPECIONAR a coluna inteira do cockpit num shot: o canvas e autorado ~625dp
        // de alto e o backbuffer (~1006px) so mostra ~503dp a dp_ratio=2 (a base do menu +
        // o log caem abaixo da dobra). dp_ratio menor (ex. 1.5 => mostra ~670dp) revela o
        // log/now-line. 0/ausente => comportamento normal (pw/960). Nao altera o motor.
        const float glintfx_dp_override = [] {
            const char* e = std::getenv("GUSWORLD_GLINTFX_DP");
            return (e != nullptr && e[0] != '\0') ? static_cast<float>(std::atof(e)) : 0.0f;
        }();
        if (!rmlui_opt_out) {
            // ADR-010 F2b (DEFAULT): cockpit REAL pelo UiLayer dirigido por DATA-MODEL
            // (valores VIVOS por frame: HP/nome/role/selecao-de-verbo/log). RCSS autorado em
            // 'dp' num canvas logico 960x540; o dp_ratio (= pixels reais / 960) escala pro
            // backbuffer. Modos de debug por env:
            //   GUSWORLD_GLINTFX_SMOKE=1 -> smoke trivial (compose puro, sem assets).
            //   GUSWORLD_GLINTFX_BAKED=1 -> cockpit F2a com valores LITERAIS (sem binding).
            //   GUSWORLD_GLINTFX_INTRO=1 -> (so no BAKED) baka o brasao no lugar do combate.
            const bool glintfx_smoke = [] {
                const char* e = std::getenv("GUSWORLD_GLINTFX_SMOKE");
                return e != nullptr && e[0] == '1';
            }();
            const bool glintfx_baked = [] {
                const char* e = std::getenv("GUSWORLD_GLINTFX_BAKED");
                return e != nullptr && e[0] == '1';
            }();
            const bool glintfx_intro = [] {
                const char* e = std::getenv("GUSWORLD_GLINTFX_INTRO");
                return e != nullptr && e[0] == '1';
            }();
            const float dp_ratio = glintfx_dp_override > 0.0f
                                       ? glintfx_dp_override
                                       : static_cast<float>(pw0) / 960.0f;
            ui.emplace(glintfx::UiLayer::Config{/*logical_width=*/960,
                                                /*logical_height=*/540,
                                                /*load_gl=*/true,
                                                /*dp_ratio=*/dp_ratio});
            if (ui->ok()) {
                std::string rml_path;
                std::string base_url;
                if (glintfx_smoke) {
                    rml_path = write_smoke_glintfx_rml();  // px puros, sem assets/base-url
                } else if (glintfx_baked) {
                    rml_path = write_baked_cockpit_rml(glintfx_intro);
                    base_url = glintfx_cockpit_stage_dir();  // dir com doc+fontes+sprites
                } else {
                    // LIVE: cria o data-model + LIGA os bindings ANTES do load (ordem
                    // obrigatoria, secao 6 da doc de embed: create_data_model -> bind_* ->
                    // load -> set_*). As views de data-binding sao compiladas no load; bind_*
                    // apos o load retornaria false. Initials cobrem o 1o frame (intro=true);
                    // os set_* vivos vem no loop (a cena so existe depois deste bloco).
                    glintfx_live = true;
                    ui->create_data_model("hud");
                    ui->bind_bool("intro", true);
                    ui->bind_string("nome", "Gus");
                    ui->bind_string("role", "VETOR DO GAMBITO");
                    ui->bind_number("hp", 55);
                    ui->bind_number("hp_max", 55);
                    ui->bind_number("sel", 2);  // Atacar (default do BattleMenu)
                    ui->bind_string("verb", "ATACAR");
                    ui->bind_string("alvo", "-");
                    // RETRATO-VIVO: caminho flat do retrato da moldura. Inicial = Gus no-bg
                    // (1o frame/intro); o alimentador troca por frame conforme o ator ativo.
                    ui->bind_string("retrato_src", "retrato_gus_combate_nobg.png");
                    ui->bind_list("log");
                    rml_path = write_live_cockpit_rml();
                    base_url = glintfx_cockpit_stage_dir();  // dir com doc+fontes+sprites
                }
                // base-url ANTES do load (pra o doc tambem resolver via ele). Caminhos
                // relativos -> base_url/path; absolutos passam direto.
                if (!base_url.empty()) {
                    ui->set_asset_base_url(base_url.c_str());
                }
                ui->load(rml_path.c_str());
                ui->set_viewport(pw0, ph0);    // pixels reais do backbuffer
                ui->set_dp_ratio(dp_ratio);    // logico 960x540 -> pixels reais
                glintfx_on = true;
                std::cout << "BattlePreview: [glintfx] UiLayer ATIVO (embed, load_gl=true) "
                          << (glintfx_smoke   ? "[SMOKE]"
                              : glintfx_baked ? (glintfx_intro
                                                     ? "[cockpit BAKED: intro/brasao]"
                                                     : "[cockpit BAKED: combate]")
                                              : "[cockpit LIVE: data-model]")
                          << " viewport=" << pw0 << "x" << ph0
                          << " dp_ratio=" << dp_ratio << " RML=" << rml_path << "\n";
            } else {
                std::cerr << "BattlePreview: [glintfx] UiLayer::ok()=false (attach falhou) "
                             "- caindo SEM UI neste run\n";
                ui.reset();
            }
        }

        // AUDIO (M6 F3, ADR-011): o AudioEngine e DONO da CASCA (aqui, mais longeva que a
        // BattleScene - re-entradas futuras recriam a cena sem recriar o device nem
        // redecodificar o SFX). device_active=true tenta o hardware real; falha degrada
        // graciosa (available()==false, API vira no-op - a cena roda muda, nunca crasha
        // por falta de placa de som). load_sfx UMA vez aqui (NUNCA no frame do contato -
        // decodificar e caro, o motivo de load_sfx existir separado de play_sfx).
        gus::platform::audio::AudioEngine audio_engine(/*device_active=*/true);
        const std::string hit_sfx_path = resolve_hit_sfx_path();
        const gus::platform::audio::SoundId hit_sfx_id =
            audio_engine.load_sfx(hit_sfx_path.c_str());
        std::cout << "BattlePreview: [audio] device "
                  << (audio_engine.available() ? "disponivel" : "INDISPONIVEL (mudo)")
                  << " - SFX de hit "
                  << (hit_sfx_id != gus::platform::audio::kInvalidSound
                          ? "carregado"
                          : "AUSENTE (silencioso)")
                  << " de " << hit_sfx_path << "\n";

        // MUSICA (M6 F4, ADR-011): mesmo AudioEngine dono da casca, mesmo padrao de
        // pre-load do SFX (load_music UMA vez aqui - o stream real so comeca em
        // play_music). Toca em LOOP com FADE-IN ao ENTRAR na batalha - este viewer
        // (--battle) E a batalha; e o ponto de "entrada" que este fluxo isolado permite
        // exercitar (ver nota de escopo do fade-out, no fim do loop, sobre o crossfade
        // tela-a-tela completo). MA_SOUND_FLAG_STREAM + looping nativo (audio_engine.cpp)
        // garante loop SEM GAP (o miniaudio reinicia o stream internamente).
        //
        // NOTA HONESTA (kCityThemeFile, ver comentario em asset_paths.hpp): e um tema de
        // CIDADE tocando na BATALHA porque e a UNICA faixa do kit CC0 provisorio (F2) -
        // serve pra PROVAR loop+fade tecnicamente, NAO pra vender o feel de combate. NAO
        // mudar o timbre/curadoria aqui (fora de escopo desta fase, ADR-011).
        const std::string music_path = resolve_music_path();
        const gus::platform::audio::SoundId music_id =
            audio_engine.load_music(music_path.c_str());
        constexpr float kMusicFadeInSeconds = 2.0f;
        audio_engine.play_music(music_id, /*loop=*/true, kMusicFadeInSeconds);
        std::cout << "BattlePreview: [audio] musica "
                  << (music_id != gus::platform::audio::kInvalidSound
                          ? "carregada (loop, fade-in " +
                                std::to_string(kMusicFadeInSeconds) + "s)"
                          : "AUSENTE (silenciosa)")
                  << " de " << music_path << "\n";

        // A cena monta o encontro de demo e ja le a fila do motor.
        BattleScene scene;
        // Ponteiro NAO-DONO (mesmo padrao de set_translator/set_portraits): a cena so
        // dispara play_sfx no evento de CONTATO do golpe (F3) - nunca decodifica, nunca
        // possui o engine. hit_sfx_id pode ser kInvalidSound (asset ausente/device
        // indisponivel) - play_sfx() ja degrada com seguranca nesse caso.
        scene.set_audio(&audio_engine, hit_sfx_id);
        // (A) Com o HUD externo (glintfx::UiLayer) ATIVO, a cena NAO desenha o cockpit/log a
        // mao - so arena/banner/floaters/fila. Evita cockpits sobrepostos.
        scene.set_hud_external(glintfx_on);

        // GLINTFX-CLICK (v0.2.5): registra o callback de clique da UI - o glintfx faz o
        // hit-test ele mesmo (o MESMO que ja move o :hover) e devolve o `id` do elemento
        // clicado; battle_cockpit_verb_click traduz esse id pra acao do motor (ver defs
        // acima). Sem restricao de ordem vs load() (a doc do glintfx garante isso) - registra
        // aqui, DEPOIS da BattleScene existir, porque o callback CAPTURA `scene` por
        // referencia (nao dava pra registrar antes, no bloco de setup do UiLayer, onde a
        // cena ainda nao existia). `scene` sobrevive ate o fim deste escopo, junto com `ui`.
        if (glintfx_on && ui) {
            ui->set_click_callback([&scene](const char* element_id) {
                battle_cockpit_verb_click(scene, element_id);
            });
        }

        // Carrega os retratos 48px da fila CTB (handles resolvidos pelo renderer) e os
        // entrega a cena. Cada id de ator -> seu retrato; ausencia degrada pro retangulo.
        const std::string dir = resolve_retratos_dir();
        BattlePortraitSet portraits;
        for (const auto* actor : scene.machine().queue().order()) {
            if (actor == nullptr) {
                continue;
            }
            const std::string path = join(dir, retrato_file_for(actor->id()));
            const gus::platform::render2d::TextureId tex =
                renderer.load_texture(path.c_str());
            portraits.by_id.emplace_back(actor->id(), tex);
        }
        scene.set_portraits(std::move(portraits));

        // SPRITE SET do GUS (W3): frames de anims/ resolvidos em TextureId e entregues
        // a cena (mesmo padrao dos retratos). Ausencia (headless/sem assets) degrada
        // pro retrato placeholder - a cena decide por ator.
        if (auto gus_set = load_gus_sprite_set(renderer)) {
            int nframes = 0;
            for (const auto& c : gus_set->clips) {
                nframes += static_cast<int>(c.frames.size());
            }
            scene.set_actor_sprites("gus", std::move(*gus_set));
            std::cout << "BattlePreview: sprite set do Gus carregado (" << nframes
                      << " frames de " << gus::core::assets::kGusBattleAnimsDir
                      << ")\n";
        } else {
            std::cout << "BattlePreview: sprite set do Gus AUSENTE (retrato "
                         "placeholder)\n";
        }

        // Carrega os icones de status (14px), indexados por StatusId (status_icon_index),
        // e os entrega a cena. Ausencia degrada pro quadradinho placeholder.
        const std::string sdir = resolve_status_icons_dir();
        BattleStatusIconSet status_icons;
        for (int i = 0; i < static_cast<int>(status_icons.by_index.size()); ++i) {
            const auto id = static_cast<gus::domain::combat::StatusId>(i);
            const std::string spath = join(sdir, std::string(status_icon_file(id)));
            status_icons.by_index[static_cast<std::size_t>(i)] =
                renderer.load_texture(spath.c_str());
        }
        scene.set_status_icons(status_icons);

        // Carrega os icones de INTENT (telegraph, incremento 5) e os entrega a cena.
        // Ausencia => marca placeholder ambar sobre o inimigo.
        const std::string sdir_intent = resolve_intent_icons_dir();
        BattleIntentIconSet intent_icons;
        intent_icons.atacar =
            renderer.load_texture(join(sdir_intent, "intent_atacar.png").c_str());
        intent_icons.defender =
            renderer.load_texture(join(sdir_intent, "intent_defender.png").c_str());
        intent_icons.aplicar_status = renderer.load_texture(
            join(sdir_intent, "intent_aplicar_status.png").c_str());
        intent_icons.ruido = renderer.load_texture(
            join(sdir_intent, "intent_ruido_patchzero.png").c_str());
        scene.set_intent_icons(intent_icons);

        // Carrega o catalogo de traducao (pt_br.md) e o entrega a cena, pra os verbos do
        // menu aparecerem com NOME legivel (incremento 3.5). Ausencia => fallback (caixa
        // colorida sem nome, mas nao crasha). O Translator vive aqui (casca), a cena so
        // aponta pra ele (nao-dono): mantemos o objeto vivo ate o fim do loop.
        gus::app::i18n::Translator translator;
        const std::string tr_path = gus::app::i18n::resolve_translations_path();
        const bool tr_ok = translator.load_from_file(tr_path);
        scene.set_translator(&translator);

        std::cout << "BattlePreview: traducao "
                  << (tr_ok ? "carregada" : "AUSENTE (fallback)") << " de " << tr_path
                  << "\n  party=" << scene.party_count()
                  << " inimigos=" << scene.enemy_count()
                  << " fila=" << scene.queue_len() << " retratos em " << dir
                  << "\n  ABERTURA: Enter = Encarar (comeca a luta) | Q = Resolver sem "
                     "encarar (placeholder)"
                  << "\n  COMBATE: Cima/Baixo navega o menu | Enter/Espaco: na sua vez "
                     "confirma o verbo, senao ACELERA o ritmo | Esc: sai\n";

        // SMOKE VISUAL (ADR-009): se GUSWORLD_RMLUI_CAPTURE=<arquivo.png>, renderiza
        // alguns frames (deixa o pacing assentar) e salva 1 PNG do framebuffer, depois
        // sai. Sem janela interativa - serve pra COMPARAR o jogo com o mock lado a lado.
        const char* capture_path = std::getenv("GUSWORLD_RMLUI_CAPTURE");
        int frame_no = 0;
        int capture_at_frame = 20;  // ~assenta o 1o layout/fonte antes do shot
        // DEBUG: GUSWORLD_RMLUI_CAPTURE_FRAME=<N> adia o shot p/ o frame N (pra pegar um
        // turno especifico - ex. jogador vs inimigo - sem driver de input). >0 sobrescreve.
        if (const char* cf = std::getenv("GUSWORLD_RMLUI_CAPTURE_FRAME")) {
            const int v = std::atoi(cf);
            if (v > 0) capture_at_frame = v;
        }

        // DIAGNOSTICO (ADR-009): GUSWORLD_RMLUI_FRAMES=N roda o LOOP INTERATIVO COMPLETO
        // (com Update/compose do HUD por frame) e sai apos N frames. Diferente da captura
        // (1 frame), exercita a composicao repetida sob ASan pra pegar leak/UAF-apos-N.
        int max_frames = 0;
        if (const char* mf = std::getenv("GUSWORLD_RMLUI_FRAMES")) {
            max_frames = std::atoi(mf);
        }
        // DIAGNOSTICO: GUSWORLD_RMLUI_AUTOSTART=1 da Encarar automaticamente no 1o frame
        // (pra capturar o estado de COMBATE sem input). So pra smoke visual/teste.
        const bool autostart = [] {
            const char* e = std::getenv("GUSWORLD_RMLUI_AUTOSTART");
            return e != nullptr && e[0] == '1';
        }();

        // DIAGNOSTICO/PROVA (HOVER dos pills): GUSWORLD_BATTLE_HOVER_SELFTEST=<prefixo> assenta a
        // cena ate a vez do jogador, FORCA a selecao em [ATACAR] e roda 4 FASES injetando um
        // UiEvent::MouseMove SINTETICO (mesmo pipeline do mouse real: process_event ->
        // Context::ProcessMouseMove -> pseudo-classe :hover), capturando 1 PNG por fase pra
        // PROVAR o hover surgindo e SUMINDO pelo movimento do ponteiro, sem mouse fisico (que nao
        // da pra simular numa captura estatica). Fases (sufixos do prefixo):
        //   _a_none        ponteiro FORA do cockpit           -> nenhum pill em hover (normal)
        //   _b_hover_unsel ponteiro sobre SCAN (nao-selec.)   -> hover PURO (fundo+borda)
        //   _c_hover_sel   ponteiro sobre ATACAR (SELECIONADO)-> sel+hover: .sel (cyan) DOMINA
        //   _d_none_again  ponteiro FORA de novo              -> hover REMOVIDO (== _a: toggle)
        // So diagnostico: dirige a cena/UI pela MESMA API publica; nao muda o motor nem o render.
        const char* hover_selftest_prefix = std::getenv("GUSWORLD_BATTLE_HOVER_SELFTEST");
        const bool hover_selftest =
            hover_selftest_prefix != nullptr && hover_selftest_prefix[0] != '\0';
        int hover_phase = 0;        // 0=none 1=unsel 2=sel 3=none-again
        int hover_phase_frame = 0;  // frames assentados na fase atual (antes de capturar)

        // DIAGNOSTICO/PROVA (ANIMACAO DE COMBATE, W2): GUSWORLD_BATTLE_ANIM_SELFTEST=
        // <prefixo> roda um SCRIPT POR FRAME (dt FIXO 1/60, deterministico) que captura 5
        // PNGs num UNICO launch, provando o battle-anim.md par.2 com os placeholders:
        //   _a_cast_travel   demo de cast (dormante): bolinha em VOO caster -> alvo
        //   _b_cast_react    impacto do projetil: alvo no TRANCO do hit-react (cosmetico)
        //   _c_melee_windup  [Atacar] do jogador: atacante DESLOCADO no meio do dash
        //   _d_melee_contact contato: atacante NO alvo + floater + tranco do alvo
        //   _e_melee_rest    recovery concluida: TODOS de volta ao repouso exato
        // Dirige a cena pela MESMA API publica (debug_cast_demo e a UNICA porta de
        // diagnostico, tambem publica); nao muda o motor nem o render.
        const char* anim_selftest_prefix = std::getenv("GUSWORLD_BATTLE_ANIM_SELFTEST");
        const bool anim_selftest =
            anim_selftest_prefix != nullptr && anim_selftest_prefix[0] != '\0';
        int anim_f0 = -1;              // frame do aim_confirm (inicio do windup melee)
        std::string anim_attacker_id;  // pro log de offsets (prova textual)
        std::string anim_target_id;
        bool anim_done_cast = false;   // fases ja disparadas (1 vez cada)
        bool anim_done_melee = false;

        // DIAGNOSTICO/PROVA (SPRITE ANIMADO, W3): GUSWORLD_BATTLE_SPRITE_SELFTEST=
        // <prefixo> roda um SCRIPT POR FRAME (dt FIXO 1/60, deterministico) que FORCA o
        // GUS como atacante (navega o PICKER §4.1 ate ele - nao o pre-selecionado por
        // SPD) e captura 4 PNGs num unico launch, provando a troca placeholder->sprite
        // (battle-anim.md par.1.1/3.2) com os frames REAIS do disco:
        //   _a_idle_rest      battle_idle no REPOUSO (antes do Encarar)
        //   _b_run_dash       frame de RUN no MEIO do dash (ida do melee)
        //   _c_attack_swing   attack_melee na CAUDA da aproximacao (swing -> contato)
        //   _d_idle_back      de volta ao battle_idle no repouso exato (pos-Return)
        // Cada captura loga o offset do director + o clip/frame do sprite (prova
        // textual). Dirige pela MESMA API publica do jogador; zero motor/render novo.
        const char* sprite_selftest_prefix =
            std::getenv("GUSWORLD_BATTLE_SPRITE_SELFTEST");
        const bool sprite_selftest =
            sprite_selftest_prefix != nullptr && sprite_selftest_prefix[0] != '\0';
        int sprite_f0 = -1;              // frame do aim_confirm do GUS (inicio do dash)
        bool sprite_done_drive = false;  // drive disparado (1 vez)

        // DIAGNOSTICO/CAPTURA: GUSWORLD_BATTLE_PUMP_TO=<actor_id> conduz o combate ate esse
        // ator ser o ATIVO, ANTES do loop de exibicao - pra CAPTURAR a fila CTB na vez de um
        // ator especifico (ex. um de SPD BAIXA como "jaci") sem driver de input. Encara,
        // bombeia o ritmo nos turnos de inimigo e AUTO-RESOLVE os turnos de jogador (Atacar
        // no alvo sugerido) ate chegar no alvo. So diagnostico: LE/dirige a cena pela MESMA
        // API publica do jogador, nao muda o motor.
        if (const char* pump_to = std::getenv("GUSWORLD_BATTLE_PUMP_TO")) {
            if (pump_to[0] != '\0') {
                const std::string want(pump_to);
                if (scene.is_intro()) {
                    scene.start_combat();
                }
                for (int i = 0; i < 600; ++i) {
                    const auto* a = scene.active_actor();
                    if ((a != nullptr && a->id() == want) || scene.combat_over()) {
                        break;
                    }
                    if (scene.waiting_player_input()) {
                        // W1 item 4: a vez da party abre no PICKER (§4.1); confirma o
                        // pre-selecionado pra chegar ao menu de verbos e seguir o pump.
                        if (scene.is_choosing_actor()) {
                            scene.actor_picker_confirm();
                        }
                        for (int k = 0; k < 8 &&
                                        scene.menu().selected_verb() != BattleVerb::Atacar;
                             ++k) {
                            scene.menu_move(+1);
                        }
                        scene.menu_confirm();  // Atacar -> entra na mira
                        if (scene.is_aiming()) {
                            scene.aim_confirm();  // confirma o alvo -> inicia o WINDUP
                        }
                        // W2 (battle-anim.md par.3.1): a resolucao do [Atacar] e
                        // DEFERIDA ate o CONTATO (fim da aproximacao). Bombeia o
                        // windup ate o motor resolver antes de seguir o pump.
                        for (int k = 0; k < 120 && scene.player_action_in_flight();
                             ++k) {
                            scene.update(1.0f / 60.0f);
                        }
                    } else {
                        scene.skip();
                        scene.update(1.0f / 60.0f);  // bombeia 1 beat de inimigo
                    }
                }
                const auto* a = scene.active_actor();
                std::cout << "BattlePreview: [pump] alvo=" << want << " ator ativo agora="
                          << (a != nullptr ? a->id() : "?")
                          << " fila=" << scene.queue_len() << "\n";
            }
        }

        // DIAGNOSTICO/CAPTURA: GUSWORLD_BATTLE_AIM=1 deixa a cena PARADA no MODO-MIRA de
        // [Atacar] do jogador ativo (assenta o pacing ate a vez do jogador, seleciona Atacar
        // e ENTRA na mira SEM confirmar) - pra CAPTURAR a previa de dano "-N" sobre o alvo,
        // sem driver de input. GUSWORLD_BATTLE_AIM_MOVE=<n> navega n passos entre os inimigos
        // (mostra o "-N" ATUALIZANDO por alvo). Combina com PUMP_TO (ex.: mira na vez do Gus).
        // Dirige pela MESMA API publica do jogador; NAO muda o motor nem o render.
        const bool stop_in_aim = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_AIM");
            return e != nullptr && e[0] == '1';
        }();
        if (stop_in_aim) {
            if (scene.is_intro()) {
                scene.start_combat();  // Encarar
            }
            // Assenta o pacing ate um turno de JOGADOR esperando input (skip/update nao
            // resolvem o turno do jogador; so avancam os beats de inimigo/anuncio).
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            // W1 item 4: atravessa o PICKER de ator (§4.1) pra chegar ao menu de verbos.
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();
            }
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                for (int k = 0; k < 8 &&
                                scene.menu().selected_verb() != BattleVerb::Atacar;
                     ++k) {
                    scene.menu_move(+1);
                }
                scene.menu_confirm();  // Atacar -> ENTRA na mira (nao confirma)
                if (const char* mv = std::getenv("GUSWORLD_BATTLE_AIM_MOVE")) {
                    const int steps = std::atoi(mv);
                    for (int k = 0; k < steps && scene.is_aiming(); ++k) {
                        scene.aim_move(+1);
                    }
                }
                // GUSWORLD_BATTLE_AIM_SHIELD=<mag>: aplica um Shield de pool <mag> no ALVO
                // mirado (ponteiro mutavel da fila) so pra CAPTURAR o "-N" REDUZIDO. Usa a
                // MESMA API de status do jogo (add_status); nao muda o motor nem o render.
                if (const char* sh = std::getenv("GUSWORLD_BATTLE_AIM_SHIELD")) {
                    const int mag = std::atoi(sh);
                    const auto* aimed = scene.aim_target();
                    if (mag > 0 && aimed != nullptr) {
                        for (gus::domain::combat::CombatActor* act :
                             scene.machine().queue().order()) {
                            if (act != nullptr && act->id() == aimed->id()) {
                                act->add_status(gus::domain::combat::StatusEffect{
                                    gus::domain::combat::StatusId::Shield, mag,
                                    /*duration=*/1, gus::domain::combat::StackRule::Replace,
                                    gus::domain::combat::CardFamily::Eletrico});
                                break;
                            }
                        }
                    }
                }
            }
            const auto* t = scene.aim_target();
            const auto* atk = scene.active_actor();
            const int previsto =
                (t != nullptr && atk != nullptr)
                    ? scene.machine().preview_basic_attack_damage(*atk, *t)
                    : -1;
            std::cout << "BattlePreview: [aim] modo-mira="
                      << (scene.is_aiming() ? "on" : "off")
                      << " atacante=" << (atk != nullptr ? atk->id() : "?")
                      << " alvo=" << (t != nullptr ? t->id() : "?")
                      << " dano_previsto=" << previsto << " (badge deve mostrar \"-"
                      << previsto << "\")\n";
        }

        // HOVER-SELFTEST (setup): assenta ate a vez do jogador (skip/update so avancam beats de
        // inimigo/anuncio; NAO resolvem o turno do jogador) e FORCA a selecao em [ATACAR]
        // (indice 2, verbo cyan). ATACAR e o PIOR CASO pra "hover nao ofuscar": seu .sel carrega
        // o glow cyan mais forte (.verb.cyan.sel), entao provar que o hover-sobre-o-selecionado
        // nao o apaga vale pros demais. A cena FICA parada em waiting_player_input (o menu segue
        // exibido); nada no loop muda a selecao (nao injeto navegacao), so o MouseMove sintetico.
        if (hover_selftest) {
            if (scene.is_intro()) {
                scene.start_combat();
            }
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            // W1 item 4: atravessa o PICKER de ator (§4.1) pra chegar ao menu de verbos.
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();
            }
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                for (int k = 0; k < 8 &&
                                scene.menu().selected_verb() != BattleVerb::Atacar;
                     ++k) {
                    scene.menu_move(+1);
                }
            }
            std::cout << "BattlePreview: [hover-selftest] waiting_player="
                      << scene.waiting_player_input()
                      << " sel_index=" << scene.menu().selected_index()
                      << " (esperado 2=ATACAR); prefixo=" << hover_selftest_prefix << "\n";
        }

        bool running = true;
        bool have_last = false;
        unsigned long long last_ns = 0;
        int glintfx_injected = 0;  // SMOKE: conta eventos injetados na UI (prova do pipeline)

        // DIAGNOSTICO/PROVA (GLINTFX-CLICK, ex-Incremento A2): GUSWORLD_BATTLE_MOUSE_SELFTEST=1
        // exercita o roteamento clique->acao SEM mouse fisico. O clique nos pills de verbo ja
        // NAO passa por hit-test de pixel algum (o glintfx resolve o `id` internamente, o mesmo
        // hit-test que move o :hover); entao (1) prova o round-trip id->indice pra cada um dos 6
        // verbos chamando battle_cockpit_verb_click DIRETO (a MESMA funcao que o
        // set_click_callback do UiLayer chama no clique real); (2) CHAMA o pill [ATACAR] e mostra
        // que entrou na MIRA; (3) CLICA o slot de um inimigo (esse SIM em coordenadas de MUNDO,
        // via battle_mouse_click) e mostra que a mira confirmou naquele alvo. So diagnostico
        // (API publica).
        const bool mouse_selftest = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_MOUSE_SELFTEST");
            return e != nullptr && e[0] == '1';
        }();
        if (mouse_selftest) {
            if (scene.is_intro()) {
                scene.start_combat();
            }
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            // W1 item 4: a 1a vez da party agora ABRE no PICKER de ator (§4.1) antes do menu
            // de verbos. Este diagnostico (pre-picker) espera o MENU: atravessa o picker
            // confirmando o pre-selecionado (maior SPD) pra chegar ao menu, como antes.
            if (scene.is_choosing_actor()) {
                scene.actor_picker_confirm();
            }
            std::cout << "BattlePreview: [mouse-selftest] pw0xph0=" << pw0 << "x" << ph0
                      << " waiting_player=" << scene.waiting_player_input() << "\n";
            // (1) round-trip: o `id` de cada pill (fonte unica: kCockpitVerbElementIds, ordem
            // = BattleVerb) -> cockpit_verb_index_for_click_id -> deve devolver o MESMO indice.
            for (int v = 0; v < gus::app::screens::kBattleVerbCount; ++v) {
                const char* id = gus::app::screens::kCockpitVerbElementIds[v];
                const int back = gus::app::screens::cockpit_verb_index_for_click_id(id);
                std::cout << "  pill[" << v << "] "
                          << kVerbLabels[static_cast<std::size_t>(v)] << " id=" << id
                          << " -> indice=" << back << (back == v ? " OK" : " MISMATCH") << "\n";
            }
            // (2) ACIONA o pill ATACAR (o MESMO caminho do callback real). Espera: entra na
            // MIRA. Chama battle_cockpit_verb_click DIRETO (nao ha pixel/evento SDL a simular:
            // o glintfx ja teria resolvido o id antes de chamar o callback registrado).
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                battle_cockpit_verb_click(
                    scene,
                    gus::app::screens::kCockpitVerbElementIds[static_cast<int>(
                        BattleVerb::Atacar)]);
                std::cout << "  CLIQUE (callback) pill ATACAR -> is_aiming="
                          << (scene.is_aiming() ? "on" : "off") << " (esperado on)\n";
            }
            // (3) CLICA o slot de um inimigo (o 2o miravel, se houver). Espera: confirma nele.
            if (scene.is_aiming()) {
                const int want = scene.aim_count() >= 2 ? 1 : 0;
                const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
                    scene.party_count(), scene.enemy_count(), scene.gus_party_index());
                const gus::core::spatial::Rect s =
                    arena.enemies[static_cast<std::size_t>(want)].rect;
                const float wcx = s.x + s.w * 0.5f, wcy = s.y + s.h * 0.5f;
                const float pxcx = wcx / 960.0f * static_cast<float>(pw0);
                const float pxcy = wcy / 540.0f * static_cast<float>(ph0);
                const std::string alvo_antes =
                    scene.aim_target() != nullptr ? scene.aim_target()->id() : "?";
                const int hit = scene.aim_index_at_arena(wcx, wcy);
                battle_mouse_click(scene, pxcx, pxcy, pw0, ph0);
                std::cout << "  CLIQUE inimigo#" << want << " (alvo pre-clique=" << alvo_antes
                          << ") world(" << wcx << "," << wcy << ") px(" << pxcx << "," << pxcy
                          << ") hit_idx=" << hit
                          << " -> is_aiming=" << (scene.is_aiming() ? "on" : "off")
                          << " (esperado off: confirmou; W2 - o contato resolve no fim "
                             "do windup)\n";
            }
            std::cout << "BattlePreview: [mouse-selftest] concluido; encerrando.\n";
            running = false;
        }

        // DIAGNOSTICO/PROVA (Escolha de ator, §4.1): GUSWORLD_BATTLE_ACTOR_SELFTEST=1 dirige uma
        // cena FRESCA ate a vez do BLOCO da party (onde is_choosing_actor()==true) e injeta INPUT
        // SINTETICO pelo MESMO roteamento do host (battle_key_down / battle_mouse_click), pra
        // PROVAR o wiring sem input fisico (dificil de simular em captura estatica):
        //   (A) SETA (battle_key_down DOWN) MOVE o cursor do picker (SEM confirmar);
        //   (B) CLIQUE no slot de um membro (battle_mouse_click) ESCOLHE e CONFIRMA aquele membro;
        //   (C) TECLA 1/2/3 (battle_key_down) escolhe e CONFIRMA o n-esimo membro na hora.
        // Cada proof usa sua PROPRIA BattleScene (o confirm consome o modo). So diagnostico:
        // dirige pela MESMA API publica do host; nao muda o motor nem o render.
        const bool actor_selftest = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_ACTOR_SELFTEST");
            return e != nullptr && e[0] == '1';
        }();
        if (actor_selftest) {
            // Assenta uma cena ate o picker abrir (party-block com >1 elegivel). skip()+update()
            // avancam os beats de inimigo/anuncio; NADA auto-confirma o picker -> para no modo.
            auto drive_to_picker = [](BattleScene& s) {
                if (s.is_intro()) {
                    s.start_combat();
                }
                for (int i = 0; i < 600 && !s.combat_over() && !s.is_choosing_actor(); ++i) {
                    s.skip();
                    s.update(1.0f / 60.0f);
                }
            };
            // id do i-esimo elegivel (0-based) na escolha corrente; "?" fora de faixa.
            auto choice_id = [](const BattleScene& s, int i) -> std::string {
                const auto cs = s.actor_choices();
                return (i >= 0 && i < static_cast<int>(cs.size()) &&
                        cs[static_cast<std::size_t>(i)] != nullptr)
                           ? cs[static_cast<std::size_t>(i)]->id()
                           : std::string("?");
            };

            // ---- (A) SETA move o cursor do picker (NAO confirma) ----
            {
                BattleScene sa;
                drive_to_picker(sa);
                bool dummy = true;
                const int n = sa.actor_pick_count();
                const std::string c0 =
                    sa.actor_pick_target() != nullptr ? sa.actor_pick_target()->id() : "?";
                battle_key_down(sa, SDLK_DOWN, dummy);  // roteamento real do host
                const std::string c1 =
                    sa.actor_pick_target() != nullptr ? sa.actor_pick_target()->id() : "?";
                const bool ok = sa.is_choosing_actor() && (n < 2 || c0 != c1);
                std::cout << "BattlePreview: [actor-selftest] (A seta) choosing="
                          << sa.is_choosing_actor() << " elegiveis=" << n << " cursor: " << c0
                          << " --[DOWN]--> " << c1 << "  " << (ok ? "OK" : "FALHA")
                          << (n < 2 ? " (n<2: sem 2o alvo p/ mover)" : "") << "\n";
            }

            // ---- (B) CLIQUE no slot de um membro ESCOLHE + CONFIRMA ----
            {
                BattleScene sb;
                drive_to_picker(sb);
                const int want_idx =
                    sb.actor_pick_count() >= 2 ? 1 : 0;  // 2o elegivel (NAO o pre-selecionado)
                const std::string want_id = choice_id(sb, want_idx);
                const gus::app::screens::ArenaLayout arena = gus::app::screens::arena_layout(
                    sb.party_count(), sb.enemy_count(), sb.gus_party_index());
                // Acha o SLOT da party cujo centro casa o elegivel want_idx pelo MESMO hit-test do
                // host (actor_pick_index_at_arena) -> nao presume o mapeamento slot->elegivel.
                float pxc = -1.0f, pyc = -1.0f, wcx = -1.0f, wcy = -1.0f;
                for (int k = 0; k < sb.party_count(); ++k) {
                    const gus::core::spatial::Rect r =
                        arena.party[static_cast<std::size_t>(k)].rect;
                    const float cx = r.x + r.w * 0.5f, cy = r.y + r.h * 0.5f;
                    if (sb.actor_pick_index_at_arena(cx, cy) == want_idx) {
                        wcx = cx;
                        wcy = cy;
                        pxc = cx / 960.0f * static_cast<float>(pw0);
                        pyc = cy / 540.0f * static_cast<float>(ph0);
                        break;
                    }
                }
                if (pxc >= 0.0f) {
                    battle_mouse_click(sb, pxc, pyc, pw0, ph0);  // roteamento real do host
                    const std::string got =
                        sb.active_actor() != nullptr ? sb.active_actor()->id() : "?";
                    const bool ok = !sb.is_choosing_actor() && got == want_id;
                    std::cout << "BattlePreview: [actor-selftest] (B clique) elegivel#" << want_idx
                              << "=" << want_id << " world(" << wcx << "," << wcy << ") px(" << pxc
                              << "," << pyc << ") -> choosing=" << sb.is_choosing_actor()
                              << " ator_ativo=" << got << "  "
                              << (ok ? "OK (escolheu+confirmou)" : "FALHA") << "\n";
                } else {
                    std::cout << "BattlePreview: [actor-selftest] (B clique) FALHA: nenhum slot "
                                 "casou o elegivel #"
                              << want_idx << " (choosing=" << sb.is_choosing_actor()
                              << " elegiveis=" << sb.actor_pick_count() << ")\n";
                }
            }

            // ---- (C) TECLA 1/2/3 escolhe + CONFIRMA na hora ----
            {
                BattleScene sc;
                drive_to_picker(sc);
                const int nth = sc.actor_pick_count() >= 2 ? 2 : 1;  // 2a hotkey se houver
                const std::string want_id = choice_id(sc, nth - 1);
                const SDL_Keycode kc = nth == 1 ? SDLK_1 : (nth == 2 ? SDLK_2 : SDLK_3);
                bool dummy = true;
                battle_key_down(sc, kc, dummy);  // roteamento real -> actor_picker_hotkey(nth)
                const std::string got =
                    sc.active_actor() != nullptr ? sc.active_actor()->id() : "?";
                const bool ok = !sc.is_choosing_actor() && got == want_id;
                std::cout << "BattlePreview: [actor-selftest] (C tecla " << nth << ") elegivel#"
                          << (nth - 1) << "=" << want_id << " -> choosing=" << sc.is_choosing_actor()
                          << " ator_ativo=" << got << "  "
                          << (ok ? "OK (escolheu+confirmou)" : "FALHA") << "\n";
            }

            std::cout << "BattlePreview: [actor-selftest] concluido; encerrando.\n";
            running = false;
        }

        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                // ADR-010 F1 SMOKE: injeta o evento na UI glintfx (caminho NOVO; a UI e
                // display-only por ora, mas ja recebe input). Em paralelo ao handler de
                // cena abaixo (ambos veem o mesmo evento). Loga os PRIMEIROS eventos
                // injetados (de qualquer tipo) + toda tecla, p/ provar que o evento SDL
                // atravessa sdl_to_glintfx -> process_event ate o motor de UI.
                if (glintfx_on && ui) {
                    glintfx::UiEvent ge{};
                    if (sdl_to_glintfx(ev, window, &ge)) {
                        ui->process_event(ge);
                        const bool is_key = ge.type == glintfx::UiEvent::Type::Key;
                        if (glintfx_injected < 6 || (is_key && ge.pressed)) {
                            std::cout << "BattlePreview: [glintfx] input injetado #"
                                      << glintfx_injected
                                      << " type=" << static_cast<int>(ge.type)
                                      << " key=" << static_cast<int>(ge.key)
                                      << " x=" << ge.x << " y=" << ge.y
                                      << " mods=" << ge.modifiers << "\n";
                        }
                        ++glintfx_injected;
                    }
                }
                if (ev.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                    // Roteamento de teclado EXTRAIDO pra battle_key_down (funcao-livre, testavel
                    // pelo self-test sintetico; espelha battle_mouse_click). Cobre menu de verbos,
                    // MODO-MIRA (§3.5), ESCOLHA DE ATOR (§4.1, prioridade + teclas 1/2/3) e Esc.
                    battle_key_down(scene, ev.key.key, running);
                } else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                           ev.button.button == SDL_BUTTON_LEFT) {
                    // MOUSE (A2): clique ESQUERDO aciona verbo (menu) ou alvo (mira). ADITIVO
                    // ao teclado (que segue igual). O forward pro glintfx (acima) ja rolou (so
                    // visual); a ACAO real vem do hit-test do host em px de janela.
                    int pw = kWindowW, ph = kWindowH;
                    SDL_GetWindowSizeInPixels(window, &pw, &ph);
                    battle_mouse_click(scene, ev.button.x, ev.button.y, pw, ph);
                } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                    // MOUSE (A2, hover): na mira, passar sobre um inimigo pre-seleciona (realce).
                    int pw = kWindowW, ph = kWindowH;
                    SDL_GetWindowSizeInPixels(window, &pw, &ph);
                    battle_mouse_hover(scene, ev.motion.x, ev.motion.y, pw, ph);
                }
            }
            if (!running) {
                break;
            }

            // dt real desde o ultimo frame (segundos): envelhece os numeros flutuantes
            // e dirige o pacing (2 beats por turno). CLAMP anti-salto: o 1o frame apos o
            // setup pesado (bake do atlas de fonte + load de texturas) chega com dt
            // ENORME (1-2s); sem teto, o pacing avancaria intro+anuncio+resolucao de uma
            // vez e a tela "abriria com o ataque ja feito" (bug pego pelo lider 3x). O
            // teto de 50ms (~3 frames a 60fps) garante que nenhum frame pule um beat.
            const unsigned long long now_ns = SDL_GetTicksNS();
            float dt = 0.0f;
            if (have_last) {
                dt = static_cast<float>(now_ns - last_ns) / 1.0e9f;
                if (dt > 0.05f) {
                    dt = 0.05f;  // clamp anti-salto (spiral-of-death / 1o frame lento)
                }
            }
            have_last = true;
            last_ns = now_ns;
            // ANIM/SPRITE-SELFTEST: dt FIXO 1/60 (script por frame deterministico; o
            // relogio real varia por maquina/driver e desalinharia as capturas).
            if (anim_selftest || sprite_selftest) {
                dt = 1.0f / 60.0f;
            }
            scene.update(dt);  // anima os floaters + pacing; nao toca a FSM

            // ANIM-SELFTEST (script de ACOES por frame; as capturas ficam no fim do
            // frame, apos render+compose). Timeline (dt 1/60): frame 1 dispara o demo
            // de cast (windup 0.5s -> projetil ~f31 -> impacto ~f52); frame 62 comeca o
            // MELEE do jogador (pump ate a vez dele + [Atacar] confirmado = windup
            // 0.7s: contato em F0+42, Return ate F0+66, delay ate F0+90).
            if (anim_selftest) {
                if (frame_no == 1 && !anim_done_cast) {
                    anim_done_cast = true;
                    scene.debug_cast_demo();  // cosmetico (intro): cast + bolinha
                    std::cout << "BattlePreview: [anim-selftest] f1 cast demo iniciado\n";
                }
                if (frame_no == 62 && !anim_done_melee) {
                    anim_done_melee = true;
                    if (scene.is_intro()) {
                        scene.start_combat();
                    }
                    for (int i = 0; i < 240 && !scene.combat_over() &&
                                    !scene.waiting_player_input();
                         ++i) {
                        scene.skip();
                        scene.update(1.0f / 60.0f);
                    }
                    if (scene.is_choosing_actor()) {
                        scene.actor_picker_confirm();  // atravessa o picker (§4.1)
                    }
                    if (scene.waiting_player_input() && !scene.is_aiming()) {
                        for (int k = 0; k < 8 && scene.menu().selected_verb() !=
                                                     BattleVerb::Atacar;
                             ++k) {
                            scene.menu_move(+1);
                        }
                        const auto* atk = scene.active_actor();
                        anim_attacker_id = atk != nullptr ? atk->id() : "?";
                        scene.menu_confirm();  // entra na mira
                        const auto* tgt = scene.aim_target();
                        anim_target_id = tgt != nullptr ? tgt->id() : "?";
                        scene.aim_confirm();  // COMANDA: windup parte agora
                        anim_f0 = frame_no;
                        std::cout << "BattlePreview: [anim-selftest] f" << frame_no
                                  << " melee confirmado: atacante=" << anim_attacker_id
                                  << " alvo=" << anim_target_id << " in_flight="
                                  << scene.player_action_in_flight() << "\n";
                    } else {
                        std::cout << "BattlePreview: [anim-selftest] FALHA: nao chegou "
                                     "na vez do jogador\n";
                        running = false;
                    }
                }
            }

            // SPRITE-SELFTEST (drive): apos capturar o idle-em-repouso (frame 8, na
            // secao de capturas), o frame 10 conduz a cena ate a vez da party, navega
            // o PICKER ate o GUS (forca ele como atacante) e confirma [Atacar] + alvo.
            // O dash parte AGORA (sprite_f0) e as capturas seguintes provam run/golpe/
            // volta-ao-idle. MESMA API publica do jogador (padrao dos self-tests).
            if (sprite_selftest && frame_no == 10 && !sprite_done_drive) {
                sprite_done_drive = true;
                if (scene.is_intro()) {
                    scene.start_combat();
                }
                for (int i = 0; i < 240 && !scene.combat_over() &&
                                !scene.waiting_player_input();
                     ++i) {
                    scene.skip();
                    scene.update(1.0f / 60.0f);
                }
                if (scene.is_choosing_actor()) {
                    // Navega o cursor ate o GUS (§4.1: o jogador comanda QUEM age).
                    for (int k = 0; k < scene.actor_pick_count() &&
                                    (scene.actor_pick_target() == nullptr ||
                                     scene.actor_pick_target()->id() != "gus");
                         ++k) {
                        scene.actor_picker_move(+1);
                    }
                    scene.actor_picker_confirm();
                }
                const auto* atk = scene.active_actor();
                if (scene.waiting_player_input() && atk != nullptr &&
                    atk->id() == "gus" && !scene.is_aiming()) {
                    for (int k = 0; k < 8 && scene.menu().selected_verb() !=
                                                 BattleVerb::Atacar;
                         ++k) {
                        scene.menu_move(+1);
                    }
                    scene.menu_confirm();  // entra na mira
                    scene.aim_confirm();   // confirma o alvo: o dash parte AGORA
                    sprite_f0 = frame_no;
                    std::cout << "BattlePreview: [sprite-selftest] f" << frame_no
                              << " melee do GUS confirmado; in_flight="
                              << scene.player_action_in_flight() << "\n";
                } else {
                    std::cout << "BattlePreview: [sprite-selftest] FALHA: nao chegou "
                                 "na vez do GUS (ativo="
                              << (atk != nullptr ? atk->id() : "?") << ")\n";
                    running = false;
                }
            }

            // DIAGNOSTICO: auto-Encarar (captura do estado de combate sem input).
            if (autostart && scene.is_intro()) {
                scene.start_combat();
            }

            int pw = kWindowW, ph = kWindowH;
            SDL_GetWindowSizeInPixels(window, &pw, &ph);
            // GUARD janela minimizada (Wayland pode reportar 0): pula render+compose deste
            // frame (so pumpa eventos no proximo), evitando viewport/FBO de tamanho 0.
            if (pw < 1 || ph < 1) {
                SDL_Delay(16);
                continue;
            }
            // A arena desenha no backbuffer GL (clear + draws). O HUD RmlUi-GL3 compoe POR
            // CIMA (layer -> backbuffer). O SWAP e UNICO (SDL_GL_SwapWindow), depois do HUD.
            // FXTEST: pula a arena (so o HUD sobre fundo preto) pra isolar os efeitos.
            if (fxtest) {
                renderer.begin_frame(gus::core::spatial::Rect{0, 0, 960, 540}, pw, ph);
                renderer.end_frame();  // so o clear (fundo preto-ish); HUD compoe por cima
            } else {
                scene.render(renderer, static_cast<float>(pw), static_cast<float>(ph));
            }
            // ADR-010 F3: glintfx compoe no slot de HUD - depois da arena, antes do swap.
            // render() e compose-only (sem clear, sem swap; salva e restaura o estado GL
            // internamente). O viewport segue os pixels reais.
            if (glintfx_on && ui) {
                if (pw != pw0 || ph != ph0) {
                    ui->set_viewport(pw, ph);
                    if (glintfx_dp_override <= 0.0f) {
                        ui->set_dp_ratio(static_cast<float>(pw) / 960.0f);  // reescala logico
                    }
                    pw0 = pw;
                    ph0 = ph;
                }
                // ADR-010 F2b: ALIMENTA o data-model com os valores VIVOS do motor (POCO),
                // a cada frame. O MOTOR (scene) e a fonte de verdade: estado intro/combate,
                // ator ativo (nome/HP), selecao de verbo (scene.menu().selected_index ->
                // binding 'sel' -> classe .sel via data-class-sel), alvo, e o log narrado.
                if (glintfx_live) {
                    ui->set_bool("intro", scene.is_intro());
                    if (!scene.is_intro()) {
                        if (const auto* a = scene.active_actor(); a != nullptr) {
                            ui->set_string("nome", a->display_name().c_str());
                            // POLISH 2: o label segue o LADO do ator ativo. Party = role
                            // de party ("VETOR DO GAMBITO"); inimigo = designacao generica
                            // "INIMIGO" (o CombatActor nao expoe tipo/arquetipo - so lado).
                            // NUNCA pintar um inimigo com o role da party. Motor = autoridade.
                            ui->set_string("role", a->is_player_side() ? "VETOR DO GAMBITO"
                                                                       : "INIMIGO");
                            ui->set_number("hp", a->hp());
                            ui->set_number("hp_max", a->max_hp());
                            // RETRATO-VIVO: a moldura segue o ator ATIVO. Inimigo -> cabeca
                            // generica do inimigo (mesma fonte da coluna/CTB); party -> seu
                            // retrato. Motor = autoridade (le active_actor, nao inventa).
                            ui->set_string("retrato_src",
                                           cockpit_retrato_flat_for(*a).c_str());
                        }
                        // Selecao do menu (motor = autoridade). O foco do glintfx (tab/nav no
                        // RML) e navegacao inerte; a classe .sel VISIVEL segue este indice.
                        const int sel = scene.menu().selected_index();
                        ui->set_number("sel", sel);
                        ui->set_string("verb",
                                       std::string(kVerbLabels[static_cast<std::size_t>(
                                                       sel < 0 || sel > 5 ? 2 : sel)])
                                           .c_str());
                        // Alvo: 1o inimigo VIVO da fila (alvo default das acoes ofensivas,
                        // canon 1B). "-" se nenhum. Le o motor (queue), nao inventa estado.
                        const char* alvo = "-";
                        std::string alvo_buf;
                        for (const auto* act : scene.machine().queue().order()) {
                            if (act != nullptr && !act->is_player_side() && act->is_alive()) {
                                alvo_buf = act->display_name();
                                alvo = alvo_buf.c_str();
                                break;
                            }
                        }
                        ui->set_string("alvo", alvo);
                        // Log VIVO: ultimas linhas narradas pelo motor (data-for "line:log").
                        // As strings precisam VIVER ate o set_list copiar (mantemos o vector).
                        // CAP 2 entradas (as mais recentes) + now-line = EXATAS 3 linhas de
                        // log. Com o RCSS white-space:nowrap+ellipsis do #log .ln cada entrada
                        // ocupa 1 linha fixa (nao quebra mais em 2), entao 2*12dp + now-line =
                        // 36dp e a coluna cabe em 540dp com folga do rodape; now-line visivel.
                        const std::vector<gus::app::screens::LogLine> lines =
                            scene.log_lines(2);
                        std::vector<const char*> ptrs;
                        ptrs.reserve(lines.size());
                        for (const auto& l : lines) ptrs.push_back(l.text.c_str());
                        ui->set_list("log", ptrs.data(), ptrs.size());
                    }
                }
                // HOVER-SELFTEST (injecao): a cada frame, coloca o ponteiro SINTETICO onde a
                // fase pede, ANTES do update() (o Context::Update reaplica o hover chain sob a
                // ultima posicao). Fases 0/3: FORA do cockpit (arena, x=80% da largura) => nenhum
                // pill em hover. Fases 1/2: no CENTRO do pill-alvo. GLINTFX-CLICK: a posicao vem
                // de UiLayer::get_element_box (v0.2.5, geometria REAL do doc carregado) em vez da
                // geometria manual aposentada (mais precisa: o glintfx sabe onde o pill
                // REALMENTE esta, nao uma copia espelhada da RCSS) -- ESTE diagnostico e' o UNICO
                // uso de get_element_box no projeto (o hit-test de CLIQUE usa o id direto via
                // set_click_callback, nunca geometria; ver battle_cockpit_verb_click).
                if (hover_selftest) {
                    float mx = static_cast<float>(pw) * 0.80f;  // fora da coluna (fases 0 e 3)
                    float my = static_cast<float>(ph) * 0.50f;
                    const int hover_pill = hover_phase == 1   ? 0    // SCAN (nao-selecionado)
                                           : hover_phase == 2 ? 2    // ATACAR (selecionado)
                                                              : -1;  // 0 e 3: sem pill
                    if (hover_pill >= 0) {
                        const glintfx::ElementBox box = ui->get_element_box(
                            gus::app::screens::kCockpitVerbElementIds[hover_pill]);
                        if (box.found) {
                            mx = box.x + box.w * 0.5f;
                            my = box.y + box.h * 0.5f;
                        }
                    }
                    glintfx::UiEvent ge{};
                    ge.type = glintfx::UiEvent::Type::MouseMove;
                    ge.x = mx;
                    ge.y = my;
                    ui->process_event(ge);
                }
                ui->update();
                ui->render();  // UI por cima da arena, mesmo contexto GL
            }

            // HOVER-SELFTEST (captura por fase): deixa ~12 frames assentarem (bake do atlas de
            // fonte + hover chain estavel) e salva 1 PNG por fase, lendo o backbuffer ANTES do
            // swap. Depois avanca a fase; encerra apos a 4a. Nao colide com a captura single-shot
            // (GUSWORLD_RMLUI_CAPTURE) porque aquela so dispara com capture_path != nullptr.
            if (hover_selftest) {
                constexpr int kHoverSettleFrames = 12;
                ++hover_phase_frame;
                if (hover_phase_frame >= kHoverSettleFrames) {
                    const char* suffix = hover_phase == 0   ? "_a_none.png"
                                         : hover_phase == 1 ? "_b_hover_unsel.png"
                                         : hover_phase == 2 ? "_c_hover_sel.png"
                                                            : "_d_none_again.png";
                    const std::string out = std::string(hover_selftest_prefix) + suffix;
                    std::vector<unsigned char> buf(
                        static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) * 4);
                    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
                        stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                        std::cout << "BattlePreview: [hover-selftest] fase " << hover_phase
                                  << " -> " << out << " (" << pw << "x" << ph << ")\n";
                    } else {
                        std::cerr
                            << "BattlePreview: [hover-selftest] gl3_read_backbuffer falhou\n";
                    }
                    ++hover_phase;
                    hover_phase_frame = 0;
                    if (hover_phase > 3) {
                        running = false;  // 4 fases capturadas; encerra
                    }
                }
            }

            // ANIM-SELFTEST (capturas): le o backbuffer ANTES do swap (mesma tecnica do
            // hover-selftest) nos frames-chave da timeline. Loga o OFFSET do atacante
            // (prova textual do desloca-golpeia-volta) junto de cada shot.
            if (anim_selftest) {
                const auto capture = [&](const char* suffix) {
                    const std::string out = std::string(anim_selftest_prefix) + suffix;
                    std::vector<unsigned char> buf(
                        static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) * 4);
                    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph,
                                                                       buf.data())) {
                        stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                        const auto aoff = anim_attacker_id.empty()
                                              ? gus::core::spatial::Vec2{}
                                              : scene.anim().offset_for(anim_attacker_id);
                        const auto toff = anim_target_id.empty()
                                              ? gus::core::spatial::Vec2{}
                                              : scene.anim().offset_for(anim_target_id);
                        std::cout << "BattlePreview: [anim-selftest] f" << frame_no
                                  << " -> " << out << " | atacante_off=(" << aoff.x << ","
                                  << aoff.y << ") alvo_off=(" << toff.x << "," << toff.y
                                  << ") projeteis=" << scene.anim().projectiles().size()
                                  << " floaters=" << scene.floaters().size() << "\n";
                    } else {
                        std::cerr << "BattlePreview: [anim-selftest] gl3_read_backbuffer "
                                     "falhou\n";
                    }
                };
                if (frame_no == 40) {
                    capture("_a_cast_travel.png");  // bolinha em voo (meio da viagem)
                } else if (frame_no == 58) {
                    capture("_b_cast_react.png");   // alvo no tranco pos-impacto
                }
                if (anim_f0 > 0) {
                    if (frame_no == anim_f0 + 21) {
                        capture("_c_melee_windup.png");   // meio do dash (deslocado)
                    } else if (frame_no == anim_f0 + 49) {
                        capture("_d_melee_contact.png");  // no alvo + floater + tranco
                    } else if (frame_no == anim_f0 + 87) {
                        capture("_e_melee_rest.png");     // todos de volta ao repouso
                        std::cout << "BattlePreview: [anim-selftest] concluido; "
                                     "encerrando.\n";
                        running = false;
                    }
                }
            }

            // SPRITE-SELFTEST (capturas): backbuffer antes do swap (mesma tecnica dos
            // demais self-tests) nos frames-chave, logando offset do director + clip/
            // frame do sprite do GUS (prova textual: qual animacao esta tocando).
            if (sprite_selftest) {
                const auto capture = [&](const char* suffix) {
                    const std::string out =
                        std::string(sprite_selftest_prefix) + suffix;
                    std::vector<unsigned char> buf(
                        static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) *
                        4);
                    if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph,
                                                                       buf.data())) {
                        stbi_write_png(out.c_str(), pw, ph, 4, buf.data(), pw * 4);
                    } else {
                        std::cerr << "BattlePreview: [sprite-selftest] "
                                     "gl3_read_backbuffer falhou\n";
                    }
                    const auto off = scene.anim().offset_for("gus");
                    const auto sf = scene.actor_sprite_frame("gus");
                    std::cout << "BattlePreview: [sprite-selftest] f" << frame_no
                              << " -> " << out << " | gus_off=(" << off.x << ","
                              << off.y << ") kind="
                              << static_cast<int>(scene.anim().kind_for("gus"))
                              << " clip="
                              << (sf.has_value()
                                      ? std::string(gus::app::screens::clip_dir_name(
                                            sf->first))
                                      : std::string("<sem sprite set>"))
                              << " frame=" << (sf.has_value() ? sf->second : -1)
                              << "\n";
                };
                if (frame_no == 8) {
                    capture("_a_idle_rest.png");     // battle_idle no repouso
                }
                if (sprite_f0 > 0) {
                    // Duracoes reais (dt fixo 1/60; +0.5f arredonda p/ evitar truncamento
                    // de float, ex. 0.7*60 vira 41.99...): Approach kPlayerMeleeApproach
                    // Seconds (1.3s ~= 78f), Return kPlayerMeleeReturnSeconds (0.7s ~= 42f),
                    // delay do Beat 2 kPacingStepDelaySeconds (0.8s ~= 48f).
                    const int approach_f = static_cast<int>(
                        gus::app::screens::kPlayerMeleeApproachSeconds * 60.0f + 0.5f);
                    const int return_f = static_cast<int>(
                        gus::app::screens::kPlayerMeleeReturnSeconds * 60.0f + 0.5f);
                    const int rel = frame_no - sprite_f0;
                    // FLIPBOOK do APPROACH inteiro (diagnostico do lider 2026-07-02): captura
                    // DENSA a cada 4 frames de jogo, do inicio do dash (rel 0) ao contato
                    // (rel approach_f). ~20 quadros pra montar o flip e VER se o ciclo de
                    // perna le pra-FRENTE (aliasing curado pela duracao maior) ou reverso
                    // (problema real de ordem/pose). Nomes _flip_NN zero-pad p/ ordenar.
                    if (rel >= 0 && rel <= approach_f && (rel % 4) == 0) {
                        const int fi = rel / 4;
                        const std::string suf = std::string("_flip_") +
                                                (fi < 10 ? "0" : "") +
                                                std::to_string(fi) + ".png";
                        capture(suf.c_str());
                    }
                    // Marcos MACRO (nomes semanticos; rel escolhidos %4==3, nao colidem com
                    // o flip): swing cravado na cauda, meio da volta, repouso final.
                    if (rel == approach_f - 3) {
                        capture("_c_attack_swing.png");  // murro de perfil cravado (<= f5)
                    } else if (rel == approach_f + return_f / 2) {
                        capture("_e_run_back.png");      // run_west no meio da volta
                    } else if (rel == approach_f + return_f + 3) {
                        capture("_d_idle_back.png");     // de volta ao idle no repouso
                        std::cout << "BattlePreview: [sprite-selftest] concluido; "
                                     "encerrando.\n";
                        running = false;
                    }
                }
            }

            // SMOKE VISUAL: captura 1 PNG no frame alvo (ANTES do swap, lendo o backbuffer)
            // e encerra. Em modo interativo, o swap apresenta na janela.
            if (capture_path != nullptr && frame_no + 1 >= capture_at_frame) {
                std::vector<unsigned char> buf(
                    static_cast<std::size_t>(pw) * static_cast<std::size_t>(ph) * 4);
                if (gus::platform::rmlui::gl3_read_backbuffer_rgba(pw, ph, buf.data())) {
                    stbi_write_png(capture_path, pw, ph, 4, buf.data(), pw * 4);
                    std::cout << "BattlePreview: [capture] PNG salvo em " << capture_path
                              << " (" << pw << "x" << ph << ")\n";
                } else {
                    std::cerr << "BattlePreview: [capture] gl3_read_backbuffer falhou\n";
                }
                running = false;
            }

            SDL_GL_SwapWindow(window);  // swap unico (arena + HUD compostos)
            ++frame_no;

            // DIAGNOSTICO: sai apos N frames (exercita o loop completo sob ASan/UBSan).
            if (max_frames > 0 && frame_no >= max_frames) {
                std::cout << "BattlePreview: [frames] limite de " << max_frames
                          << " frames atingido, encerrando.\n";
                running = false;
            }
        }

        // MUSICA: fade-out ao SAIR da batalha (M6 F4, ADR-011). Unico CHOKE-POINT de saida
        // do loop `while (running)` acima, qualquer que seja o motivo (Esc/fechar janela/
        // limite de --frames/selftest concluido) - todos convergem pra `running = false`
        // e caem aqui, entao um so ponto cobre "sair da batalha" sem precisar duplicar em
        // cada handler de saida.
        //
        // ESCOPO "fade entre telas" (veredito honesto, ADR-011 F4): este viewer roda a
        // BATALHA ISOLADA - nao ha tela anterior/posterior real neste fluxo (--battle nao
        // tem overworld<->batalha; esse loop de cena e trabalho do M5/M7). Os PRIMITIVOS
        // de fade estao implementados e provados aqui (fade-in acima ao entrar, fade-out
        // abaixo ao sair - API play_music(..., fade_in_seconds) + stop_music(fade)); o
        // CROSSFADE completo tela-a-tela (musica A esmaecendo enquanto musica B sobe) so
        // fica exercitavel quando o loop de cena overworld<->batalha existir - limitacao
        // de INTEGRACAO (nao existe segunda tela pra crossfade contra), nao de audio.
        const bool music_was_playing_before_exit = audio_engine.music_is_playing();
        constexpr float kMusicFadeOutSeconds = 1.5f;
        audio_engine.stop_music(kMusicFadeOutSeconds);
        std::cout << "BattlePreview: [audio] musica: play_music count="
                  << audio_engine.music_play_count() << " estava_tocando_ao_sair="
                  << (music_was_playing_before_exit ? "sim" : "nao") << " fade-out de "
                  << kMusicFadeOutSeconds << "s disparado.\n";

        // DIAGNOSTICO (M6 F3, ADR-011): quantos SFX de hit TOCARAM de fato nesta sessao -
        // prova rapida no console (sem precisar ouvir) de que o gancho disparou no evento
        // de contato certo, tanto em playtest manual quanto sob os selftests acima.
        std::cout << "BattlePreview: [audio] play_sfx(hit) disparou "
                  << audio_engine.sfx_play_count() << "x nesta sessao.\n";
    }  // Render2dGl3 destruido (libera recursos GL) antes de destruir o contexto

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens
