# ADR-005: Licença GPLv3 (código) + CC-BY-SA 4.0 (assets) + livros reservados

| | |
|---|---|
| **Status** | Accepted (decidido pelo líder supremo 2026-06-21; fecha o eixo RF-9 do pivot) |
| **Date** | 2026-06-21 |
| **Decisor** | petrinhu (criador supremo, titular único do copyright) |
| **Reversibilidade** | One-way door para o público. Titular único pode relicenciar o que vier; releases já publicadas permanecem sob a licença vigente na época. |
| **Substitui** | Migração para AGPL-3.0 (commit `4b850dc`); dúvida "AGPL vs GPL" e "assets a definir" registradas em `docs/tech/pivot/engine-design.md` §7 |

## Contexto

O modelo de negócio foi fechado: GusWorld é **freeware** (grátis, pra sempre) + **doação opcional** (PayPal, "buy me a coffee and some AI tokens", nunca obrigatória) + **código aberto**.

O repo havia migrado para **AGPL-3.0** (commit `4b850dc`). O design do pivot (`engine-design.md` §7) deixou duas pendências abertas para o RF-9: (1) **AGPL vs GPLv3** para um jogo desktop single-player, e (2) a **licença dos assets**, separada do código.

Pontos técnicos que pesaram:

- A cláusula de rede da AGPL (§13) é **inócua sem servidor**. GusWorld é single-player puro (sem backend), então a AGPL é "mais copyleft do que o necessário"; GPLv3 entrega o mesmo efeito prático de copyleft forte com menos atrito conceitual e é a escolha convencional para jogo desktop.
- O pivot vai usar **Qt** (LGPLv3 + alguns módulos GPL-only). GPLv3 é compatível com Qt sem licença comercial: pode-se inclusive **static-linkar** o Qt, pois o jogo inteiro é GPLv3.
- **Assets** (arte, música, lore in-game) costumam ir sob licença própria, distinta do código.
- O criador é **titular único** do copyright (solo indie, sem contribuições externas aceitas em G1, ver README "não aceita PRs externos"). Isso preserva a liberdade de relicenciar no futuro.

## Decisão

