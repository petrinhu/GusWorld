// gus/app/tools/glintfx_input_e2e/glintfx_input_e2e_probe.cpp
//
// F4-2.5 (prova de vida ponta-a-ponta do input novo): amarra pela PRIMEIRA vez o fio
// completo glintfx::App real -> GlintfxInput (com o provider ligado em
// App::is_key_down) -> InputMapper -> movimento observavel. Diferente do
// appmode_spike (app/tools/appmode_spike/, NAO commitado, GAP-1 documentado la): a
// partir do pin v0.20.0 o glintfx::App JA EXPOE App::is_key_down(Key) publicamente
// (HOSTIN-1, ver app.hpp) - NENHUM workaround GLFW cru e necessario aqui, o probe
// NUNCA inclui <GLFW/glfw3.h> nem linka glfw diretamente.
//
// TRACKED (ao contrario do spike): este probe FICA no repo. Precisa de um build
// SCRATCH proprio (GLINTFX_BACKEND_GLFW=ON, ver CMakeLists.txt deste diretorio) -
// o preset principal (linux-release) continua com o backend OFF (o cutover pra
// GLFW=ON em producao e a F4-3, fora do escopo desta fatia).
//
// Contrato de log (consumido pelo run_e2e.sh, verificacao por SCRIPT, nao por
// olho): toda vez que a tripla (dx, dy, run) computada por GlintfxInput::dx()/dy()/
// run() MUDA em relacao ao poll() anterior, uma linha
//   EDGE frame=<n> t_ms=<elapsed> dx=<dx> dy=<dy> run=<0|1>
// e escrita em <log_dir>/probe.log (1 linha por transicao, nunca por frame parado -
// idempotente, mesma semantica de "so loga na borda" que o resto da engine ja usa).
// Um arquivo sentinela <log_dir>/READY e tocado apos o 1o frame renderizado (o
// orquestrador externo poll a existencia dele antes de comecar a injetar tecla
// nenhuma - nunca assume um tempo fixo de boot).
//
// Sem RENDERIZACAO de cidade nenhuma (SEM OverworldSim/city_loader) - fora do
// escopo desta fatia ("prova de vida do INPUT", nao da cena). Um documento .rml
// minimo (main.rml/main.rcss, ao lado deste .cpp) e carregado so pra exercitar
// App::load() dentro do fio real - sem texto, sem fonte, zero dependencia de
// assets/fontes.

#include <glintfx/app.hpp>
#include <glintfx/ui_event.hpp>  // glintfx::Key

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>

#include "gus/domain/input/controls_restore.hpp"
#include "gus/platform/input/glintfx_input.hpp"

namespace fs = std::filesystem;

namespace {

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
    std::fflush(g_log);  // fflush POR LINHA (brief) - o script tail -f/poll isto.
}

