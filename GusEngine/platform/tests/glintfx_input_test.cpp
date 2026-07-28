// SPDX-License-Identifier: GPL-3.0-or-later
// GusEngine/platform/tests/glintfx_input_test.cpp
//
// Catch2 do GlintfxInput (platform/input, F4-2.3): o adaptador de TECLADO do
// backend novo (glintfx), por POLLING DE NIVEL (decisao do lider) em vez de
// evento SDL KEY_DOWN/UP. TEST-FIRST.
//
// PROVIDER FALSO (FakeKeyboard, abaixo): headless de verdade - nao abre janela,
// nao inicializa SDL nem glintfx::App. O teste dirige o "hardware" chamando
// FakeKeyboard::set_down() e so entao chama GlintfxInput::poll() (o EQUIVALENTE
// deste adaptador a "o quadro passou") - e o poll() que le o provider e
// sintetiza press()/release() no InputMapper.
//
// LICAO DURA da onda F4-1 (ver CLAUDE.md/TODO.md): a peca nova precisa ser
// CHAMADA e EXERCITADA, senao o mutante sobrevive. Este arquivo cobre:
//   1. Apertar/segurar/soltar (poll() idempotente entre transicoes).
//   2. Soltar FORA DE FOCO (clear()) - o teste de contrato da licao F4-1 (o
//      caso "Gus anda sozinho").
//   3. Tecla solta SEM EVENTO EXPLICITO (o proprio ponto do polling: o estado
//      muda "por fora" e o proximo poll() corrige sozinho, sem depender de
//      ninguem ter chamado release()).
//   4. Diagonal (2 teclas simultaneas) + soltura de uma so.
//   5. Acao NAO-movimento via is_action_active (interact).
//   6. Config remapeado (ctor) + set_controls() em runtime (mesma instancia) -
//      com tecla VIRGEM (Q) no caso de set_controls, pra provar reconstrucao
//      de verdade, nao coincidencia de keycode ja watched de fabrica.
//   7. Ctor default equivale a ctor explicito com default_controls().
//   8. Tecla morta pos-clear() (alt-tab andando): com a tecla AINDA
//      pressionada no provider, o movimento tem que VOLTAR no proximo poll().
//   9. Sentinela de seguranca: keycode sem par conhecido (fora de faixa, ou 0
//      explicito) NUNCA e polado - nenhuma tecla fisica ativa a acao por
//      causa dele.
//
// Itens 6/8/9 fecham 3 buracos apontados pela revisao adversarial (QA) desta
// fatia (GAP-QA-1/2/3 nos comentarios dos TEST_CASE correspondentes).

#include <set>

#include <catch2/catch_test_macros.hpp>

#include "gus/domain/input/controls_restore.hpp"
#include "gus/domain/input/input_binding.hpp"
#include "gus/platform/input/glintfx_input.hpp"

using gus::domain::input::default_controls;
using gus::domain::input::InputRemapConfig;
using gus::domain::input::KeyBinding;
using gus::platform::input::GlintfxInput;
using K = glintfx::Key;

namespace {

// Provider FALSO: um std::set<glintfx::Key> que o teste dirige diretamente,
// sem SDL/glintfx::App nenhum. GlintfxInput::poll() so enxerga is_down() via o
// provider() abaixo - nunca o FakeKeyboard em si (prova que o adaptador so
// depende da abstracao KeyStateProvider, nao de um tipo concreto).
class FakeKeyboard {
public:
    void set_down(K key, bool down) {
        if (down) {
            down_.insert(key);
        } else {
            down_.erase(key);
        }
    }

