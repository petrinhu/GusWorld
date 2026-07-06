# ORIENTACOES — Auditoria Completa 2026-07-06 (Caetano/CTO)

Dossiê de orientações técnicas da AUDITORIA-COMPLETA-2026-07-06 (engenharia C++20 nova + lore/narrativa, escopo do líder). Cada seção corresponde a uma linha `AC-*` na tabela do INBOX do `TODO.md`. Legado C#/Godot EXCLUÍDO por decisão do líder. O WIP interrompido do `ASSETS-VFS-F1` (asset_source + includes em battle_preview/system_menu_loop) NÃO é achado (trabalho em progresso, ignorado por instrução).

**Classificação:**
- **(A) FIX DIRETO** — sonnet-5 executa com as instruções da seção, sem decidir arquitetura/design. Onde marcado, decisão binária do líder via AskUserQuestion ANTES de executar.
- **(B) PRECISA PLANEJAR** — só descrição do problema e decisões em aberto.

**Evidência primária (estado no momento da auditoria, HEAD pós `fff81a9`):** `bash tools/check.sh` rc=0 — BUILD=0, SMOKE=0 (headless SDL dummy, cidade real .gmap carregada), GATE=0 (arch: zero Qt/SDL/RmlUi/glintfx em core/+domain/; i18n: 142 chaves, 0 falta / 0 extra), SUITE=0 (**1439/1439 testes verdes**, 5.91s).

**Confirmação de status de dívidas JÁ rastreadas (não são linhas novas):**
- `AUD-MINIAUDIO-UAF` (heap-use-after-free na lib vendorizada, trilha de falha de carga): segue ABERTO, rastreado no INBOX do TODO.md. Obrigatório antes do gate T4/ASan de v1.0.0.
- Build **Windows nunca validado**: segue pendente, gate no M8 (PI11). Ver porém AC-E10 (contradição documental de escopo).
- Fadiga de prosa era-2 (AUDIT-T4, 18 críticos de estilo): segue BLOQUEADO/pendente, rastreado em `F5-BK.AUDIT` (W7+).
- Tradução de conteúdo en_intl (7/142 traduzidas): débito deliberado (CUT.9, pt-br only em v1). Estrutura em paridade (0 falta/0 extra).
- `TODO-PARSER-BUG`: já registrado no INBOX; as linhas novas desta auditoria foram conferidas com exatamente 9 células.

---

## AC-E1 — Labels do cockpit hardcoded fora do Translator ("AP", "MANA", "ACAO")

**Achado.** `GusEngine/app/src/screens/battle_scene.cpp:1545` (`renderer.draw_text("AP", ...)`), `:1552` (`"MANA"`), `:1566` (`"ACAO"`): três strings user-facing desenhadas direto, sem passar por `tr()`. Viola o canon i18n do projeto (zero hardcoded strings user-facing, memo `project_i18n_canonico`). Agravante: a chave `HUD_AP_LABEL` JÁ EXISTE em `game/translations/pt_br.md:167` e não é consumida; "ACAO" ainda aparece sem cedilha/til na tela (a fonte stb_truetype já suporta acentos UTF-8 desde o M5 — poderia ser "AÇÃO"). O resto da cena usa `translator_->tr(...)` corretamente (ex.: linhas 418, 1648, 2070-2084).

**Classificação: (A) FIX DIRETO.**
1. Em `game/translations/pt_br.md` §5, adicionar `## HUD_MANA_LABEL` = `MANA` e `## HUD_ACTION_LABEL` = `AÇÃO` (a `HUD_AP_LABEL` já existe). Replicar as 2 chaves em `game/translations/en_intl.md` (`MANA`, `ACTION`) — o GATE de paridade reprova se faltar.
2. Em `battle_scene.cpp`, trocar os 3 literais por `translator_->tr("HUD_AP_LABEL")`, `tr("HUD_MANA_LABEL")`, `tr("HUD_ACTION_LABEL")` seguindo o mesmo padrão de guarda `translator_ != nullptr` já usado no arquivo (com fallback pro literal atual se o translator estiver ausente, padrão das outras chamadas).
3. Rodar `bash tools/check.sh` (o GATE i18n valida a paridade). Ajustar/estender teste existente de HUD se algum assert amarrar o literal.

