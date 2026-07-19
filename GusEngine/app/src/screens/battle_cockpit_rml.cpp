// gus/app/src/screens/battle_cockpit_rml.cpp
//
// Ver header. AC-E11 A2: implementacao da montagem RML/RCSS do cockpit, extraida de
// battle_preview.cpp (ADR-019). Corpo das funcoes MOVIDO verbatim - nenhuma logica mudou,
// so o arquivo que a hospeda.

#include "gus/app/screens/battle_cockpit_rml.hpp"

#include <cstdlib>  // std::getenv
#include <filesystem>
#include <fstream>

#include "gus/app/screens/battle_assets.hpp"  // join/resolve_asset_dir/retrato_file_for (AC-E11 A3)
#include "gus/core/asset_paths.hpp"
#include "gus/domain/combat/combat_actor.hpp"

// Pasta das fontes (.ttf), embutida pelo CMake (ADR-010 F2a). So usada no caminho glintfx
// (cockpit BAKED/LIVE): o @font-face do RCSS aponta pra ca. Fallback vazio se ausente.
#ifndef GUSWORLD_FONTS_DIR
#define GUSWORLD_FONTS_DIR ""
#endif

namespace gus::app::screens {

namespace {

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

/* ---- barras AP (latao) / Mana (cyan) — COCKPIT-BARRAS-MANA-AP (pedido do lider ao vivo
   2026-07-03): antes eram pips/bolinhas (.pip.ap/.pip.mana, radiais com glow); agora
   reusam a MESMA familia visual da .hpbar (geometria IDENTICA: width/height/border-radius/
   margin-top) — so o decorator (gradiente) e o box-shadow trocam de tema, igual o .pip.ap/
   .pip.mana ja faziam (latao vs cyan). NAO e barra proporcional (a .hpbar tambem nao e —
   ver comentario acima; escala real e trabalho futuro reportado la). O rotulo ganha o
   numero (mesma familia do .hpnum/.v) pra nao perder a leitura discreta "3 de 4" que o
   pip dava — NOTA DE UX pendente do lider: ele quer TESTAR barra tambem pro AP (sabendo
   que pip costuma ler melhor pra contagem em relance); reverter .apbar/.apnum pro
   .pip.ap antigo (preservado no historico git) e trivial se ele preferir ao vivo. */
.apnum, .mananum { margin-top: 5dp; font-size: 10dp; color: #8fa6b4; }
.apnum .v { color: #E8A33D; }
.mananum .v { color: #22D3EE; }
.apbar {
  width: 110dp; height: 12dp; border-radius: 6dp; margin-top: 3dp;
  decorator: vertical-gradient( #ffe6a8 #E8A33D );  /* latao: creme -> ambar (tema AP) */
  box-shadow: #E8A33D 0dp 0dp 16dp 2dp;
}
.manabar {
  width: 110dp; height: 12dp; border-radius: 6dp; margin-top: 3dp;
  decorator: vertical-gradient( #c2f6ff #22D3EE );  /* cyan: gelo -> cyan (tema Mana) */
  box-shadow: #22D3EE 0dp 0dp 16dp 2dp;
}

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
        <div class="apnum">AP <span class="v">3</span> / 4</div>
        <div class="apbar"></div>
        <div class="mananum">MANA <span class="v">2</span> / 5</div>
        <div class="manabar"></div>
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

}  // namespace gus::app::screens
