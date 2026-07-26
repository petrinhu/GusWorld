# TST-8 — Varredura de Segredos (histórico completo + árvore de trabalho)

**Item:** TST-8 (TODO.md) — prescrito em `TESTES.md` §T8 "Verificação de Secrets"
**Data:** 2026-07-25, 23:30–23:38 (UTC-3)
**Executor:** `security-engineer`
**Repositório:** `petrinhu/GusWorld` (**PÚBLICO**, GitHub como host único desde 2026-07-25)
**HEAD no momento da varredura:** `14aaa155edaccbf9585ff621265004bdceb19df7` (2026-07-25 22:29:50 -0300)

## Veredito

**PASSOU.**

Zero segredos reais versionados, no histórico completo e na árvore de trabalho. Zero token de forja (Codeberg/Forgejo) em qualquer ponto da história. Zero PII de menor versionada — o codinome "Gus Dragon" é usado de forma consistente nas 78 ocorrências do histórico, sem nenhuma aparição de nome real associada. **Nenhuma ação de revogação é necessária.**

Os 10 achados brutos do gitleaks são todos falsos positivos de heurística (identificadores de código contendo `token`/`key`), verificados um a um contra a linha-fonte original. Há 1 achado de **processo** (o gate do §T8 não consegue retornar exit 0 hoje) e 3 observações cosméticas, detalhados abaixo.

---

## 1. Ferramentas e comandos exatos

**Ferramenta:** `gitleaks` **8.30.0** (`/usr/bin/gitleaks`), regras **default** — o repo não tem `.gitleaks.toml` nem `.gitleaksignore`, logo **nenhuma supressão pré-existente** mascarou resultado.

### 1.1 Histórico completo (todos os refs)

```bash
gitleaks git --log-opts="--all" --no-banner --redact=80 \
  --report-format json --report-path /var/tmp/tst8-scan/history.json .
# exit code: 1  (gitleaks usa 1 = "achados encontrados"; ver §4 sobre os achados)
# saída: 978 commits scanned. scanned ~48469548 bytes (48.47 MB) in 7.14s
#        WRN leaks found: 10
```

### 1.2 Árvore de trabalho

```bash
gitleaks dir -c /var/tmp/tst8-scan/gitleaks-dir.toml --no-banner --redact=80 \
  --report-format json --report-path /var/tmp/tst8-scan/worktree.json .
# exit code: 1
# saída: scanned ~1404325300 bytes (1.40 GB) in 47s
#        WRN leaks found: 2
```

O config usado apenas **estende as regras default** e exclui `GusEngine/build/` (3,5 GB de objetos e dependências baixadas via FetchContent, não versionados):

```toml
[extend]
useDefault = true

[allowlist]
description = "artefatos de build locais, nao versionados"
paths = ['''GusEngine/build/.*''']
```

Nenhuma regra foi desabilitada e nenhum caminho com conteúdo versionado foi excluído.

### 1.3 Varreduras dirigidas complementares (grep sobre o stream de diff completo)

Segunda passada independente do gitleaks, cobrindo o que a heurística genérica costuma perder — arquivos de credencial, chaves privadas, PATs de formato específico e PII:

```bash
# arquivos de credencial adicionados em qualquer ponto da historia
git log --all --diff-filter=A --name-only --format='' | sort -u | grep -iE '(\.env|\.netrc|id_rsa|id_ed25519|\.pem|\.p12|credentials|\.git-credentials)$'

# chaves privadas PEM/OpenSSH
git log --all -p --format='' | grep -aiE 'BEGIN (RSA|DSA|EC|OPENSSH|PGP|PRIVATE)'

# prefixos de token de provedor
git log --all -p --format='' | grep -aoE '(ghp|gho|ghu|ghs|ghr)_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{30,}|xox[baprs]-[A-Za-z0-9-]{10,}|AKIA[0-9A-Z]{16}|sk-[A-Za-z0-9]{32,}|glpat-[A-Za-z0-9_-]{15,}'

# PAT no formato Gitea/Forgejo (40 hex) em linha com contexto de token/forja
git log --all -p --format='' | grep -aiE 'token|codeberg|forgejo' | grep -aoE '\b[0-9a-f]{40}\b'

# emails em todo o conteudo historico
git log --all -p --format='' | grep -aoE '[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}' | sort | uniq -c
```