---

## AC-E2 — CI não roda SMOKE, GATE i18n nem sanitizers (drift entre check.sh e ci.yml)

**Achado.** `tools/check.sh` roda 4 estágios (BUILD, SMOKE headless, GATE arch **+ GATE i18n**, SUITE). O CI (`.forgejo/workflows/ci.yml`) roda só: gate arch (sincronizado, ok) + Configure + Build + Test. Faltam no CI: (a) o smoke headless (`--smoke` com `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy`, que o container já suporta); (b) o gate de paridade i18n (`tools/i18n_parity.py`); (c) nenhum job ASan/UBSan em lugar nenhum do CI — recomendação explícita do dossiê AUDIT-M7-COSTURA §8 para o gate T4 de v1.0.0 (foi o ASan local que achou o `AUD-MINIAUDIO-UAF`).

**Classificação: (A) FIX DIRETO.**
1. Em `ci.yml`, após o step Build, adicionar step "Smoke" espelhando o check.sh: `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 60 GusEngine/build/linux-release/app/gusworld_app --smoke`.
2. Adicionar step "i18n parity gate": `python3 tools/i18n_parity.py` (falha != 0 já reprova).
3. Job ASan: pode ser um segundo job (ou step) que configura um build dir `build/asan` com `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g"` (mesma parametrização que o dossiê M7 usou) e roda as suites core/domain/app. NOTA: a suite `platform` ABORTA sob ASan por causa do `AUD-MINIAUDIO-UAF` (lib vendorizada) — deixar `platform` FORA do job ASan até aquele item fechar, com comentário apontando o item.
4. Lembrete de contexto: a confirmação de run verde no Codeberg foi ADIADA pelo líder (decisão 2026-06-21, runner oscilante); validar localmente via `forgejo-runner exec` como no M0.

---

## AC-E3 — Screenshot do menu de pausa em /tmp compartilhado com nome fixo

**Achado.** `GusEngine/app/src/maestro.cpp:87-90`:
```cpp
std::string frozen_city_snapshot_path() {
    return (std::filesystem::temp_directory_path() / "gusworld_frozen_city.png").string();
}
```
O fundo real congelado do menu de pausa (`open_pause_from_city`, linha 276-287) é gravado no diretório temporário COMPARTILHADO com nome FIXO e previsível. Problemas: (a) em máquina multiusuário, outro usuário pode pré-criar o arquivo ou um symlink com esse nome (o stb_image_write segue e sobrescreve o alvo com as permissões do jogador — classe clássica de vulnerabilidade de /tmp); (b) duas instâncias do jogo colidem no mesmo arquivo; (c) o arquivo não é limpo depois do uso; (d) destoa da disciplina 0700/0600 já aplicada em `platform/src/fs/settings_file_store.cpp:90-118` e `save_file_store.cpp` (dado do jogador, LGPD leve).

**Classificação: (A) FIX DIRETO.**
1. Trocar o destino para o diretório de settings do jogador (já resolvido por `gus::platform::fs::resolve_settings_dir()`, criado com 0700), ex.: `<settings_dir>/frozen_city.png`.
2. Manter a degradação existente: se a captura falhar, string vazia = vinheta (já implementado).
3. Apagar o arquivo ao fechar o menu (ou sobrescrever sempre — dentro do 0700 o risco de symlink some; a limpeza é higiene).
4. Ajustar o probe `GusEngine/app/tools/frozen_bg_probe.cpp` se ele referenciar o caminho antigo. Rodar `check.sh`.

---

## AC-E4 — Dead code pós-ADR-010: RmlUi_Renderer_SDL.* órfão + README da pasta defasado

