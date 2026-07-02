// gus/app/audio_smoke.hpp
//
// AudioSmoke: diagnostico OPCIONAL do device de audio REAL (M6 F1, ADR-011, item 5 -
// "opcional-mas-desejavel"). Acionado por env GUSWORLD_AUDIO_SMOKE=1 (mesmo padrao dos
// varios GUSWORLD_*_SELFTEST/SMOKE espalhados por app/src/screens/battle_preview.cpp).
//
// O QUE FAZ: inicializa o AudioEngine com device_active=true (tenta o device de audio
// REAL do SO - NAO o modo null-device usado pelos testes headless), gera um tom curto
// EM RUNTIME (WAV sintetico escrito num arquivo temporario - nao ha nenhum SFX/musica
// real no repo ainda; a curadoria do kit CC0 e F2, fase futura), toca via play_sfx +
// play_music/stop_music (fade), e encerra limpo. Reporta no stdout se o device real
// subiu (available()==true) ou se degradou (available()==false, ex.: CI/xvfb sem
// placa de som) - os DOIS resultados sao "sucesso" do ponto de vista deste smoke: a
// prova e que NENHUM dos dois caminhos crasha (degradacao graciosa, ADR-011).
//
// USO (manual, fora do CI - e um smoke de HARDWARE, nao entra no tools/check.sh):
//   GUSWORLD_AUDIO_SMOKE=1 ./build/linux-release/app/gusworld_app
//
// Nao abre janela nem toca SDL - roda e sai antes de qualquer SDL_Init.

#ifndef GUS_APP_AUDIO_SMOKE_HPP
#define GUS_APP_AUDIO_SMOKE_HPP

namespace gus::app {

// true se GUSWORLD_AUDIO_SMOKE=1 estiver no ambiente.
bool audio_smoke_requested();

// Roda o diagnostico (ver header). Sempre devolve 0 (o objetivo e provar ausencia de
// crash, nao decidir se o hardware existe - isso e reportado no stdout).
int run_audio_smoke();

}  // namespace gus::app

#endif  // GUS_APP_AUDIO_SMOKE_HPP
