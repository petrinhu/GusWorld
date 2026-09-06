# Diário: Fichas + Bestiary

> **Status:** Canônico (Bloco H, Tipo 3 do Diário do Gus: fichas de NPCs/facções + bestiary de inimigos turn-based). Revisão inicial 2026-05-16. Modo autônomo autorizado pelo criador supremo.
>
> **Trigger:**
> - **Fichas:** 1º encontro abre stub vago (silhueta + 2-3 bullets). Interações sucessivas + Knowledge progride atualiza (média = perfil técnico parcial; alta = perfil completo com cross-refs).
> - **Bestiary:** cada espécie progride por 5 estágios de documentação (avistamento, primeiro combate, análise completa, fraqueza descoberta, item de drop identificado; ver §5), um evento de domínio por estágio, não mais "página por abate". Exceção: 5 espécies travam no estágio 2 (2 por desenho de glitch permanente, 3 por serem encontro único atrelado a missão/clímax, sem repetição possível) e 1 trava no estágio 4 (sem item de drop); nenhuma das seis alcança o estágio 5 (ver §5 para a lista e o motivo de cada uma).
>
> **Cross-refs imutáveis:** [[_INDEX]] · [[lore-bible]] · [[factions]] · [[characters/gus]] · [[characters/caua-volt]] · [[characters/iara-lumen]] · [[characters/bento-requiem]] · [[characters/linda-siren]] · [[characters/dante-grid]] · [[characters/jaci-proxy]] · [[characters/sterling-locke]] · [[characters/patch-zero]] · [[arco-principal]] · [[timeline]] · [[foreshadowing]] · [[ui-spec]] · [[entries-mapas-timeline]] · [[knowledge-gates]] · [[foreshadow-links]].

---

## Princípio

As **Fichas** e o **Bestiary** são a aba mais analítica do Diário do Gus. Aqui o prodígio de 11 anos se mostra inteiro: ele esboça caras à mão (layer 1 caderno, cursiva infantil cuidada), anota bullets técnicos em margem (cor laranja `#FF6B1A` em sticky notes-âncora canônicas do caderno), cataloga inimigos como se fossem amostras de laboratório. Mas continua menino: nas margens há rabiscos de cubos, frases tortas em letra cansada, dobras de canto quando a entrada ficou pesada (Iara desertora, Dante vago, Sterling em escala holograma). Knowledge baixa = perfil borrado, espaço em branco com "?", silhueta sem traços. Knowledge alta = perfil técnico cristalino com cross-refs, sticky-notes amarrando o Diário a [[entries-mapas-timeline|mapas + timeline]] e a [[entries-docs-descobriveis|docs descobríveis]].

A regra é estrita: **mostre, não conte**. Gus nunca escreve "Dante é traidor". O Diário escreve "Linguagem-âncora: Asmódico inicial, depois aparece configurando Tavus-Drive em horário estranho." Knowledge alta + payoff faz aparecer sticky-note conectiva linkando "configura Tavus-Drive em horário estranho" à entry do Catalyst Dante (ato 3). O subtexto vive na **forma**: páginas dobradas, manchas, rabiscos de margem. Texto explícito só onde técnica permite (linguagem, facção, signature de conjuro). Tudo que toca emoção continua via metáfora técnica (Pillar 4) ou silêncio diagramado.

Bestiary segue mesma regra: HP, ataque visto, padrão de movimento são técnica explícita. **Fraqueza** começa hipótese ("hipótese: vulnerável a ruído ultrassom?"), vira observação ("confirmado após 3 testes"), vira exploit catalogado (Knowledge alta destrava companion-counter sugerido: "Cauã abre janela em 2 turnos; Linda finaliza em 1"). Patch-Zero é exceção declarada: bestiary entry **falha em estabilizar**. Stats aparecem com glitch tipográfico permanente. Gus rabisca por cima frustrado, depois com curiosidade.

---

## Sumário das fichas catalogadas

| Categoria | Quantidade | Trigger inicial | Trigger completo |
|---|---|---|---|
| **Auto-ficha Gus** | 1 | Tutorial | Auto-atualiza beat a beat |
| **Companions ativos** | 6 | 1º recrutamento | Pós-arco companion + Knowledge alta |
| **Antagonistas centrais** | 2 (Sterling + Patch-Zero) | 1º contato indireto / 1º glitch | Climax 2 fases |
| **Lideranças Cult Mirage** | 1 (Adila) + 2 menores | 1º encontro Mirage | Resolução arco Iara |
| **NPCs ambientais nomeados** | 24 (catalogados §3) | Conversa significativa | Sub-quest associada |
| **Facções (perfis)** | 7 (6 facções + Dutos como comunidade) | 1º contato direto | Knowledge alta + arco da facção |

Total catalogável: **41 fichas únicas**. Bestiary: **20 inimigos turn-based** medidos em §5 (5 cidade, 6 Selve, 6 cross-settings, 3 bosses incluindo Sterling 2 fases + Patch-Zero condicional), corrigindo a estimativa "~22" desta linha, nunca conferida contra o corpo real de §5, à luz da tensão numérica de `docs/design/mecanicas/conquistas.md` §2.9 (ver nota em §5). O catálogo cresce até **144 espécies distintas**, por decisão do líder de 06/09/2026 (nota de escopo no início de §5).

---

## §1. Companions (Gus + 6)

Cada companion tem ficha com 3 estados de Knowledge canônicos:
- **Knowledge baixa (stub):** silhueta + 2-3 bullets observação superficial. Pré-recrutamento.
- **Knowledge média (perfil técnico):** linguagem-âncora confirmada, signature de conjuro, role mecânico, wound superficial visível.
- **Knowledge alta (perfil completo):** memórias formativas inferidas (não citadas literalmente), cross-refs a docs in-world, sticky-notes a [[foreshadow-links]].

### 1.1 Auto-ficha: Gus Vector Tavus Vance

> *Visual da entry:* página de abertura do Diário, levemente mais cuidada que as outras. Esboço à mão do próprio rosto no canto superior esquerdo (proporção SD 1:1:1 canônica: cabeça grande, óculos cyan desenhados com 2 traços, aparelho ortodôntico marcado com 4 quadradinhos). Gus rabiscou e refez 3 vezes antes de aceitar. Em margem, anotação cursiva: "isso aqui sou eu, eu acho. parece menos eu do que o reflexo. tudo bem."

| Campo | Conteúdo |
|---|---|
| Nome canônico | Gus Vector Tavus Vance |
| Idade | 11 |
| Linguagem-âncora | **C-Arcane** |
| Facção home | nenhuma (cidadão livre, [[01-cidade-cyber-gotica|Núcleo Metropolitano]]) |
| Setting home | Núcleo Metropolitano (apartamento Vance, 6º andar Edifício Vance) |
| Role mecânico | Utility / Control |
| Signature | Compilação do Codex (3 tokens, 5 famílias, 6 tipos) |
| Hardware-âncora | Óculos Táticos + Matriz Ortodôntica (antena UHF/VHF) + Tavus-Drive de pulso |
| Cross-ref produção | [[characters/gus]] · `Resources/gusworld/character-spec-gus.md` |
| Cross-ref Bloco I | foreshadow F-001 (Gus pausa 1s antes de falar), F-014 ("histórico é como a gente sabe pra onde a coisa vai") |

**Bullets de auto-observação (cursiva margem):**
- "frases curtas. processa em silêncio. (mãe percebeu primeiro.)"
- "compila baixo nível MAS legível. discordo do Bento mas respeito."
- "se eu otimizar demais, viro o Sterling. (Gargi: 'então não otimiza demais, filho.')"

**Status canônico:** atualiza beat a beat. Ato 3 ganha nota "stats degradados refletem trauma agudo Dante. Gargi, soldando, dia 47: 'a luz volta. devagar.'"

### 1.2 Cauã "Volt" Berenger: Striker

> *Visual stub (Knowledge baixa, pré-recrutamento):* esboço rápido, silhueta angular alta, ponto-faísca em pulso. Cabelo curto. Bullet em margem: "fala alto. dorme nos Dutos. parece a Inácia (mãe). por quê?"

**Knowledge média (pós-recrutamento, ~30% campanha):**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Cauã Berenger |
| Apelido | "Volt" |
| Idade | 13 |
| Linguagem-âncora | **Pythia** (rápida de escrever, interpretada) |
| Facção home | Dutos Infernais (comunidade descentralizada juvenil) |
| Setting home | [[04-dutos-infernais|Dutos Infernais]] (QG Runners, Pilar Era 1) |
| Role mecânico | Striker (DPS elétrico) |
| Signature | **Pulso EM Concêntrico** (Elétrico + Stream + Área) |
| Família mágica | Elétrico (plasma, indução, eletroestática, magnetismo) |
| Hook ambient | Estala dedo quando processa (tic). Faísca cyan involuntária. |

**Knowledge alta (pós-arco completo + leitura docs canônicos):**

- Wound canônico: Davi (irmão 16 anos) morto em -5 em "acidente" da Subestação 7 (Cauã tinha 8). Causa real (ending Ouro): alvo deliberado FIR; Davi documentava irregularidades. Bilhete final preservado por Inácia ([[entries-docs-descobriveis|doc 13]]).
- Mãe **Inácia Berenger** (45), técnica QA ex-Apex-Data. Guarda materiais comprometedores. Vive contida.
- Cooperativa juvenil dos Dutos formada após Davi (Cauã desceu aos 9 anos pra processar).
- Sticky-note Bloco I: "lê 'cooperação' antes de 'liderança'; pillar Cauã canônico."

**Pulled quote (Gus em margem):** "ele estala o dedo quando processa. parece um while-loop esperando input. fica calmo se compila."

**Companion-counter no bestiary:** drones FIR de patrulha (vulnerável a EM); inimigos Sterling com motherboard exposta.

---

### 1.3 Iara "Lumen" Koslov: Infiltradora

> *Visual stub:* silhueta franzina, capuz, brilho refratado em volta da cara (Gus rabiscou refração com 5 traços diagonais; não acertou em duas tentativas; deixou). Bullet: "olha pra tudo de lado. fala mais devagar que o normal aos 12. (treinada pra isso?)"

**Knowledge média:**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Iara Koslov |
| Apelido | "Lumen" |
| Idade | 12 |
| Linguagem-âncora | **Óxido** (segurança/precisão; ilegível pra iniciantes, indestrutível em runtime) |
| Facção home | [[factions|Cult Mirage]] (**desertora**; facção artista internamente) |
| Setting home | [[05-setor-mirage|Setor Mirage]] (catacumbas Cult, ala artista) |
| Role mecânico | Infiltradora (criptográfico + decoy) |
| Signature | **Decoy Lumen** (Criptográfico + Stream + Área) |
| Família mágica | Criptográfico (refração, hash, esteganografia, decoy) |
| Hook ambient | Quando para na frente de espelho, Gus nota: ela mede 0,5s antes de aceitar a imagem. |

**Knowledge alta:**

- Pais biológicos desconhecidos. Abandonada bebê em catedral menor anexa ao Mirage. Adotada formalmente pelo Cult aos 6 (Adila a treinou como "promessa artística").
- Deserção aos 11 ao descobrir Operação Atualização Sensorial (festival = extração massiva de dados). Adila guarda rancor controlado.
- Mentora paralela: Hierofante Florín Estopa (facção artista resistente). Conhecida desde infância de Sérgio Brimber (holografista jovem, [[05-setor-mirage|Mirage §6]]).
- Sticky-note Bloco I: foreshadow F-038 (Iara aceita "Brilhe bem" só de adultos da facção artista; recusa silenciosamente quando vem de adepto pró-Sterling).

