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

#include "gus/app/screens/battle_hud_model.hpp"  // status_icon_file/index
#include "gus/app/screens/battle_scene.hpp"
#include "gus/core/asset_paths.hpp"             // caminhos de asset centralizados
#include "gus/domain/combat/combat_enums.hpp"  // StatusId
#include "gus/platform/render2d/render2d_gl3.hpp"  // ADR-009 GL3: backend OpenGL da arena
#include "gus/platform/rmlui/gl3_loader.hpp"  // glad load + read_backbuffer (captura)
#include "gus/platform/rmlui/rmlui_hud.hpp"  // ADR-009: HUD RmlUi-GL3 sobre a arena

// stb_image_write: captura de frame (PNG) para o SMOKE VISUAL do ADR-009 (comparar o
// jogo com o mock). IMPLEMENTACAO definida UMA vez aqui (camada app/, fora do hot path).
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Raiz resources/ do repo, embutida pelo CMake (mesma macro do resolver de sprites).
#ifndef GUSWORLD_ASSETS_DIR
#define GUSWORLD_ASSETS_DIR ""
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
  decorator: vertical-gradient( #243056 #0f1322 );  /* topo mais claro -> base escura */
  padding: 14dp 14dp 0dp 14dp;
}
/* filete cyan na borda direita (marca cyber) */
#edge {
  position: absolute; top: 0dp; right: 0dp; bottom: 0dp; width: 3dp;
  decorator: vertical-gradient( #0c0f1a #22D3EE );
  box-shadow: #22D3EE 0dp 0dp 16dp 2dp;
}

/* ---- RETRATO emoldurado (moldura TCG asset) + HALO cyan pulsante ---- */
#actor { display: block; text-align: center; }
/* #portrait e o CONTEXTO de posicionamento (relative) dos filhos absolutos (pic/frame),
   senao eles ancoram no root (0,0). Centrado na coluna com margens laterais iguais. */
#portrait {
  display: block; position: relative;
  width: 128dp; height: 165dp; margin-left: 48dp; margin-top: 8dp;
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
#pic { position: absolute; top: 16dp; left: 16dp; width: 96dp; height: 130dp;
  background-color: #0c1322;
  decorator: image( )RML";
    rml += retrato;
    rml += R"RML( cover ); }
/* a moldura (asset) POR CIMA, cobrindo o quadro inteiro */
#frame { position: absolute; top: 0dp; left: 0dp; width: 128dp; height: 165dp;
  decorator: image( )RML";
    rml += moldura;
    rml += R"RML( ); }

#name { display: block; text-align: center; margin-top: 10dp; font-size: 15dp;
  color: #eaf6fb; }
#role { display: block; text-align: center; font-size: 10dp; color: #E8A33D; }

