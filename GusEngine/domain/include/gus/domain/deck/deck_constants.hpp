// gus/domain/deck/deck_constants.hpp
//
// TODAS as constantes numericas do sistema de deck/mao (DECK-1), num header so, cada
// uma marcada //PLAYTEST (deck-mao-sistema.md secao 8c, parecer economy-designer +
// decisao do lider 2026-07-16). Afinaveis no playtest N=3; nenhum consumidor
// (gameplay_engineer, DECK-2 mao/loadout, DECK-3 economia/venda) deve hardcodar estes
// valores soltos no proprio codigo - sempre importar daqui (fonte unica).
//
// POCO puro, ZERO SDL/glintfx.
//
// Cross-ref: docs/design/mecanicas/deck-mao-sistema.md secao 3/6.2/7/8c.

#ifndef GUS_DOMAIN_DECK_DECK_CONSTANTS_HPP
#define GUS_DOMAIN_DECK_DECK_CONSTANTS_HPP

namespace gus::domain::deck {

// ---- Mao (loadout) - secao 3/4/8c --------------------------------------------------

// Mao comum base (todos os personagens, antes do ajuste por identidade/stat). //PLAYTEST
inline constexpr int kHandSizeBase = 5;

// Delta do Bento Requiem (tanque): mao base -1, trava de identidade (compensada em
// Def/HP no template dele, NAO aqui). Mao efetiva do Bento = kHandSizeBase +
// kHandDeltaBento = 4. //PLAYTEST
inline constexpr int kHandDeltaBento = -1;

// Teto do componente de stat mental/foco somado a mao: mao maxima <= toolkit-1 (~6).
// Sem este teto a mao iguala o toolkit inteiro no late-game e mata a escolha
// (anti-snowball, secao 8c). //PLAYTEST
inline constexpr int kHandSizeMentalStatCap = 6;

// Slots dedicados SO pra carta especial na mao do Gus (nao compete com os comuns;
// identidade de compilador sem torna-lo dominante). //PLAYTEST
inline constexpr int kGusSpecialHandSlots = 1;

// ---- Deck/bolsa - capacidade do deck ATIVO (progressao explicita por upgrade) ------

inline constexpr int kDeckCapacityTier1 = 34;  // //PLAYTEST
inline constexpr int kDeckCapacityTier2 = 55;  // //PLAYTEST
inline constexpr int kDeckCapacityTier3 = 89;  // //PLAYTEST

// ---- Deck morto - pilha persistente/inerte/one-way (secao 6.2/7 inv.3/4/9) --------

// Limite POR CONTAGEM (nao peso - peso e micromanagement chato pra 11+). O pipeline de
// auto-upload FIFO ao encher e ONDA 2 (DECK-1 so declara a constante; nao implementa o
// FIFO). //PLAYTEST
inline constexpr int kDeadDeckLimit = 8;

// ---- Economia de carta (onda 2/DECK-3 consome; declaradas aqui pra fonte unica) ----

// Upload/reciclagem ao repositorio-commons (credito base por carta do deck morto).
// //PLAYTEST
inline constexpr int kUploadCreditMin = 2;
inline constexpr int kUploadCreditMax = 3;

// Compra de carta comum na loja "app store". //PLAYTEST
inline constexpr int kShopBuyPriceMin = 12;
inline constexpr int kShopBuyPriceMax = 18;

// Venda de carta comum: NPC errante (so compra) x loja (compra e vende). //PLAYTEST
inline constexpr int kNpcSellPriceMin = 3;
inline constexpr int kNpcSellPriceMax = 5;
inline constexpr int kShopSellPriceMin = 4;
inline constexpr int kShopSellPriceMax = 6;

// ---- Achados na grama - encontro-surpresa com pity/anti-streak --------------------

inline constexpr int kGrassPityCap = 13;             // teto de encontros ate garantir. //PLAYTEST
inline constexpr int kGrassBaseRatePercentMin = 10;  // taxa-base minima por encontro. //PLAYTEST
inline constexpr int kGrassBaseRatePercentMax = 15;  // taxa-base maxima por encontro. //PLAYTEST

}  // namespace gus::domain::deck

#endif  // GUS_DOMAIN_DECK_DECK_CONSTANTS_HPP