1. **Código-fonte = GNU GPL v3.0 (GPLv3).** Migrado de AGPL-3.0. Cobre `.cs`, `.cpp`, `.h`/`.hpp`, `.gd`, scripts de tooling e de build. O texto verbatim está em `LICENSE` (imutável).
2. **Assets = Creative Commons BY-SA 4.0 (CC-BY-SA 4.0).** Cobre sprites, modelos, texturas, áudio/música, textos in-game e a lore canônica usada no jogo (conteúdo sob `assets/` e equivalentes). Termos oficiais: <https://creativecommons.org/licenses/by-sa/4.0/>. Fronteira detalhada em `ASSETS-LICENSE.md`.
3. **Livros-companheiros (Vol 1 bíblia + Vol 2 antologia) = direitos reservados.** Obra literária à parte, todos os direitos reservados ao autor. Vivem em `docs/book/`. A lore aparecer no jogo (CC-BY-SA) NÃO estende essa licença ao texto dos livros: suportes distintos, licenças distintas.
4. **Fronteira `.tscn`/`.tres`:** arquivo com lógica (script, expressão, máquina de estado, comportamento) = **código GPLv3**; dado/arte puro (valores, referências, paleta, balanceamento sem lógica) = **asset CC-BY-SA 4.0**.
5. **Modelo:** freeware + doação opcional via PayPal donate (merchant `9XNZQ4RND67KL`, BRL). O EULA proprietário / código fechado que o F2-LEG.1 previa fica **obsoleto** (não há mais intenção de fechar o código).
6. **Static-link do Qt liberado.** Com o jogo todo em GPLv3, o link estático com Qt LGPL/GPL é permitido sem custo nem licença comercial. O link dinâmico LGPL deixa de ser obrigatório: vira escolha técnica de empacotamento, não de licença. Atribuição de terceiros (Qt, Godot, addons, fontes, OpenSSL) em `THIRD-PARTY-LICENSES.md`.
7. **SPDX `GPL-3.0-or-later`** aplicado **apenas aos fontes C++ NOVOS** do pivot. NÃO adicionar SPDX em massa nos `.cs` atuais (serão descartados no decommission da engine C#).
8. **Titular único = relicenciamento livre no futuro.** Como não há copyright de terceiros no código próprio, o titular pode relicenciar versões futuras. **Releases já publicadas sob AGPL-3.0 permanecem AGPL-3.0** (a licença concedida a quem já recebeu o código é irrevogável); a mudança vale daqui para frente.

## Dívida conhecida: licença do submodule engine (gus_dragon-engine)

O submodule `engine/` (repo `gus_dragon-engine`, engine C# .NET 8) carrega uma **inconsistência de licença não resolvida**, registrada aqui como **dívida**, NÃO corrigida nesta ADR:

- `engine/LICENSE.md` declara **AGPL-3.0**.
- `engine/README.md:9` declara **PolyForm Noncommercial 1.0.0**.

Os dois se contradizem entre si. **Decisão: não relicenciar nem editar arquivos do submodule.** A engine C# será **descartada no pivot para C++** (RF-2.1 / marco M8 decommission), então a inconsistência morre junto com ela. Corrigir agora seria trabalho jogado fora (anti-over-engineering).

Ações fora do escopo desta ADR (responsabilidade do líder, separadas):

- **Arquivar** o repo `gus_dragon-engine` no Codeberg (decommission).
- Qualquer ajuste dentro de `engine/` só se o decommission for adiado e a engine C# voltar a ser distribuível.

## Alternativas consideradas

### A. Manter AGPL-3.0

**Pros:**
- Já era a licença vigente (commit `4b850dc`); zero trabalho.
- Compatível com Qt (AGPLv3 ↔ GPLv3 têm compatibilidade mútua; LGPLv3 usável em AGPL).

**Cons:**
- Cláusula de rede §13 é inócua num jogo desktop single-player (sem servidor): copyleft acima do necessário.
- Menos convencional para jogo desktop; confunde quem lê a licença esperando GPL.

**Decidida:** REJEITADA. GPLv3 entrega o mesmo copyleft prático com menos atrito.

### B. Licença permissiva (MIT/Apache) para o código

**Pros:**
- Máxima reutilização por terceiros.

**Cons:**
- Não é o desejo do criador (quer copyleft, código aberto que continua aberto).
- Static-link de Qt GPL-only já puxaria GPL de qualquer forma em parte do build.

**Decidida:** REJEITADA. O criador quer copyleft forte.

### C. Assets junto com o código (mesma licença)

**Pros:**
- Um arquivo de licença só.

**Cons:**
- GPL é desenhada para software; CC-BY-SA é o padrão de fato para arte/áudio/texto.
- Misturar dificulta reuso de asset isolado por terceiros (mod, fan art) sob termos claros.

**Decidida:** REJEITADA. Código GPLv3 + assets CC-BY-SA 4.0 é a separação canônica.

### D. Tudo (inclusive livros) sob CC

**Pros:**
- Consistência total.

**Cons:**
- O criador quer reter os livros como obra comercial/autoral à parte (direitos reservados).

**Decidida:** REJEITADA. Livros Vol 1/Vol 2 = direitos reservados, fora do CC.

## Consequências

### Positivas

- **Tensão de licença resolvida.** Fecha as duas pendências do RF-9 (AGPL vs GPL; assets).
- **Qt sem custo.** GPLv3 permite static-link de Qt LGPL/GPL; sem licença comercial.
- **Fronteira clara** para quem quer reusar: código (GPLv3) vs arte/áudio (CC-BY-SA) vs livros (reservados).
- **Liberdade futura preservada** (titular único pode relicenciar o que vier).

### Negativas (custos aceitos)

- A migração AGPL para GPL **não retroage** a releases já publicadas (permanecem AGPL): efeito histórico inevitável e aceito.
- Manter **três regimes de licença** no mesmo repo exige disciplina de fronteira (mitigado por `ASSETS-LICENSE.md`).
- Dívida da licença do submodule fica em aberto até o decommission (aceito; some com a engine C#).

## Ações

1. ✅ `LICENSE` trocado para GPLv3 (texto verbatim) e README raiz atualizado (feito antes desta ADR).
2. ✅ Criar `ASSETS-LICENSE.md` (fronteira código/assets/livros).
3. ✅ Criar `THIRD-PARTY-LICENSES.md` (atribuição Qt/Godot/addons/fontes/OpenSSL; incluir no release).
4. ✅ Criar `.codeberg/FUNDING.yml` (`custom: paypal.me/petrinhu`).
5. ✅ Atualizar `docs/tech/pivot/engine-design.md` §7 (AGPL para GPLv3; assets CC-BY-SA; livros reservados; static-link Qt liberado).
6. ⏳ SPDX `GPL-3.0-or-later` nos fontes C++ NOVOS do pivot, conforme forem criados (NÃO em massa nos `.cs`).
7. ⏳ **Líder:** arquivar repo `gus_dragon-engine` no Codeberg (decommission, ação separada).

## Cross-refs

- `docs/tech/pivot/engine-design.md` §7 (direção de licença do pivot, agora fechada).
- `ADR-002-csharp-aot-over-gdscript.md` (stack C# da engine que será descartada; contexto do submodule).
- `LICENSE` (texto GPLv3 verbatim, imutável).
- `ASSETS-LICENSE.md` (fronteira código/assets + exceção livros).
- `THIRD-PARTY-LICENSES.md` (atribuição de terceiros).
- `README.md` seção Licença (já reflete GPLv3).
- GPLv3: <https://www.gnu.org/licenses/gpl-3.0.html> · CC-BY-SA 4.0: <https://creativecommons.org/licenses/by-sa/4.0/>
