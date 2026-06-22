// gus/domain/combat/environment_catalog.cpp
//
// Implementacao do catalogo data-driven de ambientes (secao 18), portado de
// engine/foundation/turn_combat/EnvironmentCatalog.cs. Os numeros sao transcritos DIRETO
// das tabelas secao 18.2/18.3/18.4/18.5 (balance e canon). POCO puro, ZERO Qt, ZERO IO.
//
// Cross-ref: engine/foundation/turn_combat/EnvironmentCatalog.cs;
//            docs/design/mecanicas/combat.md secao 18/11/9; ADR-004; ADR-006.

#include "gus/domain/combat/environment_catalog.hpp"

#include <algorithm>
#include <stdexcept>

#include "gus/domain/combat/combat_constants.hpp"

namespace gus::domain::combat::EnvironmentCatalog {
namespace {

namespace cc = combat_constants;

// Apelidos das faixas canonicas (secao 18), espelhando os const do C#.
constexpr float kPico = cc::kEnvMultPico;             // 1.5
constexpr float kAlto = cc::kEnvMultAlto;             // 1.3
constexpr float kHostilLeve = cc::kEnvMultHostilLeve; // 0.85
constexpr float kHostil = cc::kEnvMultHostilForte;    // 0.66

// Constroi o catalogo canonico (espelha EnvironmentCatalog.Build do C# 1:1).
std::map<EnvironmentId, EnvironmentModifier> build() {
    std::map<EnvironmentId, EnvironmentModifier> d;

    auto add = [&d](EnvironmentModifier env) { d[env.id] = std::move(env); };

    // ---- None: neutro (retrocompat secao 11) ----
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::None;
        e.layer = EnvironmentLayer::Terreno;  // arbitrario; None nunca participa de stacking
        e.display_name_key = "ENV_NONE";
        add(std::move(e));
    }