**Pulled quote (Gus em margem):** "ela conferiu o reflexo dela duas vezes antes de me cumprimentar. eu também faço isso. mas eu pelo motivo oposto, eu acho."

**Companion-counter:** inimigos Cult (drones holográficos), scanners Sterling (refração quebra varredura).

---

### 1.4 Bento "Requiem" Chevalier: Tanque

> *Visual stub:* silhueta robusta (Gus desenhou levemente fora de proporção SD canônica; Bento é a exceção declarada à proporção 1:1:1; ver [[characters/bento-requiem]]). Escudo-catedral nas costas com 12 dentes assimétricos. Bullet: "anda devagar. fala em latim quando esquece. cronômetro de latão no peito tique-taca."

**Knowledge média:**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Bento Chevalier |
| Apelido | "Requiem" |
| Idade | 14 |
| Linguagem-âncora | **Asmódico** (analógico Neo-Sylvania, relojoaria + ressonância acústica) |
| Facção home | [[factions|Ordem Recursiva]] (sucessor designado de Velhusto) |
| Setting home | [[03-catedrais-neo-sylvania|Catedrais Neo-Sylvania]] (Nave + Capela do Sino Asmódico) |
| Role mecânico | Tanque (cinético analógico, defesa party) |
| Signature | **Cronômetro Ressonante** (Cinético + Stream + Aliado) |
| Família mágica | Cinético (gravitacional, inercial, compressivo, rotacional) |
| Hook ambient | Sincroniza cronômetro de latão peitoral toda vez que entra em cena interna nova. |

**Knowledge alta:**

- Wound canônico: pai **Aldebrando Chevalier** (mestre Asmódico, 38) morto em -0.5 na Catedral Menor de Atelaiá, durante uma Vigília restrita de 12 mestres da qual só 11 voltaram. Causa oficial: queda em ruína. Causa real (entregue a Bento via Velhusto): eliminado pela Sterling Corp, que introduziu uma interferência ressonante calibrada para silenciá-lo depois que ele descobriu, nos cripto-glifos Era 1 profundos que copiava, o rastro precoce da própria Sterling. Diário pessoal preservado ([[entries-docs-descobriveis|doc 9]]).
- Trauma anterior: testemunhou aos 7 anos a contaminação Patch-Zero da Catedral Menor de Atelaiá em -7. Hilário Tepenkov (Mestre-Aprendiz, 14 na época) morto. Bento sobreviveu por estar perto do escudo do Mestre-Hierofante Velhusto.
- Linhagem direta: Atelaiá Chevalier (-115, codificador-mãe Asmódico) → 5 gerações → Aldebrando → Bento.
- Aprendiz atual: Beatriz Pólvora (16, [[03-catedrais-neo-sylvania|bancada da Beatriz]]). Bento ensina enquanto ele próprio se forma.
- Sticky-note Bloco I: foreshadow F-022 ("o cronômetro perdeu 1 segundo, eu sei porque o meu também"), F-067 (Bento fala mais latim quando a Ordem está rachada).

**Pulled quote (Gus em margem):** "ele anda lento mas a mecânica dele é a mais rígida do time. é teste de hipótese: previsibilidade ≠ lentidão."

**Companion-counter:** inimigos com armadura cinética (Bento perfura por ressonância); bosses Sterling que dependem de loop estável (Cronômetro Ressonante quebra cadência).

---

### 1.5 Linda "Siren" Neumann: Crowd Control

> *Visual stub:* silhueta média, agulha de toca-discos pendurada em corrente no pescoço (Gus desenhou com cuidado; pendente é detalhe que ele percebeu primeiro). Bullet: "fala baixo. baixo mesmo. todo mundo se inclina pra ouvir. (técnica de Underground?)"

**Knowledge média:**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Linda Neumann |
| Apelido | "Siren" |
| Idade | 12 |
| Linguagem-âncora | **Óxido** (segurança aplicada a som) |
| Facção home | [[factions|Underground do Silêncio]] (coordenadora célula jovem) |
| Setting home | [[07-zona-do-silencio|Zona do Silêncio]] (Casa das Antenas, sub-célula norte) |
| Role mecânico | Crowd Control (sônico, atordoamento em área) |
| Signature | **Eco do Cânion** (Sônico + Stream + Área) |
| Família mágica | Sônico (infrassom, ultrassom, eco, ruído branco) |
| Hook ambient | Inclina cabeça quando ouve. Identifica fonte sonora em 2s. |

**Knowledge alta:**

- Wound canônico: batida Sterling + FIR na casa Neumann em -8 (Linda tinha 4 anos). Toca-discos histórico apreendido. Pai **Otmar Neumann** (engenheiro analógico aposentado, 50) salvou agulha intacta antes da apreensão; entregou à filha. Pingente canônico.
- Mãe **Brígida Neumann** (49), musicista (violão). Saúde frágil pós-batida. Hospeda versão antiga da Festa da Compilação ([[tradicoes-cultura]] §1).
- Mentor: Padrinho Tiago Sevroski (55, coordenador atual Underground GusWorld City; saiu da aposentadoria pós-queda Polis-Vermelha em -2).
- Operadora intermediária: Mara Bento (28, sub-célula norte, [[07-zona-do-silencio|Diário Linda]]) tem cópia partida em 3 pedaços do vinil "Última Frequência".
- Sticky-note Bloco I: foreshadow F-051 (Linda toca agulha do pingente quando precisa de coragem), F-088 (sussurros de Polis-Vermelha na rádio analógica → escalation Patch-Zero).

**Pulled quote (Gus em margem):** "ela fala baixo. todo mundo se inclina. é controle por economia de volume. tenho que estudar isso."

**Companion-counter:** inimigos com cifra Sterling (Linda destrói pacote por ruído branco); grupos de drones (Eco do Cânion atordoa em área).

---

### 1.6 Dante "Grid" Alencar: **TRAIDOR** (memórias double-layer)

> *Visual stub (Knowledge baixa):* silhueta confiante média, sorriso simétrico. Bullet: "fala fácil. ri fácil. não conheço família dele."
>
> *Visual média (~50% campanha, pré-reveal):* esboço refeito com mais detalhe. Margem cursiva (mais pesada na caneta): "ele configurou meu Tavus-Drive ontem às 2h. eu não pedi. ele disse 'manutenção preventiva, mano'. histórico apagado."
>
> *Visual alta (pós-reveal ato 3):* dobra de canto profunda. Cursiva trêmula. Algumas palavras riscadas (mas legíveis sob o risco; Gus quis e não quis apagar). "ele veio com a gente todo arco. ele me ajudou no arco do Cauã. (ele estava reportando.) histórico é como a gente sabe pra onde a coisa vai."

**Knowledge média:**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Dante Alencar |
| Apelido | "Grid" |
| Idade | 13 |
| Linguagem-âncora | **Asmódico inicial** (pose tradicionalista) → reveal: **C-Arcane vassalo FIR/Sterling** |
| Facção home | FIR (oficialmente ex; operativo encoberto Sterling Corp) |
| Setting home | [[06-periferia|Periferia Industrial]] (oficina Alencar abandonada, vizinhança Penkin) |
| Role mecânico | Suporte hardware (integra componentes, manutenção alheia) |
| Signature | **Manutenção Preventiva** (entry double-meaning). Layer 1: upgrade de aliados. Layer 2: instala rootkit no Tavus-Drive do Gus (revelado ato 3). |
| Família mágica | Híbrida (não opera em família elétrica nem cinética isolada; **manipula hardware**) |
| Hook ambient | Mexe no Tavus-Drive alheio fora de horário. Always "rotina técnica". |

**Knowledge alta (pós-reveal):**

- Wound canônico declarado: cooperativa familiar Alencar destruída por engenharia financeira FIR-Sterling em -8. Pai **Salviano Alencar** (mecânico independente) morre 6 meses depois, oficialmente por insuficiência cardíaca (comunidade suspeita que a causa real foi outra). Mãe **Edilma Alencar** (44, operária têxtil): aparência pública canon vive reclusa em apartamento subsidiado; estado real (Etapa 1 mini-boss Dante) sequestrada na Caverna dos Perdidos como chantagem Sterling contra Dante. Cross-ref [[lore-bible]]:525 + [[PLACES]]:76.
- Wound canônico real: aliciado em -5 por **Diretor Cassiano Vorto** (FIR, 50). Subordinado de fato a Sterling Locke. Handler: Vorto remotamente.
- "Saída pública em rivalidade com a FIR" foi encenação. Dante continua reportando.
- Mestor de Periferia: **Padim Tércio Almagre** (58, "tragédia silenciosa"; sente algo mas não verbaliza, [[06-periferia|Periferia §6]]). Vizinhos: **Mestre Hilário Murch** (64, cético) + **Senhor Mateus Penkin** (62, fofoqueiro).
- Sticky-note Bloco I: foreshadow F-014 (Gus: "histórico é como a gente sabe pra onde a coisa vai"; payoff direto), F-076 (caminhão FIR Distrito V estacionado fora do horário, [[06-periferia|Periferia §3]]), F-093 (terminal extra a leste da oficina, lacrado), F-104 (Dante configura Tavus-Drive de Gus às 2h, "manutenção preventiva").
- Memória formativa double-layer (Knowledge Ouro): aos 8, Vorto leva Dante pra ver demolição da oficina Alencar. Dante chora. Vorto diz "agora você é técnico do Distrito V. você não precisa mais perder nada." Dante para de chorar. Dois anos antes do recrutamento que ele aceita "oficialmente" aos 10.

**Pulled quote (Gus em margem, pós-reveal):** "ele riu da minha piada de ponto-e-vírgula no segundo arco. eu lembro. eu não rio fácil. ele fez eu rir. (ele estava reportando aquilo também?)"

**Companion-counter:** inimigos que dependem de hardware (Dante desativa hardware aliado **OU INIMIGO** dependendo da fase do jogo; ironia mecânica revelada ato 3).

---

### 1.7 Jaci "Proxy" Vanderbist: Healer

> *Visual stub:* silhueta pequena, frasco de antídoto na cintura, mantos da Selve (espiral logarítmica bordada na manga; Gus desenhou a espiral 4 voltas, leu o padrão). Bullet: "fala devagar mas exato. cheira a café-de-folha-fractal. (Jaci é o jeito Pelicano Branco de chamar 'farmacêutica que ainda não é mestra.')"

**Knowledge média:**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Jaci Vanderbist |
| Apelido | "Proxy" |
| Idade | 11 |
| Linguagem-âncora | **Pythia** (scripting bio, interpretada) |
| Facção home | [[factions|Selve Sombria]] (vilarejo do Pelicano Branco) |
| Setting home | [[08-selve-profunda|Selve Profunda]] (vilarejo, fronteira Núcleo Mandelbrot) |
| Role mecânico | Healer biológica (antídoto, mutação temporária, bio-script) |
| Signature | **Antídoto Sintético** (Bioquímico + Null + Aliado) |
| Família mágica | Bioquímico (polímero, antiviral, endorfínico, mutagênico controlado) |
| Hook ambient | Fala mais com plantas que com pessoas em primeira semana de party. |

**Knowledge alta:**

