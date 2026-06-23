# PLACES.md: Inventário canônico de lugares do GusWorld

> **Status:** Canônico. Imutável sem aprovação do criador supremo.
> **Escopo:** TODOS os lugares nomeados do canon (settings de jogo, cidades, vilarejos, catedrais, estações, pontos geográficos, sub-locais relevantes).
> **Hook futuro:** sempre que um novo lugar nomeado for criado em qualquer doc canônico, adicionar linha aqui (hook PostToolUse a expandir cobertura pra PLACES.md).
> **Atualização:** manual + via hook PostToolUse (a configurar). Auditoria sistemática contra `docs/narrative/environments/*`, `docs/narrative/in-world-docs.md`, `docs/narrative/factions.md`, `docs/narrative/timeline.md`, `docs/narrative/deep/*` pendente.

## Convenções da tabela

- **Nome:** nome próprio canônico.
- **Tipo:** Setting principal | Cidade | Vilarejo | Catedral | Estação | Ponto geográfico | Sub-local
- **Era:** 1 (Pré-Código / Neo-Sylvania) | 2 (Era do Compilador) | 3 (Era Sterling / presente) | Cross-eras
- **Localização canon:** referência geográfica/relacional (ex: "borda oeste Selve Sombria")
- **Status:** `✅ canônico` | `🟡 lendário` (mencionado mas localização hoje desconhecida) | `🔴 saqueado` | `💀 destruído` | `⚪ abandonado funcional` (mecanismos continuam operando sem habitantes)
- **Cross-ref:** doc canônico principal

---

## 1. Settings principais do jogo (Bloco F)

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **GusWorld City (Núcleo Metropolitano)** | Setting principal | 3 sobre 1+2 | Hub central, capital regional | ✅ canônico | `environments/01-cidade-cyber-gotica.md` |
| **Selve Sombria** | Setting principal | Cross-eras | Floresta-fronteira leste da Cidade | ✅ canônico | `environments/02-selve-sombria.md` |
| **Catedrais Neo-Sylvania** | Setting principal | 1 (estrutura) + 2 (Ordem) | Dentro Selve Sombria, distribuídas | ✅ canônico | `environments/03-catedrais-neo-sylvania.md` |
| **Dutos Infernais** | Setting principal | 1+2+3 | Subterrâneo industrial-arcano | ✅ canônico | `environments/04-dutos-infernais.md` |
| **Setor Mirage** | Setting principal | 2 + 3 | Adjacente ao Núcleo, distrito ofuscamento | ✅ canônico | `environments/05-setor-mirage.md` |
| **Periferia** | Setting principal | 2 + 3 | Bairro oeste da Cidade | ✅ canônico | `environments/06-periferia.md` |
| **Zona do Silêncio** | Setting principal | 2 + 3 | Distrito acusticamente morto, norte da Cidade | ⚪ abandonado funcional | `environments/07-zona-do-silencio.md` |
| **Selve Profunda** | Setting principal | Cross-eras (3 densa) | Núcleo central da Selve | ✅ canônico (climax ato 3) | `environments/08-selve-profunda.md` |