**Achado.** `GusEngine/platform/rmlui/` contém `RmlUi_Renderer_SDL.cpp` e `RmlUi_Renderer_SDL.h` que NÃO são referenciados por nenhum `CMakeLists.txt` (o único fonte compilado da pasta é `rmlui/gl3_loader.cpp`, ver `platform/CMakeLists.txt:29`) nem incluídos por nenhum outro fonte — sobra da era pré-glintfx (o backend vendorizado foi aposentado no ADR-010 F3). São os únicos pontos com `new`/`delete` crus fora de tests no tree (linhas 56-126), o que suja greps de qualidade. Além disso, `platform/rmlui/README.md` descreve arquivos que não existem mais na pasta (`RmlUi_Renderer_GL3.{h,cpp}`, `RmlUi_Platform_SDL.{h,cpp}`, `rmlui_hud.{hpp,cpp}`) e uma arquitetura pré-ADR-010.

**Classificação: (A) FIX DIRETO.**
1. `git rm GusEngine/platform/rmlui/RmlUi_Renderer_SDL.cpp GusEngine/platform/rmlui/RmlUi_Renderer_SDL.h` (histórico preserva; NÃO usar rm cru — hook anti-rm bloqueia).
2. Reescrever `platform/rmlui/README.md` para o estado pós-ADR-010: a pasta hoje só abriga `gl3_loader.{hpp,cpp}` (glad + leitura de backbuffer pro smoke visual) e `RmlUi_Include_GL3.h` (header do loader); a UI/HUD vive no glintfx (embed mode) via FetchContent.
3. Verificar antes com `grep -rn "RmlUi_Renderer_SDL" GusEngine/ --include="*.cpp" --include="*.hpp" --include="*.txt"` que só auto-referências existem. Rodar `check.sh`.

---

## AC-E5 — Diretórios-esqueleto vazios do M0 que não refletem o layout real

**Achado.** 15 diretórios contêm apenas `.gitkeep` e nada mais: `core/{ecs_lite,events,resource,rng,time}`, `domain/{combat,i18n,progression,save,templates}`, `platform/{audio,fs,input,render2d,window}` (todos sob `GusEngine/`). São o andaime do M0; o código real vive em `<camada>/src/` + `<camada>/include/`. Confundem navegação (ex.: `domain/combat/` vazio ao lado de `domain/src/combat/` cheio) e sugerem módulos (ecs_lite, events, resource) que nunca nasceram.

**Classificação: (A) FIX DIRETO.** Remover os 15 diretórios (`git rm` dos `.gitkeep`). Se preferir manter algum como reserva de design (ecs_lite/events/resource), substituir o `.gitkeep` por um `README.md` de 2 linhas dizendo "reservado, ainda não existe; código real em src/+include/". Encaixa na onda de higiene M9, mas é barato o suficiente pra qualquer momento. Rodar `check.sh` depois (nenhum CMake referencia essas pastas — conferido).

---

## AC-E6 — check.sh manda um log de ctest pro Lixo do usuário a cada execução

**Achado.** `tools/check.sh:124-128`: o resumo do ctest é `tee`-ado para `/tmp/.gusworld_ctest.$$` e depois descartado com `gio trash`. Como o script roda via hook PostToolUse a cada mudança de código, o Lixo do usuário acumula um arquivo por rodada, indefinidamente (centenas por sessão de trabalho). O `gio trash` é contorno do hook anti-rm, mas o efeito colateral é poluição do Trash.

**Classificação: (A) FIX DIRETO.** Trocar o arquivo temporário por um caminho FIXO e sobrescrito dentro do build dir (ex.: `$ENGINE/build/$PRESET/last_ctest.log`), que é sobrescrito a cada rodada e não precisa de remoção nenhuma. Alternativa mínima: manter o `tee` mas para o caminho fixo. Não usar rm (hook).

---

## AC-E7 — Deleções não-commitadas de resources legados quebram o README se commitadas