**Todas as quatro primeiras retornaram vazio.** A de e-mails é analisada no §5.

---

## 2. Cobertura

| Dimensão | Número |
|---|---|
| Commits no repo (todos os refs) | **991** |
| Commits não-merge (o que `git log -p` diffa) | 986 |
| Merges (não introduzem conteúdo próprio; verificados) | 5 |
| Commits contabilizados pelo gitleaks | **978** |
| Refs varridos (branches locais + remotos) | **20** |
| Arquivos na árvore varrida (exclui `GusEngine/build/`) | **4.000** |
| Arquivos rastreados pelo git | 1.274 |
| Untracked não-ignorados (trabalho em curso de outra frente) | 7 |
| Bytes varridos — gitleaks histórico | 48,47 MB |
| Bytes varridos — gitleaks árvore | 1,40 GB |
| Bytes varridos — passada dirigida de grep (§1.3) | **89,9 MB** |

**Nota de honestidade sobre a contagem:** o gitleaks reporta 978 commits contra os 986 não-merge do repo — uma diferença de 8 que a contagem de merges sozinha não explica (provavelmente commits de diff vazio ou dedupe interno do gitleaks; não investiguei a fundo). A lacuna está **coberta pela passada dirigida do §1.3**, que percorre o stream de `git log --all -p` inteiro — 89,9 MB, quase o dobro dos 48,47 MB que o gitleaks processou (ele pula binário e arquivo grande) — e não achou nada além do já reportado. Os 5 merges foram inspecionados individualmente: são merges de branches de trabalho (`VOZES-PARTY`, `MENU-INICIAL`, `FLASH-CTX`) cujo conteúdo já vem dos pais, todos varridos.

**Fora de cobertura:** `GusEngine/build/` (3,5 GB de artefatos locais, não versionados, não publicáveis).

---

## 3. Ferramentas ausentes — o que NÃO foi executado

Nenhum pacote de sistema foi instalado (regra: agente não instala sozinho). Ficaram fora do escopo:

| Ferramenta | Status | O que deixou de ser coberto |
|---|---|---|
| **trufflehog** | não instalada | Detecção por entropia com **verificação ao vivo** da credencial (bate no provedor pra dizer se o segredo está ativo). Como não houve nenhum candidato a segredo, a perda prática aqui é baixa. |
| **detect-secrets** | não instalada | Segunda opinião de heurística + baseline versionável. Redundante com o gitleaks neste caso. |
| **semgrep** | não instalada | Fora do escopo do TST-8 (é SAST, não secret-scan). |

Isto é um resultado negativo honesto, não uma lacuna improvisada: nenhuma das três foi substituída por gambiarra.

---

## 4. Achados

### Resumo por severidade

| Severidade | Quantidade |
|---|---|
| **CRÍTICO** | **0** |
| **IMPORTANTE** | **1** (processo, não vazamento) |
| **COSMÉTICO** | **3** |
| **FALSO POSITIVO** | **10** (achados brutos do gitleaks) + 12 e-mails de upstream |

Nenhum achado exige **revogação**. Nenhum achado exige reescrita de histórico.

---

### 4.1 [IMPORTANTE — processo] O gate do §T8 não consegue retornar exit 0 hoje

**Onde:** critério "done" do `TESTES.md` §T8 — *"Scan de secrets retorna exit 0 antes de cada push"*.

**Descrição.** O gitleaks retorna exit 1 sempre que há qualquer achado, e os 10 falsos positivos abaixo são permanentes (2 deles ainda na árvore, 8 no histórico imutável). Logo, o critério do §T8, como está escrito, é **inalcançável** — e um gate que grita lobo toda vez é um gate que passa a ser ignorado (alert fatigue). Isto não é um vazamento; é a diferença entre "não há segredo" e "o gate consegue provar que não há segredo".

