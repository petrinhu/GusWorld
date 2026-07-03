# ADR-012: Plano do M7 (paridade jogável) — costura, save e diálogo

**Status:** Accepted (decisões fechadas em brainstorm; aprovado pelo criador em 2026-07-03 com ordem de disparar a 1ª peça, `M7-COSTURA`)
**Data:** 2026-07-03
**Decisores:** criador supremo (petrus) + Caetano (CTO). A estrutura fina do maestro e os contratos do runtime de diálogo serão levados ao `software-architect` no modo colaborativo na hora de implementar cada peça.
**Cross-ref:** [ADR-006](ADR-006-crypto-hmac-formato-domain.md) (crypto SHA-256/HMAC própria, reusada pelo compilador de conteúdo), [ADR-007](ADR-007-controls-json-hash128-save-v4.md) (fork `load_save`/`deserialize_save`, decisão herdada não reaberta), [ADR-008](ADR-008-repivot-qt-to-sdl3.md) (SDL3), [ADR-010](ADR-010-adopt-glintfx-embed-mode.md) (glintfx/UI), [ADR-011](ADR-011-m6-audio-onda1-plano.md) (áudio onda 1, dívida da `battle_preview` que este ADR paga), `docs/design/narrativa/dialogue-tree-npc-intro.md` (blueprint do diálogo), memória `project_i18n_canonico` (i18n-ready desde o dia 1), memória `reference_formato_mapa_gmap` (padrão fonte→compilador→artefato selado, reusado aqui)

## Contexto

O M7 é o milestone de **integração**: o critério de saída é um playthrough de ~5 minutos (andar, encontrar um NPC, entrar em combate, salvar, carregar) rodando 100% na engine nova, sem Godot. M2 (save), M5 (combate) e M6 (áudio) já estão implementados e na `main`, mas nunca foram costurados entre si — hoje são peças isoladas. M7-DIÁLOGO é a única peça de conteúdo novo do milestone: o jogo ainda não tem nenhum sistema de conversa/NPC.

Antes de abrir a 1ª peça, o criador e o Caetano/CTO rodaram um brainstorm de design (via `AskUserQuestion`) para fechar escopo mínimo, ordem de ataque, arquitetura da costura, arquitetura do diálogo e o requisito de conteúdo cifrado, sem herdar automaticamente escopo do design antigo da era Godot nem abrir infraestrutura especulativa.

## Decisão

### 1. Escopo mínimo: prova a costura, não o conteúdo

O M7 entrega o **mínimo que prova tudo**: 1 NPC com 1 escolha que seta 1 flag; a flag sobrevive ao save/load (esse é o critério de saída do diálogo); visual da conversa é **provisório**; 1 inimigo **fixo** parado no mapa (sem spawn); save em **1 slot**; crossfade cidade↔batalha (fecha o M6). Conteúdo gordo — mais NPCs, ramificação de diálogo, spawn de inimigos, multi-slot de save — fica para PI7/PI8. O objetivo do M7 é provar que as peças já implementadas (M2, M5, M6) e a peça nova (diálogo) se encaixam de ponta a ponta, não entregar volume de conteúdo.

### 2. Ordem de ataque: Costura → Save → Diálogo

1. **Costura** primeiro — é o esqueleto (loop jogável cidade↔batalha) e fecha o M6 cedo (crossfade), com o menor risco técnico das três peças.
2. **Save** depois — barato porque o motor de persistência já existe (M2); só falta I/O real em disco.
3. **Diálogo** por último — é a peça maior e mais nova; entra sobre uma base já estável (costura + save funcionando), reduzindo o custo de descobrir problema de integração tarde.

### 3. Fork do save: contratos duplos mantidos (decisão herdada)

Mantidos os **dois contratos** já fixados no ADR-007: `load_save` (não-lançante, retorna `LoadResult`) é o contrato da camada `app`; `deserialize_save` (lançante) é a primitiva interna usada pelos testes legados e pelo próprio `load_save`. Esta é uma decisão herdada, não reaberta neste brainstorm — o M7 apenas consome `load_save` na integração real com disco.

