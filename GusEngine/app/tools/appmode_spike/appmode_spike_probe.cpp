// SPDX-License-Identifier: Apache-2.0
// gus/app/tools/appmode_spike/appmode_spike_probe.cpp
//
// F3-SPIKE-1 (GLINTFX-INTEGRACAO-F3 fatia 1): probe DESCARTAVEL de App mode do
// glintfx v0.18.0 (glintfx/include/glintfx/app.hpp). Prova, headless (Xvfb) ou ao
// vivo, que a cidade real (Render2dGl3 + OverworldSim, EXATAMENTE os mesmos tipos
// que a producao usa) pode desenhar DENTRO do frame_callback de um glintfx::App,
// com um FixedTimestep PROPRIO alimentado pelo dt que o App entrega por frame, e
// que os 4 loops modais hoje existentes em maestro.cpp (titulo/pausa/batalha/
// dialogo) TERIAM que virar uma maquina de estados explicita por-frame pra migrar
// pra ca (ver a tabela M6 no relatorio da onda) - aqui so um recorte de 2 estados
// (City/Paused) e exercitado, o suficiente pra provar o PADRAO sem reescrever os 4.
//
// ISOLADO: nao toca gus/app/maestro.cpp nem gus/app/sdl_window.cpp (producao).
// DESCARTAVEL: este arquivo (e o diretorio inteiro app/tools/appmode_spike/) NAO E
// COMMITADO - existe so pelo tempo da onda F3-SPIKE-1. Nao editar nenhum arquivo
// tracked a partir daqui.
//
// GAP CENTRAL (formulado como candidato a PROMPT pro glintfx no relatorio da
// onda): a API publica de glintfx::App NAO tem um canal de leitura de input
// fisico pro host (teclado/mouse chegam automaticamente e sao roteados PRA DENTRO
// do RmlUi via callbacks GLFW internos - ver app.hpp::process_event - mas o jogo
// nunca ve o estado cru das teclas). O WORKAROUND deste probe (documentado, NAO
// "consertado" aqui - fronteira glintfx ⇄ GusWorld, vira PROMPT nao implemento):
// GLFW e uma API GLOBAL por thread (glfwGetProcAddress/glfwGetCurrentContext nao
// pedem um GLFWwindow* nenhum; glfwGetKey/glfwGetCursorPos pedem um GLFWwindow*,
// que pegamos via glfwGetCurrentContext() - o App faz o SEU proprio contexto ficar
// corrente na thread antes de invocar o frame_callback, entao essa chamada
// devolve exatamente a janela do App, mesmo sem o App expor o ponteiro na propria
// API publica). Isto exige LINKAR glfw diretamente neste probe (ver CMakeLists.txt
// deste diretorio) - o jogo de producao NAO pode fazer isso hoje sem essa mesma
// dependencia extra + o mesmo malabarismo, daí o GAP.
//
// Uso:
//   cmake --preset linux-release && cmake --build --preset linux-release   # 1x
//   cmake -S GusEngine/app/tools/appmode_spike -B GusEngine/build/appmode_spike_scratch -G Ninja
//   cmake --build GusEngine/build/appmode_spike_scratch
//   DISPLAY=:99 GusEngine/build/appmode_spike_scratch/appmode_spike_probe
//
// Env:
//   GUSWORLD_SPIKE_RML       - caminho do main.rml (default: ao lado deste .cpp)
//   GUSWORLD_SPIKE_WINMODES  - "1" libera o caminho F1/F2/F3 de troca de modo de
//                              janela (GRADES DE SEGURANCA, ver handle_winmode_
// NOTA (qa fatia 2): FullscreenExclusive aparece SO como rotulo de string em
// window_mode_name() (switch exaustivo, protegido por -Wswitch num bump futuro
// do enum). Nenhum caminho do probe SOLICITA essa transicao; a grade F1/F2/F3
// so emite Windowed/FullscreenDesktop/Maximized.
//                              request abaixo); ausente/qualquer outro valor =
//                              F1/F2/F3 so logam WINMODES_DISABLED, nada mais.
//   GUSWORLD_SPIKE_SECONDS   - timebox de encerramento em segundos (default: 60).
//                              SESSAO LIVE: 60s NAO cobre o roteiro de 7 transicoes
//                              do RUNBOOK.md (>=5s de dwell cada = 35s + tempo
//                              humano) - setar algo como 300 antes de rodar ao
//                              vivo. Valor nao-numerico/<=0/absurdo cai no
//                              default, logado como tal.

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glintfx/app.hpp>
#include <glintfx/window_mode.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "gus/app/screens/city_loader.hpp"
#include "gus/app/screens/overworld_sim.hpp"
#include "gus/app/screens/player_sprites_loader.hpp"
#include "gus/core/asset_paths.hpp"
#include "gus/core/time/fixed_timestep.hpp"
#include "gus/platform/render2d/render2d_gl3.hpp"
#include "gus/platform/rmlui/gl3_loader.hpp"