**Ação recomendada (decisão do líder — envolve config versionada, não executada por este agente):** criar um `.gitleaks.toml` na raiz com `[extend] useDefault = true` mais um allowlist **por regra e por caminho** para os falsos positivos verificados neste relatório (não um allowlist genérico, que cegaria o scanner). Com isso, `gitleaks dir` passa a sair 0 na árvore limpa e o gate vira acionável — inclusive plugável como job de CI no GitHub Actions (o `.github/workflows/` hoje **não tem** job de secret-scan; ver §6).

---

### 4.2 [FALSO POSITIVO] 10 achados `generic-api-key` — todos identificadores de código

Cada um foi verificado contra a linha-fonte original no commit em que aparece (`git show <sha>:<arquivo>`). Nenhum é segredo; todos são a heurística do gitleaks casando as palavras `token`/`key` seguidas de string longa.

| # | Arquivo:linha | Commit | Valor mascarado | Linha real | Por que é falso positivo |
|---|---|---|---|---|---|
| 1 | `docs/tech/adr/ADR-017-action-clock-combat-unificado.md:47` (hoje :62) | `051f9ea8` | `ApEf…` | `... RepeatLastAction, DamageQuantize/Planck, RevealIntent/Scrying, TokenRefund, ApEfficiency-face1 ...` | Lista de `EffectKind`s do motor techMagic (ADR-016). `TokenRefund` é nome de efeito de carta; "Token" aqui é o recurso de combate, não credencial. |
| 2 | `GusEngine/app/src/screens/system_menu.cpp:422` (hoje :542) | `a2d4cdc9` | `result.…` | `state.controls_last_swapped_with_label_key = result.swapped_with_label_i18n_key;` | Atribuição entre dois campos de struct. O "segredo" detectado é o próprio nome do campo (`result.swapped_with_label_i18n_key`), uma **chave i18n**, não uma API key. |
| 3–10 | `GusEngine/third_party/lexy/include/lexy/grammar.hpp:86,87,88,90,91,92,94,95` | `69c4f630` | `UIN…` | `error_token_kind = UINT_LEAST16_MAX - 1;` e irmãos | Enum de tipos de token de um **parser** (lib `lexy`, vendorizada). "token" é terminologia de análise léxica. A lib foi **purgada no M9** — `git ls-files | grep -c lexy` = **0**; sobrevive só no histórico. |

**Ação:** nenhuma. Não apagar, não reescrever histórico. Entram no allowlist proposto em §4.1.

---

### 4.3 [FALSO POSITIVO] 12 e-mails de autores upstream no histórico

`alec@swapoff.org`, `miguel@miguel-martin.com`, `oneill@pcg-random.org`, `mattreecebentley@gmail.com`, `mackron@gmail.com`, `antony@teamwoods.org`, `regnirpsj@gmail.com`, `the.asteroth@gmail.com`, `msomeone@gmail.com` e os placeholders `you@example.com` / `yourteam@example.com`.

**Origem:** todos entraram no mesmo commit `69c4f63` — *"chore(deps): vendoriza 32 libs C++ em GusEngine/third_party (zero-dep)"* (2026-06-22). São cabeçalhos de LICENSE e metadados de autoria que os próprios autores publicam em seus projetos OSS (PCG random, miniaudio, plf::colony, AudioFile, etc.).

**Estado hoje:** **nenhum deles é rastreado** — todo o `third_party/` vendorizado foi purgado no M9. Permanecem apenas no histórico.

**Ação:** nenhuma. Republicar o e-mail que o autor pôs na própria licença é o comportamento normal e esperado de código vendorizado; remover seria, inclusive, mutilar atribuição de licença.

---

### 4.4 [COSMÉTICO] E-mail de terceiro rastreado em `ATTRIBUTION.md`

**Onde:** `docs/design/roster-analogos/8values-engine/ATTRIBUTION.md:89`
**Valor:** `eightv****@gmail.com` (mascarado)

O próprio documento registra a procedência: *"email listado em `results.html` do repo original para calibração/feedback do quiz"* — ou seja, endereço de contato **publicado pelo projeto original** para esse fim exato, reproduzido num arquivo de atribuição.

**Ação recomendada:** nenhuma obrigatória. Se o líder quiser reduzir superfície de scraping, trocar pelo link do repositório upstream em vez do endereço literal — mudança de higiene, não de segurança.

---

