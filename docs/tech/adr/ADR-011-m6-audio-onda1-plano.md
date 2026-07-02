# ADR-011: Plano da 1ª onda do M6 (Áudio) — escopo, prioridade, curadoria e arquitetura

**Status:** Accepted (decisões fechadas em brainstorm ao vivo 2026-07-02; registro revisado e aprovado pelo criador na mesma data, com ordem de disparar a F1)
**Data:** 2026-07-02
**Decisores:** criador supremo (petrus) + software-architect (Caetano/CTO)
**Cross-ref:** [ADR-008](ADR-008-repivot-qt-to-sdl3.md) (SDL3 + miniaudio escolhidos como stack de áudio, item 3 da decisão), `docs/narrative/deep/ontologia/leitmotivs-musicais-detalhados.md` (bíblia de leitmotivs, regra R7 "não modificar sem aprovação" — §1 sistema de camadas foreground/midground/background + canal-4 Patch-Zero, linha 109/118 tema "01. GusWorld City cyber-gótica")

## Contexto

O ADR-008 já decidiu a biblioteca de áudio (miniaudio, vendorizada em `third_party/`) mas não implementou nada: `GusEngine/platform/audio/` existe como diretório reservado vazio, ao lado de `render2d`/`input`/`window`, sem nenhum arquivo. A animação de combate (M5, `battle_anim.cpp`) roda hoje muda: o hit-react visual dispara sem nenhum som acompanhando. O M6 do board tem 3 critérios de saída conhecidos (música em loop, SFX no hit, fade entre telas, sem stutter) e o design antigo da era Godot (F2-AU.3/.5/.7) já havia ratificado uma arquitetura de 5 buses (Master/Music/SFX/UI/Voice) + ducking -12dB + snapshots de combate + música adaptive via `CombatBus.ActionResolved` (signal nativo do Godot).

Antes de abrir a 1ª onda de implementação, o criador e o Caetano/CTO rodaram um brainstorm de design (via `AskUserQuestion`, 4 tópicos) para fechar escopo, prioridade, direção criativa e arquitetura técnica desta onda especificamente, sem herdar automaticamente a arquitetura Godot completa (que dependia de infraestrutura nativa da engine antiga, hoje inexistente).

## Decisão

### 1. Escopo da onda: híbrido (kit mínimo real, não bipe sintético puro)

A camada `platform/audio` nasce nesta onda já acoplada a um kit mínimo **real**: 2-3 SFX prontos de banco livre CC0 + 1 faixa provisória que faz loop. Descartado o caminho de testar o cano técnico com tom/bipe gerado puro: um bipe prova que o device inicializa e toca, mas não prova se o hit "sente" bom — que é literalmente parte do critério de saída do M6. O `audio-designer-composer` entra **depois** desta onda para substituir/expandir o kit provisório com a bíblia de leitmotivs em mãos (produção nova, sessão dedicada).

### 2. Prioridade dentro da onda: alicerce → SFX do hit → música em loop

Ordem de trabalho, nesta sequência:

1. **Alicerce técnico** — inicializar device miniaudio + mixer/buses mínimo.
2. **SFX do hit** — mata a mudez da animação de combate hoje; prova o padrão de gatilho evento-de-jogo → som, reusável depois para footstep/carta/UI/scan.
3. **Música em loop + fade** — atacado por último porque é o pedaço tecnicamente mais espinhoso (streaming, loop sem gap audível, crossfade), só depois de já ter o device provado com o SFX.

Os 3 critérios de saída do M6 (música em loop, SFX no hit, fade entre telas, sem stutter) fecham dentro desta onda.

### 3. Direção criativa: bíblia como filtro de curadoria, não produção nova

A bíblia de leitmotivs (`docs/narrative/deep/ontologia/leitmotivs-musicais-detalhados.md`, R7, imutável sem aprovação) já define o tom macro do tema "01. GusWorld City cyber-gótica": **Lá menor, 4/4, ~55 BPM, sintetizador analógico + sinos de catedral distantes + drone urbano**. Nesta onda a bíblia serve **só de filtro** para escolher a faixa CC0 provisória — respeitar essa receita na curadoria, sem compor nada novo.

O SFX de hit segue o mesmo princípio de filtro pelo Pillar "magia = software": deve soar **digital/rúnico/sintético** (um "impacto de script executando"), nunca um thwack orgânico de espada de fantasia genérica — coerente com Gus ser um prodígio analítico de 11 anos, não um guerreiro.

Zero produção criativa nova nesta onda. A sessão criativa completa (assinatura sonora do combate, leitmotiv de Gus, canal-4 do Patch-Zero) fica para uma onda futura dedicada com `audio-designer-composer`.

### 4. Arquitetura técnica