**Achado.** Working tree com deleções NÃO commitadas: `resources/QRCode.png`, `resources/buymecoffe.png`, `resources/images/gus_vector.png`, `resources/pers_3d/{Gus,Gus_movimento,Yakov}.glb` (LFS). Porém `README.md:178` referencia `resources/buymecoffe.png` (botão de doação PayPal) e `README.md:184` referencia `resources/QRCode.png` (QR de doação). Se as deleções forem commitadas como estão, a seção de doação do README (RF-9, freeware+doação) quebra no Codeberg (imagens 404). Os `.glb` são da era Godot (3D abandonado no pivot 2D) — deleção plausivelmente intencional; as imagens de doação, provavelmente não. Estado transitório do tree = risco de commit acidental via staging amplo.

**Classificação: (A) FIX DIRETO, com decisão do líder via AskUserQuestion ANTES.** Perguntar: (a) confirmar deleção dos `.glb` + `gus_vector.png` (commitar)? (b) `QRCode.png`/`buymecoffe.png`: restaurar (`git checkout -- resources/QRCode.png resources/buymecoffe.png`) OU deletar de vez E atualizar o README (trocar o botão-imagem por link textual de doação)? Executar a combinação escolhida num commit próprio citando RF-9. Aproveitar para conferir `git status` do submodule `engine` (apareceu "m engine" no snapshot; submodule arquivado read-only, nada deveria mexer nele).

---

## AC-E8 — Arquivos soltos não-rastreados na raiz do projeto

**Achado.** Na raiz: `Sistema_Luta_RPG_Teoria_dos_Jogos_Completo.txt` (não-rastreado; aparenta ser o insumo do item `COMBATE-TEORIA-JOGOS` de PI9) e `scratchpad_caps/` (não-rastreado, capturas soltas). A memória do projeto (`feedback_arquivos_importantes_pasta_projeto`) manda material importante ficar em pasta canônica E commitado; a raiz do repo só carrega os arquivos canônicos (CLAUDE/TODO/CHARS/PLACES/sinopse/manuais).

**Classificação: (A) FIX DIRETO, com decisão do líder via AskUserQuestion ANTES.** Perguntar: (a) o .txt é insumo do COMBATE-TEORIA-JOGOS? Se sim, mover para `docs/design/mecanicas/` (ou `resources/`) e commitar citando COMBATE-TEORIA-JOGOS; (b) `scratchpad_caps/`: descartar (efêmero) ou mover o que importa pra `docs/design/mockups/` e commitar. Sem decisão, ao menos adicionar `scratchpad_caps/` ao `.gitignore` pra não sujar o status.

---

## AC-E9 — Pin do glintfx defasado nos docs (dizem v0.2.4; real é v0.3.1)

**Achado.** O pin REAL é `GIT_TAG v0.3.1` (`GusEngine/CMakeLists.txt`, bloco FetchContent do glintfx). Docs desatualizados: `README.md:90` e `README.md:128` ("pin `v0.2.4`"), `CLAUDE.md:45` ("pin `v0.2.4`"). O `ADR-010:64` também diz v0.2.4, mas ADR é registro histórico da decisão (era o pin da época) — não corrigir o corpo.

**Classificação: (A) FIX DIRETO.**
1. `README.md:90` e `:128`: trocar `v0.2.4` por `v0.3.1`. Melhor ainda: reformular para "pin atual no `GusEngine/CMakeLists.txt`" pra não re-defasar a cada bump.
2. `CLAUDE.md:45`: idem.
3. Opcional: adendo de 1 linha no fim do ADR-010 ("pins subsequentes: v0.3.0/v0.3.1, ver CHANGELOG"), sem tocar o corpo.

---

## AC-E10 — Contradição documental de escopo Windows (CUT.11 vs pivot)

**Achado.** `TODO.md` CUT.11 (decisão da era Godot, 2026-06-02): "Windows export em v1 (Linux only; ... Windows pós-v1)" — CORTADO. Mas os docs do pivot tratam Windows como alvo de v1: `README.md:7` ("Linux + Windows"), M0 ("SAÍDA: cmake --build verde em Linux e Windows"), M1 ("60fps estável nos 2 OS"), M8 ("GATE build Windows"). O build Windows nunca foi validado (dívida conhecida, confirmada nesta auditoria). Ou o CUT.11 foi implicitamente superado pelo pivot, ou o M8 está com gate a mais.