    [[nodiscard]] GlintfxInput::KeyStateProvider provider() {
        return [this](K key) { return down_.find(key) != down_.end(); };
    }

private:
    std::set<K> down_;
};

// Espelha config_with_move_forward_remapped_to_j() de sdl_input_test.cpp:
// move_forward remapeado de W (fabrica) pra J - unico binding, sem manter W.
InputRemapConfig config_with_move_forward_remapped_to_j() {
    InputRemapConfig cfg = default_controls();
    for (auto& action : cfg.actions) {
        if (action.action_name == "move_forward") {
            action.keys = {KeyBinding{.keycode = 74}};  // 'J' maiusculo (esquema Godot)
        }
    }
    return cfg;
}

// GAP-QA-1 (revisao adversarial): variante que remapeia move_forward pra Q -
// tecla VIRGEM (conferido em controls_restore.cpp: as 28 acoes de
// default_controls() usam W/S/A/D/Shift/Enter/Escape/Up/Down/Left/Right/Z/X/
// Space/1/2/3/Tab/4/I/J - Q nao aparece em NENHUMA delas). Diferente do J
// acima (que ja era watched de fabrica por diary_open='J' - um remap pra J
// nao prova que set_controls() RECONSTRUIU watched_keys_, so que o keycode
// continua na lista por coincidencia), Q so pode ficar watched se
// rebuild_watched_keys() realmente rodar de novo. Usado so no teste de
// set_controls() (a peca que a sabotagem provada pelo QA mirou).
InputRemapConfig config_with_move_forward_remapped_to_q() {
    InputRemapConfig cfg = default_controls();
    for (auto& action : cfg.actions) {
        if (action.action_name == "move_forward") {
            action.keys = {KeyBinding{.keycode = 81}};  // 'Q' maiusculo (esquema Godot)
        }
    }
    return cfg;
}

}  // namespace

TEST_CASE("GlintfxInput sem nada nao move", "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());
    in.poll();
    REQUIRE(in.dx() == 0);
    REQUIRE(in.dy() == 0);
    REQUIRE(in.run() == false);
}

TEST_CASE("GlintfxInput apertar/soltar D move +X e volta", "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());

    kb.set_down(K::D, true);
    in.poll();
    REQUIRE(in.dx() == 1);

    kb.set_down(K::D, false);
    in.poll();
    REQUIRE(in.dx() == 0);
}

TEST_CASE("GlintfxInput W move -Y (frente)", "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());
    kb.set_down(K::W, true);
    in.poll();
    REQUIRE(in.dy() == -1);
}

TEST_CASE("GlintfxInput segurar por varios quadros nao quebra (poll idempotente "
          "entre transicoes)",
          "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());

    kb.set_down(K::D, true);
    in.poll();
    REQUIRE(in.dx() == 1);
    // Mais 3 quadros SEM nenhuma mudanca no provider - dx tem que continuar 1,
    // nao "somar" nem grudar num estado invalido.
    in.poll();
    REQUIRE(in.dx() == 1);
    in.poll();
    REQUIRE(in.dx() == 1);
    in.poll();
    REQUIRE(in.dx() == 1);
}

// O TESTE DE CONTRATO da licao F4-1 ("Gus anda sozinho"): clear() (perder foco)
// zera o movimento NA HORA, sem depender de nenhum poll() seguinte.
TEST_CASE("GlintfxInput clear() (perder foco) zera o movimento na hora",
          "[glintfx_input][regressao-dialogo]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());

    kb.set_down(K::W, true);
    kb.set_down(K::LeftShift, true);
    in.poll();
    REQUIRE(in.dy() == -1);
    REQUIRE(in.run() == true);

    in.clear();
    REQUIRE(in.dy() == 0);
    REQUIRE(in.run() == false);
}