// glad (DECLARACOES; a IMPLEMENTACAO mora dentro de gusengine_platform_imported,
// ver platform/rmlui/gl3_loader.cpp) - MESMA tecnica dos probes irmaos, so pra
// termos glGetError/GL_NO_ERROR disponiveis. O glintfx usa gl3w (tabela PROPRIA),
// independente do glad - zero conflito de simbolo (ver o comentario de topo de
// gl3_loader.cpp).
#include "RmlUi_Include_GL3.h"

namespace fs = std::filesystem;

namespace {

// ---------------------------------------------------------------------------
// Utilidades de tempo/log
// ---------------------------------------------------------------------------

std::FILE* g_log = nullptr;

void log_printf(const char* fmt, ...) {
    if (g_log == nullptr) {
        return;
    }
    std::va_list args;
    va_start(args, fmt);
    std::vfprintf(g_log, fmt, args);
    va_end(args);
    std::fputc('\n', g_log);
    std::fflush(g_log);  // fflush POR LINHA (brief) - a sessao live tail -f isto.
}

long long mono_ns() {
    struct timespec ts {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<long long>(ts.tv_sec) * 1'000'000'000LL +
           static_cast<long long>(ts.tv_nsec);
}

std::string iso_realtime_now() {
    struct timespec ts {};
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm_utc {};
    gmtime_r(&ts.tv_sec, &tm_utc);
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
                  tm_utc.tm_year + 1900, tm_utc.tm_mon + 1, tm_utc.tm_mday,
                  tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec,
                  static_cast<long>(ts.tv_nsec / 1'000'000));
    return buf;
}

std::string timestamp_for_dir() {
    struct timespec ts {};
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm_utc {};
    gmtime_r(&ts.tv_sec, &tm_utc);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d%02d%02dT%02d%02d%02dZ",
                  tm_utc.tm_year + 1900, tm_utc.tm_mon + 1, tm_utc.tm_mday,
                  tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec);
    return buf;
}

// Percentil simples (copia+ordena; N desta sessao de 60s cabe folgado em memoria -
// e um probe descartavel, nao hot-path de producao). p em [0,1]; p=1.0 = max.
double percentile(std::vector<double> v, double p) {
    if (v.empty()) {
        return 0.0;
    }
    std::sort(v.begin(), v.end());
    std::size_t idx = static_cast<std::size_t>(p * static_cast<double>(v.size() - 1));
    if (idx >= v.size()) {
        idx = v.size() - 1;
    }
    return v[idx];
}

const char* window_mode_name(glintfx::WindowMode m) {
    switch (m) {
        case glintfx::WindowMode::Windowed: return "Windowed";
        case glintfx::WindowMode::Maximized: return "Maximized";
        case glintfx::WindowMode::FullscreenDesktop: return "FullscreenDesktop";
        case glintfx::WindowMode::FullscreenExclusive: return "FullscreenExclusive";
    }
    return "?";
}

// ---------------------------------------------------------------------------
// Estado do probe
// ---------------------------------------------------------------------------

enum class SpikeState { City, Paused };

