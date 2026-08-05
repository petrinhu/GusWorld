// SPDX-License-Identifier: Apache-2.0
// gus/core/spatial/depth_sort.hpp
//
// ORDEM DE DESENHO POR OCLUSÃO (DEMO-CIDADE-VESTIDA fatia I). Decide em que
// SEQUÊNCIA os desenháveis de pé do overworld (jogador, NPC, inimigo, casa,
// poste, fonte) são pintados, para que quem está mais perto da câmera apareça na
// frente de quem está mais longe.
//
// POR QUE NÃO É MAIS UM Y-SORT DE UMA CHAVE SÓ
// --------------------------------------------
// Até esta fatia a ordem vinha de UM número por desenhável (a base/pé, y+h) e um
// stable_sort. Isso responde certo enquanto cada desenho couber na própria célula.
// Não é mais o caso: a escala visual de peças (OverworldTuning::scene_prop_scale,
// 1,83) faz uma casa de 3 células ocupar 5,49 células de LARGURA, e o desenho
// passa a cobrir vizinhos que estão a duas células de distância. Nesse regime uma
// chave escalar não tem como responder "quem está atrás de quem": ela compara dois
// números, e a resposta depende de ONDE cada um pisa no chão em relação à BASE do
// outro, não de qual número é maior.
//
// O MODELO
// --------
// Cada desenhável entra com DUAS coisas, e é a separação delas que resolve o caso:
//   - o INTERVALO DE PROFUNDIDADE que ele ocupa NO CHÃO ([ground_back,
//     ground_front], em unidades de mundo, Y crescendo para baixo/para perto da
//     câmera). Para um personagem é a caixa dos pés; para uma peça é a fatia do
//     chão que ela realmente pisa (a caixa que bloqueia), NÃO o retângulo alto do
//     desenho, que sobe pelo ar e não encosta em lugar nenhum;
//   - a FAIXA LATERAL do DESENHO ([draw_left, draw_right]). É onde os pixels
//     podem se cobrir. Dois desenháveis que não se cruzam lateralmente NUNCA se
//     escondem, e por isso não recebem nenhuma restrição de ordem entre si.
//
// Com isso monta-se um GRAFO de "A tem de ser desenhado antes de B" e resolve-se
// por ordenação topológica (padrão consagrado quando a arte tem transparência e o
// desenho excede a própria célula). O resultado é uma permutação: a mesma entrada
// devolve sempre a mesma saída (nada de tremer entre quadros).
//
// A REGRA DE ARESTA, e o caso que ela conserta
// --------------------------------------------
// Ver `occlusion_relation` abaixo para o contrato item a item. O ponto que motivou
// a fatia: um personagem cujos pés caem DENTRO da fatia de chão de uma peça está
// nivelado com a base dela (encostado na parede, ao lado da fonte, colado no
// poste) e é desenhado NA FRENTE. Um corpo nunca some para dentro de uma parede:
// ou está atrás dela (pés ao norte da base) ou na frente.
//
// POCO puro, testável SEM GL/janela: aqui só a ORDEM (geometria e um id opaco),
// nunca o desenho - overworld_sim.cpp consome isto para escolher a sequência de
// chamadas de draw_*, e a lógica de cada desenho fica intacta.

#ifndef GUS_CORE_SPATIAL_DEPTH_SORT_HPP
#define GUS_CORE_SPATIAL_DEPTH_SORT_HPP

#include <vector>

namespace gus::core::spatial {

// Um desenhável ordenável. `id` é OPACO para este módulo: o chamador decide o que
// cada id significa (por exemplo, qual bloco de desenho invocar).
//
// CONVENÇÃO DE EIXO: Y cresce para BAIXO na tela, ou seja, na direção da câmera.
// `ground_front` (o Y maior) é a borda do chão MAIS PERTO do observador.
struct DepthEntry {
    // Fatia de chão ocupada, em unidades de mundo. Um personagem tem a caixa dos
    // pés; uma peça tem a caixa que bloqueia. `ground_back <= ground_front`;
    // desenhável sem profundidade conhecida entra com os dois iguais (uma linha),
    // o que degrada exatamente no Y-sort de antes.
    float ground_back = 0.0f;
    float ground_front = 0.0f;

    // Faixa horizontal do DESENHO, em unidades de mundo. É o retângulo pintado
    // (não a caixa de chão): é ele que cobre pixel de vizinho.
    float draw_left = 0.0f;
    float draw_right = 0.0f;