### 4. Arquitetura da costura: maestro leve acima das 2 telas

Um **maestro leve** (não um gerenciador de cenas completo) fica acima das telas `city_scene`/`battle_scene`. Ele guarda o estado que sobrevive à troca de tela (posição do Gus, música corrente, save) e alterna cidade↔batalha por um contrato simples: "contra quem" (cidade → batalha) e "ganhou/perdeu" (batalha → cidade). A posse da música **sobe** da tela de batalha para o maestro — hoje o `AudioEngine` é dono da `battle_preview` (dívida consciente registrada no M6/ADR-011); é essa subida de posse que destrava o crossfade e paga a dívida. Explicitamente **não** é um gerenciador de cenas genérico (anti over-engineering); vira um se e quando o jogo pedir mais de 2 telas com regras de transição mais ricas.

### 5. Transição: fade preto curto com crossfade de música

A transição cidade↔batalha é um **fade preto curto**. A música cruza (crossfade) durante o escurecimento — é o gancho natural que fecha o critério de saída do M6. Transição temática/glitch (ex: efeito visual ligado ao Pillar "magia = software") fica para o board de juice (PI7), fora do escopo do M7.

### 6. Motor de diálogo: pequeno, real, data-driven

O motor de diálogo é pequeno mas **de verdade**: falas vêm de **arquivo** (data-driven), não chumbadas em código; suporta sequência de falas + a escolha que seta a flag. Cada fala carrega uma **etiqueta de registro** (terminal frio × caixa quente, ver `DIALOGO-TERMINAL` no blueprint) e a identidade de quem fala; o motor lê e repassa a etiqueta para a apresentação — a pintura visual é provisória agora, e a onda futura de design troca só o renderizador da UI sem tocar o motor. Sem condicionais nem reconvergência de árvore nesta onda: não são exercidos com 1 NPC / 1 escolha, e adicioná-los agora seria infraestrutura especulativa.

### 7. i18n: obrigatório desde o dia 1 (canon, sem exceção)

O diálogo referencia **chaves i18n**, nunca texto literal — reusa o `Translator` e os catálogos já em produção (`GusEngine/app/src/i18n/translator.cpp`, `domain/src/i18n/md_translation_loader.cpp`, `game/translations/pt_br.md`/`en_intl.md`), o mesmo mecanismo que o combate já consome via `tr()`. Zero string hardcoded user-facing (memória `project_i18n_canonico`). O NPC do MVP fala inteiramente por chaves cadastradas em `pt_br.md`.

### 8. Crypto: artefato distribuído cifrado e selado, fonte editável

Requisito do criador: falas não editáveis/não adulteráveis pelo jogador. Solução: pipeline **fonte → artefato**, no mesmo padrão já usado pelo formato de mapa `.gmap` (memória `reference_formato_mapa_gmap`). A **fonte** de tradução (`pt_br.md`/`en_intl.md`) permanece **editável no repositório** — é onde o tradutor trabalha. No build, um **compilador de conteúdo** cifra e sela essa fonte, gerando um **artefato distribuído não editável** que o jogo carrega; o runtime valida o selo e decifra antes de usar. "Não editável" recai sobre o artefato entregue ao jogador, nunca sobre a fonte de tradução do repositório.

O compilador **reusa a crypto própria já auditada** (SHA-256/HMAC, ADR-006) — nenhuma crypto nova é criada. Ele sela o **catálogo compilado inteiro** (todo texto user-facing, incluindo os verbos de combate já existentes), não só as falas novas do diálogo: um catálogo parcialmente selado deixaria o datamine ler pelo pedaço ao lado, então é uma coisa só.