### 4.5 [COSMÉTICO] Caminhos absolutos pessoais em 4 arquivos rastreados

**Onde:** `PLACES.md`, `docs/auditoria/AUDIT-LORE-2026-07-08/BRIEF-A-revisor.md`, `docs/auditoria/AUDIT-LORE-2026-07-08/BRIEF-B-cosimo.md`, `docs/tech/build.md` (este último já marcado SUPERADO).

Contêm `/home/petrus/...`. Expõem o nome de usuário do SO e o layout de diretórios da máquina do líder. O username já é efetivamente público (o handle do projeto é `petrinhu`), e nenhum caminho aponta para conteúdo sensível — daí a classificação cosmética. O valor pra um atacante é próximo de zero; o incômodo é de higiene de doc público.

**Ação recomendada:** oportunística — trocar por caminho relativo à raiz do repo quando esses docs forem tocados por outro motivo. Não justifica commit próprio.

---

### 4.6 [COSMÉTICO] Referência a remoto morto do Codeberg

**Onde:** `TODO_ARCHIVE.md:138` — `ssh://git@codeberg.org/petrinhu/gusworld.git`

URL SSH de repositório, **não é credencial** (`git@` é o usuário SSH padrão de qualquer forja). Registro histórico num arquivo de arquivo morto, e o projeto saiu do Codeberg em 2026-07-25.

**Ação:** nenhuma. É registro histórico legítimo dentro do `TODO_ARCHIVE.md`.

---

## 5. Escopo 2 — Tokens de forja morta (Codeberg/Forgejo)

Frente investigada com prioridade máxima, porque apagar token localmente **não revoga**: um achado aqui viraria ação urgente de revogação pelo líder no Codeberg.

| Verificação | Resultado |
|---|---|
| PAT no formato Gitea/Forgejo (40 hex) em linha com contexto `token`/`codeberg`/`forgejo`, todo o histórico | **nenhum** |
| Atribuição `token=`/`secret=`/`password=`/`api_key=` com valor literal ≥16 chars, todo o histórico | **nenhum** — os únicos casamentos são identificadores de código (`auto token = preprocessPattern()`, `token = stbi__hdr_gettoken(...)`, `server_auth_token = _generate_auth_token()` do GDScript legado: **chamadas de função**, não literais) |
| Arquivos `.env`/`.netrc`/`.git-credentials`/`credentials*` adicionados em qualquer commit | **nenhum** |
| Chaves privadas PEM/OpenSSH/PGP | **nenhuma** |
| Prefixos `ghp_`/`github_pat_`/`glpat-`/`xox*`/`AKIA`/`sk-` | **nenhum** |
| `secrets.*` consumido em `.github/workflows/` | **nenhum** (os workflows não consomem segredo algum) |
| Arquivo de credencial presente no disco (mesmo gitignorado, risco de commit futuro) | **nenhum** |

**Conclusão: NENHUMA revogação necessária.** O `.forgejo/` foi removido na migração e o diretório `.codeberg/` remanescente na raiz está **vazio** (resíduo de diretório, sem arquivo).

---

## 6. Escopo 3 — PII

### 6.1 PII de menor — a verificação crítica

**Resultado: LIMPO. Nenhum nome real de menor foi encontrado em nenhum ponto do repositório — nem na árvore, nem no histórico.**

Como foi verificado (não dá pra buscar um nome que não se conhece, então a verificação foi por **padrão de coocorrência**, não por string):