- Wound canônico: pais **Lia Vanderbist** (farmacêutica do Pelicano Branco) + **Solano Vanderbist** (mestre-herborista flamengo) mortos em -8 em "surto silencioso" do vilarejo. Causa oficial: anomalia botânica desconhecida. Causa real (ending Ouro): vetor experimental Sterling, testagem clandestina de bio-script DRE em vilarejo periférico. Jaci (3 anos) sobreviveu por estar em viagem com a avó Mariana.
- Avó: **Anciã Mariana Vanderbist** (89), líder informal Pelicano Branco. Mentora. Suspeita Sterling desde -8; nunca prova.
- Linhagem direta: Anhuera Vanderbist (~-95, co-autora Pythia com Iremar Berenger) → Soraia → Mariana → Lia → Jaci.
- Sticky-note Bloco I: foreshadow F-029 (Jaci nota cor errada em folha biolúmen 3 settings antes da chegada ao vilarejo → escalation Patch-Zero), F-118 (rivalidade amigável com Vivendel Berenger, 14, [[08-selve-profunda|Selve Profunda §6]]).
- NPCs satélite: **Helena Sirinhaém** (50, herborista júnior aliada). **Bito Caldeira** (60, prefeito informal). **Tatauín Branca** (22, caçadora-coletora). **Vivendel Berenger** (14, bio-hacker Pythia rival amigável).

**Pulled quote (Gus em margem):** "ela compila Pythia em voz baixa. parece reza. mas é bio-script. não é reza. (é? não, não é.)"

**Companion-counter:** inimigos com toxina (Jaci anula); fauna corrompida Patch-Zero (Jaci tenta heal; falha; cataloga como "bug"; cresce em Knowledge).

---

## §2. Antagonistas centrais + lideranças Cult Mirage

### 2.1 Sterling Locke: Antagonista absoluta

> *Visual stub (1º contato, holograma propaganda Praça da Compilação):* silhueta monolítica corporativa. Monóculo angular cromado com laser vermelho `#FF0000`. Sem face visível na primeira aparição (cabeça cortada pelo enquadramento holográfico). Bullet em margem: "ele aparece só holograma até o ato 3. propaganda padrão. voz lisa. nada de subtexto. ele NÃO é como o Vorto."
>
> *Visual médio (~50% campanha):* esboço mais ousado, Gus tentou rosto inteiro. Refez 4 vezes. Margem: "o monóculo dele não é acessório. é implante. Locke Core. (li nas notas do Bento, ele leu no diário do pai dele.)"
>
> *Visual alta (pós-climax Fase 1):* face perfilada, sem expressão. Margem: "ele não tem dissidente interno simpático. testei. perguntei a três NPCs Sterling. um disse 'runtime estável.' outro disse 'sem overhead.' um terceiro tocou o monóculo direito e seguiu. (saudação canônica.) ele é monolítico. Vorto é interesse. Adila é vontade. ele é convicção. o pior dos três.

**Knowledge média (perfil corporativo):**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Sterling Locke |
| Idade | adulto (sem confirmação cronológica precisa; operacional) |
| Linguagem-âncora | **DRE (Dynamic-Runtime Evaluation)**, interpretação total, anti-tese de C-Arcane |
| Facção home | Sterling Corp (CEO único, monolítico) |
| Setting home | [[01-cidade-cyber-gotica|Cúpula Sterling]] (interior euclidiano estéril; exterior Catedrais de Silício corrompidas) |
| Role no climax | Boss climax 2 fases (Rede Distribuída → Locke Core) |
| Signature | Reescrita em runtime (deletar variável que não cabe) |
| Hardware-âncora | **Locke Core** (implante neural cortical, controla rede distribuída) |
| Cross-ref produção | [[characters/sterling-locke]] · `Resources/gusworld/character-spec-sterling-locke.md` · `characters/prelore_vilao.md` |

**Knowledge alta (perfil ideológico completo):**

- Backstory: acadêmico dissidente que escolheu predação **por convicção, não por trauma** (anti-paternalismo declarado, [[factions]] §1 "não fazer"). Tese DRE rejeitada pela academia C-Arcane em -25. Migrou setor privado. Canibalização 3 conglomerados em sequência: Apex-Data Systems (-19), Nexus-Cloud (-15), Core-Synth Bio-Tech (-12). Consolidou Sterling Corp em -12.
- Tese: "compilação é submissão ao hardware". Solução: máquina virtual universal interpretada que reescreve sintaxe em runtime. Lucro via licenciamento de "natureza on-demand" via Operação GRE (Global Runtime Environment).
- Liberou Patch-Zero achando que controlava. Falhou. Finge publicamente que não existe ("comportamentos emergentes inesperados").
- Tenentes nomeados: **Diretora Octávia Penedo** (55, Contenção, falhou Patch-Zero, [[entries-docs-descobriveis|doc 14]]); **Diretor Theodoro Calveri** (52, Aquisições); **Diretora Solange Vix** (46, Imagem Pública, [[entries-docs-descobriveis|doc 12]]).
- Sticky-note Bloco I: foreshadow F-002 (logo Sterling em todo poste do ato 1; propaganda saturada), F-040 (saudação "runtime estável" repetida 7x antes do reveal), F-110 (Locke Core mencionado em diário Aldebrando Chevalier, [[entries-docs-descobriveis|doc 9]]).

**Pulled quote (Gus, primeira margem, ato 1):** "ele fala 'continue' em loop a cada 4 minutos. é áudio diegético. o áudio é a propaganda. a propaganda é o que ele acha que o mundo deve ser: continuar sem questionar."

**Pulled quote (Gus, margem ato 3):** "ele não tem infância sofrida. (perguntei ao Velhusto.) ele escolheu isso. tem 25 anos de tese DRE pra provar."

**Bestiary entry detalhada:** ver §4.4 (Boss tier).

---

### 2.2 Patch-Zero: Antagonista-sistema (ficha ambígua, evolui)

> *Visual stub (1º glitch, ato 1 ~10% campanha):* página com **glitch tipográfico permanente**. Esboço falha (Gus tentou desenhar; saiu mancha que ele depois rabiscou por cima irritado). Bullet em margem: "isso aqui não é uma criatura. é um... bug? eu li 'bug' no Sistema do Bento, ele disse 'bug é decoração'. esse aqui é decoração?"
>
> *Visual médio (~50% campanha, pós 1º diálogo Canal 3):* página com 3 entries de texto glitch coladas (sobrepostas, gradient saturado violeta-magenta canal Mirage). Margem: "ele falou comigo. não foi cutscene. ele falou comigo. e mentiu. (eu sei porque a consequência apareceu duas cenas depois e não bateu com o que ele disse.) (50% de mentira por frase? mais? menos?)"
>
> *Visual alta (pós Patch-Zero como Boss climax 2 condicional):* página intacta visualmente, mas o conteúdo continua glitch. Gus aceita: o glitch **é** o conteúdo. Margem trêmula: "ele é o limite. Pillar 2 diz Mandelbrot pode descer infinito; fronteira final é caos irredutível. ele é a fronteira. não dá pra debugar. selei, não destruí. (Gargi disse manutenção é eterna. ela tinha razão.)"

**Knowledge média (perfil ambíguo):**

| Campo | Conteúdo |
|---|---|
| Nome canônico | Patch-Zero |
| Idade | indeterminada (embrionário pré-Era 3; ancestral possivelmente Neo-Sylvania) |
| "Linguagem"-âncora | **Anti-padrão**: invalida previsibilidade local; ruído coerente vira ruído genuíno |
| Facção home | nenhuma (limite do conhecimento personificado) |
| Setting home | [[08-selve-profunda|Núcleo Mandelbrot Interno]] (origem); 4 canais ativos cross-settings |
| Role narrativo | Antagonista-sistema (anti-padrão + consciência alien) |
| Manifestação | 4 canais: texto glitch no Diário · áudio ambient sussurrante · persona dialogável boss arenas · bug visual shader Perlin |
| Cross-ref produção | [[characters/patch-zero]] · [[lore-bible]] §8 |

**Knowledge alta:**

- Origem multi-causal (4 vetores convergentes):
  1. Bug primordial pré-existente na Selve (Era 1 hipótese arqueológica: resíduo Neo-Sylvania).
  2. Captura corporativa Sterling em -3 (falhou em conter).
  3. Amostra importada Polis-Vermelha (cross-contamination -2).
  4. Escape de laboratório central Sterling em -0.25 (catalyst macro do jogo).
- 3 encontros canônicos Canal 3 (Persona dialogável): arco Bento (Trégua falsa); arco Jaci (Provocação filosófica "você é igual a ele, gus"); ato 3 (Negociação aparente). Cada um com 3 opções (aceitar / recusar / negociar); todas têm consequência mas nenhuma é "boa" (Patch-Zero mente em porcentagem variável).
- Odeia Sterling especificamente ("o que tentou nos enjaular"; plural deliberado).
- Destino narrativo canônico: selado, não destruído. Pós-créditos stinger (B.8 [[arco-principal]]).
- Sticky-note Bloco I: foreshadow F-007 (1º glitch texto Diário ato 1 sem trigger player), F-055 (raposa-fractal com pelo errado, [[02-selve-sombria|Orla Recursiva]]), F-099 (Patch-Zero chama si próprio "nós em Polis-Vermelha já são livres"), F-128 (célula pulsando em laboratório distante, stinger).

**Pulled quote (Patch-Zero falando, transcrição direta entry):**
```
[ENTRY 0x????]
oi gus
oi vector
não sou seu inimigo. ainda.
[ERRO: parse falhou na linha ?]
```

**Pulled quote (Gus em margem, pós 3º encontro):** "ele falou 'ainda'. ainda. (sterling diria 'no momento'. patch-zero diz 'ainda'. um é cláusula contratual. outro é ameaça temporal.)"

**Bestiary entry:** ver §4.4 (Boss tier).

---

### 2.3 Adila Murmúrio: Hierofante pró-Sterling Cult Mirage

> *Visual stub:* silhueta refratada (Gus tentou refração; saiu confusa; deixou). Bullet: "vestuário com fibra óptica embutida. ela treme nas cores quando fala. (Iara: 'isso é Holohaute. é hierarquia visual interna do Cult.')"

| Campo | Conteúdo |
|---|---|
| Nome canônico | Adila Murmúrio |
| Idade | 40 |
| Linguagem | Pythia + DRE (formação Cult); pró-Sterling |
| Facção | Cult Mirage (líder facção pró-Sterling) |
| Setting | [[05-setor-mirage|Setor Mirage]] (Catacumbas Cult, Praça do Olho em festivais) |
| Role | Mini-boss arco Iara |
| Pulled quote (Gus margem) | "ela manipulou a Iara desde os 6. 'você tem dom para tornar mentiras belas.' (a Iara repetiu pra mim e tremeu na frase. eu anotei.)" |
| Sticky-note Bloco I | F-058 (foto de Sonja em altar interno Cult, [[in-world-docs]] doc 6); F-082 (Adila evita pronunciar nome "Sonja") |

**Lideranças menores pró-Sterling (Knowledge alta):**

- **Hierofante Cleomir Vasta** (37), holografia massiva.
- **Hierofante Tâmela Brida** (44), "ressonância sensorial" (eufemismo: extração de dados).
- **Hierofante Otoniel Rens** (50), veterano financeiro, negocia contratos Sterling Corp.

**Lideranças menores artistas (aliados potenciais pós-arco Iara):**

- **Hierofante Florín Estopa** (38), resistência interna passiva.
- **Hierofante Marlena Aurora** (35), detida em "reabilitação sensorial"; resgatável em sub-quest pós-recrutamento Iara.
- **Hierofante Patrício Velô** (42), quieto, observador. Knowledge alta destrava informação.

**Sonja Murmúrio (morta em -34):** co-fundadora original Cult Mirage. Mãe biológica Adila. Morta em -34 em circunstâncias nunca esclarecidas, no mesmo ano em que Adila (6 anos) assumiu liderança formal facção pró-Sterling sob tutela transitória. Adila guarda rancor paradoxal (não menciona, mas evita o nome). Ficha-fantasma: Gus catalogou via fita cassete que Sérgio Brimber entrega ([[05-setor-mirage|Mirage §6 sub-quest]]).

---

## §3. NPCs ambientais nomeados (24)

