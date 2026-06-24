// gus/app/screens/sprite_animation.hpp
//
// LOGICA de animacao de locomocao do jogador, POCO "fora da casca Qt" (mesma
// disciplina do OverworldSim): SEM Qt, SEM I/O, SEM GPU. So decide DUAS coisas a
// partir do movimento, pra que o render apenas escolha qual sprite mostrar:
//   1. DIRECAO cardinal (Sul/Norte/Leste/Oeste) a partir do vetor de movimento;
//   2. QUADRO da animacao de walk a partir da DISTANCIA percorrida (nao do tempo).
//
// CANON: docs/design/mecanicas/locomotion.md.
//   - 4 direcoes puras, sem flip (Pillar 3: hardware do personagem e assimetrico);
//   - troca de quadro por DISTANCIA (~8 px no tile 16 -> escalavel com o tile),
//     nao por timer, pra eliminar o "foot slide";
//   - parado = distancia 0 = quadro neutro (idle congelado);
//   - run NAO acelera o fps: a passada e mais LONGA (px maior por troca), nao mais
//     rapida (ver WalkCycle::Config::run_px_per_frame).
//
// Esta unidade e testavel sem janela (app/tests/sprite_animation_test.cpp): varios
// vetores -> direcao, acumulo de distancia -> quadro ciclico, parado -> neutro. O
// carregamento de textura + draw e a casca Qt (Render2dRhi), coberta pelo smoke.

#ifndef GUS_APP_SCREENS_SPRITE_ANIMATION_HPP
#define GUS_APP_SCREENS_SPRITE_ANIMATION_HPP

namespace gus::app::screens {

// As 4 direcoes cardinais canonicas (locomotion.md). A ordem do enum e tambem a
// ordem dos slots de sprite no render (Sul, Norte, Leste, Oeste).
enum class Direction {
    South = 0,
    North = 1,
    East = 2,
    West = 3,
};

// Numero de direcoes (pra dimensionar tabelas de sprite no render).
inline constexpr int kDirectionCount = 4;

// Numero de quadros de walk PADRAO por direcao (legado do Caua: 4, ping-pong). O
// WalkCycle aceita um frame_count diferente no ctor (Gus tem 7 frames por direcao);
// esta constante e so o DEFAULT - dimensiona testes/chamadas antigas. O numero REAL
// de quadros vive no PlayerSpriteSet (medido por personagem) e e passado ao WalkCycle.
inline constexpr int kWalkFrameCount = 4;

// Teto de quadros de walk por direcao suportado (dimensiona os arrays fixos do
// PlayerSpriteSet sem alocacao dinamica). Gus = 7; folga ate 16 pra futuras anims.
inline constexpr int kMaxWalkFrameCount = 16;

// Teto de quadros do IDLE animado por direcao (breathing do Gus = 5). Idem: array
// fixo, sem heap. Headless degradado mantem 1 (idle congelado).
inline constexpr int kMaxIdleFrameCount = 16;

// POLITICA de qual eixo manda o "olhar" do boneco na DIAGONAL (ambos dx,dy != 0).
// O lider escolhe via OverworldTuning (ponto unico de feel). Cardinal puro nunca
// depende disto. Default do tuning = LastAxisWins (pedido do lider: andar pro lado
// e acionar Norte/Sul faz o boneco VIRAR pra Norte/Sul, e vice-versa).
enum class DiagonalFacing {
    // Leste/Oeste sempre ganham na diagonal (regra LEGADA do M1).
    HorizontalWins = 0,
    // Norte/Sul sempre ganham na diagonal.
    VerticalWins = 1,
    // Vence o eixo RECEM-acionado, decidido pela MEMORIA DO INPUT (dx_prev,dy_prev),
    // NAO pela direcao anterior: se o vertical acabou de entrar (dy != 0, dy_prev == 0),
    // vira pro vertical; se o horizontal acabou de entrar, vira pro horizontal. Com
    // as duas teclas SUSTENTADAS (nada recem-acionado), MANTEM prev se ele ja for um
    // dos eixos da diagonal (estavel, sem flicker); senao cai no vertical da diagonal.
    // ATENCAO: a sobrecarga SEM memoria de input (dx_prev/dy_prev) deriva esta
    // politica do facing anterior e por isso OSCILA na diagonal sustentada; prefira a
    // sobrecarga de 6 args quando a politica for LastAxisWins.
    LastAxisWins = 2,
};

// Escolhe a direcao cardinal dominante a partir de um vetor de movimento cardinal
// cru (dx,dy em {-1,0,1}). REGRAS (documentadas e cobertas por teste):
//   - parado (0,0): MANTEM prev (a ultima direcao; idle nao gira o boneco);
//   - cardinal puro: a propria direcao;
//   - DIAGONAL (ambos != 0): resolvida pela DiagonalFacing escolhida.
//
// Sobrecargas:
//   - (dx,dy,prev): comportamento LEGADO (HorizontalWins), pra nao quebrar chamadas
//     antigas;
//   - (dx,dy,prev,policy): a politica decide a diagonal. Para LastAxisWins, deriva o
//     eixo "novo" do facing anterior - aproximacao SEM historico de teclas, que pode
//     OSCILAR numa diagonal sustentada;
//   - (dx,dy,dx_prev,dy_prev,prev,policy): a forma CORRETA para LastAxisWins. Usa a
//     MEMORIA DO INPUT do tick anterior pra detectar o eixo recem-acionado e fica
//     ESTAVEL (sem flicker) enquanto as duas teclas seguem apertadas. VerticalWins e
//     HorizontalWins ignoram dx_prev/dy_prev (resultado identico a sobrecarga de 4).
[[nodiscard]] Direction direction_from_move(int dx, int dy, Direction prev) noexcept;
[[nodiscard]] Direction direction_from_move(int dx, int dy, Direction prev,
                                            DiagonalFacing policy) noexcept;
[[nodiscard]] Direction direction_from_move(int dx, int dy, int dx_prev, int dy_prev,
                                            Direction prev,
                                            DiagonalFacing policy) noexcept;

// Ciclo de walk dirigido por DISTANCIA. Acumula a distancia andada e, a cada
// "passo de troca" (px por quadro), avanca um quadro ciclico em [0, kWalkFrameCount).
// Parado (advance com dist 0, ou reset) volta ao estado neutro: current_frame()
// devolve kNeutralFrame e is_moving() = false, pro render mostrar o sprite idle.
class WalkCycle {
public:
    // Sinaliza "sem quadro de passo" (mostrar o neutro/idle). Vem de current_frame()
    // quando o personagem nao esta andando.
    static constexpr int kNeutralFrame = -1;