// GAP-QA-2 (revisao adversarial): "tecla morta" pos-clear() - cenario
// alt-tab-andando. A tecla NUNCA e solta no FakeKeyboard (o jogador nao soltou
// W fisicamente; so a janela perdeu e recuperou o foco) - clear() zera o
// "pressed" GUARDADO de cada WatchedKey (alem do InputMapper), entao o
// PROXIMO poll() enxerga uma borda de SUBIDA nova (false->true) e o movimento
// VOLTA sozinho. Se clear() deixasse de zerar esse estado guardado, o
// proximo poll() veria false==false por engano de estado (nao ha diferenca -
// pra este WatchedKey, "pressed" ainda diria true) e ficaria sem detectar
// borda - o movimento nunca voltaria ate o jogador soltar e reapertar W. Este
// REQUIRE morre exatamente nesse mutante.
TEST_CASE("GlintfxInput: apos clear(), se a tecla continuar fisicamente "
          "pressionada, o PROXIMO poll() detecta a borda de novo e o "
          "movimento VOLTA (alt-tab andando)",
          "[glintfx_input][regressao-dialogo]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());

    kb.set_down(K::W, true);
    in.poll();
    REQUIRE(in.dy() == -1);

    in.clear();
    REQUIRE(in.dy() == 0);

    // ... o foco volta aqui (ex.: alt-tab de volta pro jogo) - o jogador NUNCA
    // soltou W fisicamente, o FakeKeyboard continua reportando W pressionada ...
    in.poll();
    REQUIRE(in.dy() == -1);
}

// O PONTO CENTRAL do polling de nivel (decisao do lider): a tecla e "solta" no
// FakeKeyboard SEM nenhum evento explicito de release ser disparado pra
// GlintfxInput - so o ESTADO consultado pelo provider muda. O poll() SEGUINTE
// tem que perceber a transicao sozinho e zerar o movimento - e exatamente a
// classe de bug "tecla grudada" que o polling mata por construcao (ao contrario
// do SdlInput por evento, que dependeria de um KEY_UP chegar).
TEST_CASE("GlintfxInput: tecla solta SEM evento explicito - o proximo poll() "
          "detecta sozinho e para o movimento",
          "[glintfx_input][polling]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());

    kb.set_down(K::W, true);
    in.poll();
    REQUIRE(in.dy() == -1);

    // ... nenhum "evento" e disparado aqui - so o estado do provider muda, como
    // se o SO tivesse silenciosamente deixado de reportar a tecla pressionada
    // (o cenario que quebrava o backend por evento) ...
    kb.set_down(K::W, false);
    in.poll();
    REQUIRE(in.dy() == 0);
}

TEST_CASE("GlintfxInput diagonal (2 teclas simultaneas) e soltura de uma so",
          "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());

    kb.set_down(K::D, true);
    kb.set_down(K::W, true);
    in.poll();
    REQUIRE(in.dx() == 1);
    REQUIRE(in.dy() == -1);

    kb.set_down(K::D, false);
    in.poll();
    REQUIRE(in.dx() == 0);
    REQUIRE(in.dy() == -1);  // W continua segurada - so D soltou
}

TEST_CASE("GlintfxInput teclas opostas cancelam", "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());
    kb.set_down(K::A, true);
    kb.set_down(K::D, true);
    in.poll();
    REQUIRE(in.dx() == 0);
}

TEST_CASE("GlintfxInput Shift ativa run", "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());
    kb.set_down(K::LeftShift, true);
    in.poll();
    REQUIRE(in.run() == true);
}

// Acao NAO-movimento via is_action_active - default_controls() liga "interact" a
// Enter (kKeyEnter, ver controls_restore.cpp).
TEST_CASE("GlintfxInput is_action_active cobre acao nao-movimento (interact)",
          "[glintfx_input]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());
    REQUIRE(in.is_action_active("interact") == false);

    kb.set_down(K::Enter, true);
    in.poll();
    REQUIRE(in.is_action_active("interact") == true);

    kb.set_down(K::Enter, false);
    in.poll();
    REQUIRE(in.is_action_active("interact") == false);
}

// M2-equivalente: config remapeado (ctor) muda tecla->acao (J move_forward, nao
// mais W) - espelha o teste analogo de sdl_input_test.cpp.
TEST_CASE("GlintfxInput(config): config remapeado muda tecla->acao (J "
          "move_forward, nao mais W)",
          "[glintfx_input][m2-controles]") {
    FakeKeyboard kb;
    GlintfxInput in(config_with_move_forward_remapped_to_j(), kb.provider());

    kb.set_down(K::J, true);
    in.poll();
    REQUIRE(in.dy() == -1);
    kb.set_down(K::J, false);
    in.poll();
    REQUIRE(in.dy() == 0);

    // A tecla de FABRICA (W) NAO aciona mais move_forward.
    kb.set_down(K::W, true);
    in.poll();
    REQUIRE(in.dy() == 0);
}