**Classificação: (A) FIX DIRETO, com decisão do líder via AskUserQuestion ANTES.** Perguntar: Windows entra em v1.0.0 (revoga CUT.11) ou fica pós-v1 (CUT.11 vale; M8/M0/M1/README ajustam o texto do gate Windows para "preparado, sem ship")? Aplicar a resposta: ou marcar CUT.11 como superado (com nota e data), ou editar as linhas M0/M1/M8 + README pra refletir Linux-only no ship de v1.

---

## AC-E11 — battle_preview.cpp com 2943 linhas (host monolito em crescimento)

**Achado.** `GusEngine/app/src/screens/battle_preview.cpp` tem 2943 linhas e segue absorvendo integrações (cockpit glintfx, SFX de hover/clique, menu de sistema, agora o retrofit do ASSETS-VFS-F1 em andamento). `battle_scene.cpp` tem 2094. São a casca app (fora do motor POCO), com testes via funções-livres extraídas (`battle_key_down`, `battle_mouse_click` etc., padrão bom), mas o arquivo-host em si virou o maior ponto de fricção de manutenção do app/ e todo incremento novo o toca. Nenhuma auditoria anterior flagrou porque o crescimento foi gradual entre os dossiês.

**Classificação: (B) PRECISA PLANEJAR.** Este achado precisa ser planejado com opus-latest via sessão de brainstorm com o líder — NÃO decidir a abordagem sozinho. Decisões em aberto: se/quando decompor (talvez só no M9-higiene, talvez antes do M7-paridade); critério de corte (por responsabilidade: bootstrap GL/janela vs wiring de input vs self-tests env-gated vs resolução de assets — esta última já está saindo com o ASSETS-VFS-F1); como preservar os call-sites dos self-tests sintéticos que os dossiês usam como prova; e o custo/risco de mexer no host estável que o líder valida ao vivo.

---

## AC-L1 — Linhagem matrilinear Atelaiá com cronologia contraditória em era-2-boom-tecnico.md

**Achado (canon factual, não estilo).** Canon central (fonte de verdade): `timeline.md:73` Antoneta nasce **-110**; `timeline.md:79` Tarsila assume **-25**; `timeline.md:80` Felícia assume **-8**; `timeline.md:197` e `CHARS.md:221` cadeia de **6 gerações** Atelaiá → Antoneta → Verônica → Tarsila → Felícia → contemporânea. Em `docs/narrative/deep/eras/era-2-boom-tecnico.md`, três passagens divergem entre si e do canon:
- **L719**: "Antoneta (filha direta, **-90**)" (canon -110), "Tarsila (**-20**)" (canon -25), "Felícia (**+5**)" (canon -8).
- **L919**: usa -25/-8 CORRETOS, mas conta "cinco gerações" numa janela diferente (de Verônica à aprendiz).
- **L1057** (voz in-character da Verônica em -45): chama Atelaiá de "minha **tetravó**" (canon: avó — só Antoneta entre elas), se declara "**segunda geração**" omitindo Antoneta da enumeração, põe Tarsila "em treinamento no horizonte de -40" e Felícia "ainda criança em -45" (tensiona com Felícia assumindo só em -8 e com a própria L719 que diz +5).
O item 5 do piloto TEXTREVIEW em `INCOHERENCES.md` ("era-2-boom-tecnico.md (R10): drift sobrenomes Tarsila/Felícia alinhar") ficou ⏳ desde 2026-05-22 e nunca foi executado; o drift real vai além de sobrenome (datas + contagem de gerações + parentesco). O tracker INCOHERENCES declara "0 abertas" para incoerências factuais, o que este achado desatualiza.