    int id = 0;
};

// Buffers reaproveitados entre quadros. O chamador guarda isto como MEMBRO (o
// render roda 60 vezes por segundo: alocar a cada quadro seria lixo no caminho
// quente). Depois dos primeiros quadros os vetores param de crescer e não há mais
// nenhuma alocação.
struct DepthSortScratch {
    // Matriz de precedência N x N: behind[a * n + b] != 0 significa "a é desenhado
    // ANTES de b" (a fica atrás). N é pequeno (o que está na tela), então a matriz
    // é mais barata e MUITO mais simples de conferir que uma lista de adjacência.
    std::vector<unsigned char> behind;
    std::vector<float> keys;
    std::vector<int> order;
    std::vector<DepthEntry> staging;
};

// Relação de oclusão entre dois desenháveis. Devolve:
//   -1  `a` é desenhado ANTES de `b` (a fica atrás);
//   +1  `b` é desenhado ANTES de `a`;
//    0  não há restrição (a ordem entre os dois é invisível ou indiferente).
//
// A decisão, em ordem:
//   1. SEM CRUZAMENTO LATERAL dos desenhos -> 0. Se os pixels não se tocam, impor
//      ordem só criaria restrição inventada (e é o que evita que o grafo fique
//      denso à toa). Encostar de raspão (borda igual à borda) não conta.
//   2. SEPARAÇÃO LIMPA em profundidade: se a fatia de chão de um termina antes de
//      a do outro começar, o que está mais ao norte fica atrás. É o caso normal.
//   3. AS FATIAS SE INTERPENETRAM. Aí:
//      3a. se a fatia de um está CONTIDA na do outro, o CONTIDO é desenhado na
//          FRENTE. É o personagem nivelado com a base da peça (ao lado da casa,
//          colado no poste, dentro da borda da fonte): ele está POR CIMA daquele
//          chão, não enterrado nele. Sem esta regra o desenho alto da peça come o
//          personagem inteiro, que foi o defeito relatado;
//      3b. senão, decide a borda de chão mais perto da câmera (`ground_front`
//          maior desenha depois);
//      3c. bordas iguais -> 0 (a ordem de entrada decide, e é estável).
[[nodiscard]] int occlusion_relation(const DepthEntry& a, const DepthEntry& b) noexcept;

// Resolve a ordem a partir de uma matriz de precedência JÁ MONTADA (Kahn).
// Exposta no contrato porque é aqui que mora a QUEBRA DE CICLO, e um caminho de
// exceção que não dá para exercitar sozinho é um caminho que não está testado.
//
//   `behind`     matriz count x count; behind[a * count + b] != 0 = "a antes de b".
//                É LIDA E ESCRITA, e sai INSERVÍVEL: a DIAGONAL é usada como marca
//                de "já saiu" (aresta de um para si mesmo não existe, então a
//                diagonal está livre e vira o estado do algoritmo sem custar
//                nenhum buffer extra), e a quebra de ciclo corta arestas. Quem
//                precisar da matriz de novo, remonta.
//   `key`        chave de desempate por desenhável (a borda de chão mais perto da
//                câmera). Entre os liberados escolhe-se a MENOR chave e, em
//                empate, o MENOR índice - determinístico e igual ao Y-sort quando
//                não há restrição nenhuma.
//   `out_order`  recebe `count` índices, a ordem de desenho (primeiro = mais atrás).
//
// CICLO: com a regra de aresta acima o grafo é acíclico por construção (todas as
// arestas menos a 3a andam no sentido crescente de `ground_front`, e 3a exige
// contenção, que não fecha volta - ver o teste que tenta montar uma). Isto aqui é
// guarda, não caminho normal: se ninguém tiver grau de entrada zero (ciclo, ou
// coordenada não-finita corrompendo as comparações), ESCOLHE o de menor chave
// entre os que sobraram, APAGA as arestas que entram nele e segue. Determinístico
// (mesma entrada, mesma saída) e sempre progride, então a função sempre devolve
// uma permutação completa - nunca trava e nunca aborta.
void resolve_draw_order(unsigned char* behind, const float* key, int count,
                        int* out_order) noexcept;

// Ordena `entries[0..count)` IN-PLACE na ordem de desenho: primeiro = pintado
// primeiro = fica ATRÁS. `entries == nullptr` ou `count <= 1` é no-op seguro.
void sort_by_occlusion(DepthEntry* entries, int count, DepthSortScratch& scratch);

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_DEPTH_SORT_HPP
