// gus/domain/settings/system_settings.hpp
//
// SystemSettings: schema MINIMO de preferencias de sistema persistidas em disco
// (MENU-PAUSA-CONFIG-SOM, M7-COSTURA). Escopo travado pelo lider - SO volume de
// musica/SFX nesta onda (NAO video/idioma/controles - isso e settings.json de um
// jogo REAL mais maduro, fora do escopo minimo). POCO puro, ZERO Qt, ZERO I/O
// (mesmo espirito de gus/domain/input/input_binding.hpp).
//
// Distinto do SAVE (gus/domain/save/save_data.hpp): SaveData e o progresso do
// JOGO (posicao, party, flags); SystemSettings e a PREFERENCIA do JOGADOR (volume),
// independente de qualquer partida - vive em ~/.gusworld/settings.json (fora da
// pasta de saves), formato JSON (nao binario+HMAC como o save - nao ha necessidade
// de anti-tamper aqui, e so uma preferencia de conforto do jogador editavel a mao).
//
// Cross-ref: gus/domain/settings/system_settings_json.hpp (serialize/parse JSON
//            proprio, mesmo padrao de gus/domain/input/controls_json.hpp);
//            gus/platform/fs/settings_file_store.hpp (I/O real em disco, app/platform).

#ifndef GUS_DOMAIN_SETTINGS_SYSTEM_SETTINGS_HPP
#define GUS_DOMAIN_SETTINGS_SYSTEM_SETTINGS_HPP

namespace gus::domain::settings {

// Versao do schema (forward-compat, mesmo espirito de InputRemapConfig::config_version).
// So cresce quando um campo novo for adicionado de um jeito que quebre o parse antigo.
inline constexpr int kSystemSettingsSchemaVersion = 1;

struct SystemSettings {
    int schema_version = kSystemSettingsSchemaVersion;

    // Volume por grupo em [0,1] (mesma faixa de AudioEngine::set_music_volume/
    // set_sfx_volume - platform/audio/audio_engine.hpp). Default 1.0f = volume
    // cheio (comportamento de sempre antes desta feature existir).
    float music_volume = 1.0f;
    float sfx_volume = 1.0f;
};

}  // namespace gus::domain::settings

#endif  // GUS_DOMAIN_SETTINGS_SYSTEM_SETTINGS_HPP