1. **Todo padrão `<Palavra capitalizada> Dragon` em todo o histórico** — o codinome sempre revelaria um nome real colado a ele, se existisse. Resultado: `Gus Dragon` (78×), `Cutscene Dragon` (8×), `Canon Dragon` (2×), `Arco Dragon` (2×), `Vance Dragon` (1×). **Nenhum nome real.**
2. **Todas as linhas com `meu filho`/`my son`** em todo o histórico: são (a) as linhas de agradecimento do README/AI-DISCLOSURE, todas usando **"Gus Dragon"**, e (b) prosa ficcional in-world (o Pyotor Vance narrando sobre o **Gustaf** — personagem, não pessoa).
3. **Nomes próprios extraídos das 513 linhas de contexto de playtest** e cruzados contra o roster canônico (`CHARS.md`): `Bertoldo` (NPC ambiente canônico), `Bruno`/`Vettore` (personagem canônico + amigo creditado), `Dee`/`Newton`/`Tesla`/`Faraday`/`Mises`/`Hayek` (mestres históricos das cartas). **Nenhum nome fora do canon.**
4. **Imagem rastreada em `resources/sprites/personagens_inspirados/gus/gus_vector.png`** — inspecionada visualmente: render 3D chibi estilizado do protagonista fictício (cabelo laranja, óculos táticos, sobretudo preto), fundo branco. **Não é foto real.** É o único arquivo rastreado dessa pasta; os demais (inclusive de `pyotor_vance`, `yakov`, `brunus_vetorial`) estão fora do índice.

### 6.2 Mídia — risco de captura de tela pessoal

Frente checada por causa do precedente do ecossistema (15 frames com tela pessoal vazaram para o repo do site em 2026-07-18).

97 imagens rastreadas. As 13 de `docs/design/mockups/menu_capturas/` são **1280×720 e 960×540** — resolução da janela do jogo, não de área de trabalho — consistentes com captura headless do próprio jogo. As demais são sprites, ícones, frames de VFX e mockups de carta. **Nenhum arquivo com nome ou dimensão sugerindo foto de monitor ou captura de desktop.** LFS carrega apenas 2 objetos (`resources/glb/Gus.glb`, `Gus_movimento.glb`), arte conceitual.

### 6.3 QR code de doação — verificado, limpo

`resources/QRCode.png` (128×128) mereceu verificação dedicada: um QR de PIX embutiria **nome legal completo, cidade e chave** do recebedor (potencialmente CPF/telefone) — PII real num repo público.

Decodificado com OpenCV 4.13 (o detector falhava por **ausência de quiet zone**; resolvido adicionando borda de 24 px e escalando 2×):

```
https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL&source=qr
```

É exatamente a URL de doação PayPal **já publicada deliberadamente** em `README.md:116/122/269/275` e `.github/FUNDING.yml`. Sem PIX, sem nome legal, sem CPF. **Nenhum dado novo é exposto pelo QR.**

### 6.4 Metadados de commit

Autor e committer de **todos** os 991 commits: um único e-mail, o do próprio líder (`petrinhu@yahoo.com.br`). Nenhum e-mail de terceiro em metadado de commit.

### 6.5 Observação para ciência do líder (não é defeito)

`AI-DISCLOSURE.md` e `README.md` divulgam, por decisão autoral explícita (agradecimentos restaurados verbatim em 2026-07-14), que o filho do autor tem **11 anos** e programa. O **codinome é respeitado em 100% das ocorrências** — nenhum nome real aparece, e nesse ponto a regra está cumprida sem exceção. Registro aqui apenas o efeito de **agregação**: idade + parentesco + a identidade pública do autor num repo público. É decisão do líder, tomada com conhecimento de causa; não recomendo mudança, apenas deixo consignado que a auditoria viu e não tratou como achado.

---

## 7. Recomendações (defense in depth)

Nenhuma é bloqueante. Em ordem de valor:

1. **Job de secret-scan no CI** — `.github/workflows/` hoje cobre build, testes, ASan e gates de arquitetura/i18n, mas **não tem secret-scan**. Um job `gitleaks dir` no PR fecha a lacuna do §T8 ("CI roda secret scan job (futuro F2-CI.X)", ainda pendente). Depende do item 2 para não nascer vermelho.
2. **`.gitleaks.toml` versionado** com allowlist por regra e caminho dos falsos positivos deste relatório (§4.1) — torna o gate acionável e o exit 0 significativo.
3. **Hook `pre-commit` local** rodando `gitleaks dir --no-banner` só nos arquivos staged — pega o segredo antes do commit, alinhado à preferência por execução local-first. Barato, e o repo já usa `core.hooksPath`.
4. **Atualizar `TESTES.md` §T8**: o comando prescrito é `gitleaks detect --no-banner`, sintaxe **da série 8.18 e anteriores**; no 8.30 os subcomandos são `gitleaks git` e `gitleaks dir`. Vale corrigir para o comando não falhar em quem seguir o doc ao pé da letra.

