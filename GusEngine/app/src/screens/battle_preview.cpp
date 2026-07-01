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
  border: 1dp #E8A33D; box-shadow: #E8A33D99 0dp 0dp 22dp -6dp; }
/* anel cyan tracejado GIRANDO (transform animado) */
.ring.r2 { top: 16dp; left: 16dp; width: 116dp; height: 116dp;
  border: 1dp #22D3EE; box-shadow: #22D3EE66 0dp 0dp 16dp -6dp;
  animation: spin 18s linear infinite; }
.ring.r3 { top: 34dp; left: 34dp; width: 80dp; height: 80dp; border: 1dp #E8A33D66; }
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

        bool running = true;
        bool have_last = false;
        unsigned long long last_ns = 0;
        int glintfx_injected = 0;  // SMOKE: conta eventos injetados na UI (prova do pipeline)
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
                ui->update();
                ui->render();  // UI por cima da arena, mesmo contexto GL
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