## 2. Catedrais Neo-Sylvania (Era 1, canon §2 deep-lore)

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Catedral-Mãe** | Catedral (mainframe litúrgico) | 1 (-950) | Região profunda Selve, hoje desconhecida (possivelmente afundada) | 🟡 lendário (lacrada por dentro -750) | `deep/eras/era-1-pre-codigo.md` §2.2, §2.6 |
| **Atelaiá** (catedral menor) | Catedral (subroutine irrigação) | 1 (~-950 derivada) | Dentro Selve, sítio conhecido | 🔴 saqueada (Sterling -3) | `deep/eras/era-1-pre-codigo.md` §2.2 + `environments/03-catedrais-neo-sylvania.md` |
| **São Vargas** (catedral menor) | Catedral (subroutine sementeira) | 1 derivada | Dentro Selve | 🔴 saqueada (Sterling -3) | `deep/eras/era-1-pre-codigo.md` §2.2 |
| **São Camilo** (catedral menor) | Catedral (subroutine acústica) | 1 derivada | Edifício dentro da Selve; acervo crítico evacuado pro arquivo de retaguarda da Ordem no Núcleo Metropolitano | 🔴 saqueada (Sterling -8); acervo crítico (Codex, glossário, dossiês) salvo no anexo-retaguarda do Núcleo, de onde opera Cassandra "Bento" Chevalier | `deep/eras/era-1-pre-codigo.md` §2.2 + `environments/03-catedrais-neo-sylvania.md` + `deep/eras/era-3-sterling.md` |
| **Quarta** (catedral menor) | Catedral (subroutine archive lítico) | 1 derivada | Dentro Selve (inscrição parcial; nome próprio perdido) | ✅ canônico (em uso ativo Ordem Recursiva, sede informal) | `deep/eras/era-1-pre-codigo.md` §2.2 |
| **Quinta** (catedral menor) | Catedral (subroutine calendário) | 1 derivada | Dentro Selve (inscrição parcial; nome próprio perdido) | ✅ canônico (em uso ativo Ordem Recursiva, sede informal) | `deep/eras/era-1-pre-codigo.md` §2.2 |
| **Boca-da-Funda** | Sub-local artefato (Catedral-Mãe Era 1) | 1 | Par de cristais piezo pareados canônicos preservados em câmara central Catedral-Mãe. Calibrados originalmente por Helíaco Vyr (Primeiro Cantor-de-Pedras canon §6.3 deep-lore). Função: amplificação ressonante acústica-piezo ritual. Hoje submerso com Catedral-Mãe (-750 selagem) | 🟡 lendário (submerso) | `deep/eras/era-1-pre-codigo.md` §6.2 + `R9 conto 14 Helíaco` |

## 3. Cidades-irmãs (mencionadas, não-jogáveis em G1)

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Polis-Vermelha** | Cidade | 3 | Distante, residual | 🟡 lendário (sussurros via rádio, foreshadow Patch-Zero plural F104) | `factions.md` + `in-world-docs.md` doc 11 |
| **Cidades-Gêmeas** | Cidade | 3 | Distante | 🟡 lendário (rádio Zona do Silêncio mid-ato 2) | `environments/07-zona-do-silencio.md` + `foreshadowing.md` F130 |
| **Heliópolis-Nova** | Cidade | 3 | Distante, eufemismo Sterling | 🟡 lendário (referenciada como destino "transferido" pelo Manual Octantes Reabsorção) | `in-world-docs.md` DD-021 |

## 4. Vilarejos-fronteira (descendentes Êxodo Neo-Sylvania -720)

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Pelicano Branco** | Vilarejo-fronteira | Cross-eras (fundado -720) | Borda da Selve Sombria | ✅ canônico (Jaci + Anciã Mariana) | `factions.md` + `environments/08-selve-profunda.md` + `deep/eras/era-1-pre-codigo.md` §2.7 |
| **Vilarejo do Acaceiro Antigo** | Vilarejo-fronteira | Cross-eras (fundado -720) | Região noroeste da Catedral-Mãe submersa | ✅ canônico (descendência direta Família-Pilastra Argéndia) | `deep/eras/era-1-pre-codigo.md` §9.2 + §9.5 |
| **Vilarejo do Vale Ferraz** | Vilarejo-fronteira | Cross-eras (fundado -720) | Região central, descendência Família-Pilastra Ferraz | ✅ canônico (variedade canônica de milho ferraz) | `deep/eras/era-1-pre-codigo.md` §9.2 + §9.5 |
| **Vilarejo da Margem-do-Pelicano** | Vilarejo-fronteira | Cross-eras (fundado -720) | Região meridional, descendência Família-Pilastra Vanderbist | ✅ canônico (variedade canônica de batata-de-margem; sob proteção Pelicano Branco) | `deep/eras/era-1-pre-codigo.md` §9.2 + §9.6 |
| **Vilarejo da Selve-do-Norte** | Vilarejo-fronteira | Cross-eras (fundado -720) | Região setentrional, descendência Família-Pilastra | ✅ canônico (variedade canônica de raiz-amarga) | `deep/eras/era-1-pre-codigo.md` §9.2 + §9.6 |
| **Vilarejo da Pedra-Cantante** | Vilarejo-fronteira | Cross-eras (fundado -720) | Região oriental, descendência Família-Pilastra | ✅ canônico (variedade canônica de erva-de-pedra) | `deep/eras/era-1-pre-codigo.md` §9.2 + §9.6 |
| **Vilarejo do Vale Boroshova** | Vilarejo-fronteira | Cross-eras (fundado -720) | Refúgio canônico da Linhagem Boroshova-Vance | ✅ canônico (origem Vesperina Boroshova-Vance, Anselmo Boroshova-Vance, Praxídice Boroshova-Vance) | `deep/eras/era-1-pre-codigo.md` §8.4 + §9.3 + §9.5 + §9.7 |

## 4b. Clãs vilarejos-fronteira menores (Selve oriental, anel menor não-Êxodo)

Canon F5-BK.AUDIT T2-C3 resolução "modelo dois-anéis": 10 vilarejos pós-Êxodo (§4 + §5b = 7+3) + 3 clãs menores anel oriental tributária NÃO-Êxodo = 13 vilarejos total (valor recorrente canon).

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Tucano-Cinza** | Clã vilarejo-fronteira menor | Era 1 (origem antropológica não-Êxodo) | Norte Selve oriental tributária | ✅ canônico (Berenice Quaresma anciã) | `factions.md:399-401` + `environments/02-selve-sombria.md` + `environments/08-selve-profunda.md` + `pelicano-branco.md` + `cosmologia-deep.md:67` |
| **Pelicano Roxo** | Clã vilarejo-fronteira menor | Era 1 (origem antropológica não-Êxodo) | Oeste Selve oriental tributária | ✅ canônico (Otília Pamonha anciã) | `factions.md:399-401` + `environments/02-selve-sombria.md` + `environments/08-selve-profunda.md` + `pelicano-branco.md` |
| **Tartaruga-Fractal** | Clã vilarejo-fronteira menor | Era 1 (origem antropológica não-Êxodo) | Sul Selve oriental tributária | ✅ canônico (Bartolomeu Quintino ancião) | `factions.md:399-401` + `environments/02-selve-sombria.md` + `environments/08-selve-profunda.md` + `pelicano-branco.md` |