struct SpikeMetrics {
    std::vector<double> dt_callback_s;
    std::vector<double> dt_own_s;
    std::vector<double> abs_delta_dt_s;
    // buckets: [0]=0 ticks, [1]=1, [2]=2, [3]=3, [4]=4, [5]=">=5" (clamp).
    std::array<long long, 6> ticks_histogram{};
    long long ticks_ge3_count = 0;
    long long frame_count = 0;
};

struct SpikeApp {
    // GL/sim/render - init lazy na 1a invocacao do frame_callback (ver on_frame).
    bool gl_ready = false;
    std::unique_ptr<gus::platform::render2d::Render2dGl3> renderer;
    std::unique_ptr<gus::app::screens::OverworldSim> sim;
    gus::core::time::FixedTimestep clock{1.0 / 60.0, 5};
    double last_alpha = 0.0;

    SpikeState state = SpikeState::City;
    bool quit_requested = false;

    // dt PROPRIO (CLOCK_MONOTONIC medido de dentro do hook) - comparado ao dt que
    // o App entrega (dt_seconds do callback) pra medir confiabilidade (M2/M7).
    long long last_own_ns = 0;

    // edge-detect de teclas por-tecla (P/Q/F1-F3 - MOVIMENTO fica continuo, sem
    // edge, ver read_movement).
    bool prev_p = false, prev_q = false, prev_f1 = false, prev_f2 = false, prev_f3 = false;

    // mouse - throttle 10Hz (100ms), so loga se houve delta real.
    double last_mouse_x = 0.0, last_mouse_y = 0.0;
    bool mouse_baseline_set = false;
    long long last_mouse_log_ns = 0;

    // GRADES DE SEGURANCA winmodes (item 6 do brief).
    bool winmodes_enabled = false;
    long long last_transition_mono_ns = -1;  // -1 = nenhuma transicao aceita ainda.
    int transitions_accepted = 0;

    // GUSWORLD_SPIKE_SELFTEST_WM (SO pra auto-validacao do implementador sob Xvfb -
    // MESMO idioma de self-test ja usado em producao, ex. maestro.cpp
    // GUSWORLD_NEWGAME_VIA_TITLE_SELFTEST): sob Xvfb SEM window manager, xdotool nao
    // consegue dar foco de teclado a janela GLFW (confirmado empiricamente nesta
    // onda - getwindowfocus reporta foco preso na root window, keys/mouse nao
    // chegam ao processo) - entao chama handle_winmode_request() DIRETO, 1x
    // Maximized + 1x Windowed, exercitando o MESMO codigo/log WM-REQ/WM-DONE que o
    // handler de tecla chamaria, SEM depender de input sintetico fragil. NUNCA
    // usado no roteiro live com o lider (aquele so acontece via teclado real).
    bool selftest_wm_enabled = false;
    bool selftest_wm_did_maximize = false;
    bool selftest_wm_did_restore = false;

    long long last_heartbeat_mono_ns = 0;
    long long boot_mono_ns = 0;

    SpikeMetrics metrics;

    // GAP-1 (ver o comentario de topo do arquivo): o unico jeito de obter a janela
    // do App sem a API publica expor o ponteiro - GLFW e global-por-thread.
    // Preenchido dentro do 1o frame_callback (contexto ja garantido corrente ali).
    GLFWwindow* raw_window = nullptr;
};