**Classificação: (B) PRECISA PLANEJAR.** Este achado precisa ser planejado com opus-latest via sessão de brainstorm com o líder — NÃO decidir a abordagem sozinho. Decisões em aberto: quais datas ficam canônicas em cada passagem (a L1057 é voz in-character e PODE ser unreliable narrator deliberado, como o canon já fez com DD-003 — ou é drift a corrigir); como reconciliar "tetravó"/"segunda geração" com a cadeia de 6; se a correção entra na refatoração grande da era-2 (F5-BK.AUDIT, fadiga de prosa T4) ou como fix cirúrgico antecipado; e o registro do reaberto em INCOHERENCES.md (o doc diz "não modificar sem reaudit" — esta auditoria é o reaudit que o justifica). Execução de prose sempre via narrative-writer (memória `feedback_deep_lore_sempre_narrative_writer`).

---

## AC-L2 — "Bento Chevalier-Berenger" em tech-3-eras-deep.md (linhagem Berenger é do Cauã)

**Achado (canon factual).** `docs/narrative/deep/ontologia/tech-3-eras-deep.md:28` e `:50` dizem que Iremar Berenger é "ancestral direto ... do **Bento Chevalier-Berenger** Era 3". Canon central: a linhagem Berenger é do **Cauã** — `timeline.md:74` ("Iremar Berenger, ancestral de [[caua-volt]]"), `timeline.md:199` (cadeia Berenger: Iremar → ... → Salvador → Davi e Cauã), `deep/settings/04-dutos.md:33` ("Iremar Berenger, ancestral direto de Cauã"). O nome canônico do companion é **Bento "Requiem" Chevalier** (`CHARS.md:30`), pais Aldebrando Chevalier + Atelaiana de Sevra Chevalier — zero sangue Berenger nos docs do Bento (grep "Berenger" nos characters/bento-* = vazio). O sobrenome composto "Chevalier-Berenger" só existe nessas 2 linhas — viola a memória `feedback_nomes_personagens_canonicos` (nunca trocar/compor nomes canônicos). É quase certamente troca de personagem: onde se lê Bento, o descendente Era 3 do Iremar deveria ser o Cauã (a co-fundação da Garça-Preta-Nova pelo Iremar-Velho e as contagens de geração devem ser re-conferidas na mesma passada).

**Classificação: (A) FIX DIRETO, com aprovação do líder via AskUserQuestion ANTES (é mudança de canon).**
1. Apresentar ao líder a evidência acima e a proposta: nas 2 ocorrências, "Bento Chevalier-Berenger Era 3" → "Cauã Berenger Era 3" (linha 28: "vigésima primeira geração do Cauã Berenger Era 3"; linha 50: "Iremar é ancestral direto em quinta geração de Cauã Berenger" — conferir a contagem de gerações contra `timeline.md:199`, que dá Cauã como 5ª geração a partir do Iremar, consistente com a linha 50).
2. Aprovado, despachar o `narrative-writer` para o fix cirúrgico (regra do projeto: prose deep-lore nunca inline; exceção de tabela/meta não se aplica aqui pois é prose corrida).
3. Atualizar o cross-ref na entrada do Cauã/Bento em `CHARS.md` se necessário (a linha 30 não referencia tech-3-eras; provavelmente nada a fazer).

---

## AC-L3 — Ficha do Brunus com marcador de status ambíguo

**Achado.** `docs/narrative/characters/brunus-vetorial.md:3` e `:194`: "**Status:** Canônico (rascunho para revisão do criador supremo)" — autocontraditório (ou é canônico, ou é rascunho aguardando revisão). O TODO.md marca `PERSONAGEM-BRUNUS-VETORIAL` como ✅ e o CHARS.md:85 registra o Brunus como "✅ canônico", com o arco do clímax "canonizado 2026-07-03". Tudo indica que a revisão do criador já aconteceu e o header ficou pra trás.

**Classificação: (A) FIX DIRETO.** Confirmar com o líder (1 pergunta AskUserQuestion, pode ir junto de outras): a ficha + conto estão revisados e canônicos? Se sim, trocar as 2 ocorrências por "**Status:** Canônico (revisado pelo criador supremo, 2026-07-03)". Se não, trocar por "Aguarda revisão do criador supremo" e ajustar CHARS.md:85 pra refletir.

