// SPDX-License-Identifier: Apache-2.0
// gus/core/spatial/depth_sort.cpp
// Ordem de desenho por oclusão (DEMO-CIDADE-VESTIDA fatia I). Ver o header para o
// modelo, a regra de aresta e o porquê de não ser mais um sort de chave escalar.

#include "gus/core/spatial/depth_sort.hpp"

namespace gus::core::spatial {

namespace {

// Os DESENHOS se cruzam lateralmente? Comparação ESTRITA de propósito: dois
// retângulos que só encostam borda com borda não compartilham nenhum pixel, então
// a ordem entre eles é invisível e não vale gastar uma aresta.
[[nodiscard]] bool draws_overlap_laterally(const DepthEntry& a,
                                            const DepthEntry& b) noexcept {
    return a.draw_left < b.draw_right && b.draw_left < a.draw_right;
}

// A fatia de chão de `inner` cabe inteira dentro da de `outer`?
[[nodiscard]] bool ground_contains(const DepthEntry& outer,
                                   const DepthEntry& inner) noexcept {
    return inner.ground_back >= outer.ground_back &&
           inner.ground_front <= outer.ground_front;
}

}  // namespace

int occlusion_relation(const DepthEntry& a, const DepthEntry& b) noexcept {
    // 1. Sem cruzamento lateral, nenhum dos dois esconde o outro: sem restrição.
    if (!draws_overlap_laterally(a, b)) {
        return 0;
    }

    // 2. Separação limpa em profundidade: quem termina antes de o outro começar
    // está atrás. É o caso normal (dois personagens em linhas diferentes, um
    // personagem bem ao norte de uma casa).
    //
    // AS DUAS CONDIÇÕES SÃO TESTADAS ANTES DE DECIDIR, de propósito. Testar só a
    // primeira e sair fazia a relação deixar de ser antissimétrica no caso
    // DEGENERADO: duas fatias de comprimento ZERO na mesma altura (peça
    // atravessável, que entra como uma linha) satisfazem `front <= back` NAS DUAS
    // DIREÇÕES, e cada uma saía "estritamente atrás" da outra - um ciclo de 2 que
    // o teste de busca exaustiva pegou (7 trios). Fatias coincidentes e
    // degeneradas são EMPATE, não separação.
    const bool a_atras = a.ground_front <= b.ground_back;
    const bool b_atras = b.ground_front <= a.ground_back;
    if (a_atras && b_atras) {
        return 0;  // só acontece com as quatro bordas iguais: empate estável
    }
    if (a_atras) {
        return -1;
    }
    if (b_atras) {
        return 1;
    }

    // 3. As fatias de chão se interpenetram.
    const bool a_in_b = ground_contains(b, a);
    const bool b_in_a = ground_contains(a, b);
    // 3a. O CONTIDO é desenhado na frente: está nivelado com a base do outro (ao
    // lado da casa, colado no poste, dentro da borda da fonte), e um corpo não some
    // para dentro de parede. Fatias IDÊNTICAS satisfazem as duas contenções e caem
    // no critério de baixo - ninguém "contém" ninguém quando os dois são a mesma
    // fatia.
    if (a_in_b && !b_in_a) {
        return 1;  // `a` fica na frente, logo `b` desenha antes
    }
    if (b_in_a && !a_in_b) {
        return -1;
    }

    // 3b. Nem separados nem contidos: decide a borda de chão mais perto da câmera.
    if (a.ground_front < b.ground_front) {
        return -1;
    }
    if (b.ground_front < a.ground_front) {
        return 1;
    }
    // 3c. Empate exato: sem restrição, a ordem de entrada prevalece (estável).
    return 0;
}

void resolve_draw_order(unsigned char* behind, const float* key, int count,
                        int* out_order) noexcept {
    if (behind == nullptr || key == nullptr || out_order == nullptr || count <= 0) {
        return;
    }

    // A DIAGONAL guarda o estado: behind[i * count + i] != 0 significa "i já saiu".
    // Aresta de um para si mesmo não existe, então a diagonal está livre - e assim
    // Kahn roda sem NENHUM buffer além do que o chamador já passou.
    const auto emitted = [behind, count](int i) noexcept {
        return behind[i * count + i] != 0;
    };

    for (int step = 0; step < count; ++step) {
        // Liberado = ninguém que ainda não saiu precisa vir antes dele. Entre os
        // liberados, o de MENOR chave; em empate, o de menor índice (é o que
        // preserva a ordem de entrada e reproduz o Y-sort quando não há restrição).
        int pick = -1;
        for (int i = 0; i < count; ++i) {
            if (emitted(i)) {
                continue;
            }
            bool blocked = false;
            for (int j = 0; j < count; ++j) {
                if (j != i && !emitted(j) && behind[j * count + i] != 0) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) {
                continue;
            }
            if (pick < 0 || key[i] < key[pick]) {
                pick = i;
            }
        }

        if (pick < 0) {
            // CICLO (ou coordenada não-finita envenenando as comparações): ninguém
            // está liberado. Escolhe o de MENOR chave entre os que sobraram e CORTA
            // as arestas que entram nele. Determinístico e sempre progride, então a
            // saída continua sendo uma permutação completa - nunca trava, nunca
            // aborta, e a cena não treme entre quadros.
            for (int i = 0; i < count; ++i) {
                if (!emitted(i) && (pick < 0 || key[i] < key[pick])) {
                    pick = i;
                }
            }
            if (pick < 0) {
                return;  // inalcançável enquanto step < count: sempre sobra alguém
            }
            for (int j = 0; j < count; ++j) {
                behind[j * count + pick] = 0;
            }
        }

        behind[pick * count + pick] = 1;  // marca "já saiu"
        out_order[step] = pick;
    }
}

void sort_by_occlusion(DepthEntry* entries, int count, DepthSortScratch& scratch) {
    if (entries == nullptr || count <= 1) {
        return;
    }

    const std::size_t n = static_cast<std::size_t>(count);
    scratch.behind.assign(n * n, 0);
    scratch.keys.assign(n, 0.0f);
    scratch.order.assign(n, 0);
    scratch.staging.assign(entries, entries + count);

    for (int a = 0; a < count; ++a) {
        // Chave de desempate: a borda de chão mais perto da câmera. Sem nenhuma
        // aresta, ordenar por ela devolve exatamente o Y-sort de antes - o modelo
        // novo só ACRESCENTA as restrições que a chave sozinha não expressa.
        scratch.keys[static_cast<std::size_t>(a)] = entries[a].ground_front;
        for (int b = a + 1; b < count; ++b) {
            const int rel = occlusion_relation(entries[a], entries[b]);
            if (rel < 0) {
                scratch.behind[static_cast<std::size_t>(a) * n +
                               static_cast<std::size_t>(b)] = 1;
            } else if (rel > 0) {
                scratch.behind[static_cast<std::size_t>(b) * n +
                               static_cast<std::size_t>(a)] = 1;
            }
        }
    }

    resolve_draw_order(scratch.behind.data(), scratch.keys.data(), count,
                       scratch.order.data());

    for (int i = 0; i < count; ++i) {
        entries[i] = scratch.staging[static_cast<std::size_t>(scratch.order[i])];
    }
}

}  // namespace gus::core::spatial