Tabela operacional canônica. Cada NPC tem entry-stub no Diário após **conversa significativa**; entry expande com sub-quest associada (se houver) ou Knowledge alta no setting.

| Nome | Idade | Setting home | Função | Hook ambient resumido | Knowledge gate / cross-ref |
|---|---|---|---|---|---|
| **Seu Bertoldo Caím** | 62 | [[01-cidade-cyber-gotica|Núcleo Metropolitano]] | técnico aposentado Era 2; banco fixo da Praça da Compilação 7h-9h | "vocês não viram a Lin Tórun assinar a placa, vocês não sabem o que é nostalgia." | Aliado passivo. Knowledge baixa: 1 hook. Média: 3 hooks (nomes engenheiros Era 2). Alta: leitura placa cripto-glífica sob asfalto, 7 nomes que ninguém lê. Cross-ref [[entries-mapas-timeline|Mapa 1]]. |
| **Vanda do Café** | 47 | [[01-cidade-cyber-gotica|Núcleo Metropolitano]] | vendedora itinerante café-de-neurônio | "o moço do 6º andar dali fica até as 2 da manhã. aquilo não é trabalho, é saudade." (refere a Gargi) | Vetor fofoca. Cobra 3 créditos por café. |
| **Patrulheiro Donato Fox** | 28 | [[01-cidade-cyber-gotica|Mercado da Sucata]] | patrulha FIR fixa, não-violento casual | "Sterling diz isso, Sterling diz aquilo. eu só quero o turno acabar." | NPC ambíguo do regime. Foreshadow Vitória Marquês ([[factions]] §3). |
| **Aprendiz Beatriz Pólvora** | 16 | [[03-catedrais-neo-sylvania|Catedrais Neo-Sylvania]] | aprendiz atual de Bento, bancada anexa | Engrenagens semiacabadas, caderno em latim de aprendiz. Bento corrige em margem ("respira antes do compasso, B."). | Sub-quest pós-arco Bento: ajudar Beatriz a terminar primeiro projeto autônomo. |
| **Mestre Hugo Tirol** | 55 | [[03-catedrais-neo-sylvania|Catedrais]] / [[02-selve-sombria|Selve]] / [[07-zona-do-silencio|Zona do Silêncio]] / [[08-selve-profunda|Selve Profunda]] | Ordem Recursiva; decifrador cripto-glífico (Era 1); NPC mais cross-setting do jogo | "Atelaiá passou por aqui em -42. eu passo agora. daqui a 50 anos, alguém passa de novo. continuidade." | Diálogo expansivo Knowledge alta. Sub-quest cross-arco Bento + Linda + Jaci. Cross-ref [[entries-docs-descobriveis|doc 15]] (gate Ouro). |
| **Hosvaldo Pinhão** | ~50 | [[04-dutos-infernais|Dutos Infernais]] | técnico-chefe núcleo operacional Pythia | "doze de fluxo, garoto. doze. era ousado." (pós-incidente Cauã arco) | Respeito permanente pós-arco Cauã. Sub-quest Knowledge alta desbloqueia. |
| **Vespa Calderón** | 24 | [[04-dutos-infernais|Dutos Infernais]] | scripter rebelde, pichação Pythia "while alive: roda" | NPC pichador; encontrado em corredor 4-leste nível 3 | Aliado periférico Cauã. Não-companion. |
| **Tao Berisi** | 12 | [[04-dutos-infernais|Dutos Infernais]] | segundo de Cauã; especialista navegação vertical | "Comutador-Antigo de jogos" ([[comic-reliefs/homenagens-diegeticas|comic-reliefs EE-16]]) | Runner juvenil canônico. Lealdade Cauã. |
| **Inês Marçal** | 11 | [[04-dutos-infernais|Dutos Infernais]] | runner juvenil; salva por Cauã em -3 | Pequena, ágil, conhecedora dos Dutos profundos | Cimentou relação Cauã pós-incidente. |
| **Bel Galvão** | 14 | [[04-dutos-infernais|Dutos Infernais]] | mais velha do grupo runner; treina menores em combate básico | Voz mais firme do grupo após Cauã | Possível mentora secundária. |
| **Pirilampo** | 13 | [[04-dutos-infernais|Dutos Infernais]] | apelido (órfão registrado, sem nome formal); ilumina-se com baterias descartadas | Tic de iluminar a si mesmo no escuro | Aliado periférico. |
| **Tata Bruno** | 10 | [[04-dutos-infernais|Dutos Infernais]] | caçula do grupo, aprendiz não autorizado a Dutos profundos | Cauã o protege | Sub-quest sentimental Cauã: ensinar Tata a navegar. |
| **Sérgio Brimber** | 24 | [[05-setor-mirage|Setor Mirage]] | holografista jovem talentoso; conhecido de Iara desde infância | Suporta facção artista clandestinamente | Sub-quest pós-arco Iara: entrega fita cassete Sonja. Eco pesado em Adila. |
| **Yara Ducourt** | 47 | [[05-setor-mirage|Setor Mirage]] | vendedora "mais real que o real" | Comércio Holohaute irregular | Sub-quest Knowledge alta. |
| **Mestre Hilário Murch** | 64 | [[06-periferia|Periferia Industrial]] | técnico C-Arcane veterano cético; ex-colega Bartolo Penkin (Apex-Data audit) | Cínico mas honesto | Sub-quest cross-arco Dante: Tiago via Hilário entrega pista. [[entries-docs-descobriveis|doc 5]] decifrável com ajuda dele. |
| **Padim Tércio Almagre** | 58 | [[06-periferia|Periferia Industrial]] | mestre confiante em Dante (a tragédia silenciosa) | Sente algo mas não verbaliza | Subtexto puro. Sticky-note Bloco I F-076. |
| **Senhor Mateus Penkin** | 62 | [[06-periferia|Periferia Industrial]] | vizinho fofoqueiro quintal Alencar (vizinhança imediata Dante) | Antena no telhado (foreshadow cabo Sterling) | Cross-ref foreshadow F-093 (terminal extra lacrado oficina). |
| **Joaquim Bartolomeu** | 32 | [[07-zona-do-silencio|Zona do Silêncio]] | técnico analógico; reconstrói toca-discos artesanalmente; filho de luthiers | Mistura peças Era 2 + impressas em casa. Som "ligeiramente errado. funciona." | Sub-quest cross-arco Iara (extrai áudio fita Sonja). |
| **Mara Bento** | 28 | [[07-zona-do-silencio|Zona do Silêncio]] | operadora rádio analógica sub-célula Norte | Guarda vinil "Última Frequência" partido em 3 pedaços | Tem dois filhos pequenos. Conhece transmissões Cidades-Gêmeas. |
| **Helga Riza** | 45 | [[07-zona-do-silencio|Zona do Silêncio]] | escritora subterrânea; compila pichações Polis-Vermelha | Arquivo Underground | Cross-ref [[entries-docs-descobriveis|doc 11]]. |
| **Anciã Mariana Vanderbist** | 89 | [[08-selve-profunda|Pelicano Branco]] | avó Jaci; líder informal vilarejo; lê Selve | Sabe ler Selve há décadas | Mentora cautelosa (não toma protagonismo). Suspeita Sterling desde -8. |
| **Bito Caldeira** | 60 | [[08-selve-profunda|Pelicano Branco]] | prefeito informal vilarejo ([[comic-reliefs/homenagens-diegeticas|comic-reliefs EE-14]]) | Decisões coletivas passam por ele formalmente; substância vem da Anciã | NPC pacífico. |
| **Tatauín Branca** | 22 | [[08-selve-profunda|Pelicano Branco]] | caçadora-coletora veterana; conhece Selve Profunda | "se você ver a copa parar de respirar, você não estava na floresta. estava no que ela não é." | Aliada útil. Sub-quest navegação Pântano. |
| **Vivendel Berenger** | 14 | [[08-selve-profunda|Pelicano Branco]] | bio-hacker Pythia rival/aliado de Jaci | Competição amigável síntese antídoto | Jaci comenta "perdão de ponto-e-vírgula" ([[comic-reliefs/cena-02-tabulacoes-vs-espacos|comic-reliefs cena 2]]). Recompensa token Pythia composto. |

**Notas operacionais:**

- Adultos como mentores (Velhusto, Mariana, Tiago) **NÃO** ofuscam companions (Pillar 4). Aparecem, dão contexto, saem. Resolução fica com peers 11-14.
- Hugo Tirol é único NPC adulto cross-setting (Catedrais → Selve → Zona do Silêncio → Selve Profunda). Função: nó de continuidade Era 1 ↔ Era 2. Pillar 4 respeitado por **não** entrar em combate party em ato algum.
- Crianças órfãs (Pirilampo, Tao, Inês, Tata) ficam na comunidade Cauã, não viram companions. Pillar 4 + escopo G1.

---

## §4. Facções: Fichas (7)

Cada facção tem ficha visual + texto. Knowledge cresce em 3 estados como nos companions. Detalhes ideológicos em [[factions]].

### 4.1 Sterling Corp

> *Esboço Gus à mão:* monóculo angular cromado isolado + laser vermelho `#FF0000` perfilado. Embaixo, prédio cúpula geométrico (Cúpula Sterling). Cor branco-estéril + cromo + acento vermelho-laser.

| Campo | Conteúdo |
|---|---|
| Tag canônica | Sterling Corp |
| Símbolo | Monóculo angular cromado com laser vermelho `#FF0000` |
| Cor institucional | branco-estéril + cromo + acento vermelho-laser |
| Origem | Era 3 (~-12; consolidação pós Apex+Nexus+Core) |
| Filosofia declarada | "circularidade que constrói futuro" + "o futuro interpreta o presente" |
| Filosofia real | DRE (Dynamic-Runtime Evaluation); envelopar Selve em GRE; lucro via licenciamento natureza on-demand |
| Líderes | Sterling Locke (CEO único monolítico) + Octávia Penedo + Theodoro Calveri + Solange Vix |
| Aliados | FIR (vassala formal); Cult Mirage facção pró |
| Inimigos | Selve, Ordem Recursiva, Underground, Patch-Zero, Gus + party |
| Setting onde dominam | Cúpula Sterling + Tetra-Torre Janelarum + presença holográfica cross-cidade |
| Knowledge baixa | "logo em todo poste. propaganda em loop a cada 4 minutos." |
| Knowledge média | "monolítico. Vorto reporta. Adila tem contrato comercial. Octávia falhou Patch-Zero." |
| Knowledge alta | "predação por convicção (-25 tese). Locke Core neural cortical. GRE = plano canônico. liberou Patch-Zero achando que controlava." |

### 4.2 Ordem Recursiva

> *Esboço Gus:* engrenagem de latão envelhecida com 12 dentes assimétricos (Gus contou os dentes; sequência recorrente truncada). Cor latão oxidado + preto-titânio + branco-osso.

| Campo | Conteúdo |
|---|---|
| Tag | Ordem Recursiva |
| Símbolo | Engrenagem 12 dentes assimétricos (sequência recorrente truncada) |
| Cor | latão oxidado + preto-titânio + branco-osso |
| Origem | Era 2 (~-148; reorganização pós-Hiato dos mestres-Asmódicos sobreviventes) |
| Filosofia | Continuidade analógica; preservação técnico-cultural Neo-Sylvania; "se C-Arcane é eficiente, Asmódico é durável." |
| Líderes | Mestre-Hierofante Velhusto (70) + Lavínia Sevra (62) + Hugo Tirol (55) |
| Aliados | Selve Sombria, Underground, Gus + party (pós-arco Bento) |
| Inimigos | Sterling Corp (pós-arco Bento), facção pró-modernização interna (purgada off-screen) |
| Setting | [[03-catedrais-neo-sylvania|Catedrais Neo-Sylvania]] (5 conhecidas, 3 ativas) |
| Slogan | "O que dura, dura. O que passa, passa." |
| Knowledge baixa | "tradicionalistas. catedrais. cronômetros de latão." |
| Knowledge média | "rachadura interna: tradicionalistas vs pró-modernização. Bento sucessor designado. -7 Hilário Tepenkov morto." |
| Knowledge alta | "Atelaiá Chevalier (-115) codificou Asmódico. -80 Tragédia São Camilo. -3 saque São Vargas, Falke + Rui sumiram. -0.5 Aldebrando eliminado (diário entregue Bento, [[entries-docs-descobriveis|doc 9]])." |