---

## 8. Evidência do resultado

| Varredura | Comando | Exit code | Achados brutos | Achados reais |
|---|---|---|---|---|
| Histórico (991 commits / 20 refs) | `gitleaks git --log-opts="--all" ...` | 1 | 10 | **0** |
| Árvore de trabalho (4.000 arquivos / 1,40 GB) | `gitleaks dir -c <cfg> ...` | 1 | 2 | **0** |
| Dirigida: credenciais/chaves/PATs (§1.3) | `git log --all -p` + grep | — | 0 | **0** |
| Dirigida: PII de menor (§6.1) | coocorrência com codinome | — | 0 | **0** |
| Delta concorrente (24 arquivos, §8.1) | `gitleaks dir <delta>` | **0** | 0 | **0** |

### 8.1 Passada incremental sobre o delta concorrente

Outra frente de trabalho editou o repositório **durante** esta auditoria: entre o início (23:30) e o fim (23:41), a árvore passou de 4 arquivos modificados + 2 untracked para 17 modificados + 2 untracked (`.github/workflows/windows.yml`, `CLAUDE.md`, `CONTRACT.md`, `README.md`, `ROADMAP.md`, `TESTES.md`, presets e ADRs). A varredura do §1.2 fotografou a árvore às ~23:32, portanto **não** cobria essas edições.

Para o veredito cobrir o estado atual, os 24 arquivos com modificação não commitada (17 modificados + os untracked de `GusEngine/app/tools/`) foram copiados para um diretório de scratch e varridos à parte:

```bash
gitleaks dir --no-banner --redact=80 --report-format json \
  --report-path /var/tmp/tst8-scan/delta.json /var/tmp/tst8-scan/delta
# scanned ~255116 bytes (255.12 KB) in 34ms
# INF no leaks found
# exit code: 0
```

**Exit 0, zero achados** — e este é o exit 0 literal que o critério do §T8 pede, obtido sem allowlist nenhum, porque nesse subconjunto não há sequer falso positivo. O trabalho em curso das outras frentes está limpo.

**Sobre os exit codes 1:** no gitleaks, `1` significa "achados encontrados", não "erro de execução" — ambas as varreduras completaram com sucesso (o próprio log confirma: `978 commits scanned`, `scanned ~1.40 GB in 47s`). Os 12 achados brutos foram individualmente reconciliados contra a linha-fonte e classificados como falsos positivos no §4.2. **Enquanto o `.gitleaks.toml` do §4.1 não existir, o exit 1 é o resultado esperado de uma árvore limpa** — e é exatamente essa ambiguidade que a recomendação 2 elimina.

## 9. Disciplina de execução

Varredura **read-only** sobre o git: nenhum commit, push, `reset`, `checkout -f`, `clean`, `stash` ou `filter-repo` foi executado. A árvore tinha trabalho **não commitado** de outra frente (4 arquivos modificados + 2 untracked em `GusEngine/app/`) no início e no fim da varredura, intacto. Todo scratch em `/var/tmp/tst8-scan/` (o `/tmp` desta máquina é tmpfs/RAM). Nenhum pacote de sistema instalado. Nenhum segredo aparece em claro neste relatório.

---

## 10. Adendo de remediação — 2026-07-25

> **Natureza deste adendo.** Os §§1–9 acima são registro **datado** do que a auditoria encontrou em 2026-07-25 e não foram editados: quem quiser saber o que o repositório era naquele momento lê o corpo, não isto. Esta seção registra o que foi **feito depois**, no mesmo dia (remediação executada entre 23:41 e 23:56 de 2026-07-25; adendo redigido às 00:0x de 2026-07-26, logo após).
>
> Decisão do líder em 2026-07-25: implementar **apenas** a recomendação nº 2 do §7. O job de secret-scan no CI (recomendação nº 1) ficou **adiado por decisão dele**, não por esquecimento.

### 10.1 O que foi implementado

**Recomendação nº 2 (§4.1 / §7) — `.gitleaks.toml` versionado: FEITA.**
Commit **`8df65e6`** — *"chore(qa): .gitleaks.toml torna o gate do TST-8 acionavel"* (`.gitleaks.toml` 95 linhas, `TESTES.md` §T8, `TODO.md`).

