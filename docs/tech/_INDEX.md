<!--
SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
SPDX-License-Identifier: AGPL-3.0-or-later
-->

# docs/tech

Decisão técnica viva e registro histórico de decisão morta que ainda ensina. Nada aqui é código —
o código do GusWorld nasce do zero, sempre assentado sobre o GlintFx (LEI ZERO, L-01).

- `convencao-formatos-gw.md` — a convenção `.gw.<tipo>` para todo formato próprio do projeto; decisão
  do líder de 24/08/2026, canônica e vigente.
- `contrato-i18n.md` — contrato de i18n do projeto (item `D20`, sem pré-requisito): convenção de
  chave, locale como dimensão aberta, cadeia de fallback, placeholder/plural/gênero (recomendação
  de subconjunto ICU MessageFormat, decisão pendente do líder), termo de lore sem tradução honesta,
  orçamento de expansão. `B9` e `D19` dependem dele.
- `ai-assets-provenance.md` — proveniência e cadeia de titularidade dos assets gerados por IA;
  resgatado do `gusworld_legacy` em 25/08/2026, com correção de regime (o texto fala em CC-BY-SA,
  o regime vigente é todos-os-direitos-reservados por L-08; o motivo de fundo — titularidade do
  líder sobre a ferramenta de IA — sobrevive intacto).
- `mapas-formato-legado.md` — registro histórico do formato `.gmap` do projeto anterior. O formato
  morreu (mapa é hoje contrato do GlintFx, L-30 deles); sobrevivem as decisões de 23/06/2026 (mapa
  selado contra adulteração, fonte editável separada do artefato compilado, identificador fixo
  anti-troca) como insumo de pedido ao GlintFx pelo bus.
- `algoritmos-combate-rpg-turnos.md` — pesquisa consolidada em 11/08/2026 sobre IA de combate por
  turnos (relógio híbrido, atributos, fórmulas, Utility AI, targeting, coordenador de pressão,
  dificuldade, IA de aliados, autobattle, e o bloco inteiro de anti-degeneração); trazida pelo
  próprio líder do `gusworld_legacy` em 25/08/2026, verbatim, ainda sem reconferência contra as leis
  vigentes. Cita arquivo de código do projeto anterior que não existe aqui (L-01).
- `adr/` — **14 ADRs vivos**, numerados `001` a `025` com **onze números aposentados** que não se reaproveitam; a lista dos aposentados está em `adr/_INDEX.md`.

## Achados desta varredura (25/08/2026) que ainda não têm correção aplicada

Quatro achados, todos no mesmo padrão: o ADR já existe aqui verbatim (trazido em passada anterior),
mas uma lei posterior revogou parte do que ele decidiu, sem que o documento ganhasse marcador de
superação — diferente de ADR-005, que já se auto-corrige.

Registrados aqui para o líder decidir — nenhuma edição foi feita nos ADRs abaixo (L-14: agente
reporta, não decide o que está morto):

- **ADR-006** (`crypto-hmac-formato-domain.md`) decidiu escrever a própria primitiva HMAC-SHA256 em
  `core/`. A **L-25** de hoje revoga isso ("nada de primitiva escrita em casa"; crypto vem do
  GlintFx). O ADR não tem nenhum marcador de superação — ao contrário de ADR-005, que já se
  auto-anota "SUPERADO". O formato de envelope que ele desenha (magic/length/payload/hmac) é o
  ancestral direto do envelope da L-25 (magia/versão/tipo/nonce/dado/selo) — essa parte é decisão
  viva, só a fonte da primitiva morreu.
- **ADR-007** (`controls-json-hash128-save-v4.md`) desenhou controles de jogador em **JSON texto
  legível**. A **L-18** de hoje proíbe formato de texto para configuração do jogador — proibição
  explícita, sem exceção. Sem marcador de superação. O mecanismo de segurança que o ADR desenha
  (detectar sem prevenir, diff mostrado ao jogador com escolha continuar/restaurar, backup embutido
  em todo save, hash sobre a forma canônica para não disparar falso positivo por reformatação
  cosmética, chave derivada por KDF em vez de crua) é o desenho de segurança mais detalhado do corpus
  e antecede a L-25 quase ponto a ponto — só o formato-texto do arquivo em si morreu.
- **ADR-004** (`environment-modifier-contract.md`) tem o mesmo conteúdo de fórmula de combate
  (`multAmbiente`) que já vive, canônico e vigente, em `docs/design/mecanicas/combat.md`. Não há
  contradição de lei — é redundância de conteúdo entre um ADR de implementação (C#, morto) e a spec
  de design (viva). Nenhuma ação necessária; registrado para não haver dúvida se alguém notar a
  sobreposição depois.
- **ADR-021** (`licenciamento-apache-assets-reservados.md`) rotacionou o código para **Apache
  License 2.0** e dividiu assets em duas zonas por data de corte. Nenhuma das duas coisas é a
  licença vigente hoje: **L-08** fixa **AGPL-3.0-or-later** para o código (motivo técnico
  registrado na própria lei: o GlintFx é AGPL, e a obra combinada herda o copyleft mais forte) e
  **todos os direitos reservados**, regime único sem zonas, para assets e lore. O ADR-021 não tem
  marcador de superação — é o único dos três ADRs de licença (005→021→L-08) sem um "Superseded by"
  no topo, porque quando o repositório anterior foi encerrado ele ainda era a decisão vigente.

Nenhum destes quatro achados foi decidido por este agente. `GODS_LAWS.md` L-14 e L-24 se aplicam:
o líder decide entre acrescentar um marcador de superação (como já existe em ADR-005) ou manter
como está.