**Limitação honesta (registrada por dever de não vender proteção que não existe):** crypto client-side eleva a barreira de entrada e detecta adulteração — o jogo recusa um pacote mexido, porque o selo falha a validação — mas não é um cofre inviolável. A chave de decodificação mora no próprio binário, pois o jogo precisa mostrar as falas em tela; um extrator determinado sempre consegue quebrar. É "anti-tamper prático", a mesma garantia (nem mais, nem menos) já oferecida pelo `.gmap` e pelo save.

## Opções consideradas

1. **Escopo cheio (multi-NPC, ramificação, spawn, multi-slot) já no M7** — provaria mais conteúdo, mas confunde "provar a costura" com "entregar conteúdo" e atrasa o critério de saída do milestone de integração. Rejeitada para o M7; fica mapeada para PI7/PI8.
2. **Diálogo primeiro (maior peça, atacar logo)** — reduziria a incerteza da peça mais nova mais cedo, mas construiria sobre uma base (costura, save) ainda não provada, elevando o custo de descobrir problema de integração tarde. Rejeitada; costura e save vêm antes por serem mais baratos e destravarem o M6.
3. **Gerenciador de cenas completo em vez de maestro leve** — resolveria transições mais ricas no futuro, mas é infraestrutura especulativa para 2 telas hoje. Rejeitada para esta onda; o maestro leve é aditivo caso o jogo cresça para mais telas.
4. **Transição temática/glitch em vez de fade preto** — mais expressiva visualmente, mas é trabalho de juice fora do escopo funcional do M7. Adiada para PI7.
5. **Motor de diálogo com condicionais/reconvergência já no M7** — antecipa a peça de design mais complexa antes de ser exercida (1 NPC / 1 escolha não justifica). Rejeitada para esta onda.
6. **Não cifrar o catálogo agora (ficar só com i18n em texto claro)** — mais simples, mas descumpre requisito explícito do criador ("falas não editáveis") e deixaria o combate já publicado sem essa garantia quando o diálogo a exigir. Rejeitada.
7. **Cifrar só as falas novas do diálogo, deixando o catálogo de combate em claro** — reduziria escopo do compilador, mas cria a brecha do datamine ler pelo catálogo ao lado (a mesma informação, exposta por outro arquivo). Rejeitada; o selo cobre o catálogo compilado inteiro.
8. **Pipeline fonte→cifra+sela→artefato, reusando a crypto do ADR-006 — ESCOLHIDA** — atende o requisito do criador sem inventar crypto nova, mantém a fonte editável para o tradutor, e é honesta sobre o limite de proteção client-side.

## Consequências

**Positivas:** o playthrough de ~5min sem Godot vira alcançável dentro de uma sequência de 3 peças de risco crescente-controlado; o M6 fecha definitivamente com o crossfade da costura, pagando a dívida da posse de música; o motor de diálogo nasce data-driven e i18n-ready, evitando retrabalho quando a onda de design entrar com mais conteúdo; o compilador de conteúdo cifrado atende o requisito do criador sem crypto nova e sem sacrificar a editabilidade da fonte de tradução; o maestro leve resolve a integração sem abrir um gerenciador de cenas genérico prematuro.

**Negativas / aceitas:** o M6 permanece formalmente 🔄 até o crossfade da costura ser entregue (não fecha sozinho); M7-DIALOGO-RUNTIME fica bloqueado por M2-SAVE-IO (dependência dura: a flag da escolha só prova sobreviver ao save se o save já grava em disco de verdade); o compilador de conteúdo cifra+sela é infraestrutura nova que passa a intermediar o carregamento de **todo** texto user-facing do jogo, incluindo o catálogo de combate já em produção — é a maior superfície de mudança do M7, mitigada por TDD e usando o oráculo de roundtrip do save (ADR-007) como referência de teste; a limitação honesta do crypto client-side é uma concessão aceita e documentada, não um problema a resolver agora; risco herdado fora do escopo do M7 mas registrado aqui: o build Windows nunca foi validado desde o M0 — gate recomendado no M8, não neste milestone.

