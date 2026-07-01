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

#include "gus/app/screens/battle_cockpit_pills.hpp"  // hit-test de mouse dos pills (A2)
#include "gus/app/screens/battle_hud_model.hpp"  // status_icon_file/index
#include "gus/app/screens/battle_layout.hpp"     // arena_layout (selftest de mouse A2)
#include "gus/app/screens/battle_scene.hpp"
#include "gus/core/asset_paths.hpp"             // caminhos de asset centralizados
#include "gus/domain/combat/combat_enums.hpp"  // StatusId
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
    // SINCRONIA COM O HIT-TEST DE MOUSE (Incremento A2): a regra `.verb` (width 110dp /
    // height 18dp / margin-bottom 4dp / padding 0dp 12dp / border 1dp) e o `#cockpit`
    // padding-left 12dp definem a geometria dos pills que gus/app/screens/battle_cockpit_pills
    // .hpp REPLICA pra decidir em qual pill o clique caiu. Ao mudar esses numeros AQUI,
    // ATUALIZE battle_cockpit_pills.hpp (e RE-MEDIR a origem-Y se mexer no que vem ANTES do
    // .menu -- retrato/nome/vitals -- que empurra o 1o pill).
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
   que dispute) e mantendo border 1dp (= .verb): NAO mexe na geometria => o hit-test de clique
   do Incremento A2 (battle_cockpit_pills.hpp) segue valido. RmlUi 6.3 casa :hover NATIVO (o
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
      <div class="menu">
        <div class="verb"><span class="glyph"></span><span class="lbl">SCAN</span></div>
        <div class="verb"><span class="glyph"></span><span class="lbl">GAMBITO</span></div>
        <div class="verb cyan sel"><span class="glyph"></span><span class="lbl">ATACAR</span></div>
        <div class="verb"><span class="glyph"></span><span class="lbl">DEFENDER</span></div>
        <div class="verb latao"><span class="glyph"></span><span class="lbl">COMPILAR</span></div>
        <div class="verb"><span class="glyph"></span><span class="lbl">FUGIR</span></div>
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
    replace_all(
        "      <div class=\"menu\">\n"
        "        <div class=\"verb\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">SCAN</span></div>\n"
        "        <div class=\"verb\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">GAMBITO</span></div>\n"
        "        <div class=\"verb cyan sel\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">ATACAR</span></div>\n"
        "        <div class=\"verb\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">DEFENDER</span></div>\n"
        "        <div class=\"verb latao\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">COMPILAR</span></div>\n"
        "        <div class=\"verb\"><span class=\"glyph\"></span><span "
        "class=\"lbl\">FUGIR</span></div>\n"
        "      </div>",
        "      <div class=\"menu\">\n"
        "        <div class=\"verb\" data-class-sel=\"sel == 0\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">SCAN</span></div>\n"
        "        <div class=\"verb\" data-class-sel=\"sel == 1\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">GAMBITO</span></div>\n"
        "        <div class=\"verb cyan\" data-class-sel=\"sel == 2\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">ATACAR</span></div>\n"
        "        <div class=\"verb\" data-class-sel=\"sel == 3\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">DEFENDER</span></div>\n"
        "        <div class=\"verb latao\" data-class-sel=\"sel == 4\"><span "
        "class=\"glyph\"></span><span class=\"lbl\">COMPILAR</span></div>\n"
        "        <div class=\"verb\" data-class-sel=\"sel == 5\"><span "
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

// ADR-010 / Incremento A2 (MOUSE): o glintfx NAO expoe hit-test/geometria de elemento (so
// data-model + process_event), entao o CLIQUE REAL e resolvido AQUI, no host, em paralelo ao
// forward pro glintfx (que segue so pro visual/hover interno do RmlUi). Duas conversoes de
// coordenada DISTINTAS, porque o cockpit RCSS e a arena NAO compartilham a escala vertical:
//   - COCKPIT/pills: dp UNIFORME (dp_ratio = pw/960, o mesmo do UiLayer) -> dp = px/dp_ratio
//     em X E Y (ambos por pw). A geometria vem do POCO puro (cockpit_pill_index_at).
//   - ARENA/inimigos: a projecao ESTICA o mundo 960x540 pra (pw x ph), NAO-uniforme (ver
//     viewport_transform world_to_screen) -> world_x = px/pw*960, world_y = px/ph*540 (Y por
//     ph!). O hit-test vem do motor de cena (aim_index_at_arena, casa arena_rect_for_actor).
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
    if (scene.is_intro()) {
        return;  // na ABERTURA os pills nao existem (so o brasao) -> clique nao aciona verbo
    }
    // Clique num PILL de verbo: coordenadas 'dp' do cockpit (uniforme; dp_ratio = pw/960).
    const float dp_ratio = static_cast<float>(pw) / 960.0f;
    const int idx = cockpit_pill_index_at(mx / dp_ratio, my / dp_ratio);
    if (idx >= 0) {
        // Clique = SELECIONA e CONFIRMA. menu_move (delta ate o indice) + menu_confirm; ambos
        // ja sao NO-OP fora do turno do jogador (mesma guarda do teclado) -> seguro em turno
        // de inimigo/combate acabado. menu_move faz WRAP, mas o delta idx-sel (ambos 0..5) cai
        // exato no indice. menu_confirm respeita 'enabled' (verbo sem AP: seleciona, nao aciona).
        scene.menu_move(idx - scene.menu().selected_index());
        scene.menu_confirm();
    }
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

        // A cena monta o encontro de demo e ja le a fila do motor.
        BattleScene scene;
        // (A) Com o HUD externo (glintfx::UiLayer) ATIVO, a cena NAO desenha o cockpit/log a
        // mao - so arena/banner/floaters/fila. Evita cockpits sobrepostos.
        scene.set_hud_external(glintfx_on);

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
                        for (int k = 0; k < 8 &&
                                        scene.menu().selected_verb() != BattleVerb::Atacar;
                             ++k) {
                            scene.menu_move(+1);
                        }
                        scene.menu_confirm();  // Atacar -> entra na mira
                        if (scene.is_aiming()) {
                            scene.aim_confirm();  // confirma o alvo sugerido -> resolve
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

        // DIAGNOSTICO/PROVA (Incremento A2): GUSWORLD_BATTLE_MOUSE_SELFTEST=1 injeta CLIQUES
        // SINTETICOS pelo MESMO battle_mouse_click do loop (com a janela real pw0/ph0), pra
        // PROVAR o roteamento clique->acao sem mouse fisico (dificil de simular numa captura
        // estatica). Assenta ate a vez do jogador, entao: (1) round-trip px->pill de cada
        // verbo; (2) CLICA o pill [ATACAR] e mostra que entrou na MIRA; (3) CLICA o slot de um
        // inimigo e mostra que a mira confirmou naquele alvo. So diagnostico (API publica).
        const bool mouse_selftest = [] {
            const char* e = std::getenv("GUSWORLD_BATTLE_MOUSE_SELFTEST");
            return e != nullptr && e[0] == '1';
        }();
        if (mouse_selftest) {
            const float dpr = static_cast<float>(pw0) / 960.0f;
            if (scene.is_intro()) {
                scene.start_combat();
            }
            for (int i = 0; i < 240 && !scene.combat_over() &&
                            !scene.waiting_player_input();
                 ++i) {
                scene.skip();
                scene.update(1.0f / 60.0f);
            }
            std::cout << "BattlePreview: [mouse-selftest] pw0xph0=" << pw0 << "x" << ph0
                      << " dp_ratio=" << dpr
                      << " waiting_player=" << scene.waiting_player_input() << "\n";
            // (1) round-trip: centro px de cada pill -> cockpit_pill_index_at.
            for (int v = 0; v < gus::app::screens::kCockpitPillCount; ++v) {
                const gus::core::spatial::Rect r = gus::app::screens::cockpit_pill_rect(v);
                const float dpcx = r.x + r.w * 0.5f, dpcy = r.y + r.h * 0.5f;
                const float pxcx = dpcx * dpr, pxcy = dpcy * dpr;
                const int back =
                    gus::app::screens::cockpit_pill_index_at(pxcx / dpr, pxcy / dpr);
                std::cout << "  pill[" << v << "] "
                          << kVerbLabels[static_cast<std::size_t>(v)] << " dp(" << dpcx
                          << "," << dpcy << ") px(" << pxcx << "," << pxcy
                          << ") -> hit=" << back << (back == v ? " OK" : " MISMATCH") << "\n";
            }
            // (2) CLICA o pill ATACAR (indice 2). Espera: entra na mira.
            if (scene.waiting_player_input() && !scene.is_aiming()) {
                const gus::core::spatial::Rect r =
                    gus::app::screens::cockpit_pill_rect(static_cast<int>(BattleVerb::Atacar));
                const float pxcx = (r.x + r.w * 0.5f) * dpr, pxcy = (r.y + r.h * 0.5f) * dpr;
                battle_mouse_click(scene, pxcx, pxcy, pw0, ph0);
                std::cout << "  CLIQUE pill ATACAR px(" << pxcx << "," << pxcy
                          << ") -> is_aiming=" << (scene.is_aiming() ? "on" : "off")
                          << " (esperado on)\n";
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
                          << " (esperado off: confirmou e resolveu o turno)\n";
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
                // pill em hover. Fases 1/2: no CENTRO do pill-alvo (dp * dp_ratio -> px), a mesma
                // conversao uniforme do cockpit (dp_ratio = pw/960) usada no clique do A2.
                if (hover_selftest) {
                    const float dpr = static_cast<float>(pw) / 960.0f;
                    float mx = static_cast<float>(pw) * 0.80f;  // fora da coluna (fases 0 e 3)
                    float my = static_cast<float>(ph) * 0.50f;
                    const int hover_pill = hover_phase == 1   ? 0    // SCAN (nao-selecionado)
                                           : hover_phase == 2 ? 2    // ATACAR (selecionado)
                                                              : -1;  // 0 e 3: sem pill
                    if (hover_pill >= 0) {
                        const gus::core::spatial::Rect r =
                            gus::app::screens::cockpit_pill_rect(hover_pill);
                        mx = (r.x + r.w * 0.5f) * dpr;
                        my = (r.y + r.h * 0.5f) * dpr;
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
    }  // Render2dGl3 destruido (libera recursos GL) antes de destruir o contexto

    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

}  // namespace gus::app::screens