---

## AC-L4 — Cross-ref de pillar impreciso na ficha do Brunus

**Achado.** `docs/narrative/characters/brunus-vetorial.md:164`: "(Pillar 4: toda escuridão serve a propósito)". A frase "Toda escuridão serve a propósito narrativo claro" vive na INTRODUÇÃO de `docs/design/pillars.md` (linha 17, seção de visão/anti-grimdark), não no corpo do Pillar 4 (linha 113, "Prodígio de 11 anos"). A outra citação da ficha (linha 9: "adultos são antagonistas ou ausentes, exceto mentores derrotados" — essa SIM é mecânica do Pillar 4) está correta.

**Classificação: (A) FIX DIRETO.** Na linha 164, trocar "(Pillar 4: toda escuridão serve a propósito)" por "(pillars.md, princípio anti-dark-gratuito da visão: toda escuridão serve a propósito; e Pillar 4)". Edição de cross-ref/meta, não de prose narrativa (exceção permitida da regra narrative-writer).

---

## AC-L5 — TEXTREVIEW incremental não rodado nos docs canonizados pós-auditorias T1-T10

**Achado (processo).** O workflow canônico (memória `feedback_audit_incremental_textreview`) exige piloto TEXTREVIEW Lote 1 imediato após cada doc deep-lore canonizado com ≥1k palavras de delta. Canonizados DEPOIS dos audits T1-T10 (2026-05-22/30) sem marcador de TEXTREVIEW encontrado: `brunus-vetorial.md` (~3.9k palavras, 2026-06-24 + arco do clímax 2026-07-03), `brunus-vetorial-conto.md` (~1.1k), e o pacote conlang `docs/narrative/lingua/` (4 docs, ~2.4k somadas — individualmente <1k, como lote passam do gatilho). O ganho é o de sempre: cross-ref canon (CHARS/PLACES/timeline) + gramática + vícios, antes que drift cascateie (o AC-L2 é exatamente o tipo de erro que esse passo pega).

**Classificação: (A) FIX DIRETO (dispatch, não edição).** Despachar o agente `revisor-textual` (protocolo TEXTREVIEW.md v2) em 2 lotes: (1) brunus-vetorial.md + conto; (2) lingua/00-03 como lote único. Saída padrão 5-pack; achados voltam pro líder decidir. Nenhuma edição direta por quem executar este item.

---

## AC-L6 — INBOX do TODO.md chama Bertoldo de "androide" (Bertoldo é humano canon)

**Achado.** `TODO.md`, bullet `NPCS-INIMIGOS-ROTINA-HORARIO` (INBOX): "(Bertoldo, o androide fixo do M7-COSTURA)". Canon: Seu Bertoldo Caím é HUMANO — `CHARS.md:118` (62 anos, técnico aposentado, lê jornal na Praça da Compilação) e o próprio `game/dialogues/npc_intro_bertoldo.dlg.txt` ("Bertoldo e humano, 'o coracao fala'", registro warm/caixa quente que existe POR ele ser humano). O androide fixo do M7-COSTURA é o INIMIGO de encontro (entidade separada). O bullet fundiu os dois.

**Classificação: (A) FIX DIRETO.** No bullet, trocar "(Bertoldo, o androide fixo do M7-COSTURA)" por "(o NPC humano Bertoldo e o inimigo-androide fixo do M7-COSTURA)". Só texto do TODO.md, sem impacto de canon.

---

*Auditoria executada solo por Caetano (CTO), 2026-07-06, por instrução do líder. Evidências reproduzíveis: `tools/check.sh` (rc=0, 1439 testes), greps e arquivo:linha citados em cada seção. Dossiês anteriores consultados para não duplicar: AUDIT-M3, AUDIT-M5-MOTOR, AUDIT-M7-COSTURA, AUDIT-PROJETO-2026-05-29, AUDIT-T1..T10-V2, INCOHERENCES.md.*
