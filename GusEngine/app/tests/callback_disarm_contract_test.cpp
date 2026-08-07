// SPDX-License-Identifier: Apache-2.0
// GusEngine/app/tests/callback_disarm_contract_test.cpp
//
// CALLBACK-DTOR-DESARME - o teste que prova a PREMISSA do conserto, nao o conserto.
//
// O conserto (destrutor das 4 telas: `if (ui_) ui_->set_*_callback(nullptr);`) NAO e
// observavel por teste de runtime, e isso esta declarado, nao escondido: as classes de
// tela vivem em namespace ANONIMO dentro do .cpp (inalcancaveis daqui), e no caminho
// normal `exit()` ja fez `ui_.reset()` ANTES do destrutor rodar - o desarme e no-op ali.
// O unico caminho em que ele age e uma excecao escapando de enter()/tick()/handle_event(),
// que o codigo de hoje nao lanca (MESMA razao pela qual a fatia irma CALLBACK-DTOR-ORDER
// nao conseguiu teste de runtime; ver o docstring de tools/callback_dtor_order.py).
//
// O QUE ISTO TESTA, ENTAO: a PREMISSA em que o conserto se apoia - "passar nullptr
// DESARMA de verdade o callback no glintfx do pin". Se um bump futuro do glintfx quebrar
// essa semantica, o `set_*_callback(nullptr)` das 4 telas vira um no-op silencioso e
// NENHUM outro teste fica vermelho: o conserto morre sem aviso. Este teste e o alarme.
// Ele mede o pin (v0.30.0 quando escrito), nao o nosso codigo - de proposito.
//
// As DUAS familias que as telas usam de fato entram aqui, uma por TEST_CASE:
//   - set_hover_callback  -> title_menu_loop / difficulty_menu_loop / system_menu_loop
//   - set_click_callback  -> battle_preview
//
// CONTRATO DO GLINTFX (v0.30.0), lido no fonte pinado, nao presumido:
//   - `void set_*_callback(std::function<...> cb)` por VALOR; nao existe metodo dedicado
//     de "clear" nesta familia (conferido em glintfx/include/glintfx/ui_layer.hpp) - o
//     desarme E passar nullptr/std::function vazio;
//   - o listener guarda com `if (!cb_ || !*cb_) return;` (glintfx/src/rml/bootstrap.cpp,
//     ClickEventListener/HoverEventListener::ProcessEvent), entao vazio = no-op identico
//     a "nunca houve callback";
//   - o proprio ~UiLayer::Impl() deles desarma assim (`clock.set_cursor_callback(nullptr)`
//     como 1a instrucao, glintfx/src/ui_layer.cpp:138), fixando um bug real de
//     heap-use-after-free que so o ASan pegou. E contrato deles, nao gambiarra nossa.
//
// HARNESS: MESMA receita GlTestEnv/try_boot_gl dos irmaos (save_load_menu_interaction_
// test.cpp / battle_preview_interaction_test.cpp), copiada AUTO-CONTIDA por convencao
// desta suite - janela SDL ESCONDIDA + contexto GL real (Xvfb :99). Degrada seguro
// (0 assercoes, NUNCA falha) se GL/display estiver indisponivel.
//
// SEM FONTE de proposito: o documento e feito de <div> de tamanho FIXO e SEM texto, entao
// nao depende de metrica de fonte nenhuma (diferente do harness de save_load, que MEDE
// posicao e por isso precisa injetar a fonte real). Menos acoplamento, menos flake.

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <SDL3/SDL.h>

#include <glintfx/ui_event.hpp>
#include <glintfx/ui_layer.hpp>

#include "gus/platform/rmlui/gl3_loader.hpp"
#include "tmp_dir_test_support.hpp"

