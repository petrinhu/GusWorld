// gus/domain/save/save_policy.hpp
//
// POLITICA DE AUTOSAVE POR LOCAL (SAVE-LOAD-UI etapa 5, decisao do lider). POCO puro
// (ZERO I/O, ZERO SDL/Qt) - so o PREDICADO "o autosave pode gravar agora?" dado o local
// e 2 overrides. Mesma memoria canonica: project_save_dungeon_pem_faraday.
//
// REGRA CANONICA:
//   - CIDADE (LocationKind::City): autosave SEMPRE permitido (save livre).
//   - DUNGEON estilo Zelda (LocationKind::Dungeon): autosave OFF por padrao (save so
//     na porta de entrada/saida, fora do escopo deste hook) - EXCETO se o jogador ja
//     descobriu o PEM escondido daquela dungeon (has_pem_discovered) OU tem a carta
//     passiva "Gaiola de Faraday" (has_faraday_card). Qualquer um dos 2 liga o
//     autosave dentro da dungeon.
//
// NESTA FATIA (vertical slice, M4) SO EXISTE CIDADE - nenhuma dungeon real ainda
// (DUNGEON-PEM-ITEM-BRAINSTORM/FARADAY-DUNGEON-ITENS seguem em brainstorm). O
// CHAMADOR (gus::app::Maestro::maybe_autosave) consulta este hook com
// LocationKind::City + has_pem_discovered=false + has_faraday_card=false
// hardcoded POR ORA - o predicado ja esta pronto e testado pro dia em que a 1a
// dungeon existir (so os 2 booleanos passam a vir de estado REAL de
// inventario/exploracao; a CHAMADA em si nao muda).
//
// Cross-ref: memoria project_save_dungeon_pem_faraday; gus/app/maestro.cpp
//            (maybe_autosave, unico consumidor por ora); item TODO.md SAVE-LOAD-UI
//            etapa 5; DUNGEON-PEM-ITEM-BRAINSTORM/FARADAY-DUNGEON-ITENS (dungeon real
//            futura).

#ifndef GUS_DOMAIN_SAVE_SAVE_POLICY_HPP
#define GUS_DOMAIN_SAVE_SAVE_POLICY_HPP

namespace gus::domain::save {

// Tipo de local pra fins de politica de save/autosave. So 2 valores hoje (a
// vertical slice so tem cidade) - dungeons futuras continuam Dungeon (nao ganham
// um enumerador por dungeon: a distincao de QUAL dungeon nao importa pra esta
// regra, so "e uma dungeon estilo Zelda ou nao").
enum class LocationKind {
    City,
    Dungeon,
};

// Predicado PURO: true se o autosave pode gravar agora, dado o local e os 2
// overrides (irrelevantes quando location==City, sempre true nesse caso).
// Dungeon: true SO se has_pem_discovered OU has_faraday_card (OR logico - qualquer
// um dos 2 basta).
[[nodiscard]] constexpr bool autosave_allowed_at(LocationKind location,
                                                  bool has_pem_discovered,
                                                  bool has_faraday_card) noexcept {
    if (location == LocationKind::City) return true;
    return has_pem_discovered || has_faraday_card;
}

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_POLICY_HPP