/* ---- HP barra (degrade verde + glow) + numeros ---- */
#vitals { margin-top: 8dp; }
.hpbar {
  height: 14dp; border-radius: 7dp; margin-top: 3dp;
  decorator: vertical-gradient( #7dffbe #2e9c63 );  /* verde claro -> verde escuro */
  box-shadow: #3FB97a 0dp 0dp 16dp 2dp;            /* glow verde, spread POSITIVO */
}
.hpnum { font-size: 10dp; color: #8fa6b4; }
.hpnum .v { color: #5fe3a0; }

/* ---- pips AP (latao) / Mana (cyan), radiais com glow ---- */
.pips { display: block; margin-top: 7dp; font-size: 10dp; color: #8fa6b4; }
.pip { display: inline-block; width: 12dp; height: 12dp; border-radius: 6dp;
  margin-right: 4dp; border: 1dp #2a3450; background-color: #0c1322; }
.pip.ap.on { decorator: radial-gradient( circle closest-side, #ffe6a8, #E8A33D );
  box-shadow: #E8A33D 0dp 0dp 12dp 2dp; }
.pip.mana.on { decorator: radial-gradient( circle closest-side, #c2f6ff, #22D3EE );
  box-shadow: #22D3EE 0dp 0dp 12dp 2dp; }

/* ---- MENU de verbos: pill rococo com GLOW neon nos 3 estados ---- */
.menu { margin-top: 16dp; }
.verb {
  display: block; height: 38dp; margin-bottom: 10dp; padding: 0dp 14dp;
  border-radius: 19dp;
  decorator: vertical-gradient( #2a3658 #131a2e );  /* topo mais claro (3D sutil) */
  border: 1dp #38456e;
  color: #d6e6ef; font-size: 13dp;
}
.verb .lbl { display: inline-block; line-height: 38dp; }
/* NORMAL: cor do verbo no glifo lateral */
.verb .glyph { display: inline-block; width: 8dp; height: 8dp; margin-right: 10dp;
  border-radius: 2dp; background-color: #8fa6b4; }
.verb.cyan .glyph { background-color: #22D3EE; box-shadow: #22D3EE 0dp 0dp 8dp 1dp; }
.verb.latao .glyph { background-color: #E8A33D; box-shadow: #E8A33D 0dp 0dp 8dp 1dp; }
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
#log { margin-top: 12dp; padding-top: 9dp; border-top: 1dp #2a3450; font-size: 11dp; }
#log .ln { color: #6f8593; }
#log .who { color: #22D3EE; }
#log .hit { color: #E11D74; }
#log .now { color: #cfe6ee; }

/* ---- ABERTURA: brasao GusWorld (monograma V + aneis girando + AGUARDANDO ORDEM) ---- */
#opening { display: block; text-align: center; margin-top: 90dp; }
#crest { display: block; position: relative; width: 148dp; height: 148dp;
  margin-left: 52dp; margin-bottom: 18dp; }
.ring { position: absolute; border-radius: 74dp; }
.ring.r1 { top: 0dp; left: 0dp; width: 148dp; height: 148dp;
  border: 1dp #E8A33D; box-shadow: #E8A33D99 0dp 0dp 22dp -6dp; }
/* anel cyan tracejado GIRANDO (transform animado) */
.ring.r2 { top: 16dp; left: 16dp; width: 116dp; height: 116dp;
  border: 1dp #22D3EE; box-shadow: #22D3EE66 0dp 0dp 16dp -6dp;
  animation: spin 18s linear infinite; }
.ring.r3 { top: 34dp; left: 34dp; width: 80dp; height: 80dp; border: 1dp #E8A33D66; }
@keyframes spin { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }
/* monograma V (Vance) centrado, com glow latao */
.mono { position: absolute; top: 38dp; left: 0dp; width: 148dp; text-align: center;
  font-size: 62dp; color: #E8A33D; }
#otitle { display: block; font-size: 16dp; color: #cfe6ee; }
#osub { display: block; font-size: 10dp; color: #6f8593; margin-top: 2dp; }
#ostatus { display: block; margin-top: 16dp; font-size: 11dp; color: #22D3EE; }
#ostatus .dot { display: inline-block; width: 7dp; height: 7dp; border-radius: 4dp;
  margin-right: 8dp; background-color: #22D3EE; box-shadow: #22D3EE 0dp 0dp 8dp;
  animation: blink 1.1s step-start infinite; }
@keyframes blink { from { opacity: 1; } 50% { opacity: 0.25; } to { opacity: 1; } }
</style>
</head>
<body>
  <div id="cockpit" data-model="hud">
    <div id="edge"></div>

    <!-- ABERTURA (C): brasao GusWorld. So aparece quando intro (data-if). -->
    <div id="opening" data-if="intro">
      <div id="crest">
        <div class="ring r1"></div>
        <div class="ring r2"></div>
        <div class="ring r3"></div>
        <div class="mono">V</div>
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

      <div class="menu">
        <div class="verb"><span class="glyph"></span><span class="lbl">SCAN</span></div>
        <div class="verb"><span class="glyph"></span><span class="lbl">GAMBITO</span></div>
        <div class="verb cyan sel"><span class="glyph"></span><span class="lbl">ATACAR</span></div>
        <div class="verb"><span class="glyph"></span><span class="lbl">DEFENDER</span></div>
        <div class="verb latao"><span class="glyph"></span><span class="lbl">COMPILAR</span></div>
        <div class="verb"><span class="glyph"></span><span class="lbl">FUGIR</span></div>
      </div>

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

// DIAGNOSTICO DE EFEITOS (#1): doc com efeitos MAXIMAMENTE OBVIOS pra provar se os
// shaders de gradiente/box-shadow/radial chegam a tela. Cores de alto contraste (vermelho
// -> azul, glow magenta forte). Se ISTO renderizar flat, o problema e o pipeline GL3.
std::string load_fxtest_rml() {
    return R"RML(<rml>
<head>
<style>
  body { font-family: "Pixel Operator Mono"; background: transparent; }
  #grad {
    position: absolute; top: 40dp; left: 40dp; width: 300dp; height: 120dp;
    decorator: vertical-gradient( #ff2244 #2244ff );  /* vermelho -> azul (obvio) */
    border-radius: 16dp;
  }
  #shadow {
    position: absolute; top: 200dp; left: 40dp; width: 300dp; height: 80dp;
    background-color: #222244;
    border-radius: 12dp;
    box-shadow: #ff00ff 0dp 0dp 40dp 8dp;  /* glow magenta FORTE e largo */
  }
  #radial {
    position: absolute; top: 320dp; left: 40dp; width: 120dp; height: 120dp;
    decorator: radial-gradient( circle closest-side, #ffff00, #ff0000 );  /* amarelo->vermelho */
    border-radius: 60dp;
  }
  #lin {
    position: absolute; top: 320dp; left: 200dp; width: 200dp; height: 120dp;
    decorator: horizontal-gradient( #00ff88 #8800ff );
    border-radius: 12dp;
  }
  #txt { position: absolute; top: 470dp; left: 40dp; font-size: 16dp; color: #ffffff; }
</style>
</head>
<body>
  <div id="grad"></div>
  <div id="shadow"></div>
  <div id="radial"></div>
  <div id="lin"></div>
  <div id="txt">FXTEST: gradiente(V) | box-shadow magenta | radial | gradiente(H)</div>
</body>
</rml>
)RML";
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

    SDL_Window* window =
        SDL_CreateWindow("GusWorld BattlePreview (M5, GL3)", kWindowW, kWindowH,
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
        // ADR-009 F1: HUD RmlUi-GL3 compondo POR CIMA da arena, COM efeitos nativos
        // (gradiente/box-shadow/glow). E o caminho PADRAO (sem env). A arena desenha
        // primeiro (backbuffer), o RmlUi compoe por cima (layer -> backbuffer), o swap e
        // unico (SDL_GL_SwapWindow). Opt-out (debug do cockpit a-mao antigo):
        // GUSWORLD_RMLUI_OFF=1.
        // ====================================================================
        gus::platform::rmlui::RmlUiHud hud;
        bool rmlui_hud_on = false;
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
        if (!rmlui_opt_out) {
            if (hud.init(/*gl_active=*/true, pw0, ph0, /*logical_w=*/960,
                         /*logical_h=*/540)) {
                // O data-model PRECISA existir ANTES de carregar o doc (o data-model="hud"
                // e resolvido no parse). Bind primeiro, depois load.
                hud.bind_demo_model("hud", "Gus");
                hud.set_intro(true);  // comeca na abertura (brasao)
                // Base pra resolver os image() do RCSS (moldura/retrato): um .rml ficticio
                // no diretorio dos assets do cockpit.
                hud.set_asset_base_url(join(cockpit_asset_base_dir(), "cockpit.rml"));
                const std::string doc_rml = fxtest ? load_fxtest_rml() : load_cockpit_rml();
                if (hud.load_document_from_memory(doc_rml)) {
                    rmlui_hud_on = true;
                    if (fxtest) {
                        hud.set_intro(false);  // o doc de FX nao usa intro
                    }
                    std::cout << "BattlePreview: HUD RmlUi-GL3 ATIVO ("
                              << (fxtest ? "FXTEST" : "cockpit + abertura")
                              << ") sobre a arena (caminho padrao)\n";
                } else {
                    std::cerr << "BattlePreview: falha ao carregar o RML\n";
                }
            } else {
                std::cerr << "BattlePreview: RmlUi-GL3 nao inicializou; caindo no cockpit "
                             "a-mao\n";
            }
        }

        // A cena monta o encontro de demo e ja le a fila do motor.
        BattleScene scene;
        // (A) Com o HUD externo (RmlUi) ATIVO, a cena NAO desenha o cockpit/log a mao -
        // so arena/banner/floaters/fila. Evita os DOIS cockpits sobrepostos.
        scene.set_hud_external(rmlui_hud_on);

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
        constexpr int kCaptureAtFrame = 20;  // ~assenta o 1o layout/fonte antes do shot

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

        bool running = true;
        bool have_last = false;
        unsigned long long last_ns = 0;
        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                    switch (ev.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        // Navegacao do menu de verbos (incremento 3). So opera no turno
                        // de jogador (a cena ignora fora dele); a cena auto-encadeia os
                        // turnos de inimigo ate o proximo turno de jogador ou o fim.
                        case SDLK_UP:
                        case SDLK_W:
                            scene.menu_move(-1);
                            break;
                        case SDLK_DOWN:
                        case SDLK_S:
                            scene.menu_move(+1);
                            break;
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:  // Enter do numpad tambem confirma
                        case SDLK_SPACE:
                            // ABERTURA (lider 2026-06-25): na tela "BATALHA!" parada,
                            // Enter ENCARA e comeca o combate. Depois: na vez do jogador
                            // confirma o verbo; fora dela (anuncio/delay) ACELERA o ritmo.
                            if (scene.is_intro()) {
                                scene.start_combat();  // Encarar
                            } else if (scene.waiting_player_input()) {
                                scene.menu_confirm();
                            } else {
                                scene.skip();
                            }
                            break;
                        case SDLK_Q:
                            // "[Q] Resolver sem encarar" (verbo OPT-IN, so TRASH na
                            // abertura). Placeholder neste incremento: a cena loga
                            // "[auto-resolve: a implementar]" e nao faz nada destrutivo.
                            scene.request_auto_resolve();
                            break;
                        default:
                            break;
                    }
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
            scene.update(dt);  // anima os floaters + pacing; nao toca a FSM

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
            if (rmlui_hud_on) {
                // (C) ESTADO abertura vs combate: na intro mostra o brasao; depois de
                // Encarar mostra o cockpit com os valores VIVOS do ator ativo (POCO).
                hud.set_intro(scene.is_intro());
                if (!scene.is_intro()) {
                    if (const auto* a = scene.active_actor(); a != nullptr) {
                        hud.set_hud_values(a->display_name(), "VETOR DO GAMBITO", a->hp(),
                                           a->max_hp(), a->ap(), a->max_ap(), a->mana(),
                                           a->max_mana());
                    }
                }
                hud.set_pixel_size(pw, ph);
                hud.update();
                hud.compose();  // HUD por cima da arena, mesmo contexto GL
            }

            // SMOKE VISUAL: captura 1 PNG no frame alvo (ANTES do swap, lendo o backbuffer)
            // e encerra. Em modo interativo, o swap apresenta na janela.
            if (capture_path != nullptr && frame_no + 1 >= kCaptureAtFrame) {
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
    }  // Render2dGl3 destruido (libera recursos GL) antes de destruir o contexto

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens
