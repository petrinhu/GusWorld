// gus/app/src/boot_pixel_overlay.cpp
//
// Implementacao de BootPixelOverlay. Ver header. Travado por
// GusEngine/app/tests/boot_pixel_overlay_test.cpp (TEST-FIRST).

#include "gus/app/boot_pixel_overlay.hpp"

#include <iostream>
#include <string>

#include "gus/core/anim/boot_pixel_sequence.hpp"

namespace gus::app {

namespace {

using gus::core::spatial::Rect;
using gus::platform::render2d::DrawColor;
using gus::platform::render2d::kInvalidTexture;
using gus::platform::render2d::TextureId;
using gus::platform::render2d::UvRect;

std::string join_path(const std::string& a, const std::string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/') {
        return a + b;
    }
    return a + "/" + b;
}

// "00".."19" (2 digitos, sem <cstdio>/snprintf) - os indices desta onda cabem em
// 0..kBootPixelFrameCount-1, sempre < 100 (um 3o digito so seria necessario se o
// asset crescesse alem de 99 frames).
std::string two_digit(int n) {
    if (n < 10) {
        return "0" + std::to_string(n);
    }
    return std::to_string(n);
}

// Cor do retangulo solido de seguranca: gunmetal escuro #0c1322, o MESMO tom de fundo
// do cockpit (battle_preview.cpp, RCSS `background-color: #0c1322;`) - reusado, nao
// inventado (mesmo espirito de glitch_overlay.cpp reusar o cyan/magenta do cockpit,
// modulo aposentado por este aqui).
constexpr float kGunmetalR = 0x0c / 255.0f;
constexpr float kGunmetalG = 0x13 / 255.0f;
constexpr float kGunmetalB = 0x22 / 255.0f;

}  // namespace

bool BootPixelOverlay::load(gus::platform::render2d::IRenderer& renderer,
                             const std::string& frames_dir) {
    frames_.clear();
    frames_.reserve(static_cast<std::size_t>(gus::core::anim::kBootPixelFrameCount));
    bool all_ok = true;
    for (int i = 0; i < gus::core::anim::kBootPixelFrameCount; ++i) {
        const std::string path =
            join_path(frames_dir, "frame_" + two_digit(i) + ".png");
        const TextureId tex = renderer.load_texture(path.c_str());
        frames_.push_back(tex);
        if (tex == kInvalidTexture) {
            all_ok = false;
            std::cerr << "boot_pixel: frame ausente/ilegivel (" << path << ")\n";
        }
    }
    if (!all_ok) {
        std::cerr << "boot_pixel: overlay indisponivel (frame(s) ausente(s)) - "
                     "boot segue sem animacao de pixel.\n";
    }
    ready_ = all_ok;
    return ready_;
}

bool BootPixelOverlay::ready() const noexcept { return ready_; }

void BootPixelOverlay::draw(gus::platform::render2d::IRenderer& renderer,
                             const Rect& screen_rect,
                             gus::core::anim::BootPixelLeg leg, float t) const {
    const float alpha = gus::core::anim::boot_pixel_safety_alpha(leg, t);
    if (alpha <= 0.0f) {
        return;  // tela limpa - mesmo invariante do fade/glitch antigos.
    }

    // 1) CAMADA DE SEGURANCA: retangulo solido gunmetal, escalado pelo alpha desta
    // perna (boot_pixel_safety_alpha - MESMA forma que o fade liso original usava) -
    // protege a troca de renderer mesmo se os frames falharem ao carregar ou tiverem
    // borda transparente (aliasing do PNG).
    renderer.draw_filled_rect(screen_rect,
                              DrawColor{kGunmetalR, kGunmetalG, kGunmetalB, alpha});

    if (!ready_) {
        return;  // frames indisponiveis (headless/asset ausente) - so o solido acima.
    }

    // 2) FRAME CORRENTE da sequencia (gus::core::anim::boot_pixel_frame_index ja
    // escolhe a sub-faixa certa desta perna, formando UM arco continuo com as outras
    // 3), cobrindo a tela inteira. draw_textured_rect ja e NEAREST sampling (contrato
    // do IRenderer) - o upscale 320x180 -> screen_rect (960x540 logico) sai crisp,
    // sem blur, sem logica de escala manual aqui.
    const int idx = gus::core::anim::boot_pixel_frame_index(
        leg, t, static_cast<int>(frames_.size()));
    renderer.draw_textured_rect(screen_rect, frames_[static_cast<std::size_t>(idx)],
                                UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                                DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
}

void BootPixelOverlay::draw_idle(gus::platform::render2d::IRenderer& renderer,
                                  const Rect& screen_rect, int frame_index) const {
    // CAMADA DE SEGURANCA: SEMPRE opaca (alpha 1) - fundo PERSISTENTE da tela de
    // titulo, nao uma transicao com progresso (draw() varia alpha com o `t` de UMA
    // metade fisica; aqui nao ha metade fisica nenhuma, a tela fica aberta por tempo
    // indefinido). Protege a mesma borda/aliasing E cobre a tela se os frames nao
    // carregaram (ready_==false) - a falha JA foi logada em load(), aqui so degrada
    // visualmente (regra do projeto: todo efeito, bom ou ruim, loga - o log ja
    // aconteceu no load(), nao precisa repetir a cada frame desenhado).
    renderer.draw_filled_rect(screen_rect,
                              DrawColor{kGunmetalR, kGunmetalG, kGunmetalB, 1.0f});

    if (!ready_ || frames_.empty()) {
        return;  // frames indisponiveis - so o solido acima (MESMO invariante de draw()).
    }

    int idx = frame_index;
    if (idx < 0) {
        idx = 0;
    } else if (idx > static_cast<int>(frames_.size()) - 1) {
        idx = static_cast<int>(frames_.size()) - 1;
    }
    renderer.draw_textured_rect(screen_rect, frames_[static_cast<std::size_t>(idx)],
                                UvRect{0.0f, 0.0f, 1.0f, 1.0f},
                                DrawColor{1.0f, 1.0f, 1.0f, 1.0f});
}

}  // namespace gus::app