void write_metrics_json(const std::string& path, const SpikeMetrics& m,
                         long long total_ns) {
    std::ofstream out(path, std::ios::trunc);
    const double total_s = static_cast<double>(total_ns) / 1e9;
    const double fps_avg = total_s > 0.0
                               ? static_cast<double>(m.frame_count) / total_s
                               : 0.0;

    out << "{\n";
    out << "  \"frame_count\": " << m.frame_count << ",\n";
    out << "  \"duration_seconds\": " << total_s << ",\n";
    out << "  \"fps_avg\": " << fps_avg << ",\n";
    out << "  \"dt_callback_s\": {\n"
        << "    \"p50\": " << percentile(m.dt_callback_s, 0.50) << ",\n"
        << "    \"p95\": " << percentile(m.dt_callback_s, 0.95) << ",\n"
        << "    \"p99\": " << percentile(m.dt_callback_s, 0.99) << ",\n"
        << "    \"max\": " << percentile(m.dt_callback_s, 1.00) << "\n"
        << "  },\n";
    out << "  \"dt_own_s\": {\n"
        << "    \"p50\": " << percentile(m.dt_own_s, 0.50) << ",\n"
        << "    \"p95\": " << percentile(m.dt_own_s, 0.95) << ",\n"
        << "    \"p99\": " << percentile(m.dt_own_s, 0.99) << ",\n"
        << "    \"max\": " << percentile(m.dt_own_s, 1.00) << "\n"
        << "  },\n";
    out << "  \"abs_delta_dt_callback_vs_proprio_s\": {\n"
        << "    \"p95\": " << percentile(m.abs_delta_dt_s, 0.95) << "\n"
        << "  },\n";
    out << "  \"ticks_histogram\": {\n"
        << "    \"0\": " << m.ticks_histogram[0] << ",\n"
        << "    \"1\": " << m.ticks_histogram[1] << ",\n"
        << "    \"2\": " << m.ticks_histogram[2] << ",\n"
        << "    \"3\": " << m.ticks_histogram[3] << ",\n"
        << "    \"4\": " << m.ticks_histogram[4] << ",\n"
        << "    \"5_ou_mais\": " << m.ticks_histogram[5] << "\n"
        << "  },\n";
    out << "  \"ticks_ge3_count\": " << m.ticks_ge3_count << "\n";
    out << "}\n";
}

// GRADES DE SEGURANCA winmodes (item 6 do brief - incidente real documentado pelo
// glintfx: rajada de trocas de modo matou o roteamento de input do compositor
// KDE/Wayland ate reboot). SO chamado de dentro do handler de tecla (nunca
// por-frame, nunca automatico) - ver on_frame abaixo.
void handle_winmode_request(glintfx::App& app, SpikeApp& s, long long now,
                             glintfx::WindowMode mode, const char* label) {
    if (!s.winmodes_enabled) {
        log_printf("WM-REQ %s WINMODES_DISABLED", label);
        return;
    }
    if (s.transitions_accepted >= 10) {
        log_printf("WM-REQ %s CAP_REACHED (10 transicoes ja aceitas neste "
                    "processo) - recusado",
                    label);
        return;
    }
    if (s.last_transition_mono_ns >= 0 &&
        now - s.last_transition_mono_ns < 2'000'000'000LL) {
        const double since_ms =
            static_cast<double>(now - s.last_transition_mono_ns) / 1e6;
        log_printf("WM-REQ %s DEBOUNCED (%.0fms desde a ultima transicao "
                    "aceita, minimo 2000ms)",
                    label, since_ms);
        return;
    }

    log_printf("WM-REQ %s ts=%lld", label, now);
    const long long t0 = mono_ns();
    const bool ret = app.set_window_mode(mode);
    const long long t1 = mono_ns();
    const glintfx::WindowMode live = app.window_mode();
    const double elapsed_ms = static_cast<double>(t1 - t0) / 1e6;
    log_printf("WM-DONE %s ret=%d live=%s elapsed_ms=%.1f", label,
               ret ? 1 : 0, window_mode_name(live), elapsed_ms);

    s.last_transition_mono_ns = now;
    ++s.transitions_accepted;
}