**Desenho do allowlist.** Nenhuma entrada isenta um caminho inteiro nem uma regra inteira. Cada uma casa **três eixos ao mesmo tempo** (`condition = "AND"`):

| Eixo | Campo | Papel |
|---|---|---|
| Qual regra dispara | `targetRules` | só `generic-api-key`; as demais regras seguem ativas |
| Onde o FP vive | `paths` | arquivo específico, nunca diretório |
| Que texto confunde | `regexes` | o identificador exato, nunca curinga |

São **3 entradas** (lista de EffectKinds do ADR-017; chave i18n do `system_menu.cpp`; enum do `lexy` citado como evidência **neste relatório**) mais **1** allowlist de caminho puro, `GusEngine/build/`, que cobre artefato local não versionado — o único caso em que caminho puro se justifica, porque por definição não entra em commit.

Consequência verificada: nesses mesmos arquivos isentos, segredo de verdade **continua sendo detectado**, seja por outra regra, seja pela mesma `generic-api-key` com qualquer conteúdo diferente do listado (§10.3).

### 10.2 Decisão de escopo: árvore, não histórico

O allowlist mira **`gitleaks dir`** (o gate do dia a dia). Os **8 achados de histórico do `lexy`** (`GusEngine/third_party/lexy/include/lexy/grammar.hpp`) foram deixados **deliberadamente de fora**:

- não afetam o gate — o caminho não existe mais na árvore (purgado no M9);
- isentá-lo abriria **ponto cego numa pasta de dependência vendorizada**, que é justamente onde um segredo de terceiro entraria sem ninguém reparar;
- em troca de silenciar uma varredura que **não é gate**, e sim auditoria deliberada, feita com um humano lendo a saída.

**Portanto, por desenho: `gitleaks git --log-opts="--all"` continua saindo 1, com 8 achados conhecidos.** Isso é o esperado, não regressão. Confira contra o §4.2 antes de tratar qualquer achado de histórico como novo.

⚠️ **Efeito colateral registrado por precisão:** o histórico caiu de **10 para 8** achados. As entradas do `ADR-017` e do `system_menu.cpp` são por caminho, e silenciam também as ocorrências históricas **desses dois arquivos**. Ou seja, não é "histórico intocado" — é "sobrou só o `lexy`". Verificado: os 8 restantes são todos do mesmo arquivo do `lexy`.

### 10.3 Verificação

**Gate (não quebrou):**

```bash
gitleaks dir --no-banner .    # config lido da raiz, sem -c
# INF scanned ~1404357027 bytes (1.40 GB) in 46s
# INF no leaks found
# exit code: 0
```

**Prova adversarial (não cegou o scanner).** Credenciais falsas plantadas em arquivos **tracked e limpos** — e de propósito **dentro do caminho isento**, que é o teste mais duro:

| Mutante | Onde | Resultado |
|---|---|---|
| Mesma regra `generic-api-key`, mesmo arquivo isento, conteúdo diferente (`api_key` de 32 chars) | `ADR-017` (**isento**) | **DETECTADO** |
| `github-pat` (`ghp_…`) | `AUDITORIAS.md` (não isento) | **DETECTADO** |
| `AKIAZZ7EXAMPLE9QTEST` | `ADR-017` (**isento**) | não detectado → **apurado**, ver abaixo |

O terceiro mutante não disparar exigiu apuração em vez de suposição. Dois controles fecharam a causa: (a) a **mesma** string AKIA num arquivo **não isento** também **não** dispara; (b) um AKIA no alfabeto válido da regra (`AKIA` + `[A-Z2-7]{16}`), plantado **dentro do arquivo isento**, **dispara** (`aws-access-token`). Causa real: a string falsa continha um `9`, fora do alfabeto que a regra da AWS exige. **Não** era cegueira do allowlist.

**Re-execução independente pelo orquestrador** (implementer ≠ verificador): mutante `api_key` de 32 caracteres plantado dentro do arquivo isento → **detectado, exit 1**; restauração conferida por **md5 byte-idêntico**. Resultado concordante com o desta seção, obtido por outro executor.