## Reversibilidade

Two-way door. O maestro leve isola a lógica de transição atrás de um contrato pequeno ("contra quem" / "ganhou-perdeu"); crescer para um gerenciador de cenas completo depois é aditivo, não reescrita. A API do motor de diálogo (ler nó, repassar etiqueta) separa o motor do renderizador, então trocar a pintura provisória pela definitiva não toca o motor nem os dados. O formato do artefato cifrado pode ganhar versão nova sem quebrar a fonte editável, seguindo o mesmo padrão do `.gmap`. Sem releases públicas (jogo em DEV) — nenhuma decisão aqui é irreversível para o jogador final.

## Execução (faseada, prevista — NÃO iniciada)

- **Pré-req:** aprovação final do criador sobre este registro.
- **Onda 1 — `M7-COSTURA`:** maestro leve + estado sobrevivente à troca de tela (posição do Gus, música, save) + swap cidade↔batalha (contrato "contra quem" → "ganhou/perdeu") + 1 inimigo fixo → colisão dispara batalha → vitória volta ao ponto de origem na cidade → inimigo derrotado some do mapa + posse da música sobe da tela de batalha para o maestro + fade preto curto com crossfade de música. Esta onda **fecha o M6** (crossfade). Agente previsto: `backend-engineer` / `engine-graphics-programmer`.
- **Onda 2 — `M2-SAVE-IO`:** I/O real em disco (`~/.gusworld/saves`, permissões `0700`/`0600`); `controls.json` e save passam a persistir via `load_save`; janela de aviso ao usuário quando houver diff de versão de save. Agente previsto: `backend-engineer`.
- **Onda 3 — `M7-DIALOGO`** (sub-peças em sequência):
  - `M7-DIALOGO-FORMATO` — formato de dados do diálogo (nós, falas, escolha, flag), chaves i18n, etiqueta de registro (terminal frio × caixa quente).
  - `M7-DIALOGO-COMPILADOR` — pipeline fonte→cifra+sela→artefato, reusando a crypto do ADR-006.
  - `M7-DIALOGO-RUNTIME` — carregador POCO C++20 (TDD): valida o selo, decifra, roda a árvore de diálogo, seta a flag persistida via `M2-SAVE-IO`. **Bloqueado por `M2-SAVE-IO`** (dependência dura).
  - `M7-DIALOGO-NPC-MVP` — 1 NPC, trigger de conversa, falas via chaves i18n, a escolha que seta a flag e a flag sobrevive ao save/load.
  - Agente previsto: `backend-engineer` / `gameplay_engineer` (formato/runtime/NPC-MVP), `security-engineer` ou `backend-engineer` em revisão de crypto no compilador.
- **Fecho do milestone:** `M7` fecha quando o playthrough de ~5min (andar, NPC, combate, save, carregar) roda ponta-a-ponta 100% na engine nova, sem Godot.

### Riscos e dependências registrados

1. O M6 permanece 🔄 até o crossfade da Onda 1 ser entregue.
2. `M7-DIALOGO-RUNTIME` depende de `M2-SAVE-IO` — a flag da escolha só prova sobreviver ao save se o I/O em disco já existir; dependência dura, não apenas prioridade WSJF.
3. O compilador de conteúdo (cifra+sela) é infraestrutura nova que muda o carregamento de todo texto user-facing do jogo — maior superfície de mudança do M7; mitigado com TDD e usando o oráculo de roundtrip do save (ADR-007) como referência.
4. Limitação honesta do crypto client-side (item 8 da Decisão): anti-tamper prático, não cofre inviolável — a chave mora no binário porque o jogo precisa exibir as falas.
5. Risco herdado, fora do escopo do M7, só registrado aqui: build Windows nunca validado desde o M0; gate recomendado no M8.

Nenhuma fase acima foi executada até a data deste registro.