## 5. Pontos geográficos / sub-locais relevantes

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Praça da Compilação** | Sub-local (Núcleo) | 2 + 3 | Centro civil Núcleo Metropolitano | ✅ canônico | `environments/01-cidade-cyber-gotica.md` |
| **Distritos Inferiores** | Sub-local (Núcleo↔Periferia) | 2 + 3 | Borda baixa do Núcleo Metropolitano descendo à charneira da Periferia; engloba a Praça da Compilação e a descida sul ao portão da Periferia. Área-âncora do Vertical Slice (cidade ato 1, ~5min explore) | ✅ canônico (VS; canonizado 2026-06-03 via F2-G.1 DA-1) | `docs/design/levels/blockout-distritos-inferiores.md` + `environments/01-cidade-cyber-gotica.md` |
| **Tetra-Torre Janelarum** | Sub-local (Núcleo) | 3 | Lado norte Núcleo | ✅ canônico | `environments/01-cidade-cyber-gotica.md` |
| **Mercado da Sucata Honesta** | Sub-local (Núcleo↔Periferia) | 2+3 | Zona-charneira | ✅ canônico | `environments/01-cidade-cyber-gotica.md` |
| **Edifício Vance** | Sub-local (Núcleo) | 2 (~-90 a -70) | Lado leste Núcleo, apartamento Vance 6º andar | ✅ canônico (safe base Gus) | `environments/01-cidade-cyber-gotica.md` |
| **Setor Tavus** | Sub-local (Núcleo GusWorld City) | 2+3 | Histórico Núcleo Metropolitano. Residência linhagem Vance pós-Gustaf I (**-150 fundação Setor Tavus**, mesmo ano da primeira linha C-Arcane compilada; antecede em 10 anos a fundação formal de GusWorld City em -140 sobre sítio Neo-Sylvania de superfície). Preservado pela tradição canon Tavus. 21 quadras iniciais traçadas por Gustaf I em proporção razão acaceiro (valor recorrente) | ✅ canônico | `CHARS.md` §4 + `deep/antologia/10-pyotor-vance-pai.md` R9 + `timeline.md` -150/-140 |
| **Anel Verde** | Sub-local (fronteira) | 2 | Fronteira leste Cidade, entrada Selve | ⚪ abandonado funcional (checkpoint FIR corrompido) | `environments/01-cidade-cyber-gotica.md` |
| **Cúpula Sterling** | Sub-local (Núcleo) | 3 | Skyline tumoral cresce ato 2/3, sede Sterling Corp | ✅ canônico (climax ato 3 setting variação) | `lore-bible.md` + `in-world-docs.md` |
| **Subestação 7** | Sub-local (Dutos) | 3 | Dutos Infernais | 💀 destruído (alvo Sterling -5, morte Davi Berenger aos 16; Cauã tinha 8) | `characters/caua-volt.md` + `in-world-docs.md` DD-013 |
| **Subestação 11** | Sub-local (Periferia ostensiva) | 3 | Periferia | ✅ canônico (foreshadow Dante) | `environments/06-periferia.md` |
| **Oficina Alencar** | Sub-local (Periferia) | 2 (cooperativa) + 3 (FIR) | Periferia, oficina herdada Salviano | ⚪ abandonado funcional (Dante usa pra disfarce) | `environments/06-periferia.md` + `foreshadowing.md` F019, F129 |
| **Caverna dos Perdidos** | Sub-local (Selve Sombria) | 1 (origem ancestral) + 3 (Sterling usa) | **Subterrânea profunda Selve Sombria, nordeste do Pântano Markov, abaixo de raízes ancestrais. Passagem secreta conhecida só por Sterling brass. Família Dante (mãe Edilma Alencar) presa lá pra chantagear Dante** (canon arco Dante Beat Ten reveal) | ✅ canônico (reveal arco Dante) | `memory/project_dragon_victory_canon.md` + `docs/narrative/deep/characters/dante-grid.md` R4 |
| **Castelo Vance** | Sub-local (geográfico histórico Era 1) | 1 (último uso ~-720) | Local não-precisado, cenário cutscene Dragon Victory. Defendido por Pyotor I Draco Vance + Vyrdragon contra cerco em Era 1 | ✅ histórico Era 1 (canon cutscene) | `memory/project_dragon_victory_canon.md` |
| **Estação Espelho Acústico (Posto 7)** | Sub-local (Zona Silêncio) | 3 | Zona do Silêncio | ⚪ abandonado funcional (Operação Espelho Acústico Sterling) | `environments/07-zona-do-silencio.md` |
| **Câmara do Equinócio Acústico** | Sub-local (Zona Silêncio) | 1 (descoberta Era 3) | Zona do Silêncio | 🟡 lendário (Joaquim Bartolomeu descobriu -3, anti-cancelamento orgânico) | `environments/07-zona-do-silencio.md` |
| **Vilarejo Pelicano Branco** | Sub-local Pelicano Branco | Cross-eras | Selve Profunda | ✅ canônico | `environments/08-selve-profunda.md` |
| **Núcleo Mandelbrot Interno** | Sub-local (Selve Profunda) | Cross-eras (3 GRE) | Coração Selve Profunda | ✅ canônico (climax ato 3, 3 rotas Bronze/Prata/Ouro) | `environments/08-selve-profunda.md` |
| **Catedral do Cronômetro-Hilário** (Pátio) | Sub-local (Catedrais) | 1+2+3 | Catedrais Neo-Sylvania (wound environmental Bento) | ✅ canônico | `environments/03-catedrais-neo-sylvania.md` + `characters/bento-requiem.md` |
| **Pilar Era 1 (canal esculpido)** | Sub-local (Dutos) | 1 | Dutos Infernais | ✅ canônico (vestígio Era 1 visível) | `environments/04-dutos-infernais.md` |
| **Trilha dos Pioneiros** | Sub-local (Selve) | 2 | Selve Sombria | ✅ canônico (50 placas latão Era 2) | `environments/02-selve-sombria.md` |
| **Orla Recursiva** | Região (Selve Sombria) | Cross-eras (catalogada -95) | Faixa de transição entre Selve média e Selve Profunda, biomas paramétricos densos | ✅ canônico (R6 Magic, catalogação Anhuera Vanderbist -95/-89, 144 organismos catalogados) | `docs/narrative/deep/magic/natureza-matematica-rigida-deep.md` R6 §2-§3 |
| **Rio Verdor** | Sub-local (Orla Recursiva, Selve Sombria) | Cross-eras | Rio meandroso, coastline Mandelbrot fractal, profundidade self-similar 3 escalas | ✅ canônico (R6) | `docs/narrative/deep/magic/natureza-matematica-rigida-deep.md` R6 §2.1 |
| **Rio Vesperal** | Sub-local (Orla Recursiva, Selve Sombria) | Cross-eras | Rio meandroso paralelo ao Verdor, coastline fractal idêntica em estrutura mas espelhada | ✅ canônico (R6) | `docs/narrative/deep/magic/natureza-matematica-rigida-deep.md` R6 §2.1 |
| **Rio Lentíssimo** | Sub-local (Orla Recursiva, Selve Sombria) | Cross-eras | Terceiro rio Orla Recursiva, fluxo lento, leito largo, coastline self-similar | ✅ canônico (R6) | `docs/narrative/deep/magic/natureza-matematica-rigida-deep.md` R6 §2.1 |
| **Trilha do Pelicano** | Sub-local (rota canon Selve) | Cross-eras (estabelecida -720) | Direção noroeste da Catedral-Mãe submersa | ✅ canônico (terminação Pelicano Branco; marcos: Pedra do Carrego, Fonte da Margem-Clara, Cruzamento dos Acaceiros Antigos) | `deep/eras/era-1-pre-codigo.md` §8.9 + §9.6 |
| **Trilha da Garça-Preta** | Sub-local (rota canon Selve) | Cross-eras (estabelecida -720) | Direção sudoeste da Catedral-Mãe submersa | ✅ canônico (terminação Margem-do-Pelicano; marcos: Charco da Garça, Pedra do Salto) | `deep/eras/era-1-pre-codigo.md` §8.9 + §9.6 |
| **Trilha do Caracará-Cinza** | Sub-local (rota canon Selve) | Cross-eras (estabelecida -720) | Direção amplamente distribuída em horizonte geográfico amplo | ✅ canônico (marcos: Encruzilhada das Três Águas, Pedra-Cantante, Vale da Selve-do-Norte, Cruzamento do Vale Ferraz, Limites da Selve Profunda) | `deep/eras/era-1-pre-codigo.md` §8.9 + §9.6 |
| **Praça do Êxodo** | Sub-local (ponto canônico) | Cross-eras (estabelecida -720) | Ponto canônico de partida do Êxodo de -720 | ✅ canônico (ata fundacional preservada in-situ em placa basaltica; ritos cerimoniais anuais conduzidos pela Ordem Recursiva: Vigília do Aniversário + Leitura Institucional + Vigília Noturna da Continuidade) | `deep/eras/era-1-pre-codigo.md` §8.9 + §9.6 |
| **Rota da Semente Recursiva** | Sub-local (rota etnobotânica Era 2) | 2 tardia (-250 a -130) | Circulação entre os 5 vilarejos-fronteira canônicos | ✅ canônico (ciclo institucional de aproximadamente 13 meses; circulação descentralizada de sementes-relíquia preservadas em ampolas herméticas) | `deep/eras/era-1-pre-codigo.md` §9.2 |
| **Quintal Penkin (antena clandestina)** | Sub-local (Periferia) | 3 | Periferia, vizinho Dante | ✅ canônico (foreshadow Dante) | `environments/06-periferia.md` |
| **Caleidoscópio (Mirage)** | Sub-local (Mirage) | 3 | Setor Mirage | ✅ canônico | `environments/05-setor-mirage.md` |
| **Catacumbas Cult (Mirage níveis -1/-2/-3)** | Sub-local (Mirage subterrâneo) | 1 (-3 nascente Neo-Sylvania) + 3 | Sob Setor Mirage | ✅ canônico (Iara descobriu nascente) | `environments/05-setor-mirage.md` |
| **Praça Sem Eco (Zona Silêncio)** | Sub-local (Zona Silêncio) | 3 | Zona do Silêncio | ✅ canônico | `environments/07-zona-do-silencio.md` |
| **Torre Mater (Zona Silêncio)** | Sub-local (Zona Silêncio) | 2 | Zona do Silêncio | ⚪ abandonado funcional | `environments/07-zona-do-silencio.md` |
| **Casa Neumann (Zona Silêncio)** | Sub-local (Zona Silêncio) | 2 | Zona do Silêncio | ✅ canônico (Linda Neumann origem) | `environments/07-zona-do-silencio.md` |
| **Câmara Última Frequência** | Sub-local (Zona Silêncio) | 2+3 | Câmara ritual Underground Silêncio. Sede da Última Frequência (canon R9 conto 5 Linda Siren). Acesso restrito sub-célula Norte Mara Bento | 🟡 lendário (descobrível ato 2) | `factions.md` Underground + `R9 conto 5` |
| **Estação Pythia Bio sucateada** | Sub-local (Selve Profunda) | 2 | Selve Profunda | ⚪ abandonado funcional | `environments/08-selve-profunda.md` |
| **Câmara Neo-Sylvania de Coleta de DNA** | Sub-local (Selve Profunda) | 1 | Selve Profunda | ✅ canônico (sementes-relíquia) | `environments/08-selve-profunda.md` |
| **Trilha das Sementes** | Sub-local (Selve Profunda) | 1-2-3 | Principal entre 13 caminhos entrada Pelicano Branco → Núcleo Mandelbrot | ✅ canônico (117 placas latão Era 2, 23 desaparecidas, canon F5-BK.AUDIT T2-M10 resolução) | `deep/settings/08-selve-profunda.md:21` |
| **Distrito V (Distrito 5)** | Sub-local (Periferia) | 3 | Bunker FIR canon, pavimento tesselado profanado | ✅ canônico (sede FIR operacional, canon F5-BK.AUDIT T2-L3 resolução) | `dante-grid.md:165` + `factions.md:301` + `foreshadowing.md` F005 + `diary/entries-fichas-bestiary.md:228-229` |

## 5a. Sub-locais Era 1 (canon §6 deep-lore, Ordem Recursiva)

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Biblioteca Cintilante** | Sub-local (Catedral-Mãe anexo subterrâneo) | 1 (auge cooperativo) | Anexo adjacente à Catedral-Mãe, subterrâneo 3 níveis verticais escalonados | 🟡 lendário (inacessível desde selagem -750) | `deep/eras/era-1-pre-codigo.md` §6.5 |
| **Ala oeste Catedral-Mãe** | Sub-local (Catedral-Mãe) | 1 | Parede piezo-litúrgica de calibração onde Helíaco Vyr cegou de luz reativa | 🟡 lendário (Catedral-Mãe inacessível) | `deep/eras/era-1-pre-codigo.md` §6.3 |
| **Scriptorium subterrâneo Catedral-Mãe** | Sub-local (Catedral-Mãe) | 1 | Sob a nave principal, sessões prolongadas Salomão Tessar Vyrcátrix | 🟡 lendário (selado -750) | `deep/eras/era-1-pre-codigo.md` §6.4 |
| **Câmara comemorativa Lúcio Ostraconis** | Sub-local (São Camilo) | 3 (memorial Era 1) | acervo no anexo-retaguarda da Ordem no Núcleo, exibe pingente cerâmico reduzido | ✅ acervo evacuado -8 pro anexo-retaguarda Núcleo (câmara física na Selve saqueada) | `deep/eras/era-1-pre-codigo.md` §6.5 |
| **Banco litúrgico Catedral de São Camilo** | Sub-local (São Camilo) | 3 (vitrine Era 1) | Vitrine fria que exibe placa conjunta assinada por Helíaco Vyr + Cira Boroshova | ✅ acervo evacuado -8 pro anexo-retaguarda Núcleo (câmara física na Selve saqueada) | `deep/eras/era-1-pre-codigo.md` §6.3 |
| **Câmara fria principal arquivo lítico São Camilo** | Sub-local (São Camilo) | 3 (preserva Era 1) | 23 placas canônicas do Codex Cantata + cordós originais, evacuados pré-saque pro anexo-retaguarda Núcleo | ✅ acervo evacuado -8 pro anexo-retaguarda Núcleo (câmara física na Selve saqueada) | `deep/eras/era-1-pre-codigo.md` §6.2 + §6.4 + §6.5 |
| **Câmara reservada (dossiês paralelos)** | Sub-local (São Camilo) | 3 (preserva Era 1) | ~40 dossiês históricos Era 1, acesso restrito Mestre-Hierofante + Inquisidor-Mestre atual, evacuados pré-saque pro anexo-retaguarda Núcleo | ✅ acervo evacuado -8 pro anexo-retaguarda Núcleo (câmara física na Selve saqueada) | `deep/eras/era-1-pre-codigo.md` §6.6 |

## 5b. Sub-locais Era 1 — §7 (Bancos sementes + Famílias-Pilastra + Êxodo)

9 lugares novos do §7 deep-lore. Cobre bancos centrais (MÁ trajetória), casas comerciais (BOA), vilarejos cooperativos (BOA→MÁ ao serem absorvidos), vilarejos-fronteira pós-Êxodo (BOA emergente).

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Banco Central de Sementes-Relíquia da Catedral-Mãe** | Sub-local (Catedral-Mãe) | 1 (auge cooperativo + deriva) | Anexo arquitetônico da Catedral-Mãe | 🟡 lendário (selado com Catedral-Mãe -750) | `deep/eras/era-1-pre-codigo.md` §7.1, §7.2, §7.5 |
| **Quatro Bancos Subordinados (Atelaiá, São Vargas, São Camilo, Quarta)** | Sub-locais (Catedrais menores) | 1 derivados | Anexos das 4 catedrais menores | parcial (São Camilo 🔴 saqueado -8, acervo evacuado pro annex Núcleo; Atelaiá/Vargas 🔴 saqueados; Quarta ✅ ativo) | `deep/eras/era-1-pre-codigo.md` §7.1 |
| **Casa Comercial Argéndia** | Sub-local (vilarejo-cidade) | 1 (auge cooperativo) | Vilarejo-cidade canon Era 1 | 🟡 lendário (perdido no Êxodo) | `deep/eras/era-1-pre-codigo.md` §7.3 |
| **Forja Ferraz** | Sub-local (vilarejo-cidade) | 1 (auge cooperativo) | Vilarejo-cidade canon Era 1, paralela à Argéndia | 🟡 lendário (perdido no Êxodo) | `deep/eras/era-1-pre-codigo.md` §7.3 |
| **Vilarejo Vanguarda** | Sub-local (vilarejo cooperativo) | 1 (auge) | Periferia da Catedral-Mãe | 🔴 saqueado/absorvido pela Ordem central pré-Êxodo | `deep/eras/era-1-pre-codigo.md` §7.4, §7.5 |
| **Vilarejo Garça-Preta** | Sub-local (vilarejo cooperativo) | 1 (auge) | Borda Selve, vilarejo cooperativo preservado | ⚪ abandonado pós-Êxodo (semente fundadora de Garça-Preta-Nova) | `deep/eras/era-1-pre-codigo.md` §7.4 |
| **Vilarejo-fronteira Garça-Preta-Nova** | Vilarejo-fronteira | Cross-eras (fundado -720) | Borda da Selve Sombria, descendente de Garça-Preta original | ✅ canônico (família Penkin ancestral) | `deep/eras/era-1-pre-codigo.md` §7.8 |
| **Vilarejo-fronteira Caracará-Cinza** | Vilarejo-fronteira | Cross-eras (fundado -720) | Borda da Selve Sombria | ✅ canônico | `deep/eras/era-1-pre-codigo.md` §7.8 |
| **Vilarejo-fronteira Sabiá-de-Bronze** | Vilarejo-fronteira | Cross-eras (fundado -720) | Borda da Selve Sombria | ✅ canônico | `deep/eras/era-1-pre-codigo.md` §7.8 |

## 6. Pontos geográficos históricos (Era 1, canon §2 deep-lore)

| Nome | Tipo | Era | Localização canon | Status | Cross-ref |
|---|---|---|---|---|---|
| **Borda oeste da Selve Sombria** | Ponto geográfico | Cross-eras | Onde se encontram cripto-glifos primários -1100 | ✅ canônico | `deep/eras/era-1-pre-codigo.md` §2.1 |
| **Leitos secos de rios (Era 1)** | Ponto geográfico | 1 (cursos mudaram) | Dentro Selve | ✅ canônico | `deep/eras/era-1-pre-codigo.md` §2.1 |

## 7. Stub: lugares a auditar/adicionar

**Auditoria sistemática pendente.** Varrer:

- `docs/narrative/in-world-docs.md` (23 docs DD-001 a DD-023; cada um menciona lugares — Catedral Menor de São Vargas DD-022, etc.)
- `docs/narrative/environments/*.md` (sub-locais por setting; muitos escaparam à tabela §5)
- `docs/narrative/factions.md` (sedes de facções, territórios)
- `docs/narrative/timeline.md` (eventos datados com localização)
- `docs/narrative/lore-bible.md` (lugares mencionados em prosa)
- `docs/narrative/tradicoes-cultura.md` (locais de celebrações)
- `docs/narrative/diary/entries-mapas-timeline.md` (9 mapas Bloco H, sub-locais)
- `docs/narrative/deep/*` (futuros docs deep-lore vão introduzir lugares novos)
- `docs/narrative/foreshadowing.md` (130 plants, alguns referenciam lugares)

Quando varredura sistemática rodar (via narrative-designer agent em modo auditor), adicionar entries faltantes preservando convenção da tabela.

---

## Lugares rejeitados / não-canon

- "Cidade Janelarum" (não canon — Janelarum é OS, não cidade)
- "Reino Tolkien-like" (não-canon, sem reinos monárquicos no GusWorld)
- "Polis-Vermelha completa" (apenas residual via rádio em G1; expansão em sequel)

---

## Cross-ref

- **CHARS.md** em `/home/petrus/IDrive/Documentos/projetos_claudebrain/Projects/gusworld/CHARS.md` (inventário canônico de personagens; complementar a este)
- `docs/narrative/environments/_INDEX.md` (Bloco F navegação)
- `docs/narrative/in-world-docs.md` (docs descobríveis canon)
- `docs/narrative/factions.md` (facções e territórios)
- `docs/narrative/timeline.md` (cronologia eventos+lugares)
- `docs/narrative/lore-bible.md` (3 eras + cosmologia)
- `docs/narrative/deep/eras/era-1-pre-codigo.md` (§2 cronologia Era 1 + horizontes)

---

**Última revisão:** 2026-05-16. Versão inicial pós-Blocos F/G/H/I + deep-lore §2 Era 1.
