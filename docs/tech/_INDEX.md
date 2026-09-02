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
  chave, locale como dimensão aberta, cadeia de fallback, placeholder/plural/gênero (subconjunto de
  ICU MessageFormat, decidido pelo líder em 01/09/2026), termo de lore sem tradução honesta,
  orçamento de expansão. `B9` e `D19` dependem dele.
- `ai-assets-provenance.md` — proveniência e cadeia de titularidade dos assets gerados por IA;
  resgatado do `gusworld_legacy` em 25/08/2026, com correção de regime (o texto fala em CC-BY-SA,
  o regime vigente é todos-os-direitos-reservados por L-08; o motivo de fundo — titularidade do
  líder sobre a ferramenta de IA — sobrevive intacto).
- `mapas-formato-legado.md` — registro histórico do formato `.gmap` do projeto anterior. O formato
  morreu (mapa é hoje contrato do GlintFx, L-30 deles); sobrevivem as decisões de 23/06/2026 (mapa
  selado contra adulteração, fonte editável separada do artefato compilado, identificador fixo
  anti-troca) como insumo de pedido ao GlintFx pelo bus.
- `contrato-mapa-glintfx.md` — item `B10`, sem pré-requisito, servindo `D10`, `D11`, `E1`, `E3`:
  espelho do lado consumidor do contrato de formato de mapa que o GlintFx fechou e congelou em
  22/08/2026 no bus (lista de volumes com `sensor`/`enabled`, `blocks_path` separado de
  `blocks_move`, UUID de mapa com destino de teleporte, canal de propriedades nomeadas, camada
  única na v1 com reserva aditiva, preservação de bloco desconhecido ao regravar, selo aberto de
  integridade); mais a correção de 25/08/2026 (mapa canônico e save são problemas diferentes,
  servidos em modos distintos do mesmo envelope). O GlintFx é dono do formato; este documento
  registra, não propõe.
- `algoritmos-combate-rpg-turnos.md` — pesquisa consolidada em 11/08/2026 sobre IA de combate por
  turnos (relógio híbrido, atributos, fórmulas, Utility AI, targeting, coordenador de pressão,
  dificuldade, IA de aliados, autobattle, e o bloco inteiro de anti-degeneração); trazida pelo
  próprio líder do `gusworld_legacy` em 25/08/2026, verbatim, ainda sem reconferência contra as leis
  vigentes. Cita arquivo de código do projeto anterior que não existe aqui (L-01).
- `adr/` — **13 ADRs vivos**, numerados `001` a `025` com **doze números aposentados** que não se reaproveitam; a lista dos aposentados está em `adr/_INDEX.md`.

## Varredura de 25/08/2026, resolvida em `G21` (28/08/2026)

A varredura de 25/08/2026 achou quatro ADRs (`ADR-004`, `ADR-006`, `ADR-007`, `ADR-021`) com
decisão que uma lei posterior já tinha revogado, sem marcador de superação no próprio documento.
O líder decidiu (verbatim: *"edite e deixe apenas as partes vivas. O que não tiver mais nada vivo,
apague"*) e cada um foi tratado pelo que tinha: `ADR-004` e `ADR-006` foram editados, mantendo só
o que segue vigente; `ADR-007` foi editado, mantendo só as camadas do save que não dependiam do
JSON banido pela L-18; `ADR-021` não tinha nada vivo fora do que a L-08 já cobre e foi apagado
(listado em `adr/_INDEX.md`, seção "Não presentes"). Detalhe por ADR em `adr/_INDEX.md`.