**Restauração provada nos dois rounds:** `git checkout -- <arquivos>` seguido de `git status --porcelain` **vazio** e `md5sum -c` batendo com os hashes tirados **antes** do plantio. Os hashes foram gravados de antemão justamente para não depender de conferência visual, e nada foi plantado em arquivo untracked — ali o `git checkout` não restauraria e o `git diff` mentiria.

### 10.4 Efeito de segunda ordem: este relatório criou 2 falsos positivos

Contraintuitivo, e a próxima pessoa precisa saber antes de estranhar a contagem: **commitar este relatório aumentou o número de achados na árvore de 2 para 4.** O motivo é que o §4.2 cita as linhas dos falsos positivos **verbatim**, como evidência — então a mesma heurística que casou no código original casa de novo no documento que a analisa.

Não é defeito do relatório (citar a linha-fonte é o que torna a classificação auditável), nem do scanner. É uma propriedade de qualquer documento de auditoria que transcreva o que encontrou, e por isso o `.gitleaks.toml` isenta explicitamente as ocorrências **neste arquivo**, com o mesmo rigor de três eixos. **Ao escrever auditoria futura que transcreva achado, conte com isso.**

### 10.5 O que continua aberto

| Item | Status |
|---|---|
| **Job de secret-scan no CI** (lacuna **F2-CI.X** do §T8; rec. nº 1) | **ADIADO por decisão do líder em 2026-07-25.** O `.github/workflows/` segue sem job de secret-scan; o gate existe, mas depende de alguém rodar. Pré-requisito técnico (o `.gitleaks.toml`) já está pronto, então a implementação é barata quando for retomada. |
| **Scan de histórico saindo 1** com 8 achados conhecidos | **POR DESENHO**, ver §10.2. Não tratar como regressão. |
| **Rec. nº 3 — hook `pre-commit` local** rodando `gitleaks` nos arquivos staged | **IMPLEMENTADA em 2026-07-26** (aprovada pelo líder na madrugada de 26/07; a redação anterior desta linha, "NÃO IMPLEMENTADA e não decidida", valia até então). Lógica versionada em `tools/gitleaks_staged_check.sh`, chamada pelo `pre-commit` LOCAL do repo, que o shim global de `core.hooksPath` encadeia; **o dir global NÃO foi tocado**. Comando: `gitleaks git --staged` (a série 8.30 não tem `detect` nem `protect`), com `-v` obrigatório, porque sem ele a saída é só "leaks found: 1", sem arquivo, linha nem regra. Custo medido: **~50 ms** contra os 47 s da varredura de árvore. Bloqueia de verdade, auto-desativa em repo sem `.gitleaks.toml`, e avisa+passa se o `gitleaks` não estiver instalado. Escape de uma execução: `GUSWORLD_SKIP_GITLEAKS=1`; deliberadamente **sem** marker file, que desligaria o gate até alguém lembrar de apagar. **Prova re-executada pelo orquestrador:** segredo staged = bloqueio com exit 1 e saída acionável (arquivo, linha, `RuleID`, valor redigido a 20%); commit legítimo = exit 0 em 0,137 s; árvore idêntica ao baseline depois, arquivo de teste na lixeira. **Limitação herdada:** `.git/hooks/` não é versionado, então clone limpo não ganha o hook. Não é regressão desta entrega, é o mesmo furo do `crash_journal_check.sh` e do gate ASan do pre-push, e sobe o valor do `install_hooks.sh` pendente no item `CRASH-HOOKS` do `TODO.md`, que passa a ter **três** clientes. |
| **Rec. nº 4 — corrigir a sintaxe do §T8** | **FEITA**, no mesmo commit `8df65e6`. O `TESTES.md` §T8 agora prescreve `gitleaks dir` / `gitleaks git`, avisa que `gitleaks detect` foi removido na série 8.30, e aponta para o §4.2 deste relatório como lista de referência dos achados de histórico conhecidos. |

**O veredito do §"Veredito" permanece PASSOU e não muda com este adendo** — a remediação tornou o gate *acionável*, não alterou nada sobre o que foi (ou não foi) encontrado em 2026-07-25.