    struct Config {
        // Px percorridos por troca de quadro ANDANDO. Default ~8 px (locomotion.md,
        // tile 16). Escalavel: se o tile mudar, escalar na mesma proporcao.
        float walk_px_per_frame = 8.0f;
        // Px por troca CORRENDO: passada mais LONGA (nao mais rapida). ~11 px
        // (faixa 10-12 do canon). run >= walk garante "pe colado" na corrida.
        float run_px_per_frame = 11.0f;
        // HISTERESE (coast) do estado de animacao, em SEGUNDOS. Buffer que mantem o
        // estado "andando" apos o ultimo movimento REAL antes de cair pro idle. Sem
        // ele, spammar a direcao (taps com micro-gaps) resetava a anim pro neutro a
        // cada gap e o Gus DESLIZAVA sem animar (estilo Zelda/Stardew: a anim segue o
        // ESTADO de movimento, nao o input do frame exato). So vale na sobrecarga
        // advance(dist, running, dt). 0 = corte seco (legado). Default sobreposto pelo
        // OverworldTuning (anim_walk_coast_seconds).
        float coast_seconds = 0.0f;
    };

    WalkCycle() = default;
    explicit WalkCycle(Config cfg) noexcept : cfg_(cfg) {}
    // frame_count = numero REAL de quadros do ciclo desta arte (Caua 4, Gus 7).
    // Saneado: < 1 vira kWalkFrameCount (default seguro). O wrap usa este valor.
    WalkCycle(Config cfg, int frame_count) noexcept
        : cfg_(cfg),
          frame_count_(frame_count >= 1 ? frame_count : kWalkFrameCount) {}

    // Avanca o ciclo com a distancia percorrida NESTE passo (>= 0, ja em px de
    // mundo) e se estava correndo. distance <= 0 => PARADO: zera o acumulador e
    // entra em estado neutro (idle). distance > 0 => acumula; cada vez que o
    // acumulado cruza o passo de troca, avanca 1 quadro ciclico.
    //
    // Sobrecarga LEGADA (sem dt): SEM histerese - parar corta seco pro neutro. Mantida
    // pra nao quebrar chamadas/testes antigos.
    void advance(float distance, bool running) noexcept;

    // Sobrecarga com HISTERESE (coast). Igual a de cima quando ha movimento real
    // (distance > 0): avanca os quadros e RECARREGA o buffer de coast. Sem movimento
    // (distance <= 0): NAO corta seco - decrementa o buffer pelo dt (segundos) e,
    // enquanto ele dura, SEGURA o estado "andando" e o quadro ATUAL (nao avanca: nada
    // de marchar parado). So cai pro neutro (reset) quando o buffer expira. Cura o
    // "Gus desliza ao spammar a direcao": micro-gaps entre taps ficam dentro do coast
    // e a anim segue rodando. dt <= 0 e tratado como 0 (nao mexe no buffer).
    void advance(float distance, bool running, float dt) noexcept;

    // Forca o estado neutro (idle): zera acumulador e marca parado. Usado quando o
    // movimento e exatamente nulo no passo.
    void reset() noexcept;

    // true se esta em movimento (mostrar quadro de walk); false = idle (neutro).
    [[nodiscard]] bool is_moving() const noexcept { return moving_; }

    // Quadro atual: [0, kWalkFrameCount) se andando; kNeutralFrame se parado.
    [[nodiscard]] int current_frame() const noexcept {
        return moving_ ? frame_ : kNeutralFrame;
    }

    // Distancia acumulada ainda nao "gasta" numa troca (debug/teste).
    [[nodiscard]] float accumulated() const noexcept { return accum_; }

    // Numero de quadros do ciclo (>= 1). Usado pelo wrap; util pra teste/debug.
    [[nodiscard]] int frame_count() const noexcept { return frame_count_; }

private:
    // Nucleo do avanco por DISTANCIA (assume distance > 0): marca moving e consome o
    // acumulado em passos de troca. Compartilhado pelas duas sobrecargas de advance.
    void advance_moving(float distance, bool running) noexcept;

    Config cfg_{};
    float accum_ = 0.0f;  // distancia acumulada desde a ultima troca
    int frame_ = 0;       // quadro de walk corrente [0, frame_count_)
    bool moving_ = false; // false = neutro (idle)
    int frame_count_ = kWalkFrameCount;  // quadros do ciclo (Caua 4, Gus 7)
    // Buffer de HISTERESE restante (s): recarregado no movimento real, decai parado
    // (sobrecarga com dt). Enquanto > 0 segura o estado "andando" entre taps do spam.
    float coast_remaining_ = 0.0f;
};

}  // namespace gus::app::screens

#endif  // GUS_APP_SCREENS_SPRITE_ANIMATION_HPP