// set_controls() (troca em RUNTIME, sem reconstruir o GlintfxInput) - mesma
// semantica de SdlInput::set_controls, espelhando o teste analogo.
//
// GAP-QA-1 (revisao adversarial): usa Q (config_with_move_forward_remapped_to_q,
// tecla VIRGEM - nunca watched de fabrica) em vez de J. Prova de VERDADE que
// set_controls() RECONSTROI watched_keys_: se a sabotagem trocasse o
// InputMapper mas NAO chamasse rebuild_watched_keys() de novo, Q jamais teria
// entrado na lista polada e o REQUIRE de "Q move" abaixo falharia (dy() ficaria
// 0 pra sempre, mesmo com Q fisicamente pressionada) - com J este mesmo teste
// passaria por COINCIDENCIA (diary_open='J' ja deixava J watched no config de
// fabrica, entao a lista "velha" ainda conteria J mesmo sem reconstruir).
TEST_CASE("GlintfxInput::set_controls troca o mapa em runtime (mesma instancia)",
          "[glintfx_input][m2-controles]") {
    FakeKeyboard kb;
    GlintfxInput in(kb.provider());  // nasce com default_controls() (W move_forward)

    kb.set_down(K::W, true);
    in.poll();
    REQUIRE(in.dy() == -1);

    in.set_controls(config_with_move_forward_remapped_to_q());

    // W (ainda fisicamente "pressionada" antes da troca) NAO fica "presa": o
    // InputMapper novo comeca com pressed_ vazio.
    REQUIRE(in.dy() == 0);

    // A tecla nova (Q, virgem) agora move; W sozinha nao move mais.
    kb.set_down(K::Q, true);
    in.poll();
    REQUIRE(in.dy() == -1);

    kb.set_down(K::Q, false);
    // W continua fisicamente "pressionada" no FakeKeyboard desde antes da
    // troca - o poll() seguinte tem que CONTINUAR sem mover por W (a troca
    // recalculou watched_keys_ para o esquema novo, entao W nem e mais
    // observada por este esquema).
    in.poll();
    REQUIRE(in.dy() == 0);
}

// GAP-QA-3 v2 (revisao adversarial - a v1 sobreviveu a uma sabotagem real):
// keycode SEM par conhecido (sentinela) nunca entra em watched_keys_ - prova de
// seguranca. Vetor real: controls.json/save adulterado, ou uma acao futura
// so-de-gamepad com keycode=0 (o InputMapper ja trata 0 como "sem binding":
// InputMapper::press(0) e no-op, mas aqui a prova e que o adaptador nem CHEGA a
// chamar press/release por causa desse binding).
//
// LICAO DA v1 (sobrevivencia real, nao hipotetica): a v1 so apertava W e S (as
// teclas de fabrica de move_forward/move_backward) pra provar que nao ativavam
// a acao. Uma sabotagem real em rebuild_watched_keys() trocou o `continue` do
// caso nullopt por "vigia a tecla A" (key = glintfx::Key::A em vez de pular) -
// e passou pelos 2 REQUIRE porque W/S nunca chegaram nem perto de A. O
// invariante VERDADEIRO nao e "estas 2 teclas nao ativam", e "NENHUMA tecla do
// enum ativa" - o sabotador escolhe a tecla, entao so uma VARREDURA COMPLETA
// (mesma tecnica do TEST_CASE "cobertura total" de key_translation_glintfx_
// test.cpp, None+1..Count) fecha isso: nenhum indice fica de fora pro
// sabotador se esconder.
//
// ISOLAMENTO: a config usada aqui zera TODAS as outras acoes (so move_forward
// fica com o binding sem par, keycode 999999 - fora de qualquer faixa real,
// nao e nenhum par da tabela key_translation_glintfx.cpp) - senao teclas
// LEGITIMAS de outras acoes (ex.: A de move_left) tambem passariam no sweep e
// a assercao "nenhuma tecla ativa ESTA acao invalida" ficaria confundida com
// "nenhuma tecla ativa NADA", que nao e o que queremos provar.
TEST_CASE("GlintfxInput: keycode sem par conhecido (sentinela) nunca e polado - "
          "varre TODO o enum glintfx::Key e nenhuma tecla ativa a acao invalida",
          "[glintfx_input][sentinela]") {
    InputRemapConfig cfg = default_controls();
    for (auto& action : cfg.actions) {
        if (action.action_name == "move_forward") {
            action.keys = {KeyBinding{.keycode = 999999}};  // sem par conhecido
        } else {
            action.keys.clear();  // ISOLAMENTO: nenhuma outra acao fica bound
        }
    }

    // None (indice 0) fica de fora - nao e tecla real (mesmo criterio do
    // TEST_CASE de cobertura total de key_translation_glintfx_test.cpp).
    for (int i = 1; i < static_cast<int>(K::Count); ++i) {
        const auto key = static_cast<K>(i);
        INFO("indice do enum pressionado sozinho: " << i);

        // Instancia NOVA por indice (com FakeKeyboard novo): isola cada tecla
        // do enum, sem estado residual de uma iteracao vazando pra proxima.
        FakeKeyboard kb;
        GlintfxInput in(cfg, kb.provider());
        kb.set_down(key, true);
        in.poll();

        REQUIRE(in.dy() == 0);
        REQUIRE(in.is_action_active("move_forward") == false);
    }
}