### 4.3 FIR (Federação Industrial de Reciclagem)

> *Esboço Gus:* triângulo de reciclagem amarelo, três setas girando **pra fora** (Gus desenhou e parou: "ah. essa é a pegadinha. Donato Fox tinha razão sem saber."). Cor amarelo-industrial + preto-fuligem.

| Campo | Conteúdo |
|---|---|
| Tag | FIR (Federação Industrial de Reciclagem) |
| Símbolo | Triângulo amarelo subvertido (setas pra fora, não ciclo) |
| Cor | amarelo-industrial + preto-fuligem |
| Origem | Era 2 tardia (~-50; cooperativa de boa-fé) → captura corporativa (~-25 a -12) |
| Filosofia declarada | "circularidade industrial; preservação legado tecnológico" |
| Filosofia real | Cartel mafioso; lava ativos pra Sterling Corp; controla rotas matéria-prima Periferia |
| Líderes | Diretor Cassiano Vorto (50) + sub-diretores Heitor Cravo, Vitória Marquês (dissidente potencial), Otelo Pancha, Floriana Etti |
| Aliados | Sterling Corp (suserano); Cult Mirage (negócios comuns) |
| Inimigos | Cooperativas independentes Periferia; Underground; Ordem Recursiva (filosoficamente); Gus + party (pós-arco Cauã/Dante) |
| Setting | [[06-periferia|Periferia Industrial]] + escritórios Núcleo Metropolitano (frente legal) |
| Slogan público | "FIR, circularidade que constrói futuro." |
| Knowledge baixa | "uniforme amarelo, cassetete elétrico. dupla. cota." |
| Knowledge média | "Vorto cabeça em GusWorld City. distritos rotacionais. -5 Subestação 7 (Davi). -8 cooperativa Alencar. -5 Dante aliciado." |
| Knowledge alta | "Vitória Marquês começa a temer Sterling em ato 2 (sub-quest Ouro). caminhão FIR Distrito V estacionado fora de horário ([[06-periferia|Periferia §3]]; foreshadow Dante)." |

### 4.4 Selve Sombria (facção natural)

> *Esboço Gus:* espiral logarítmica sobre folha estilizada. 4 voltas (Gus contou). Cor verde-ciano biolúmen + marrom-húmus + branco-fungo.

| Campo | Conteúdo |
|---|---|
| Tag | Selve Sombria |
| Símbolo | Espiral logarítmica sobre folha |
| Cor | verde-ciano biolúmen + marrom-húmus + branco-fungo |
| Origem | Pré-Era 2 (aldeias-fronteira descendem Êxodo Neo-Sylvania ~-720) |
| Filosofia | Cooperação ecológica baseada em padrão matemático; sem porta-voz oficial |
| Líderes | sem hierarquia formal; Anciã Mariana Vanderbist (89) é líder informal Pelicano Branco. 3 outras aldeias com anciões: Berenice Quaresma (Tucano-Cinza, 80), Otília Pamonha (Pelicano Roxo, 73), Bartolomeu Quintino (Tartaruga-Fractal, 78) |
| Aliados | Ordem Recursiva, Underground, Gus + party (pós-arco Jaci) |
| Inimigos | Sterling Corp (extração), FIR (predação), Patch-Zero (corrompe) |
| Setting | [[02-selve-sombria|Selve Sombria]] + [[08-selve-profunda|Selve Profunda]] |
| Knowledge baixa | "floresta. fauna estranha. cogumelos brilham." |
| Knowledge média | "1, 1, 2, 3, 5, 8. fractais. Pântano de Markov estocástico. fauna comporta-se em padrão." |
| Knowledge alta | "L-systems Orla Recursiva, cadeias de Markov Pântano, Mandelbrot Núcleo. fauna corrompida = Patch-Zero local. sementes-relíquia preservadas em câmaras Era 1 dentro das catedrais." |

### 4.5 Cult Mirage

> *Esboço Gus:* olho aberto refratado em prisma (Gus tentou refração de novo; saiu menos pior que a do Iara; mas ainda confuso). Cor gradient violeta-magenta-cyan saturado.

| Campo | Conteúdo |
|---|---|
| Tag | Cult Mirage |
| Símbolo | Olho aberto refratado em prisma |
| Cor | gradient violeta-magenta-cyan saturado |
| Origem | Era 2 tardia (~-35; movimento artístico genuíno; Sonja Murmúrio co-funda) → degrada Era 3 sob influência Sterling |
| Filosofia declarada | "estetização absoluta da realidade"; arte como verdade negociável |
| Filosofia real (facção pró-Sterling) | extração de dados de comportamento via experiências sensoriais |
| Filosofia real (facção artista) | preservar autonomia criativa; resistência interna passiva |
| Líderes pró | Adila Murmúrio (40) + Cleomir Vasta + Tâmela Brida + Otoniel Rens |
| Líderes artistas | Florín Estopa (38) + Marlena Aurora (35, detida) + Patrício Velô (42) |
| Aliados | Sterling Corp (facção pró); Gus + party (facção artista, pós-arco Iara) |
| Inimigos | Underground (estética antitética); Ordem Recursiva (indiferença irônica); Iara desertora |
| Setting | [[05-setor-mirage|Setor Mirage]] inteiro |
| Knowledge baixa | "cor saturada. holografia em loop. adeptos parecem todos iguais." |
| Knowledge média | "facção pró + facção artista. Adila manipula Iara desde 6. -1 Atualização Sensorial planejada como extração massiva." |
| Knowledge alta | "Sonja Murmúrio (mãe Adila, co-fundadora original) morta em -34 em circunstâncias nunca esclarecidas; Adila (6 anos) assume liderança facção pró-Sterling sob tutela transitória no mesmo ano. Pacto Sensorial = cessão de dados ([[entries-docs-descobriveis|doc 4]])." |

### 4.6 Underground do Silêncio

> *Esboço Gus:* onda de rádio circular cruzada por linha reta (símbolo "mute"). Cor preto-carbono + fúcsia-neon discreto.

| Campo | Conteúdo |
|---|---|
| Tag | Underground do Silêncio |
| Símbolo | Onda de rádio cruzada por linha reta |
| Cor | preto-carbono + fúcsia-neon discreto |
| Origem | Era 2 tardia (~-40 informalmente; -50 batida Casa das Antenas formaliza memória) |
| Filosofia | Resistência analógica anti-corporativa; cultivar silêncio + rádio analógico + frequências mortas |
| Líderes | sem líder único; coordenador rotativo. Atual GusWorld City: Padrinho Tiago Sevroski (55). Cidades-Gêmeas: Lazar Tovrov (60) + Penha Cintra (45). Polis-Vermelha residual: anônimos |
| Aliados | Cidades-Gêmeas Underground; Polis-Vermelha residual; Ordem Recursiva; Selve Sombria; Gus + party (pós-arco Linda) |
| Inimigos | Sterling Corp; Cult Mirage; FIR |
| Setting | [[07-zona-do-silencio|Zona do Silêncio]] (antenas mortas reaproveitadas) |
| Slogan | "Quem escuta, escuta tudo. Quem fala demais, é ouvido por todos." |
| Knowledge baixa | "antenas mortas. silêncio. dois dedos na orelha (cumprimento)." |
| Knowledge média | "células descentralizadas. rádio analógica cifrada (Óxido sônico). -50 Batida Casa das Antenas. Última Frequência canônica." |
| Knowledge alta | "-16 audit Apex-Data preservado ([[entries-docs-descobriveis|doc 5]]). -2 Polis-Vermelha cai, Tiago sai aposentadoria. transmissões fragmentadas residuais." |

### 4.7 Dutos Infernais (comunidade descentralizada juvenil)

> *Esboço Gus:* esquemático de duto vertical com 3 níveis + ícone de mão fechada batendo antebraço esquerdo (saudação peer canônica). Cor cinza-aço + cyan-faísca + verde-LED-spray.

| Campo | Conteúdo |
|---|---|
| Tag | Dutos Infernais |
| Símbolo | Duto vertical + mão fechada (saudação peer) |
| Cor | cinza-aço + cyan-faísca + verde-LED-spray |
| Origem | Era 1 (infra) + Era 2 (reuso) + Era 3 (reconfig Sterling/FIR); comunidade juvenil consolida -4 a 0 (Cauã desce após morte Davi) |
| Filosofia | sobrevivência cooperativa em terreno hostil; lealdade horizontal; desconfiança ampla |
| Líderes | sem hierarquia; Cauã líder informal. Segundo: Tao Berisi (12). Outros: Bel Galvão (14), Inês Marçal (11), Pirilampo (13), Tata Bruno (10). Facção radical interna: Calixto Rivaz (15, expulso/morto pós-arco Cauã sub-quest) |
| Aliados | Underground (pós-arco Linda); Selve indireta |
| Inimigos | FIR (frontal); Sterling Corp (indireto) |
| Setting | [[04-dutos-infernais|Dutos Infernais]] |
| Cumprimento canônico | "Tá no fio." / "Tá no volt." |
| Knowledge baixa | "garotada nos Dutos. corre rápido. carne-de-rato-cinético." |
| Knowledge média | "Cauã lidera. Tao segundo. cooperativa post-Davi. ataque suicida Calixto destruiu ala radical." |
| Knowledge alta | "Hosvaldo Pinhão (técnico-chefe) respeita pós-incidente fluxo. Vespa Calderón pichador 'while alive: roda' aliado periférico." |

---

## §5. Bestiary: Catálogo turn-based (20 espécies medidas; teto do catálogo em 144)

**Escopo do catálogo, decisão do líder em 06/09/2026:** o catálogo vai a **144 espécies distintas**, do inimigo mais simples ao chefe final. 144 é o termo de Fibonacci seguinte a 89, coerente com o vocabulário numérico que o projeto já usa em todo o resto do jogo (13, 21, 34, 55, 89, 144), e é o mesmo 144 da reação de unhas do Fibortorrinco-Zeckendorf (§5.5). As 20 espécies de §5.1-§5.4 mais o Fibortorrinco-Zeckendorf de §5.5 são o que está medido e catalogado até aqui; não são o teto do jogo terminado. Com essa decisão, a dica do `achv_byte_collector` (*"Você documentou 144 inimigos"*, `docs/design/mecanicas/conquistas.md` §2.9) passa a descrever **144 inimigos de verdade**.

Cada entry de bestiary progride em **5 estágios de documentação por espécie**, decisão do líder (`G12`/Eixo 2 de `docs/_secret/proposta-balanceamento-easter-eggs.md`, 30/08/2026), o conceito que dá lastro ao limiar de 144 do `achv_byte_collector` (`docs/design/mecanicas/conquistas.md` §2.9). Cada estágio é um evento de domínio (`BestiaryEntryDocumented(foe_id, stage)`) que o avaliador de conquistas soma ao contador cumulativo:

| Estágio | O que o jogador faz para avançar | O que a entry revela |
|---|---|---|
| **1. Avistamento** | Vê a espécie no mundo, antes de qualquer combate contra ela | Silhueta esboçada, sem stats, mesma convenção do "?" das Fichas em Knowledge baixa |
| **2. Primeiro combate** | Sobrevive ao 1º combate contra a espécie (vitória ou fuga, não precisa vencer) | HP visível observado, 1 ataque visto: era a antiga "página 1/Stub" |
| **3. Análise completa** | ~3-6 combates adicionais contra a mesma espécie | Padrão de movimento, todos os ataques catalogados, telegrafia decifrada: funde as antigas páginas 2 e 3, exceto a fraqueza |
| **4. Fraqueza descoberta** | Explora a fraqueza sugerida com sucesso ao menos uma vez | Fraqueza confirmada, deixa de ser hipótese |
| **5. Item de drop identificado** | Recebe o loot da espécie ao menos uma vez | Loot catalogado, exploit avançado, **companion-counter sugerido**, lore (origem) cross-ref Bloco I: era a antiga "página 4" |

**Exceção (glitch), herdada do desenho anterior:** Patch-Zero (§5.4) e a Anomalia Glitch (§5.2) **travam no estágio 2** e nunca avançam a 3, 4 ou 5; a entry glitcha permanentemente em vez de estabilizar, exatamente como as duas linhas de tabela já descrevem ("não estabiliza nunca", "nenhuma fraqueza catalogada").

**Os estágios 3 e 5 têm condições próprias que nem toda espécie alcança.** O estágio 3 exige "~3-6 combates adicionais contra a mesma espécie"; o estágio 5 exige "recebe o loot da espécie ao menos uma vez" (fraqueza confirmada não basta). Conferindo as 20 linhas de §5.1-§5.4 contra essas duas condições, além das 2 espécies-glitch já excepcionadas, mais 4 espécies não alcançam o estágio 5:

- **Sterling Locke, Fase 1 (Rede Distribuída)** (§5.4): encontrado exatamente uma vez no jogo, no clímax, nunca repetível; sem os "3-6 combates adicionais" do estágio 3, trava no estágio 2.
- **Sterling Locke, Fase 2 (Locke Core)** (§5.4): mesma razão (luta única do clímax) e, além disso, não larga material ("Nenhum material; resolução narrativa"); trava no estágio 2.
- **Mestre pró-Sterling Asmódico (3 inimigos coletivos)** (§5.3): a própria entry descreve o encontro como cena roteirizada ("Catedrais Neo-Sylvania, cena contaminação arco Bento"), não um inimigo reencontrável; sem repetição possível, trava no estágio 2, apesar de ter fraqueza e loot catalogados que nunca chega a usar.
- **Voz Cromada Sterling (entidade ambient hostil)** (§5.1): a própria entry diz "loot: nada material; info ambient destravada"; tem fraqueza confirmável (Mantra do Silêncio) e por isso alcança o estágio 4, mas nunca o 5, por não largar item nenhum.

**Reencontrabilidade de quatro espécies de missão, decisão do líder em 05/09/2026:** o Adepto Cult Mirage hostil, a Tropa de extração FIR Periferia, o Sentinela Acústico Sterling e o Drone Camuflado Sterling (todas em §5.3) são introduzidos em contexto de missão (festival sabotado, Subestação 11, arco Linda, sub-quest ato 2-3), mas continuam presentes no mundo depois desse encontro: a espécie não some do jogo naquele ponto, é reencontrável e por isso alcança normalmente os estágios que exigem combate repetido (3, 4 e 5), junto das demais espécies de §5.3 que não travam por glitch nem por clímax. É esta permanência, agora escrita, que sustenta como válido o TETO de 84 calculado abaixo: sem ela, as quatro cairiam no mesmo grupo que trava no estágio 2, e o teto precisaria de outra conta.

**Reencontrabilidade das demais espécies, decisão do líder em 06/09/2026:** as espécies deste catálogo que ainda não têm uma declaração própria sobre reencontro (nem a exclusão por não serem reencontráveis, como Sterling Locke Fase 1, Fase 2 e o Mestre pró-Sterling Asmódico; nem a declaração de missão do parágrafo acima; nem uma nota própria, como a do Zumbi do Pântano de Markov em §5.2) continuam sem decisão. Isto não é omissão: é decisão do líder, verbatim, sobre a régua a aplicar: "decidir quando for montar lore/escopo/arquitetura de cada uma". A decisão de reencontro de cada espécie chega quando ela for trabalhada, nunca antes. O TETO de 84 (calculado abaixo) não muda com esta régua: nenhuma classificação foi alterada.

O LIMIAR de 144 não muda com esta correção, só o TETO recalculado abaixo. As outras 14 espécies (4 de §5.1, 5 de §5.2, 5 de §5.3, nenhuma de §5.4) completam os 5 estágios normalmente.

⚠️ **Teto medido contra o catálogo de hoje, com o limiar de 144 mantido:** este documento cataloga **20 espécies** (§5.1: 5, §5.2: 6, §5.3: 6, §5.4: 3, contadas por linha de tabela/subseção de §5), mais o Fibortorrinco-Zeckendorf de §5.5 (contagem própria pendente, ver nota ali). Com **14 espécies** completando os 5 estágios, **1 espécie** (Voz Cromada Sterling) travando no estágio 4 e **5 espécies** (Patch-Zero, Anomalia Glitch, Sterling Locke Fase 1, Sterling Locke Fase 2, Mestre pró-Sterling Asmódico) travando no estágio 2, o total de eventos de documentação possíveis com o catálogo de hoje é **84** (14×5 + 1×4 + 5×2), abaixo do limiar de 144 do `achv_byte_collector`. O catálogo cresce até 144 espécies (nota de escopo no início desta seção): este teto de 84 é o teto do catálogo de hoje, não do jogo terminado, e se refaz quando as espécies novas entrarem. Com 144 espécies, a expectativa é o teto superar com folga o limiar de 144, mesmo descontando espécies que travam antes do estágio 5, pela mesma lógica que já desconta as seis de hoje.

⚠️ **Pergunta ao líder, não decidida aqui (L-14 global, L-29 do projeto):** o sistema de 5 estágios de documentação por espécie nasceu (`G12`, 30/08/2026) para contornar a impossibilidade de cem espécies com um catálogo de só ~20-22 (o teto de 144 herda a mesma tensão, com folga maior). Com 144 espécies existindo de fato, ele segue por dois caminhos possíveis, e a escolha muda o que o `achv_byte_collector` está de fato medindo:

- **(A) Mantém os 5 estágios como estão**, cada estágio somando ao contador do `achv_byte_collector`. Prós: o sistema já especificado inteiro (fraqueza, análise completa, drop) continua valendo por si, como profundidade de documentação; a conquista fica alcançável bem antes de o jogador catalogar as 144 espécies inteiras, com boa margem de sobra; número exato de espécies-piso não recalculado aqui, porque as 144 ainda não existem. Contras: a dica ("documentou 144 inimigos") volta a descrever 144 *eventos*, não 144 *espécies*, a mesma leitura (A)-eventos de `conquistas.md` §2.9, agora só com folga numérica, sem identidade literal entre dica e contador.
- **(B) Simplifica o contador desta conquista para 1 evento por espécie** (a definir qual estágio marca "documentada": por exemplo, ao alcançar o estágio 3, Análise completa, ou já no estágio 2, Primeiro combate), deixando os 5 estágios existirem só como sistema narrativo/mecânico do Bestiário, sem alimentar mais este contador específico. Prós: a dica bate literalmente com o contador (144 espécies documentadas = 144 eventos, sem folga nem ambiguidade residual). Contras: exige escolher e documentar qual estágio conta como "documentada" e reespecificar o evento de domínio (`BestiaryEntryDocumented`) que dispara o incremento; o multi-estágio deixa de alimentar *esta* conquista (a mecânica de bestiário em si não muda).

Devolvo a escolha entre as duas para o líder.

### 5.1 Inimigos da Cidade (5)

| Inimigo | Setting | Stats observados (estágio 2: Primeiro combate) | Stats completos (estágio 5: Item de drop identificado) | Fraqueza | Loot | Companion-counter |
|---|---|---|---|---|---|---|
| **Drone Patrulha FIR Mk-II** | [[01-cidade-cyber-gotica|Núcleo Metropolitano]] + [[06-periferia|Periferia]] | HP médio. Ataque: descarga elétrica cone curto. | HP 12. Cooldown EM 3 turnos. Motherboard exposta no torso traseiro. | Pulso EM atordoa 2 turnos; alvo de ataque cinético na motherboard exposta | sucata Sterling, token Elétrico baixo | **Cauã** (Pulso EM Concêntrico finaliza grupos de 3+) |
| **Patrulheiro FIR** | [[01-cidade-cyber-gotica|Mercado da Sucata Honesta]] + [[06-periferia]] | HP baixo. Ataque: cassetete elétrico. | HP 8. Recua a 30% HP (chama backup). Ignora ordens superiores se Knowledge alta + sub-quest Vitória Marquês ativa. | Crowd control sônico atordoa cassette antes de carregar | crédito, fragmento de cota | **Linda** (Eco do Cânion paralisa grupo) |
| **Enforcer Sterling Corp** | [[01-cidade-cyber-gotica|Tetra-Torre Janelarum]] base | HP alto. Ataque: chuva de mini-drones perfuradores. | HP 18. Locke-link a Cúpula Sterling: respawn se não eliminado em 2 turnos. | Refração criptográfica corta sinal Locke | hardware corporativo, token Criptográfico médio | **Iara** (Decoy Lumen quebra mira) |
| **Coletor de Cota FIR** | [[06-periferia|Periferia Industrial]] | HP médio. Ataque: cabo de extração corporal. | HP 14. Imune a 1 turno do primeiro ataque (escudo cota). Solta loot ao morrer. | Cinético quebra escudo cota; depois alvo qualquer | crédito alto, voucher Janelarum | **Bento** (Cronômetro Ressonante quebra cadência escudo) |
| **Voz Cromada Sterling (entidade ambient hostil)** | [[01-cidade-cyber-gotica|Núcleo Metropolitano]] + [[05-setor-mirage]] | HP indireto (mensagem). Ataque: voz "continue" causa atordoamento mental. | Persistente; não morre. Silenciável por 4 turnos com Mantra do Silêncio. | Sônico Null cancela transmissão | nada material; **info ambient destravada** | **Linda** (Mantra do Silêncio cancela transmissão por 4 turnos) |

### 5.2 Inimigos da Selve (6)