- **Local:** `GusEngine/platform/audio/`, seguindo a mesma convenção de 4 camadas do projeto (core/domain POCO puro; platform/app na fronteira).
- **Invariante preservada:** o domínio (`domain/combat/combat_state_machine.hpp`, a FSM de combate) permanece 100% POCO puro e NUNCA chama áudio diretamente. Áudio só é disparado da camada de apresentação (`app/screens`).
- **API mínima desta onda** (deliberadamente pequena, anti over-engineering): `play_sfx(id)`, `play_music(id, loop)`, `stop_music(fade)`, `set_master_volume(v)`. Os 5 buses completos (Master/Music/SFX/UI/Voice, design herdado da era Godot em F2-AU.3/.5/.7 e já ratificado), o ducking -12dB e os snapshots de combate ficam para uma onda futura, junto com a música adaptive de verdade.
- **Mecanismo de disparo: chamada direta da tela, sem event bus.** No instante em que `app/src/screens/battle_anim.cpp` já marca o contato do golpe (mesmo ponto onde o hit-react visual dispara hoje), a tela chama `play_sfx("hit")` diretamente. Fade de música na troca `battle_scene` ↔ `city_scene`.
- **Motivo de NÃO criar um event bus agora** (decisão explícita do Caetano/CTO): é infraestrutura especulativa que 1 tela + poucos emissores não justifica ainda, violando "arquitetura mais simples que satisfaz os requisitos reais". O design Godot antigo (`CombatBus.ActionResolved`) dependia de signals nativos da engine, grátis lá; aqui teria que ser construído do zero. A migração para um event bus fica reavaliada quando os emissores de áudio se multiplicarem (footstep, carta, UI, scan, stinger — já mapeados no design antigo); a migração seria barata porque a API (`play_sfx` etc.) não muda.

## Opções consideradas

1. **Bipe/tom sintético puro para provar o cano técnico** — mais rápido de montar, mas não valida se o hit "sente" bom (parte do critério de saída do M6) nem dá material real para playtest. Rejeitada.
2. **Herdar de imediato a arquitetura completa da era Godot (5 buses + ducking + snapshots + event bus `CombatBus`)** — replicaria infraestrutura dimensionada para signals nativos do Godot, inexistentes em C++/SDL3; sobre-engenharia para 1 tela + poucos emissores nesta onda. Rejeitada para esta onda (fica mapeada para o futuro).
3. **Híbrido: kit mínimo real + API mínima + chamada direta sem event bus — ESCOLHIDA** — prova o padrão de gatilho evento-de-jogo → som com material que já soa como o jogo pretende soar, sem construir infraestrutura que a onda atual não precisa.

## Consequências

**Positivas:** fecha os 3 critérios de saída do M6 dentro de uma única onda; o SFX do hit resolve a mudez da animação de combate herdada do M5; a curadoria via bíblia garante que o kit provisório já soa coerente com o canon (Lá menor/55 BPM/sintetizador analógico para a cidade; digital/rúnico para o hit), reduzindo retrabalho quando o `audio-designer-composer` entrar; API mínima mantém a fronteira `platform/audio` simples e fácil de testar; a invariante do domínio POCO puro (combat_state_machine sem áudio) se mantém intacta.

**Negativas / aceitas:** o kit CC0 é provisório e será substituído; os 5 buses completos, ducking e snapshots de combate ficam adiados (mitigado por não serem parte do critério de saída desta onda); a ausência de event bus significa que, se os emissores de áudio se multiplicarem antes de uma refatoração deliberada, `battle_anim.cpp` (e telas futuras) acumulam chamadas diretas — risco monitorado, não bloqueante, porque a API não muda na migração futura.

## Reversibilidade

Two-way door. A API mínima (`play_sfx`/`play_music`/`stop_music`/`set_master_volume`) isola os consumidores (`app/screens`) do backend miniaudio; trocar o kit CC0 provisório pela produção definitiva do `audio-designer-composer` não exige mudança de assinatura. Introduzir um event bus depois é aditivo, não uma reescrita. Sem releases públicas (jogo em DEV).

## Execução (faseada, prevista — NÃO iniciada)

- **Pré-req:** aprovação final do criador sobre este registro.
- **F1:** alicerce técnico — inicializar device miniaudio + mixer/buses mínimo em `GusEngine/platform/audio/`. Agente previsto: `backend-engineer` / `engine-graphics-programmer`.
- **F2:** curadoria do kit provisório (2-3 SFX CC0 "digital/rúnico" + 1 faixa CC0 "Lá menor/4-4/~55 BPM/sintetizador analógico + sinos + drone urbano") filtrada pela bíblia de leitmotivs. Agente previsto: `audio-designer-composer`.
- **F3:** SFX do hit — hook em `app/src/screens/battle_anim.cpp` no ponto onde o hit-react visual já dispara, chamando `play_sfx("hit")` diretamente. Agente previsto: `backend-engineer` / `engine-graphics-programmer`.
- **F4:** música em loop sem gap audível + fade na troca `battle_scene` ↔ `city_scene`. Agente previsto: `backend-engineer` / `engine-graphics-programmer`.
- **F5:** validação dos 3 critérios de saída do M6 (música em loop, SFX no hit, fade entre telas, sem stutter).

Nenhuma fase acima foi executada até a data deste registro.