namespace {

constexpr int kWinW = 960;
constexpr int kWinH = 540;

// Caixa de 200x100 na origem (com id) + uma 2a caixa logo abaixo, pra poder SAIR de uma
// e ENTRAR na outra (o dedup de hover do glintfx so redispara quando o id MUDA - ver o
// doc-comment de set_hover_callback em ui_layer.hpp).
constexpr const char* kRml =
    "<rml><head><style>\n"
    "body { width: 960px; height: 540px; }\n"
    "div  { display: block; width: 200px; height: 100px; background-color: #808080; }\n"
    "</style></head><body>\n"
    "<div id=\"caixa_a\"/>\n"
    "<div id=\"caixa_b\"/>\n"
    "</body></rml>\n";

// Pontos garantidamente DENTRO de cada caixa (200x100 empilhadas a partir de 0,0).
constexpr float kAx = 100.0f, kAy = 50.0f;
constexpr float kBx = 100.0f, kBy = 150.0f;

// MESMA receita de bootstrap GL dos harnesses irmaos - `ok=false` = ambiente sem
// GL/display, e o TEST_CASE degrada em vez de falhar.
struct GlTestEnv {
    SDL_Window* window = nullptr;
    SDL_GLContext gl = nullptr;
    bool ok = false;

    ~GlTestEnv() {
        if (gl != nullptr) SDL_GL_DestroyContext(gl);
        if (window != nullptr) SDL_DestroyWindow(window);
    }
};

GlTestEnv try_boot_gl() {
    GlTestEnv env;
    if (!SDL_WasInit(SDL_INIT_VIDEO) && !SDL_InitSubSystem(SDL_INIT_VIDEO)) return env;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    env.window = SDL_CreateWindow("callback_disarm_contract_test", kWinW, kWinH,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (env.window == nullptr) return env;

    env.gl = SDL_GL_CreateContext(env.window);
    if (env.gl == nullptr) return env;
    SDL_GL_MakeCurrent(env.window, env.gl);

    if (!gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(SDL_GL_GetProcAddress))) {
        return env;
    }
    env.ok = true;
    return env;
}

// Grava kRml num arquivo temporario. unique_temp_dir (NAO ScopedTempDir): o UiLayer
// referencia o caminho enquanto vive - MESMA nota do harness de save_load.
std::filesystem::path write_rml() {
    const std::filesystem::path stage =
        gus::test_support::unique_temp_dir("gusworld_callback_disarm_contract_test");
    std::error_code ec;
    std::filesystem::create_directories(stage, ec);
    const std::filesystem::path rml_path = stage / "caixas.rml";
    std::ofstream f(rml_path);
    f << kRml;
    return rml_path;
}

// 2x update() por seguranca (layout assentado apos o load() - MESMA cautela dos irmaos).
bool boot_ui(glintfx::UiLayer& ui, const std::filesystem::path& rml_path) {
    if (!ui.ok()) return false;
    ui.set_asset_base_url(rml_path.parent_path().string().c_str());
    if (!ui.load(rml_path.string().c_str())) return false;
    ui.set_viewport(kWinW, kWinH);
    ui.set_dp_ratio(1.0f);
    ui.update();
    ui.update();
    return true;
}

void move_mouse(glintfx::UiLayer& ui, float x, float y) {
    glintfx::UiEvent ev;
    ev.type = glintfx::UiEvent::Type::MouseMove;
    ev.x = x;
    ev.y = y;
    ui.process_event(ev);
}

void click_mouse(glintfx::UiLayer& ui, float x, float y) {
    move_mouse(ui, x, y);
    glintfx::UiEvent down;
    down.type = glintfx::UiEvent::Type::MouseButton;
    down.button = 0;
    down.pressed = true;
    ui.process_event(down);
    glintfx::UiEvent up = down;
    up.pressed = false;
    ui.process_event(up);
}

}  // namespace

// ------------------------------------------------------------------ hover (3 telas)