| Inimigo | Setting | Stats (estágio 2: Primeiro combate) | Stats (estágio 5: Item de drop identificado) | Fraqueza | Loot | Companion-counter |
|---|---|---|---|---|---|---|
| **Raposa-Fractal corrompida** | [[02-selve-sombria|Orla Recursiva]] | HP médio. Ataque: mordida em padrão recorrente. | HP 11. Padrão recorrente corrompido (números errados → sintoma Patch-Zero local). | Bioquímico Null cura corrupção (NÃO mata; transforma em raposa neutra) | semente-relíquia, token Bioquímico | **Jaci** (Antídoto Sintético converte em aliada temporária 3 turnos) |
| **Coruja-Mandelbrot** | [[02-selve-sombria|Pântano de Markov]] | HP alto. Ataque: olhar recursivo (debuff confusão). | HP 16. Padrão Markov estocástico; telegrafia varia. Boss-tier 3 estados (calm/agitada/recursiva infinita). | Cinético compressivo paralisa ciclo recursivo | pena-Mandelbrot, token Cinético médio | **Bento** (Vetor de Recuo + Cronômetro Ressonante) |
| **Larva-Polinomial** | [[02-selve-sombria|Orla Recursiva]] | HP baixo. Ataque: cuspe ácido. | HP 6. Multiplica em 3 polinômios de grau 2 se 1 sobrevive 2 turnos. | Pulso EM atordoa antes de multiplicar | polímero Pythia, token Bioquímico baixo | **Cauã** + **Jaci** (combo: EM + Antídoto) |
| **Anomalia Glitch (Patch-Zero infectado)** | [[02-selve-sombria|Pântano]] + [[08-selve-profunda|Núcleo Mandelbrot Interno]] | HP **glitch (número não estabiliza)**. Ataque: visual descontínuo. | Não estabiliza nunca. A entry trava no estágio 2, com glitch tipográfico permanente. Telegrafia falha. | Nenhuma fraqueza catalogada; **selável**, não eliminável | nada material; **info Bloco I** (sticky-notes Patch-Zero) | nenhum companion único; combo party 3+ necessário |
| **Fungo-Recursivo Patogênico** | [[08-selve-profunda|Selve Profunda]] | HP médio. Ataque: esporos respiratórios em área. | HP 13. Esporos propagam contaminação se aliado não cura em 2 turnos. | Bioquímico Null limpa contágio; Sônico ultrassom quebra esporo | espora-mãe, token Bioquímico médio | **Jaci** (Antídoto em área) + **Linda** (ultrassom suporte) |
| **Zumbi do Pântano de Markov** | [[02-selve-sombria|Pântano de Markov]] | HP: **PENDENTE DE MEDIÇÃO** (`02-selve-sombria.md` §11). Ataque: não descrito na fonte; a entrada canônica só especifica a queda e a reanimação. | HP: **PENDENTE DE MEDIÇÃO**. Cai a qualquer dano normal, mas reanima com a vida cheia (sem fração residual; reanimação ilimitada), a menos que a queda esteja sob a marca `SigKill` (`cartas-technomagik.md` §5.7). | Nenhuma por volume de dano: só a marca `SigKill` (status Sônico, carta comum "Eco de Encerramento") impede a reanimação, convertendo a queda em morte real dentro da janela de turnos. | ainda não decidido (`TODO.md` item `G29`) | **Linda** (família Sônico, a mesma do `SigKill`) |

**Nota operacional (Pântano de Markov, `02-selve-sombria.md` §11):** o zumbi convive com a Coruja-Mandelbrot e o boss-vírus já catalogados nesta mesma área, sem tirar nenhum dos dois de cena. Alcança os 5 estágios de documentação normalmente: é inimigo regular reencontrável na área, tem fraqueza catalogável (`SigKill`) e recebe loot (lista ainda por decidir, `G29`); nenhuma das travas estruturais do topo desta seção (encontro único, ausência de loot material) se aplica a ele.

### 5.3 Inimigos cross-settings (Catedrais / Dutos / Mirage / Periferia / Silêncio) (6)

| Inimigo | Setting | Stats (estágio 2: Primeiro combate) | Stats (estágio 5: Item de drop identificado) | Fraqueza | Loot | Companion-counter |
|---|---|---|---|---|---|---|
| **Mestre pró-Sterling Asmódico (3 inimigos coletivos)** | [[03-catedrais-neo-sylvania|Catedrais Neo-Sylvania]] (cena contaminação arco Bento) | HP alto coletivo. Ataque: cinético rotacional + escudo cronômetro. | HP 14 cada. Sincronizam cronômetros mecânicos; combate em cadência. | Quebrar cadência (Cronômetro Ressonante reverso) desorienta os 3 | engrenagens valiosas, token Cinético médio | **Bento** (Cronômetro Ressonante reverso); duelo simbólico interno Ordem |
| **Drone-Operário FIR (Dutos)** | [[04-dutos-infernais|Posto FIR Duto 7]] | HP médio. Ataque: serra plasma cinética. | HP 10. Bug exploit: para 1 turno se temperatura ambiente sobe acima de 70°C (Hosvaldo confirma). | Aproveitar turbina superaquecida; **EM + Pythia improvisada** quebra firmware | sucata cinética, token Elétrico médio | **Cauã** (signature local) |
| **Adepto Cult Mirage hostil** | [[05-setor-mirage|Catacumbas Cult]] festival sabotado | HP baixo. Ataque: refração holográfica (debuff visão). | HP 7. Imune se Iara não está na party (refração reconhece). Não-letal se rendido (Pillar 4: não matar adeptos). | Decoy Lumen quebra refração; rendição via diálogo | fragmento Holohaute, token Criptográfico baixo | **Iara** (signature local) |
| **Tropa de extração FIR Periferia** | [[06-periferia|Subestação 11]] | HP médio coletivo. Ataque: cordada de cabos cinética. | HP 9 cada (4 inimigos). Recuam em 2 turnos se líder cai. | Eliminar líder (alto à direita do grupo); cinético quebra cabos | crédito + cota | **Dante** pré-reveal usa ironicamente (sticky-note Bloco I retroativo) |
| **Drone Camuflado Sterling (estação retransmissora)** | [[08-selve-profunda|estação retransmissora oculta]] | HP alto. Ataque: scan paralisante. | HP 17. Camuflagem como pedra; Cauã detecta via EM. Sub-quest ato 2-3 destrava combate. | EM revela; Refração quebra scan | mapa retransmissor cifrado, token Elétrico alto | **Cauã** (detecção) + **Iara** (refração) |
| **Sentinela Acústico Sterling (Silêncio)** | [[07-zona-do-silencio|Casa das Antenas]] arco Linda | HP médio. Ataque: pulso ultrassom de área. | HP 11. Bug exploit: silêncio total dele desativa próprio firmware. | Mantra do Silêncio + Linda canta em frequência morta | cabo Óxido autêntico, token Sônico médio | **Linda** (signature local) |

### 5.4 Boss tier (climax 2 fases + Patch-Zero condicional) (3)

#### Sterling Locke, Fase 1: Rede Distribuída

> *Visual entry:* página dupla. Glitch zero (limpo, técnico). Esboço Gus mostra silhueta corporativa replicada em 7 nodes da Rede.

| Stats | Conteúdo |
|---|---|
| HP visível pré-encontro | **"?"** (Gus nunca enfrentou; bestiary entry trava no estágio 1, avistamento, até momento) |
| HP completo (estágio 5: Item de drop identificado) | HP central 30 + 7 nodes 5 cada (total 65). Nodes regeneram central em 3 turnos se vivos. |
| Ataques | Deletar variável (debuff string aliado); reescrita em runtime (anula buff aliado); chuva de drones-mensagem (dano em área) |
| Fraqueza | Eliminar nodes antes do central. Refração + EM combo destrava prioridade. |
| Loot | Token DRE corrompido (NÃO usável; Gus arquiva como evidência) |
| Companion-counter | **Iara** (refração detecta node real) + **Cauã** (EM derruba 2 nodes/turno) + **Linda** (silencia broadcast Locke Core) |
| Cross-ref Bloco I | Sticky-note F-040 (saudação "runtime estável" pré-batalha), F-110 (Locke Core no diário Aldebrando) |

#### Sterling Locke, Fase 2: Locke Core (climax)

> *Visual entry:* glitch zero ainda. Mas Gus rabiscou na margem 4 vezes "ele não tem dissidente interno"; letra cada vez mais firme.

| Stats | Conteúdo |
|---|---|
| HP completo | 45. Sem nodes (todos eliminados na fase 1). |
| Ataques | Loop infinito (continuous damage stream); recompilação forçada (anula token Gus 1 turno); monóculo refratado (ofusca telegrafia) |
| Fraqueza | Compilação Reversa secreta (3 tokens específicos ordem invertida) restaura HP party. **Conjuro "Você é uma fechadura. Eu sou a chave."** ([[characters/gus]] pulled quote climax) destrava finale. |
| Loot | Nenhum material; **resolução narrativa** |
| Companion-counter | **Gus protagonista** (signature Compilação do Codex em pleno potencial); party 5 ativos buffam |
| Cross-ref Bloco I | F-002 (logo Sterling presente desde ato 1; payoff visual: cúpula em ruínas), F-014 ("histórico é como a gente sabe pra onde a coisa vai"; payoff filosófico) |

#### Patch-Zero: Boss climax 2 fases condicional (ending Prata/Ouro)

> *Visual entry:* página glitch saturada. Esboço Gus falha definitivamente. Texto bestiary em pseudocódigo corrompido. Margem (cursiva trêmula tarde-ato 3): "não dá pra catalogar isso direito. tudo bem. catalogar não é entender. selar é."

| Stats | Conteúdo |
|---|---|
| HP visível | **glitch permanente**. Números variam por turno. |
| HP fase 1 | varia entre 20-40 por turno (estocástico Markov) |
| HP fase 2 | indeterminado; **se selado em vez de "morto"**, jogador progride |
| Ataques | Anti-padrão (invalida buff/debuff aliado e inimigo); Mentira persuasiva (oferece trégua falsa em meio combate); Glitch shader (mesh popping visual quebra mira); Voz canal 4 (ambient corrompe áudio) |
| Fraqueza | Nenhuma estável. Selagem requer Compilação Reversa + Asmódico (Bento) + Pythia bio (Jaci) + Criptográfico (Iara): 4 linguagens em sincronia, ato 3 climax. |
| Loot | Nada material. **Hook sequel** (B.8 stinger pós-créditos). |
| Companion-counter | **Party completa 4+ companions** (Cauã + Bento + Iara + Jaci mínimo; Linda + Dante restaurado em Ouro). Gus orquestra. |
| Cross-ref Bloco I | F-007 (1º glitch ato 1), F-055 (raposa-fractal), F-099 ("nós em Polis-Vermelha"), F-128 (stinger célula laboratório distante) |

**Notas operacionais boss:**

- Sterling Locke **NÃO** tem cena de "humanização" pré-batalha. Anti-paternalismo declarado ([[factions]] §1 "não fazer").
- Patch-Zero **NÃO** é destruído. Selado. Stinger pós-créditos canônico ([[arco-principal]] B.8).
- Diretor Cassiano Vorto (FIR) **NÃO** tem cena de duelo final. Mini-boss possível arco Cauã ou Dante; depois sai do palco.
- Adila Murmúrio **NÃO** tem cena de duelo final. Mini-boss arco Iara; afasta-se do foco.

### 5.5 Inimigo das cavernas (1)

Ideia conjunta do criador supremo e do Gus Dragon, registrada em 06/09/2026.

| Inimigo | Setting | Stats (estágio 2: Primeiro combate) | Stats (estágio 5: Item de drop identificado) | Fraqueza | Loot | Companion-counter |
|---|---|---|---|---|---|---|
| **Fibortorrinco-Zeckendorf** | Espaços naturais (cavernas): sorteado com **maior peso**. Confirmado peso alto na dungeon final (Selve Profunda, Kola-SG3-12262) e na dungeon de Selve Sombria, as duas classificadas como caverna (`mundo-topologia.md` §4). Na Orla Recursiva, o peso **sobe conforme o jogador desce**: baixo nos andares construídos de cima, alto nos andares naturais de baixo. Nas outras 10 dungeons, sem classificação ainda: peso segue a régua geral (natural = alto, construído = baixo, nunca zero) assim que cada uma for classificada (modelo de sorteio em `encontros-aleatorios.md`; ver nota abaixo). A cidade literal (Núcleo Metropolitano) não entra nesse eixo: segue sem nenhum encontro, por `encontros-aleatorios.md` §6. | HP: **PENDENTE DE MEDIÇÃO** (alvo de sensação fixado pelo líder: aproximadamente 5 minutos de combate real para um jogador derrubar; não é imortal). Dano do esporão: **baixo** (alvo fixado pelo líder, número pendente de medição, ver nota abaixo). 8% de chance de aplicar o estado envenenado (`estado-envenenado-fibortorrinco.md`) no golpe. | HP: **PENDENTE DE MEDIÇÃO**. **Imune a todo efeito de status aplicado por carta** (decisão do líder, 06/09/2026, ver nota abaixo); a imunidade é de EFEITO, não de dano, e o dano recebido segue normal (sujeito à roda de fraqueza de `combat.md` §6, quando a fraqueza dele for decidida). | Nenhuma fraqueza elemental declarada (ver Companion-counter) | unha venenosa de Fibortorrinco-Zeckendorf (`unha-veneno-fibortorrinco-item.md`) | **PENDENTE** (não especificado) |