// Nao-regressao (menor, complementar ao sweep acima): com a config NAO
// isolada (move_forward=999999 sem par, move_backward=0 sentinela explicita,
// resto de fabrica intacto), o restante do esquema continua funcionando ao
// redor dos 2 bindings invalidos - move_left/move_right (A/D) nao foram
// mexidos.
TEST_CASE("GlintfxInput: bindings sentinela (999999 e 0) nao afetam o resto "
          "do esquema de controles ao redor deles",
          "[glintfx_input][sentinela]") {
    InputRemapConfig cfg = default_controls();
    for (auto& action : cfg.actions) {
        if (action.action_name == "move_forward") {
            action.keys = {KeyBinding{.keycode = 999999}};
        }
        if (action.action_name == "move_backward") {
            action.keys = {KeyBinding{.keycode = 0}};  // sentinela "sem binding"
        }
    }

    FakeKeyboard kb;
    GlintfxInput in(cfg, kb.provider());

    kb.set_down(K::W, true);
    kb.set_down(K::S, true);
    in.poll();
    REQUIRE(in.dy() == 0);
    REQUIRE(in.is_action_active("move_forward") == false);
    REQUIRE(in.is_action_active("move_backward") == false);

    kb.set_down(K::A, true);
    in.poll();
    REQUIRE(in.dx() == -1);
}

// Nao-regressao: ctor default (GlintfxInput(provider)) equivale a construir
// explicitamente com default_controls().
TEST_CASE("GlintfxInput(): ctor default equivale a GlintfxInput(default_controls())",
          "[glintfx_input][m2-controles]") {
    FakeKeyboard kb_a;
    FakeKeyboard kb_b;
    GlintfxInput default_ctor(kb_a.provider());
    GlintfxInput explicit_ctor(default_controls(), kb_b.provider());

    kb_a.set_down(K::W, true);
    kb_b.set_down(K::W, true);
    default_ctor.poll();
    explicit_ctor.poll();
    REQUIRE(default_ctor.dy() == explicit_ctor.dy());

    kb_a.set_down(K::D, true);
    kb_b.set_down(K::D, true);
    default_ctor.poll();
    explicit_ctor.poll();
    REQUIRE(default_ctor.dx() == explicit_ctor.dx());

    kb_a.set_down(K::LeftShift, true);
    kb_b.set_down(K::LeftShift, true);
    default_ctor.poll();
    explicit_ctor.poll();
    REQUIRE(default_ctor.run() == explicit_ctor.run());
}