TEST_CASE("glintfx (contrato do pin): set_hover_callback(nullptr) DESARMA - depois do "
          "desarme o callback NAO e mais chamado, e a premissa do destrutor das telas "
          "de menu (CALLBACK-DTOR-DESARME) segue de pe",
          "[callback_disarm][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export DISPLAY=:99).");
        SUCCEED("ambiente sem GL - contrato nao exercitado");
        return;
    }

    const std::filesystem::path rml_path = write_rml();
    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kWinW,
                                                 /*logical_height=*/kWinH,
                                                 /*load_gl=*/true,
                                                 /*dp_ratio=*/1.0f});
    if (!boot_ui(ui, rml_path)) {
        INFO("UiLayer::ok()/load() falhou neste ambiente - harness pulado.");
        SUCCEED("UiLayer indisponivel - contrato nao exercitado");
        return;
    }

    int hits = 0;
    ui.set_hover_callback([&hits](const char*, bool) { ++hits; });

    // ARMADO: entrar na caixa A tem de disparar. Se ISTO falhar, o harness esta
    // errado (nao o contrato) - e a assercao que impede o teste de "passar de graca"
    // medindo zero contra zero.
    move_mouse(ui, kAx, kAy);
    REQUIRE(hits > 0);

    // DESARMADO: a partir daqui, NADA mais pode chegar ao callback.
    const int hits_ao_desarmar = hits;
    ui.set_hover_callback(nullptr);

    // Atravessa A -> B -> A -> fora: cada troca de id redispararia o callback se ele
    // ainda estivesse armado (o dedup do glintfx so suprime o MESMO id repetido).
    move_mouse(ui, kBx, kBy);
    move_mouse(ui, kAx, kAy);
    move_mouse(ui, kBx, kBy);
    move_mouse(ui, 900.0f, 500.0f);
    CHECK(hits == hits_ao_desarmar);

    // REARMAR volta a funcionar (prova que o desarme e um ESTADO reversivel do
    // glintfx, nao um efeito colateral de o documento ter parado de despachar -
    // sem isto, um documento morto daria o mesmo "hits parado" e o teste mentiria).
    //
    // ACHADO MEDIDO nesta fatia (v0.30.0), nao obvio - registrado pra ninguem
    // re-diagnosticar: o desarme CONGELA a maquina de dedup do hover. O
    // HoverEventListener::ProcessEvent do glintfx sai no `if (!cb_ || !*cb_) return;`
    // ANTES de atualizar o `current_hover_id_` dele, entao todo movimento feito com o
    // callback desarmado e invisivel pra esse estado: ele continua valendo "caixa_a"
    // (o ultimo id visto ARMADO), mesmo tendo o mouse passeado por B e saido pra fora.
    // Consequencia pratica: rearmar e voltar pra "caixa_a" NAO dispara (o dedup ve o
    // MESMO id) - a primeira versao deste teste falhou exatamente assim. A sonda de
    // rearme tem de mirar um id DIFERENTE do que estava em hover quando se desarmou.
    // Nao e bug: o contrato publico do set_hover_callback so promete dedup por id.
    ui.set_hover_callback([&hits](const char*, bool) { ++hits; });
    move_mouse(ui, kBx, kBy);
    CHECK(hits > hits_ao_desarmar);
}

// ------------------------------------------------------------- click (battle_preview)

TEST_CASE("glintfx (contrato do pin): set_click_callback(nullptr) DESARMA - depois do "
          "desarme o callback NAO e mais chamado, e a premissa do destrutor da "
          "BattleScreen (CALLBACK-DTOR-DESARME) segue de pe",
          "[callback_disarm][gl]") {
    GlTestEnv env = try_boot_gl();
    if (!env.ok) {
        INFO("GL/display indisponivel neste ambiente (sem Xvfb) - harness pulado "
             "(degradacao segura, 0 assercoes). Rode com Xvfb :99 (export DISPLAY=:99).");
        SUCCEED("ambiente sem GL - contrato nao exercitado");
        return;
    }

    const std::filesystem::path rml_path = write_rml();
    glintfx::UiLayer ui(glintfx::UiLayer::Config{/*logical_width=*/kWinW,
                                                 /*logical_height=*/kWinH,
                                                 /*load_gl=*/true,
                                                 /*dp_ratio=*/1.0f});
    if (!boot_ui(ui, rml_path)) {
        INFO("UiLayer::ok()/load() falhou neste ambiente - harness pulado.");
        SUCCEED("UiLayer indisponivel - contrato nao exercitado");
        return;
    }

    int hits = 0;
    ui.set_click_callback([&hits](const char*) { ++hits; });

    move_mouse(ui, kAx, kAy);
    click_mouse(ui, kAx, kAy);
    REQUIRE(hits > 0);

    const int hits_ao_desarmar = hits;
    ui.set_click_callback(nullptr);

    click_mouse(ui, kAx, kAy);
    click_mouse(ui, kBx, kBy);
    CHECK(hits == hits_ao_desarmar);

    // REARMAR volta a funcionar (MESMO racional do teste de hover acima).
    ui.set_click_callback([&hits](const char*) { ++hits; });
    click_mouse(ui, kAx, kAy);
    CHECK(hits > hits_ao_desarmar);
}