long long mono_ms() {
    struct timespec ts {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<long long>(ts.tv_sec) * 1000LL +
           static_cast<long long>(ts.tv_nsec) / 1'000'000LL;
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

// Estado do probe, capturado por referencia no frame_callback (mesmo idioma dos
// probes irmaos, ex. appmode_spike_probe.cpp::SpikeApp).
struct ProbeState {
    long long boot_mono_ms = 0;
    long long frame_count = 0;
    bool quit_requested = false;

    int last_dx = 0;
    int last_dy = 0;
    bool last_run = false;

    bool prev_q = false;  // edge-detect manual de Q (sair) - NAO faz parte do
                           // esquema de controles (InputRemapConfig); e so um
                           // atalho do PROBE, lido direto via App::is_key_down.

    bool ready_touched = false;
    std::string ready_path;
};

void on_frame(glintfx::App& app, ProbeState& s,
              gus::platform::input::GlintfxInput& input, float /*dt_seconds*/) {
    ++s.frame_count;
    const long long t_ms = mono_ms() - s.boot_mono_ms;

    input.poll();
    const int dx = input.dx();
    const int dy = input.dy();
    const bool run = input.run();
    if (dx != s.last_dx || dy != s.last_dy || run != s.last_run) {
        log_printf("EDGE frame=%lld t_ms=%lld dx=%d dy=%d run=%d", s.frame_count,
                   t_ms, dx, dy, run);
        s.last_dx = dx;
        s.last_dy = dy;
        s.last_run = run;
    }

    // Q (sair) - atalho cru do probe, edge-detect manual, MESMO idioma do
    // appmode_spike_probe.cpp (prev_q/prev_p) so que via App::is_key_down (sem
    // GLFW cru - GAP-1 fechado pelo pin v0.20.0/HOSTIN-1).
    const bool q_now = app.is_key_down(glintfx::Key::Q);
    if (q_now && !s.prev_q) {
        log_printf("QUIT t_ms=%lld reason=key_q", t_ms);
        s.quit_requested = true;
    }
    s.prev_q = q_now;

    if (!s.ready_touched) {
        std::ofstream marker(s.ready_path, std::ios::trunc);
        marker << "1\n";
        s.ready_touched = true;
        log_printf("READY t_ms=%lld frame=%lld", t_ms, s.frame_count);
    }
}

}  // namespace

int main() {
    const std::string ts = timestamp_for_dir();
    const char* logdir_env = std::getenv("GUSWORLD_E2E_LOGDIR");
    const std::string log_dir = (logdir_env != nullptr && logdir_env[0] != '\0')
                                     ? std::string(logdir_env)
                                     : ("/var/tmp/gusworld_glintfx_input_e2e/" + ts);
    std::error_code ec;
    fs::create_directories(log_dir, ec);
    if (ec) {
        std::fprintf(stderr, "FAIL: create_directories(%s): %s\n", log_dir.c_str(),
                     ec.message().c_str());
        return 1;
    }

    const std::string log_path = log_dir + "/probe.log";
    g_log = std::fopen(log_path.c_str(), "w");
    if (g_log == nullptr) {
        std::fprintf(stderr, "FAIL: fopen(%s) falhou\n", log_path.c_str());
        return 1;
    }

    log_printf("glintfx_input_e2e_probe iniciando. log_dir=%s", log_dir.c_str());

    const char* rml_env = std::getenv("GUSWORLD_E2E_RML");
    const std::string rml_path =
        (rml_env != nullptr && rml_env[0] != '\0')
            ? std::string(rml_env)
            : (fs::path(__FILE__).parent_path() / "main.rml").string();

    glintfx::AppConfig cfg{};
    cfg.title = "glintfx_input_e2e_probe";
    cfg.width = 640;
    cfg.height = 480;
    cfg.window_mode = glintfx::WindowMode::Windowed;

    glintfx::App app(cfg);
    if (!app.ok()) {
        log_printf("FATAL: glintfx::App::ok()==false (janela/GL/UI falhou na "
                    "construcao).");
        std::fclose(g_log);
        return 1;
    }
    log_printf("glintfx::App ok. title=%s %dx%d", cfg.title, cfg.width, cfg.height);

    const bool load_ok = app.load(rml_path.c_str());
    log_printf("app.load(%s) ok=%d", rml_path.c_str(), load_ok ? 1 : 0);

    // GlintfxInput (F4-2.3) com o provider ligado DIRETO em App::is_key_down - o
    // ponto central desta fatia (ver header do arquivo).
    gus::domain::input::InputRemapConfig controls =
        gus::domain::input::default_controls();
    gus::platform::input::GlintfxInput input(
        controls, [&app](glintfx::Key k) { return app.is_key_down(k); });

    ProbeState state{};
    state.boot_mono_ms = mono_ms();
    state.ready_path = log_dir + "/READY";

    app.set_frame_callback([&app, &state, &input](float dt) {
        on_frame(app, state, input, dt);
    });

    constexpr double kDefaultSeconds = 20.0;
    double duration_seconds = kDefaultSeconds;
    {
        const char* v = std::getenv("GUSWORLD_E2E_SECONDS");
        if (v != nullptr && v[0] != '\0') {
            char* endptr = nullptr;
            const double parsed = std::strtod(v, &endptr);
            if (endptr != v && *endptr == '\0' && parsed > 0.0) {
                duration_seconds = parsed;
            } else {
                log_printf("GUSWORLD_E2E_SECONDS=%s invalido - caindo no "
                            "default=%.0fs.",
                            v, kDefaultSeconds);
            }
        }
    }
    log_printf("timebox=%.2fs", duration_seconds);

    while (app.running() && !state.quit_requested) {
        app.poll_events();
        app.update();
        app.render();

        const double elapsed_s =
            static_cast<double>(mono_ms() - state.boot_mono_ms) / 1000.0;
        if (elapsed_s >= duration_seconds) {
            log_printf("Timebox de %.2fs atingido - encerrando.", duration_seconds);
            break;
        }
    }

    log_printf("glintfx_input_e2e_probe encerrando. frames=%lld duration_ms=%lld",
               state.frame_count, mono_ms() - state.boot_mono_ms);
    std::fclose(g_log);
    return 0;
}
