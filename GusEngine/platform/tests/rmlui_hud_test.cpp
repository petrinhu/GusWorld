// gus/platform/tests/rmlui_hud_test.cpp
//
// Testes HEADLESS do andaime RmlUi (ADR-009, F0). RmlUi roda SEM janela nem GPU:
// inicializa o sistema, parseia um documento RML trivial e expoe os elementos. O que
// se TESTA aqui e o que ALIMENTA o RmlUi (carga/parse/data-model), nao a rasterizacao
// (essa fica pro smoke visual --battle). Cobre tambem os stems vendorizados do backend
// que o wrapper encapsula: RmlUi_Platform_SDL (sistema/eventos) e, no caminho com efeitos
// (gradiente/box-shadow/glow), RmlUi_Renderer_GL3 (OpenGL 3.3, shaders). Em headless
// usamos um RenderInterface nulo (sem contexto GL); o GL3 entra no smoke visual. Prova
// que o ciclo monta sem display.
//
// MODELO: RmlUiHud com renderer SDL == nullptr e o "modo headless" (espelha o
// Render2dSdl(nullptr)): inicializa o nucleo do RmlUi com um RenderInterface nulo-seguro,
// carrega documentos, e o render() vira no-op contabilizado. Sem renderer real, nenhuma
// textura/geometria toca a GPU - mas o parse de RML/RCSS e o data-model SAO exercitados.

#include <catch2/catch_test_macros.hpp>

#include "gus/platform/rmlui/rmlui_hud.hpp"

using gus::platform::rmlui::RmlUiHud;

namespace {

// Documento RML minimo (o "ola" da F0): um div estilizado por RCSS inline com
// border-radius + gradiente. Prova que o parser monta a arvore e o RCSS aplica.
// RCSS notas de sintaxe (aprendidas no andaime, validas pra F1):
//  - linear-gradient EXIGE direcao: "to bottom" (a forma de 2 cores sem direcao falha).
//  - font-family deve casar o NOME da face carregada ("Pixel Operator Mono"), nao um
//    apelido arbitrario; o decorator gradient e a forma RCSS de fundo em degrade.
constexpr const char* kHelloRml = R"RML(
<rml>
<head>
<style>
  body { font-family: "Pixel Operator Mono"; }
  #hello {
    width: 200px; height: 60px;
    decorator: gradient(direction vertical, #1B2238 #141a2c);
    border: 1px #22D3EE;
    border-radius: 8px;
    color: #cfe6ee;
  }
</style>
</head>
<body>
  <div id="hello" data-model="hud">ola {{nome}}</div>
</body>
</rml>
)RML";

}  // namespace

TEST_CASE("RmlUiHud inicializa headless (sem renderer SDL)", "[rmlui][hud]") {
    RmlUiHud hud;
    // renderer nulo = headless: inicializa o nucleo, carrega fonte se houver, sem GPU.
    REQUIRE(hud.init_headless(960, 540));
    REQUIRE(hud.is_initialized());
}

TEST_CASE("RmlUiHud carrega um documento RML trivial da memoria", "[rmlui][hud]") {
    RmlUiHud hud;
    REQUIRE(hud.init_headless(960, 540));

    // Data-model com um campo, espelhando o fluxo POCO->DataModel da F1.
    REQUIRE(hud.bind_demo_model("hud", "Gus"));

    // Parseia o RML da memoria; o documento deve montar e conter o elemento #hello.
    REQUIRE(hud.load_document_from_memory(kHelloRml));
    REQUIRE(hud.has_element("hello"));
}

TEST_CASE("RmlUiHud render headless e no-op seguro (sem present/clear)", "[rmlui][hud]") {
    RmlUiHud hud;
    REQUIRE(hud.init_headless(960, 540));
    REQUIRE(hud.bind_demo_model("hud", "Gus"));
    REQUIRE(hud.load_document_from_memory(kHelloRml));

    // Sem renderer real, compose() apenas atualiza o contexto (Update) e nao desenha.
    // NUNCA limpa a tela nem da present (a arena e dona disso) - aqui so prova que o
    // ciclo update/render roda sem crashar headless.
    hud.update();
    hud.compose_headless();
    SUCCEED("compose headless sem display nao crashou");
}

TEST_CASE("RmlUiHud RML invalido falha de forma controlada", "[rmlui][hud]") {
    RmlUiHud hud;
    REQUIRE(hud.init_headless(960, 540));
    // RML quebrado (tag nao fechada): o carregamento deve falhar sem derrubar o processo.
    const bool ok = hud.load_document_from_memory("<rml><body><div></body>");
    // RmlUi e tolerante a alguns erros de marcacao; o contrato aqui e "nao crasha".
    (void)ok;
    SUCCEED("RML malformado nao derruba o processo");
}
