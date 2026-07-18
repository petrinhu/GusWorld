// gus/domain/save/new_game.hpp
//
// MENU-INICIAL (ACHADO 1, backend_engineer): "Novo Jogo" alcancado FORA do boot -
// pausa na cidade -> Menu Inicial -> tela de titulo -> "Novo Jogo" (ver
// gus/app/maestro.hpp::show_title_screen(), MENU-INICIAL) - precisa reconstruir o
// SaveData equivalente a um jogo do ZERO. Ate esta onda, "estado fresco" so existia
// IMPLICITAMENTE em Maestro::save_{} (default-construction, ver o membro em
// maestro.hpp) - correto SO enquanto show_title_screen() era chamada UMA vez, no
// boot, ANTES de qualquer gameplay (a suposicao original documentada la). Agora que
// a mesma tela e alcancavel EM PLENO JOGO, o "estado fresco" precisa de um NOME, um
// CONTRATO e um TESTE - nao pode continuar implicito num default-member-initializer
// que ninguem mais chama de novo depois do boot.
//
// POCO puro (ZERO Qt/SDL/I-O, engine-design.md secao 2): so monta o VALOR do
// SaveData; NAO grava nada em disco (isso e save_game(), plataforma/fs) nem mexe em
// posicao de jogador/markers/audio (isso e app/, orquestrado pelo Maestro - ver o
// comentario de Maestro::reset_to_new_game() em maestro.hpp).
//
// Cross-ref: gus/domain/save/save_data.hpp (SaveData/DifficultyLevel),
//            gus/app/maestro.hpp (reset_to_new_game(), o ORQUESTRADOR que consome
//            isto + reseta a posicao do jogador/inimigo/playtime em memoria).

#ifndef GUS_DOMAIN_SAVE_NEW_GAME_HPP
#define GUS_DOMAIN_SAVE_NEW_GAME_HPP

#include "gus/domain/save/save_data.hpp"

namespace gus::domain::save {

// Devolve o SaveData de um jogo NOVO "do zero": schema atual (kSaveSchemaVersion),
// playtime 0, cena/posicao/rotacao vazias, TODOS os mapas (flags/inventory/quest_
// progress/relations/character_states/enemy_knowledge) vazios, credits 0,
// difficult_recovery_stage 0 - e a dificuldade ESCOLHIDA na tela de selecao
// (`difficulty`, o UNICO campo que nao tem um default universal: e a escrita
// LEGITIMA de criacao de save, ver o comentario de DifficultyLevel acima). slot_id
// fica -1 (origem desconhecida ATE o 1o save_game gravar num slot real - mesmo
// significado que o campo ja documenta).
//
// Equivalente, POR CONSTRUCAO, a `SaveData data{}; data.difficulty = difficulty;
// return data;` - a razao de existir NAO e a complexidade (e trivial), e ser a
// fonte UNICA de verdade, NOMEADA e TESTADA, do "shape" de um save novo: qualquer
// campo futuro que precise de uma regra de criacao DIFERENTE do default-member-
// initializer (ex.: um valor inicial de credits != 0) muda AQUI, uma vez so, em vez
// de cada call-site (boot, Novo Jogo pos-boot, testes futuros) reimplementar a
// mesma decisao a mao e arriscar divergir.
[[nodiscard]] SaveData fresh_new_game_save_data(DifficultyLevel difficulty) noexcept;

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_NEW_GAME_HPP