// Le o estado CONTINUO de movimento (sem edge - WASD/setas seguram-se) + Shift
// (correr). GAP-1: usa glfwGetKey direto na janela do App (ver o comentario de
// topo do arquivo).
void read_movement(GLFWwindow* w, int& out_dx, int& out_dy, bool& out_run) {
    const bool left = glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS ||
                       glfwGetKey(w, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool right = glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS ||
                        glfwGetKey(w, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool up = glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS ||
                     glfwGetKey(w, GLFW_KEY_UP) == GLFW_PRESS;
    const bool down = glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS ||
                       glfwGetKey(w, GLFW_KEY_DOWN) == GLFW_PRESS;
    out_run = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
              glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    out_dx = (right ? 1 : 0) - (left ? 1 : 0);
    out_dy = (down ? 1 : 0) - (up ? 1 : 0);
}

// O frame_callback registrado via App::set_frame_callback (app.hpp) - chamado 1x
// por render()/run(), com o contexto GL corrente, ANTES do passe de UI (a cidade
// compoe SOB o HUD, MESMO contrato documentado la).
void on_frame(glintfx::App& app, SpikeApp& s, float dt_callback) {
    const long long now = mono_ns();

    // --- (0) init GL da engine, SO na 1a invocacao (contexto ja garantido
    // corrente aqui pelo App, ver app.hpp::set_frame_callback) -------------
    if (!s.gl_ready) {
        s.raw_window = glfwGetCurrentContext();  // GAP-1, ver topo do arquivo.
        const bool loaded = gus::platform::rmlui::gl3_load_functions(
            reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress));
        if (!loaded) {
            log_printf("FATAL: gl3_load_functions falhou dentro do 1o "
                        "frame_callback (glad).");
            s.quit_requested = true;
            return;
        }
        s.renderer = std::make_unique<gus::platform::render2d::Render2dGl3>(
            /*gl_active=*/true);
        gus::app::screens::CityLoadOutcome city =
            gus::app::screens::load_city_or_fallback();
        s.sim =
            std::make_unique<gus::app::screens::OverworldSim>(std::move(city.sim));
        const std::string gus_dir = gus::app::screens::resolve_sprites_dir(
            std::string(gus::core::assets::kGusSpritesDir));
        s.sim->set_player_sprites(
            gus::app::screens::load_gus_sprites(*s.renderer, gus_dir));
        s.gl_ready = true;
        log_printf("GL-INIT: gl3_load_functions ok; Render2dGl3+OverworldSim+"
                    "cidade carregados dentro do 1o frame_callback. "
                    "raw_window=%p (glfwGetCurrentContext, GAP-1)",
                    static_cast<void*>(s.raw_window));

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            log_printf("GL-INIT: glGetError apos boot = 0x%x (nao-fatal, so "
                        "registrado)",
                        static_cast<unsigned>(err));
        }
    }

    // --- (1) metricas de dt (M1/M2) ---------------------------------------
    double dt_own = 0.0;
    if (s.metrics.frame_count > 0) {
        dt_own = static_cast<double>(now - s.last_own_ns) / 1e9;
    }
    s.last_own_ns = now;
    s.metrics.dt_callback_s.push_back(static_cast<double>(dt_callback));
    s.metrics.dt_own_s.push_back(dt_own);
    s.metrics.abs_delta_dt_s.push_back(
        std::fabs(static_cast<double>(dt_callback) - dt_own));
    ++s.metrics.frame_count;

    // --- (2) heartbeat 1Hz (item 5 do brief) ------------------------------
    if (now - s.last_heartbeat_mono_ns >= 1'000'000'000LL) {
        const double elapsed_boot_s =
            static_cast<double>(now - s.boot_mono_ns) / 1e9;
        const double fps = elapsed_boot_s > 0.0
                                ? static_cast<double>(s.metrics.frame_count) /
                                      elapsed_boot_s
                                : 0.0;
        log_printf("HB %s %lld frame=%lld fps=%.2f", iso_realtime_now().c_str(),
                   now, s.metrics.frame_count, fps);
        s.last_heartbeat_mono_ns = now;
    }

    // --- (2b) GUSWORLD_SPIKE_SELFTEST_WM (ver o comentario do campo em SpikeApp) -
    // dispara os 2 lados da transicao (Maximized aos ~2s, Windowed aos ~4s) SEM
    // depender de input sintetico - so sob Xvfb, so pra auto-validacao. ---------
    if (s.selftest_wm_enabled) {
        const double elapsed_boot_s =
            static_cast<double>(now - s.boot_mono_ns) / 1e9;
        if (!s.selftest_wm_did_maximize && elapsed_boot_s >= 2.0) {
            log_printf("SELFTEST-WM: disparando Maximized (elapsed_boot_s=%.2f)",
                       elapsed_boot_s);
            handle_winmode_request(app, s, now, glintfx::WindowMode::Maximized,
                                    "SELFTEST/Maximized");
            s.selftest_wm_did_maximize = true;
        } else if (s.selftest_wm_did_maximize && !s.selftest_wm_did_restore &&
                   elapsed_boot_s >= 4.0) {
            log_printf("SELFTEST-WM: disparando Windowed (elapsed_boot_s=%.2f)",
                       elapsed_boot_s);
            handle_winmode_request(app, s, now, glintfx::WindowMode::Windowed,
                                    "SELFTEST/Windowed");
            s.selftest_wm_did_restore = true;
        }
    }

    // --- (3) input (GAP-1: workaround GLFW cru, ver topo do arquivo) ------
    int dx = 0, dy = 0;
    bool run = false;
    if (s.raw_window != nullptr) {
        read_movement(s.raw_window, dx, dy, run);

        const bool p_now = glfwGetKey(s.raw_window, GLFW_KEY_P) == GLFW_PRESS;
        const bool q_now = glfwGetKey(s.raw_window, GLFW_KEY_Q) == GLFW_PRESS;
        const bool f1_now = glfwGetKey(s.raw_window, GLFW_KEY_F1) == GLFW_PRESS;
        const bool f2_now = glfwGetKey(s.raw_window, GLFW_KEY_F2) == GLFW_PRESS;
        const bool f3_now = glfwGetKey(s.raw_window, GLFW_KEY_F3) == GLFW_PRESS;

        if (p_now && !s.prev_p) {
            s.state = (s.state == SpikeState::City) ? SpikeState::Paused
                                                     : SpikeState::City;
            log_printf("IN %lld KEY P edge-down -> state=%s", now,
                       s.state == SpikeState::City ? "City" : "Paused");
            app.set_property("pause_overlay", "display",
                              s.state == SpikeState::Paused ? "block" : "none");
        }
        if (q_now && !s.prev_q) {
            log_printf("IN %lld KEY Q edge-down -> quit_requested", now);
            s.quit_requested = true;
        }
        if (f1_now && !s.prev_f1) {
            handle_winmode_request(app, s, now, glintfx::WindowMode::Windowed,
                                    "F1/Windowed");
        }
        if (f2_now && !s.prev_f2) {
            handle_winmode_request(app, s, now,
                                    glintfx::WindowMode::FullscreenDesktop,
                                    "F2/FullscreenDesktop");
        }
        if (f3_now && !s.prev_f3) {
            handle_winmode_request(app, s, now, glintfx::WindowMode::Maximized,
                                    "F3/Maximized");
        }
        s.prev_p = p_now;
        s.prev_q = q_now;
        s.prev_f1 = f1_now;
        s.prev_f2 = f2_now;
        s.prev_f3 = f3_now;

        // Mouse: throttle 10Hz (100ms), so loga se ha delta real desde o ultimo log.
        double mx = 0.0, my = 0.0;
        glfwGetCursorPos(s.raw_window, &mx, &my);
        if (!s.mouse_baseline_set) {
            s.last_mouse_x = mx;
            s.last_mouse_y = my;
            s.mouse_baseline_set = true;
        }
        const double mdx = mx - s.last_mouse_x;
        const double mdy = my - s.last_mouse_y;
        if ((mdx != 0.0 || mdy != 0.0) &&
            (now - s.last_mouse_log_ns >= 100'000'000LL)) {
            log_printf("IN %lld MOUSE dx=%.1f dy=%.1f pos=(%.1f,%.1f)", now, mdx,
                       mdy, mx, my);
            s.last_mouse_log_ns = now;
            s.last_mouse_x = mx;
            s.last_mouse_y = my;
        }
    }

    // --- (4) simulacao + render ---------------------------------------------
    if (s.gl_ready) {
        int fb_w = 0, fb_h = 0;
        app.get_window_size(fb_w, fb_h);
        if (fb_w <= 0) fb_w = 960;
        if (fb_h <= 0) fb_h = 540;

        if (s.state == SpikeState::City) {
            // FixedTimestep NOSSO, alimentado pelo dt QUE O APP ENTREGA por
            // frame (dt_callback) - MESMA receita de sdl_window.cpp::step_with_
            // fade (linhas 281-288 da producao), so trocando "dt real medido via
            // SDL_GetTicksNS" por "dt_seconds que o App ja mediu e repassou".
            const gus::core::time::FrameSteps steps =
                s.clock.advance(static_cast<double>(dt_callback));
            for (int i = 0; i < steps.ticks; ++i) {
                s.sim->step_fixed(dx, dy, run, static_cast<float>(s.clock.fixed_dt()));
            }
            s.last_alpha = steps.alpha;

            const std::size_t idx =
                static_cast<std::size_t>(std::min(steps.ticks, 5));
            ++s.metrics.ticks_histogram[idx];
            if (steps.ticks >= 3) {
                ++s.metrics.ticks_ge3_count;
            }
        }
        // Paused: NAO avanca a simulacao (nenhum step_fixed) - mas segue
        // desenhando com o ULTIMO alpha conhecido (cidade CONGELADA visualmente
        // sob o overlay, mesma sensacao do menu de pausa em producao, que
        // congela via snapshot - aqui o congelamento e so "nao avancar o clock",
        // mais simples, decisao DELIBERADA de spike - ver o relatorio da onda).
        s.sim->render(*s.renderer, 960.0f, 540.0f,
                      static_cast<float>(s.last_alpha), static_cast<float>(fb_w),
                      static_cast<float>(fb_h));

        char hud[160];
        std::snprintf(hud, sizeof(hud), "F3-SPIKE-1 state=%s frame=%lld dt=%.4fs",
                      s.state == SpikeState::City ? "City" : "Paused",
                      s.metrics.frame_count, static_cast<double>(dt_callback));
        app.set_text("hud_status", hud);
    }
}

}  // namespace