    // =================================================================
    // CLIMA (8) - secao 18.2
    // =================================================================
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Neblina;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_NEBLINA";
        e.family_mults = {{CardFamily::Sonico, kAlto}, {CardFamily::Criptografico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Disrupt, /*mag*/1, /*dur*/0};
        e.hardware = HardwareHook{/*scan_ap*/+1, /*scan_free*/false, /*prever*/-1, /*spd*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Chuva;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_CHUVA";
        e.family_mults = {{CardFamily::Eletrico, kPico}, {CardFamily::Bioquimico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Stun, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Calor;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_CALOR";
        e.family_mults = {{CardFamily::Bioquimico, kAlto}, {CardFamily::Eletrico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Corrode, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::TempestadeEletrica;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_TEMPESTADE_ELETRICA";
        e.family_mults = {{CardFamily::Eletrico, kPico}, {CardFamily::Criptografico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Stun, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/+1, /*scan_free*/false, /*prever*/+1, /*spd*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Vento;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_VENTO";
        e.family_mults = {{CardFamily::Sonico, kAlto}, {CardFamily::Bioquimico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Knockback, /*mag*/1, /*dur*/0};
        e.hardware = HardwareHook{/*scan_ap*/0, /*scan_free*/false, /*prever*/+1, /*spd*/+1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Estatica;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_ESTATICA";
        e.family_mults = {{CardFamily::Criptografico, kPico}, {CardFamily::Sonico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Decrypt, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Fumaca;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_FUMACA";
        e.family_mults = {{CardFamily::Bioquimico, kAlto}, {CardFamily::Criptografico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Poison, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/+1, /*scan_free*/false, /*prever*/0, /*spd*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::EscuridaoTotal;
        e.layer = EnvironmentLayer::Clima;
        e.display_name_key = "ENV_ESCURIDAO_TOTAL";
        e.family_mults = {{CardFamily::Sonico, kPico}, {CardFamily::Criptografico, kHostilLeve}};
        e.facilitated_status = FacilitatedStatus{StatusId::Disrupt, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/+2, /*scan_free*/false, /*prever*/0, /*spd*/0};
        add(std::move(e));
    }

    // =================================================================
    // PERIODO (roda de 4 fases) - secao 18.3. Duracoes Fibonacci 5/2/5/2.
    // =================================================================
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Dia;
        e.layer = EnvironmentLayer::Periodo;
        e.display_name_key = "ENV_DIA";
        e.family_mults = {{CardFamily::Bioquimico, kAlto}, {CardFamily::Criptografico, kHostilLeve}};
        e.facilitated_status = FacilitatedStatus{StatusId::Regen, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/0, /*scan_free*/true, /*prever*/0, /*spd*/0};
        e.period_duration = 5;
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Crepusculo;
        e.layer = EnvironmentLayer::Periodo;
        e.display_name_key = "ENV_CREPUSCULO";
        // transicao neutra: sem mult de familia.
        e.facilitated_status = FacilitatedStatus{StatusId::Disrupt, /*mag*/1, /*dur*/0};
        e.period_duration = 2;
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Noite;
        e.layer = EnvironmentLayer::Periodo;
        e.display_name_key = "ENV_NOITE";
        e.family_mults = {
            {CardFamily::Criptografico, kPico},
            {CardFamily::Sonico, kAlto},
            {CardFamily::Bioquimico, kHostilLeve},
        };
        e.facilitated_status = FacilitatedStatus{StatusId::Decrypt, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/+1, /*scan_free*/false, /*prever*/0, /*spd*/0};
        e.period_duration = 5;
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Aurora;
        e.layer = EnvironmentLayer::Periodo;
        e.display_name_key = "ENV_AURORA";
        e.family_mults = {{CardFamily::Eletrico, kAlto}};
        e.facilitated_status = FacilitatedStatus{StatusId::Haste, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/0, /*scan_free*/false, /*prever*/+1, /*spd*/+1};
        e.period_duration = 2;
        add(std::move(e));
    }

    // =================================================================
    // TERRENO - Tier VISIVEL - secao 18.4
    // =================================================================
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Lamacento;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_LAMACENTO";
        e.family_mults = {{CardFamily::Eletrico, kAlto}, {CardFamily::Cinetico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Knockback, /*mag*/1, /*dur*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Seco;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_SECO";
        e.family_mults = {{CardFamily::Cinetico, kAlto}, {CardFamily::Eletrico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Break, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Vinhas;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_VINHAS";
        e.family_mults = {{CardFamily::Bioquimico, kAlto}, {CardFamily::Cinetico, kHostilLeve}};
        e.facilitated_status = FacilitatedStatus{StatusId::Slow, /*mag*/1, /*dur*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Gelo;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_GELO";
        e.family_mults = {{CardFamily::Cinetico, kAlto}, {CardFamily::Bioquimico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Break, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::AguaAlagado;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_AGUA_ALAGADO";
        e.family_mults = {{CardFamily::Eletrico, kAlto}, {CardFamily::Cinetico, kHostilLeve}};
        e.facilitated_status = FacilitatedStatus{StatusId::Stun, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::MetalCondutor;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_METAL_CONDUTOR";
        e.family_mults = {{CardFamily::Eletrico, kAlto}, {CardFamily::Sonico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Corrode, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/0, /*scan_free*/true, /*prever*/0, /*spd*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::Bioluminescencia;
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_BIOLUMINESCENCIA";  // SO SELVE
        e.family_mults = {{CardFamily::Sonico, kAlto}, {CardFamily::Eletrico, kHostilLeve}};
        e.facilitated_status = FacilitatedStatus{StatusId::Regen, /*mag*/0, /*dur*/1};
        e.hardware = HardwareHook{/*scan_ap*/0, /*scan_free*/true, /*prever*/+1, /*spd*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::PavimentoTesselado;  // T1
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_PAVIMENTO_TESSELADO";
        e.family_mults = {{CardFamily::Criptografico, kAlto}, {CardFamily::Sonico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Expose, /*mag*/13, /*dur*/0};  // Fibonacci canon
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::TaludeInstavel;  // T2
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_TALUDE_INSTAVEL";
        e.family_mults = {{CardFamily::Cinetico, kPico}, {CardFamily::Criptografico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Slow, /*mag*/2, /*dur*/0};  // pune o inativo
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::AshlarBruto;  // T4
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_ASHLAR_BRUTO";
        e.family_mults = {{CardFamily::Cinetico, kAlto}, {CardFamily::Eletrico, kHostil}};
        // sem facilitated_status (Defender -> Shield x1.5 e jogo posterior).
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::SoloFertilRecursivo;  // T5 - SO SELVE
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_SOLO_FERTIL_RECURSIVO";
        e.family_mults = {{CardFamily::Bioquimico, kPico}, {CardFamily::Cinetico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Poison, /*mag*/0, /*dur*/1};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::AnomaliaPerlin;  // T6 - vetor anti-padrao EXCLUSIVO Patch-Zero (boss)
        e.layer = EnvironmentLayer::Terreno;
        e.display_name_key = "ENV_ANOMALIA_PERLIN";
        // family_mults vazio de proposito (secao 18.4): NAO mexe no dano (mult_ambiente 1.0).
        add(std::move(e));
    }

    // =================================================================
    // TERRENO - Tier CODEX (revela so apos Scan-ambiente 1 AP) - secao 18.5
    // =================================================================
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::EspelhoRessonante;  // T3
        e.layer = EnvironmentLayer::Terreno;
        e.tier = EnvironmentTier::Codex;
        e.display_name_key = "ENV_ESPELHO_RESSONANTE";
        e.family_mults = {{CardFamily::Sonico, kPico}, {CardFamily::Bioquimico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Disrupt, /*mag*/0, /*dur*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::DutoCondutorPressurizado;  // T7 - SO CIDADE
        e.layer = EnvironmentLayer::Terreno;
        e.tier = EnvironmentTier::Codex;
        e.display_name_key = "ENV_DUTO_CONDUTOR_PRESSURIZADO";
        e.family_mults = {{CardFamily::Eletrico, kAlto}, {CardFamily::Bioquimico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Disrupt, /*mag*/0, /*dur*/0};
        add(std::move(e));
    }
    {
        EnvironmentModifier e{};
        e.id = EnvironmentId::ElevacaoDominante;  // T8
        e.layer = EnvironmentLayer::Terreno;
        e.tier = EnvironmentTier::Codex;
        e.display_name_key = "ENV_ELEVACAO_DOMINANTE";
        e.family_mults = {{CardFamily::Cinetico, kAlto}, {CardFamily::Sonico, kHostil}};
        e.facilitated_status = FacilitatedStatus{StatusId::Haste, /*mag*/1, /*dur*/0};  // 1o da fila
        add(std::move(e));
    }

    return d;
}

// Catalogo unico construido na primeira chamada (Meyers singleton const, sem estado
// mutavel global: a tabela e read-only depois de construida).
const std::map<EnvironmentId, EnvironmentModifier>& entries() {
    static const std::map<EnvironmentId, EnvironmentModifier> kEntries = build();
    return kEntries;
}

// Produto das camadas que afetam a familia, com cap final. Espelha MultAmbiente do C#:
// acc = 1.0f; para cada env != None, acc *= env.mult_for(family); clamp [cap_min, cap_max].
float compute(CardFamily family, const std::vector<const EnvironmentModifier*>& active) {
    float product = cc::kMultAmbienteDefault;  // 1.0
    for (const auto* env : active) {
        if (env->id == EnvironmentId::None)
            continue;
        product *= env->mult_for(family);
    }
    return std::clamp(product, cc::kMultAmbienteCapMin, cc::kMultAmbienteCapMax);
}

}  // namespace

const std::map<EnvironmentId, EnvironmentModifier>& all() { return entries(); }

const EnvironmentModifier& none() { return get(EnvironmentId::None); }

const EnvironmentModifier& get(EnvironmentId id) {
    const auto& e = entries();
    const auto it = e.find(id);
    if (it == e.end())
        throw std::out_of_range("Ambiente fora do catalogo secao 18");
    return it->second;
}

float mult_ambiente(CardFamily family, const std::vector<EnvironmentModifier>& active) {
    std::vector<const EnvironmentModifier*> ptrs;
    ptrs.reserve(active.size());
    for (const auto& e : active)
        ptrs.push_back(&e);
    return compute(family, ptrs);
}

float mult_ambiente(CardFamily family, std::initializer_list<EnvironmentModifier> active) {
    std::vector<const EnvironmentModifier*> ptrs;
    ptrs.reserve(active.size());
    for (const auto& e : active)
        ptrs.push_back(&e);
    return compute(family, ptrs);
}

float mult_ambiente_ids(CardFamily family, const std::vector<EnvironmentId>& active_ids) {
    std::vector<const EnvironmentModifier*> ptrs;
    ptrs.reserve(active_ids.size());
    for (const auto id : active_ids)
        ptrs.push_back(&get(id));
    return compute(family, ptrs);
}

}  // namespace gus::domain::combat::EnvironmentCatalog
