// SPDX-License-Identifier: GPL-3.0-or-later
// gus/platform/input/glintfx_input.hpp
//
// GlintfxInput (platform/input, F4-2.3): adaptador de TECLADO do backend novo
// (glintfx) para o InputMapper - irmao do SdlInput, mas por POLLING DE NIVEL
// (decisao do lider, ver key_translation_glintfx.hpp) em vez de evento SDL
// KEY_DOWN/KEY_UP. A cada quadro, o CHAMADOR roda poll(): ele pergunta ao
// PROVIDER injetado "esta tecla FISICA esta pressionada AGORA?" para cada
// glintfx::Key que aparece no esquema de controles ativo, e SINTETIZA as
// transicoes press()/release() que o InputMapper ja consome - matando por
// construcao a classe de bug "tecla grudada" (o SdlInput dependia do SO
// entregar o KEY_UP correspondente; aqui, se o SO nunca entregar o evento, o
// PROXIMO poll() ja corrige sozinho, porque le o ESTADO, nao um evento).
//
// PROVIDER INJETAVEL (o ponto central desta fatia): o adaptador NAO depende de
// glintfx::App pra ser testado - so de um std::function<bool(glintfx::Key)> que
// o teste substitui por um provider FALSO, dirigindo o teclado sem janela, sem
// GL, sem Xvfb. No uso real (fora do escopo desta fatia - F4-2.5/F4-3, o
// amarrado com o App de verdade), o provider sera um lambda fino sobre
// glintfx::App::is_key_down(key).
//
// QUAIS TECLAS POLAR: derivadas do InputRemapConfig (o MESMO mapa logico que
// SdlInput/InputMapper ja usam) via a direcao REVERSA da tabela de traducao
// (godot_keycode_to_glintfx_key, key_translation_glintfx.hpp) - dado o keycode
// persistido de cada KeyBinding, descobre-se qual glintfx::Key perguntar ao
// provider. Bindings sem correspondente conhecido (sentinela std::nullopt, ex.:
// uma das 4 exclusoes documentadas, ou uma acao so-gamepad com keycode=0) NUNCA
// sao polados - mesma semantica de sentinela que o resto da costura de input ja
// usa (SdlInput/InputMapper::press ja ignora keycode 0).
//
// GAMEPAD FICA DE FORA (fora do escopo desta fatia - "adaptador de TECLADO do
// backend novo"): dx()/dy()/run() aqui refletem SO o teclado, ao contrario de
// SdlInput::dx()/dy()/run() (que fundem com GamepadState). A fusao com gamepad
// entra numa fatia futura, quando/se o glintfx tiver um provider de gamepad.

#ifndef GUS_PLATFORM_INPUT_GLINTFX_INPUT_HPP
#define GUS_PLATFORM_INPUT_GLINTFX_INPUT_HPP

#include <functional>
#include <string_view>
#include <vector>

#include <glintfx/ui_event.hpp>  // glintfx::Key

#include "gus/platform/input/input_mapper.hpp"

namespace gus::platform::input {

class GlintfxInput {
public:
    // Provider de estado de tecla: devolve true se a tecla FISICA `key` esta
    // pressionada AGORA. Chamado a cada poll(), nunca guardado como side effect
    // por este adaptador. No host real: lambda fino sobre glintfx::App::
    // is_key_down(key). Nos testes: um provider fake que o teste dirige (ex.:
    // um std::set<glintfx::Key> mutavel de fora, capturado por referencia).
    using KeyStateProvider = std::function<bool(glintfx::Key)>;

    // Constroi com o esquema de fabrica (gus::domain::input::default_controls()).
    // O provider e OBRIGATORIO (sem ele o adaptador nao tem como saber o estado
    // de nenhuma tecla) - por isso NAO ha ctor sem argumento nenhum, ao
    // contrario de SdlInput() (que nao precisa de fonte externa: le SDL direto).
    explicit GlintfxInput(KeyStateProvider provider);

    // Constroi com um esquema de controles EXPLICITO (ex.: remap persistido do
    // jogador, carregado do disco) - mesma razao de SdlInput(config): o
    // MAESTRO alimenta o remap no BOOT sem duplicar a logica de traducao daqui.
    GlintfxInput(gus::domain::input::InputRemapConfig config, KeyStateProvider provider);

    GlintfxInput(const GlintfxInput&) = delete;
    GlintfxInput& operator=(const GlintfxInput&) = delete;

    // --- caminho de POLLING (chamar 1x por quadro) --------------------------
    // Pergunta ao provider o estado ATUAL de cada tecla observada (derivada do
    // config ativo) e sintetiza press()/release() no InputMapper interno para
    // toda transicao detectada desde o poll() anterior. Idempotente entre
    // transicoes: segurar uma tecla por N quadros sem o estado mudar NAO gera N
    // press() - so 1, na subida (o InputMapper::press ja e idempotente por
    // conjunto, mas este adaptador nem chega a chamar de novo: so chama na
    // BORDA, coerente com o nome "sintetiza transicao").
    void poll();

    // Esvazia o estado (ex.: ao perder foco da janela) - MESMA semantica de
    // SdlInput::clear(): zera o InputMapper interno E o estado "anterior" de
    // cada tecla observada, para nenhuma tecla ficar "presa" achando que o
    // proximo poll() e uma nao-transicao.
    void clear() noexcept;

    // Troca o esquema de controles em RUNTIME (mesma semantica de
    // SdlInput::set_controls): reconstroi o InputMapper com o config novo e
    // RECALCULA a lista de teclas observadas - nenhuma tecla do esquema antigo
    // fica sendo polada a toa, nenhuma do esquema novo fica de fora. Nenhuma
    // tecla fica "presa" na troca (o InputMapper novo nasce com pressed_ vazio,
    // igual ao SdlInput).
    void set_controls(gus::domain::input::InputRemapConfig config);

    // --- intencao (teclado apenas - ver nota do header) ---------------------
    [[nodiscard]] int dx() const noexcept;
    [[nodiscard]] int dy() const noexcept;
    [[nodiscard]] bool run() const noexcept;

    // Acesso generico a qualquer acao do esquema (ex.: interact, combat_cast,
    // menu_confirm) - repassa direto pro InputMapper::is_action_active, MESMO
    // contrato que o resto da costura de input ja usa.
    [[nodiscard]] bool is_action_active(std::string_view action_name) const;

private:
    struct WatchedKey {
        glintfx::Key key;
        long long godot_keycode = 0;
        bool pressed = false;  // estado no poll() ANTERIOR (pra detectar a borda)
    };

    // Reconstroi watched_keys_ a partir de config_ (ctor e set_controls()).
    void rebuild_watched_keys();

    gus::domain::input::InputRemapConfig config_;
    InputMapper mapper_;
    KeyStateProvider provider_;
    std::vector<WatchedKey> watched_keys_;
};

}  // namespace gus::platform::input

#endif  // GUS_PLATFORM_INPUT_GLINTFX_INPUT_HPP