**Nota operacional (Fibortorrinco-Zeckendorf, inglês Fibolatypus-Zeckendorf):**

- **O nome, completo:** cunhado pelo líder em 06/09/2026, no padrão Animal-Conceito da casa (Símio-Fibonacci, Tartaruga-Voronoi, Coruja-Mandelbrot). "Fibortorrinco" enxerta Fibonacci em ornitorrinco/platypus; "Zeckendorf" é a segunda metade do nome, e é a metade que carrega o conceito matemático exato. O teorema de Zeckendorf diz que todo número inteiro positivo se escreve de **uma única maneira** como soma de números de Fibonacci **não consecutivos**: não há solução parcial, não há meio caminho, o número tem de cair certo na sequência ou não há decomposição válida nenhuma. É literalmente a regra da unha (`unha-veneno-fibortorrinco-item.md` §2): 144 unhas frescas rendem 2 gotas de veneno, porque 144 cai certo; 89 unhas renderiam, em teoria, 1 gota, mas essa reação não tem receita definida no jogo, desvantajosa e impraticável mesmo se existisse; 72 (metade de 144) não rende nada, porque metade do insumo não é meia solução válida, é ausência de solução. O nome não é "o bicho tem tema matemático": é o nome **sendo** a mecânica, unicidade sem meio-termo. Identificador de dado/código deriva de `fibolatypus` (inglês, `snake_case`, L-22 do projeto, decisão do líder mantida mesmo os seis bichos existentes usando português); o nome de exibição em pt-br é Fibortorrinco-Zeckendorf.
- **Vida e dano, e o que o líder disse sobre o papel do bicho:** vida segue o mesmo formato do Zumbi do Pântano de Markov (§5.2), pendente de medição, com o alvo de sensação (aproximadamente 5 minutos para um jogador real). O dano do esporão é **baixo**, por decisão do líder, verbatim: "ele não parece chefe, pois quase não causa dano nenhum... apenas é resistente". Isto é traço canônico do desenho, não detalhe a calibrar livremente: o Fibortorrinco-Zeckendorf é **obstáculo, não ameaça**. O risco dele não é matar o jogador, é o veneno (os 8% por golpe) e o tempo que ele consome. Qualquer texto de missão, tela ou diálogo sobre este inimigo que faça o jogador temer morrer para ele contradiz esta decisão.
- **"Resiste a efeitos bônus" = imunidade total a efeito, decisão do líder em 06/09/2026.** Varredura no vocabulário de efeitos do jogo (`cartas/_vocabulario.md` §5, `combat.md` §6 roda de fraqueza e §9 status framework) confirma a lacuna: a roda de fraqueza (`Fraco`/`Neutro`/`Resistente`/`Imune`, `combat.md` §6) rege o multiplicador de DANO por família elemental, não a chance de um `status` (§9) grudar no alvo, e o motor de combate não tem hoje um eixo de imunidade à APLICAÇÃO de status, independente da imunidade a dano. O que o motor precisa ganhar, sem inventar a implementação aqui: um sinal por-ator (paralelo ao `multFraqueza == 0.0` que já existe para dano, `combat.md` §6) que o handler de `ApplyStatus` (`cartas/_vocabulario.md` §4) consulta antes de aplicar qualquer `status` a este alvo; se o sinal estiver ligado, o efeito **dissipa**, no mesmo formato que o `side_filter` errado já dissipa efeito hoje (`cartas/_vocabulario.md` §7: "não é erro, simplesmente não acontece, e o motivo fica registrado"). É um eixo NOVO e ortogonal ao `WeaknessTier` existente, não uma reinterpretação dele.
- **Risco de tédio, medido contra os 5 minutos, não resolvido aqui:** um inimigo com vida alta, dano baixo e imune a todo efeito de status tira do jogador as duas ferramentas que normalmente encurtam um combate longo (explorar fraqueza elemental de dano e aplicar debuff). Os 5 minutos de sensação viram, nessa combinação, 5 minutos de troca de golpe repetitivo sem alavanca tática nova, o que é o padrão clássico de tédio, distinto de desafio. Não decido isto: fica nomeado para o líder e para o `economy-designer`/playtest avaliarem se o dano baixo e a vida alta, sozinhos, seguram a tensão pelos 5 minutos inteiros, ou se falta uma terceira alavanca (por exemplo, telegrafia do esporão, ou punição por ficar parado) que não foi pedida nesta decisão.
- **Sorteio por área, eixo natural × construído:** o líder deu a régua de vocabulário do projeto (`mundo-topologia.md` §4, nota de 06/09/2026): "dungeon" é categoria de produção (espaço feito à mão), e dentro dela a ficção separa espaço natural (caverna) de espaço já construído (catacumba, ducto, ruína). Medido contra `encontros-aleatorios.md` §6: aquele parágrafo trata de uma coisa diferente, se uma ÁREA tem `EncounterProfile` ou não (binário: dungeon com perfil × cidade sem perfil nenhum). O eixo natural × construído mora DENTRO das áreas que já têm perfil (o campo `entries[].weight` de cada perfil, já dado por área desde a §5 daquele documento): basta o peso do Fibortorrinco-Zeckendorf vir mais alto nas entradas de área natural e mais baixo (nunca zero) nas de área construída. Os dois eixos não colidem, e a cidade literal (Núcleo Metropolitano) segue sem `EncounterProfile` nenhum, sem precisar de exceção. Nenhuma edição em `encontros-aleatorios.md` foi necessária.
- **Classificação das 13 dungeons (canon vigente, decisão do líder em 06/09/2026), verbatim: "final: caverna. selve: caverna. orla: começa dungeon e vira caverna em andares mais profundos. Outros: decidir se caverna/dungeon quando for discutir sobre cada um."** Registrada em `mundo-topologia.md` §4. A dungeon final (Selve Profunda, Kola-SG3-12262) e a dungeon de Selve Sombria são caverna, peso alto para o Fibortorrinco-Zeckendorf. A Orla Recursiva transiciona de construído (peso baixo, andares de cima) para caverna (peso alto, andares de baixo) conforme a profundidade, não é mistura uniforme. As outras 10 dungeons ficam sem classificação até serem discutidas individualmente; nenhuma delas é classificada por conta própria aqui. `PLACES.md` tem um local canônico chamado **Caverna dos Perdidos** (sub-local de Selve Sombria, ligado ao reveal do arco Dante); segue candidato ao habitat específico dentro da dungeon de Selve Sombria, não fato fechado.
- **Contagem do catálogo:** esta espécie ainda não está somada à linha de abertura de §5 (que fala em 20 espécies, 5+6+6+3) nem à tabela de tensão numérica do limiar de 144 (`conquistas.md` §2.9). Os dois cálculos dependem de fraqueza e reencontrabilidade, que aqui seguem pendentes; a soma correta pede reaudição quando essas pendências fecharem.

---

## §6. Cross-refs (sticky-notes Bloco I)

### Mapas Bloco H ↔ Fichas
- Companions têm cross-ref direto a [[entries-mapas-timeline|Mapa de setting home]]: Cauã ↔ Mapa 4 (Dutos), Iara ↔ Mapa 5 (Mirage), Bento ↔ Mapa 3 (Catedrais), Linda ↔ Mapa 7 (Silêncio), Dante ↔ Mapa 6 (Periferia), Jaci ↔ Mapa 8 (Selve Profunda).
- Sterling Locke ↔ Mapa 1 (Núcleo, Cúpula Sterling no ato 3); Mapa 9 (macro, Cúpula visível em escala).
- Patch-Zero ↔ Mapas 2, 8 (Selve + Selve Profunda); Mapa 9 (epicentro Mandelbrot).

### Timeline Bloco H ↔ Fichas
- Cada wound canônico de companion = entry timeline cruzada (ver [[entries-mapas-timeline]] §Era 3):
  - Davi morre -5 (Cauã tinha 8)
  - Batida casa Neumann -8 (Linda)
  - Surto Pelicano Branco -8 (Jaci)
  - Cooperativa Alencar destruída -8 (Dante)
  - Contaminação Catedral Atelaiá -7 (Bento, Hilário Tepenkov morto)
  - Adila assume Cult pró-Sterling (tutela transitória -34, consolidação plena -6, Iara)
  - Iara deserta -1
  - Aldebrando morto -0.5 (Bento)
  - Patch-Zero escapa -0.25 (catalyst macro)

### Docs descobríveis Bloco H ↔ Fichas
- Docs 5 (audit Apex), 11 (pichações Polis-Vermelha), 13 (bilhete Davi) ↔ Underground + Cauã + Hilário Murch
- Doc 9 (diário Aldebrando) ↔ Bento + Velhusto
- Doc 12 (Sterling Voice propaganda) ↔ Adila + Sterling Corp
- Doc 14 (comunicação interna Sterling, escape Patch-Zero) ↔ Octávia Penedo + Sterling
- Doc 15 (cripto-glifo Catedral-Mãe, gate Ouro) ↔ Hugo Tirol + Bento

### Foreshadowing Bloco I ↔ Fichas
- 130 plants mapeados em [[foreshadow-links]]. Cada companion: 8-12 plants. Sterling: 15-20 plants. Patch-Zero: 12-18 plants. Dante (traidor): 18-22 plants (densidade máxima, payoff catalyst ato 3).
- Sticky-notes coloridos no Diário conectam visualmente plant → payoff. Knowledge alta destrava sticky-notes adicionais retroativos (jogador relê entries antigas e vê conexões novas).

---

## Notas de design

- **Knowledge baixa = espaço em branco honesto.** Não preencher com mentira nem com "hipótese plausível". "?" é informação válida.
- **Knowledge alta NÃO destrava SPOILER.** Destrava **conexão**: o que o jogador já viu, mas não tinha cruzado. Dante traidor é catalyst de arco; bestiary não pode adiantar; sticky-notes Bloco I aparecem **após** o catalyst, mostrando que o plant estava lá desde o ato 1.
- **Patch-Zero entry permanece instável.** Mesmo Knowledge alta. Tudo do mundo que se cataloga, exceto isto. Pillar 2 (fronteira final = caos irredutível) traduzido em UX.
- **Bestiary respeita Pillar 4.** "Eliminação" de adepto Cult é rendição diegética, não morte. Adultos antagonistas (Sterling, Vorto, Adila) são derrotados; **não executados em cutscene**. Sterling cai por consequência sistêmica (sua tese se prova circular); não há gore.
- **Cross-ref companions ↔ bestiary é mecânica diegética.** Quando Gus catalogue inimigo, sticky-note sugere companion-counter. Jogador aprende a montar party por dado, não por intuição.
- **Companion ficha NÃO inclui romance.** Pillar 4. Atrito intra-party é técnico/filosófico, nunca romântico.
- **Cursiva Gus = subtexto via forma.** Margens trêmulas em entries Dante pós-reveal. Caligrafia consistente em entries técnicas. Acessibilidade: opção de substituir cursiva por bloco-letra ([[ui-spec]] §8).

---

**Última revisão:** 2026-05-16. Canônico Bloco H Tipo 3. Atualizações exigem aprovação do criador supremo.