int main() {
    const std::string ts = timestamp_for_dir();
    const std::string log_dir = "/var/tmp/gusworld_spike_appmode/" + ts;
    std::error_code ec;
    fs::create_directories(log_dir, ec);
    if (ec) {
        std::fprintf(stderr, "FAIL: create_directories(%s): %s\n",
                     log_dir.c_str(), ec.message().c_str());
        return 1;
    }

    const std::string log_path = log_dir + "/probe.log";
    g_log = std::fopen(log_path.c_str(), "w");
    if (g_log == nullptr) {
        std::fprintf(stderr, "FAIL: fopen(%s) falhou\n", log_path.c_str());
        return 1;
    }

    log_printf("F3-SPIKE-1 appmode_spike_probe iniciando. log_dir=%s",
               log_dir.c_str());

    const char* rml_env = std::getenv("GUSWORLD_SPIKE_RML");
    const std::string rml_path =
        (rml_env != nullptr && rml_env[0] != '\0')
            ? std::string(rml_env)
            : (fs::path(__FILE__).parent_path() / "main.rml").string();

    glintfx::AppConfig cfg{};
    cfg.title = "gusworld appmode spike";
    cfg.width = 960;
    cfg.height = 540;
    cfg.window_mode = glintfx::WindowMode::Windowed;

    glintfx::App app(cfg);
    if (!app.ok()) {
        log_printf("FATAL: glintfx::App::ok()==false (janela/GL/UI falhou na "
                    "construcao).");
        std::fclose(g_log);
        return 1;
    }
    log_printf("glintfx::App ok. title=%s %dx%d", cfg.title, cfg.width, cfg.height);

    if (!app.load(rml_path.c_str())) {
        log_printf("FATAL: app.load(%s) falhou.", rml_path.c_str());
        std::fclose(g_log);
        return 1;
    }
    log_printf("app.load(%s) ok.", rml_path.c_str());

    SpikeApp state{};
    {
        const char* v = std::getenv("GUSWORLD_SPIKE_WINMODES");
        state.winmodes_enabled = (v != nullptr && std::string(v) == "1");
    }
    log_printf("GUSWORLD_SPIKE_WINMODES=%s",
               state.winmodes_enabled ? "1 (winmodes ATIVO)"
                                      : "ausente/!=1 (winmodes DESATIVADO)");
    {
        const char* v = std::getenv("GUSWORLD_SPIKE_SELFTEST_WM");
        state.selftest_wm_enabled =
            state.winmodes_enabled && (v != nullptr && std::string(v) == "1");
    }
    log_printf("GUSWORLD_SPIKE_SELFTEST_WM=%s (so Xvfb/auto-validacao, ver "
                "comentario do campo em SpikeApp)",
               state.selftest_wm_enabled ? "1 (ATIVO)" : "ausente/!=1 (INATIVO)");

    state.boot_mono_ns = mono_ns();
    state.last_heartbeat_mono_ns = state.boot_mono_ns;
    state.metrics.dt_callback_s.reserve(4096);
    state.metrics.dt_own_s.reserve(4096);
    state.metrics.abs_delta_dt_s.reserve(4096);

    app.set_frame_callback(
        [&app, &state](float dt) { on_frame(app, state, dt); });

    constexpr double kDefaultDurationSeconds = 60.0;
    double duration_seconds = kDefaultDurationSeconds;
    {
        const char* v = std::getenv("GUSWORLD_SPIKE_SECONDS");
        if (v != nullptr && v[0] != '\0') {
            char* endptr = nullptr;
            const double parsed = std::strtod(v, &endptr);
            if (endptr == v || *endptr != '\0' || !std::isfinite(parsed) ||
                parsed <= 0.0) {
                log_printf("GUSWORLD_SPIKE_SECONDS=%s invalido (nao-numerico, "
                           "<=0 ou absurdo) - caindo no default=%.0fs.",
                           v, kDefaultDurationSeconds);
            } else {
                duration_seconds = parsed;
            }
        }
    }
    log_printf("GUSWORLD_SPIKE_SECONDS: default=%.0fs efetivo=%.2fs",
               kDefaultDurationSeconds, duration_seconds);
    const double kDurationSeconds = duration_seconds;
    constexpr double kSnapshotAtSeconds = 5.0;
    bool did_snapshot = false;
    const long long start_ns = mono_ns();

    while (app.running() && !state.quit_requested) {
        app.poll_events();
        app.update();

        const double elapsed_s =
            static_cast<double>(mono_ns() - start_ns) / 1e9;

        if (!did_snapshot && elapsed_s >= kSnapshotAtSeconds) {
            const std::string snap_path = log_dir + "/snapshot.ppm";
            const bool snap_ok = app.snapshot(snap_path.c_str());
            log_printf("SNAPSHOT-M5: path=%s ok=%d elapsed_s=%.2f",
                       snap_path.c_str(), snap_ok ? 1 : 0, elapsed_s);
            did_snapshot = true;
        } else {
            app.render();
        }

        if (elapsed_s >= kDurationSeconds) {
            log_printf("Timebox de %.0fs atingido - encerrando.",
                       kDurationSeconds);
            break;
        }
    }

    // Caminho de quit (item 6, ultimo bullet do brief): restaura Windowed se
    // ficou em outro modo - so se o caminho winmodes esteve ativo nesta sessao.
    if (state.winmodes_enabled && app.ok() &&
        app.window_mode() != glintfx::WindowMode::Windowed) {
        log_printf("WM-QUIT-RESTORE: modo vivo=%s != Windowed no encerramento - "
                    "forcando Windowed uma ultima vez.",
                    window_mode_name(app.window_mode()));
        const bool ret = app.set_window_mode(glintfx::WindowMode::Windowed);
        log_printf("WM-QUIT-RESTORE: ret=%d live=%s", ret ? 1 : 0,
                   window_mode_name(app.window_mode()));
    }

    const long long total_ns = mono_ns() - start_ns;
    write_metrics_json(log_dir + "/metrics.json", state.metrics, total_ns);
    log_printf("appmode_spike_probe encerrando. frames=%lld duration_s=%.2f",
               state.metrics.frame_count, static_cast<double>(total_ns) / 1e9);

    std::fclose(g_log);
    return 0;
}
