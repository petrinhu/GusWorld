// gus/core/spatial/depth_sort.hpp
//
// Y-SORT (M7-COSTURA/M7-DIALOGO, colisao solida + profundidade no desenho): a
// ORDEM em que os "desenhaveis de personagem" do overworld (jogador, NPC, inimigo)
// sao pintados na tela - padrao consagrado de RPG top-down (Zelda ALttP/Stardew
// Valley): quem esta mais "embaixo" na tela (Y maior, mais perto da camera) DESENHA
// NA FRENTE de quem esta mais "em cima" (Y menor) - assim, ao passar do LADO de um
// NPC/inimigo (a colisao solida - ver grid_collision.hpp::ObstacleSpan - impede
// OCUPAR a mesma posicao, mas nao impede ficar adjacente), a leitura visual de
// profundidade fica correta em vez de uma ordem FIXA (sempre "fundo -> inimigo ->
// NPC -> jogador", independente de onde cada um esta).
//
// POCO puro, testavel SEM GL/janela: aqui so a ORDENACAO (a chave de profundidade e
// o ID de cada entrada), NAO o desenho em si - overworld_sim.cpp consome isto pra
// decidir em que SEQUENCIA chamar cada bloco de draw_* (a logica de cada desenho
// fica intacta, so a ordem de invocacao muda).

#ifndef GUS_CORE_SPATIAL_DEPTH_SORT_HPP
#define GUS_CORE_SPATIAL_DEPTH_SORT_HPP

namespace gus::core::spatial {

// Uma entrada ordenavel: `depth_key` e a profundidade (tipicamente a BASE/pe do
// desenhavel, aabb.y + aabb.h - quanto MAIOR, mais "embaixo" na tela, mais na
// FRENTE); `id` e OPACO pra este modulo - o chamador decide o que cada id significa
// (por exemplo, qual bloco de desenho invocar).
struct DepthEntry {
    float depth_key = 0.0f;
    int id = 0;
};

// Ordena `entries[0..count)` IN-PLACE por depth_key ASCENDENTE (menor primeiro =
// desenha PRIMEIRO = fica ATRAS; maior por ultimo = desenha por CIMA). Estavel: em
// EMPATE exato de depth_key, preserva a ordem de entrada (deterministico, sem
// "flicker" por reordenacao arbitraria de itens na mesma linha/Y). `entries ==
// nullptr` ou `count <= 1` e no-op seguro.
void sort_by_depth(DepthEntry* entries, int count) noexcept;

}  // namespace gus::core::spatial

#endif  // GUS_CORE_SPATIAL_DEPTH_SORT_HPP
