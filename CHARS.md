# CHARS.md: Inventário canônico de personagens do GusWorld

> **Status:** Canônico. Imutável sem aprovação do criador supremo.
> **Escopo:** TODOS os personagens nomeados do canon (party, antagonistas, NPCs adultos relevantes, NPCs ambientais, personagens históricos das 3 eras, personagens in-world-docs).
> **Hook futuro:** sempre que um novo personagem nomeado for criado em qualquer doc canônico, adicionar linha aqui (ver `docs/claude/hook-novo-personagem.md` quando criado).
> **Atualização:** manual + via hook PostToolUse (a configurar). Auditoria sistemática contra `docs/narrative/characters/*`, `docs/narrative/environments/*`, `docs/narrative/in-world-docs.md`, `docs/narrative/deep/*` pendente.

## Convenções da tabela

- **Nome:** nome próprio canônico.
- **Apelido / codinome:** entre aspas se for codinome de combate (party); título se for honorífico (Mestre, Padrinho, Diretor); apelido casual em parênteses; vazio (—) se não tem.
- **Características:** 3-5 traços principais (papel, facção, linguagem mágica, traço visual canônico, wound chave, função narrativa).
- **1ª aparição:** doc canônico onde foi mencionado primeiro (path relativo ao repo).
- **Status:** `✅ canônico` | `🟡 secundário` | `⚪ ambient` | `🔴 antagonista` | `💀 morto pré-jogo`.

---

## 1. Protagonista

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Gus Vector Tavus Vance | **"Dragon"** | 11 anos, prodígio analítico, ruivo asimétrico, óculos cyan, aparelho ortodôntico, Tavus-Drive pulso esquerdo, sobretudo gótico cinza, Utility/Control, fala C-Arcane (vence climax) | `sinopse.md` + `docs/narrative/characters/gus.md` | ✅ canônico |

## 2. Party (6 companions, peers 11-14)

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Cauã Berenger | **"Volt"** | 13 anos, Striker EMP cinético, Pythia, Dutos Infernais, irmão Davi morto pela Sterling | `docs/narrative/characters/caua-volt.md` | ✅ canônico |
| Iara Koslov | **"Lumen"** | 12 anos, Infiltradora ofuscamento (clones holo), Óxido, Setor Mirage, ex-Cult Mirage manipulada por Adila | `docs/narrative/characters/iara-lumen.md` | ✅ canônico |
| Bento Chevalier | **"Requiem"** | 14 anos, Tanque gravitacional, Asmódico (exceção Pillar 2 magia analógica mecânica), Catedrais Neo-Sylvania, irmão-aprendiz morto em zona Patch-Zero | `docs/narrative/characters/bento-requiem.md` | ✅ canônico |
| Linda Neumann | **"Siren"** | 12 anos, Crowd Control sônico, Óxido, Zona do Silêncio, toca-discos histórico destruído por Sterling+FIR aos 8 | `docs/narrative/characters/linda-siren.md` | ✅ canônico |
| Dante Alencar | **"Grid"** | 13 anos, Suporte/fortificação, Asmódico → C-Arcane late game (sinal Sterling), Periferia, **TRAIDOR canônico** recrutado por Vorto aos 8 | `docs/narrative/characters/dante-grid.md` | 🔴 antagonista (oculto) |
| Jaci Vanderbist | **"Proxy"** | 11 anos, Healer biológica, Pythia bio-hacking, Selve Sombria (vilarejo Pelicano Branco), pais mortos em surto -8 | `docs/narrative/characters/jaci-proxy.md` | ✅ canônico |

## 3. Antagonistas

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Sterling Locke | — | Adulto corporativo, acadêmico dissidente → predador (Apex-Data, Nexus-Cloud, Core-Synth Bio-Tech via Chapter 11), criador do paradigma DRE, objetivo GRE (envelopar Selve em VM interpretada), geometria angular monolítica | `docs/narrative/characters/sterling-locke.md` + `prelore_vilao.md` | 🔴 antagonista principal |
| Patch-Zero | — | Antagonista-sistema não-humano, anti-padrão + consciência alien, origem multi-causal (bug primordial + emergência Selve + captura Sterling + Polis-Vermelha residual), manifestação multi-canal (texto glitchado, sussurro áudio, persona dialogável, bug Perlin), **contível, não destrutível** | `docs/narrative/characters/patch-zero.md` | 🔴 antagonista-sistema |

## 4. Família do Gus (NPCs adultos canônicos)

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Vênea Vance | — | Mãe do Gus, técnica de bancada, safe base apartamento Vance, conserta placa-mãe à noite. **Separada amigavelmente de Pyotor desde Gus aos 6 anos** (ano -5 timeline); neutra-cordial com Pyotor; presença cotidiana primária do Gus | `sinopse.md` + `docs/narrative/environments/01-cidade-cyber-gotica.md` + memo `project_familia_vance_canonica` | ✅ canônico |
| Pyotor Vance | — | Pai do Gus, **médico-cyber itinerante** (medicina + bioengenharia + bioeletrônica + eng robótica + implantes) em rotação fronteira-Selve atendendo vilarejos pós-Êxodo (Pelicano Branco, Garça-Preta-Nova, Caracará-Cinza, Sabiá-de-Bronze). **Hobby: programação** — apresentou ao Gus que ultrapassou pai; Pyotor pede ajuda técnica ao filho (inversão pedagógica wholesome). Operou antena rádio caseira no telhado Edifício Vance pré-separação. Cartas raras (3-4 no jogo todo). **Separado amigavelmente de Vênea desde Gus aos 6 anos** (ano -5 timeline); neutro-cordial com Vênea; contato contínuo com Gus via cartas + visitas em rotações | `sinopse.md` + `docs/narrative/in-world-docs.md` DD-019 + memo `project_familia_vance_canonica` | ✅ canônico (distante, presente) |
| Yakov Vance | "Tio Yakov" | **Tio paterno do Gus, irmão mais novo de Pyotor (4 anos a menos).** Um dos melhores **engenheiros de software do reino + geólogo**. Combina ambas formações na **maior mineradora do reino**. Cria softwares pra robôs, implantes cyber, veículos. **Inovação canon: maior sistema de segurança contra acidentes em minas com veículos** (mortalidade reduzida 89% — Fibonacci). Stack prospecção solo NÃO-INVASIVA: (1) sinais de radar GPR, (2) escuta sísmica de explosões controladas, (3) **tomografia muônica passiva** (raios cósmicos atravessando solo), (4) **magnetotelúrica de fluxo iônico** (campos eletromagnéticos naturais), (5) **espectrometria microbiana bioluminescente** (fungos bioluminescentes mapeando subsolo via troca iônica). Pragmático-libertário pequeno-empresário-corporativo. Tio acessível ao Gus: apresentou xadrez (complementar à programação de Pyotor), passa hardware velho da mineradora. Cordial-respeitoso com Vênea pós-separação | memo `project_familia_vance_canonica` | ✅ canônico |
| Belinor Vance | — | Avó morta do Gus, "toda lógica começa numa horta", "errei muito, ainda planto". Mãe biológica de Pyotor e Yakov Vance | `sinopse.md` | 💀 morta pré-jogo |

## 5. Famílias dos companions (NPCs adultos canônicos)

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Inácia Berenger | — | Mãe do Cauã, sucateira no Mercado da Sucata Honesta, guarda peças Apex-Data Era 2 corporativa morta | `docs/narrative/environments/01-cidade-cyber-gotica.md` + `factions.md` | ✅ canônico |
| Davi Berenger | — | Irmão do Cauã, morto na Subestação 7 (alvo Sterling), referenciado in-world-docs DD-013 | `docs/narrative/characters/caua-volt.md` + `in-world-docs.md` DD-013 | 💀 morto pré-jogo |
| Vivendel Berenger | — | Bio-hacker rival/aliado da Jaci na Selve Profunda | `docs/narrative/environments/08-selve-profunda.md` | 🟡 secundário |
| Salviano Alencar | — | Pai morto do Dante, mestre cooperativo Era 2 (ferro de solda "S.A." na bancada), referenciado in-world-docs DD-016 | `docs/narrative/foreshadowing.md` F129 + `in-world-docs.md` DD-016 | 💀 morto pré-jogo |
| Aldebrando Chevalier | — | Pai morto do Bento, falecido em -0.5 | `docs/narrative/characters/bento-requiem.md` | 💀 morto pré-jogo |
| Atelaiana de Sevra Chevalier | — | **Mãe biológica viva do Bento**, irmã consanguínea de Mestre Lavínia Sevra. Partiu da Catedral três meses após o parto (canon: "pais distantes, missão sempre primeiro"). Instalou-se em apartamento próprio na Zona do Silêncio. Coordena há doze anos circuito de assistência clínica privada entre Underground do Silêncio e Ordem Recursiva. Vínculo com Padrinho Tiago em ofício logístico. Cross-link Linda-Bento via outra rota | `docs/narrative/deep/characters/bento-requiem.md` R4 canon | ✅ canônico (distante, presente) |
| Salvador Berenger | — | **Pai biológico morto do Cauã**, falecido em acidente de tráfego na Periferia, ano -16, antes do Cauã nascer. Inácia viúva desde então. Cauã nasceu em -13. Inácia já era viúva quando Davi morreu na Subestação 7 (canon) | `docs/narrative/deep/characters/caua-volt.md` R4 canon | 💀 morto pré-jogo |
| Atelaiá Chevalier | — | Engenheira Era 2, desenvolveu Asmódico moderno (~-115), codificadora-mãe = Hari Seldon arquétipo canon, ancestral direta de Bento "Requiem" Chevalier + Cassandra "Bento" Chevalier + Verônica Atelaiá | `docs/narrative/lore-bible.md` §3.2 + Praça Compilação 01-cidade + `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.3-§9.7 | 💀 histórico Era 2 |
| Eusébio Argéndia-Chevalier | — | Cônjuge co-codificador Atelaiá Chevarier (~-145 a ~-90), descendente direto Albertina Argéndia §7.3 por 4 gerações sucessivas, articulou parte prática-operacional codificação Asmódico canônico -115 enquanto Atelaiá articulou teoria paleográfica. Paralelo Dors Seldon (Asimov Foundation) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §10 (canon §10 working file) | 💀 histórico Era 2 |
| Lia Vanderbist | — | Mãe morta da Jaci, surto silencioso -8 no Pelicano Branco | `docs/narrative/characters/jaci-proxy.md` + DD-020 | 💀 morta pré-jogo |
| Solano Vanderbist | — | Pai morto da Jaci, surto silencioso -8, plantador Pelicano Branco | `docs/narrative/characters/jaci-proxy.md` + DD-020 | 💀 morto pré-jogo |
| Anciã Mariana Vanderbist | — | Avó da Jaci, líder do vilarejo Pelicano Branco, levantou sementes-relíquia em -1 | `docs/narrative/characters/jaci-proxy.md` + `factions.md` | ✅ canônico (idosa) |

## 6. Líderes / contatos de facção

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Mestre-Hierofante Velhusto | "Velhusto" | Líder Ordem Recursiva (Catedrais), mentor distante de Bento, mantém biblioteca canônica | `docs/narrative/factions.md` | ✅ canônico |
| Diretor Cassiano Vorto | "Vorto" | Diretor FIR (vassalo Sterling), handler de Dante desde os 8 anos, frequenta café da esquina no Núcleo | `docs/narrative/factions.md` + `environments/01-cidade-cyber-gotica.md` | 🔴 antagonista lateral |
| Hierofante Adila Murmúrio | "Adila" | Líder Cult Mirage (Reality), manipulou Iara | `docs/narrative/factions.md` | 🔴 antagonista lateral |
| Sonja Murmúrio | — | Ex-Cult Mirage, morta, circunstâncias não esclarecidas (irmã ou parente de Adila?) | `docs/narrative/factions.md` | 💀 morta pré-jogo |
| Padrinho Tiago | "Padrinho" | Coordenador Underground do Silêncio, atravessa Núcleo ↔ Zona, vínculo com Linda e família Bento (Brígida) | `docs/narrative/factions.md` + `environments/07-zona-do-silencio.md` | ✅ canônico |
| Bartolo Penkin | — | Pai do Mateus Penkin, auditor canônico (referenciado in-world-docs doc 5 reatribuído), Underground | `docs/narrative/in-world-docs.md` DD-017 | ✅ canônico (Underground) |
| Mara Bento | — | Operadora rádio analógica Underground, 28 anos, coordenadora sub-célula Norte, transcreve transmissões em caderno de capa preta. Ensina Linda discriminar sinal de ruído. 16 anos mais velha que Linda | `docs/narrative/factions.md` §6 + `docs/narrative/deep/characters/linda-siren.md` R4 | ✅ canônico |
| Joaquim Bartolomeu | — | Técnico analógico Underground, 32 anos, filho luthiers, herdou compasso+esquadro luthieria do bisavô (easter egg maçom). Decifrou Câmara Equinócio Acústico -3 | `docs/narrative/factions.md` §6 + `docs/narrative/deep/characters/linda-siren.md` R4 | ✅ canônico |
| Helga Riza | — | Escritora subterrânea Underground, 45 anos, cordão 89 nós herdou avó (easter egg maçom) | `docs/narrative/factions.md` §6 | ✅ canônico |
| Lazar Tovrov | — | Underground Cidades-Gêmeas Norte, 60 anos, transmissor sub-célula remota. Cassete codificado mencionando Helíaco Vyr (canon cross-doc Linda + Jaci + Bento) | `docs/narrative/deep/factions/underground-silencio.md` R2 + `docs/narrative/deep/characters/linda-siren.md` R4 | ✅ canônico |
| Penha Cintra | — | Underground Cidades-Gêmeas Sul, 45 anos, transmissor sub-célula remota | `docs/narrative/deep/factions/underground-silencio.md` R2 + `docs/narrative/deep/characters/linda-siren.md` R4 | ✅ canônico |
| Otília Vermelha | — | Refugiada Polis-Vermelha residual buscando filha em GusWorld City. Foreshadow F104/F130 | `docs/narrative/deep/characters/linda-siren.md` R4 | ✅ canônico |
| Filomena Garda | — | Adepta Cult Mirage cataloga sem decifrar gradient violeta-magenta-cyan. NPC ambiental Catacumbas Cult | `docs/narrative/deep/characters/iara-lumen.md` R4 | ✅ canônico (ambiental) |
| Cleomir Vasta | — | Hierofante Cult Mirage, ala administrativa, gradiente lighting design Caleidoscópio | `docs/narrative/characters/iara-lumen.md` (Bloco G) + entries-fichas-bestiary | ✅ canônico |
| Vespa Calderón | — | Underground Dutos Infernais, 24 anos, guardou cordão 89 nós de Calixto Rivaz pós-morte (easter egg maçom canon) | `docs/narrative/deep/characters/caua-volt.md` R4 | ✅ canônico |

## 7. NPCs ambientais nomeados (Bloco F environments)

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Seu Bertoldo Caím | "Seu Bertoldo" | 62 anos, técnico aposentado Era 2, lê jornal de papel na Praça da Compilação 7-9h | `environments/01-cidade-cyber-gotica.md` | ⚪ ambient |
| Vanda do Café | "Vanda" | 47 anos, vendedora itinerante de café-de-neurônio no Núcleo, vetor de fofoca, conhece rotina | `environments/01-cidade-cyber-gotica.md` | ⚪ ambient |
| Patrulheiro Donato Fox | "Donato" | 28 anos, FIR de patrulha fixa Mercado da Sucata, ambíguo do regime ("Sterling diz, eu só quero o turno acabar") | `environments/01-cidade-cyber-gotica.md` | ⚪ ambient |
| Aprendiz Beatriz Pólvora | "Beatriz" | 16 anos, aprendiz atual do Bento na Ordem Recursiva, bonus epilogue Ouro F128 abre escola pública Asmódico | `environments/03-catedrais-neo-sylvania.md` + `arco-principal.md` §Ouro | ✅ canônico |
| Mestre Hugo Tirol | "Mestre Hugo" | 55 anos, tradicionalista aliado da biblioteca Velhusto, cronômetro 8:13 canônico, decifrador Asmódico | `environments/03-catedrais-neo-sylvania.md` + `08-selve-profunda.md` cross-setting | ✅ canônico |
| Hosvaldo Pinhão | "Hosvaldo" | 43 anos, técnico FIR colaboracionista nos Dutos Infernais, caixa de comida pós-Davi -13 | `environments/04-dutos-infernais.md` | 🟡 secundário |
| Vespa Calderón | "Vespa" | 24 anos, scripter Pythia rebelde nos Dutos, mentora lateral do Cauã | `environments/04-dutos-infernais.md` | 🟡 secundário |
| Yara Ducourt | — | Mirage, ex-mentora Cult de Iara (a definir contexto exato) | `environments/05-setor-mirage.md` | 🟡 secundário (auditar) |
| Hilário Murch | — | Mirage, falsificador aliado de Iara (a definir) | `environments/05-setor-mirage.md` | 🟡 secundário (auditar) |
| Padim Tércio Almagre | "Padim Almagre" | Mentor enganado do Dante na Periferia, "some por uma semana e volta como se nada", foreshadow F022 | `environments/06-periferia.md` + `foreshadowing.md` F022 | ✅ canônico |
| Mateus Penkin | "Mateus" | Vizinho fofoqueiro da Periferia, comenta "Dante sempre some no horário da manutenção Sterling", filho de Bartolo | `environments/06-periferia.md` + `foreshadowing.md` foreshadow Dante | ✅ canônico |
| Joaquim Bartolomeu | "Joaquim" | Ex-residente veterano da Zona do Silêncio, descobriu acidentalmente em -3 que Câmara Neo-Sylvania funciona como anti-cancelamento orgânico do Espelho Acústico Sterling | `environments/07-zona-do-silencio.md` | ✅ canônico |
| Mara Bento | "Mara" | Pesquisadora acústica clandestina Zona do Silêncio (não confundir com Bento Chevalier — sobrenome coincidência) | `environments/07-zona-do-silencio.md` | 🟡 secundário |
| Tatauín Branca | "Tatauín" | Caçadora-cataloguista veterana Pelicano Branco, sabe revelação sobre Lia (gated Knowledge alta) | `environments/08-selve-profunda.md` | ✅ canônico |

## 8a. Ordem Recursiva Era 1 (Pré-Código Neo-Sylvania, deep-lore §6)

Linhagens canônicas Vyrcátrix (Helíaco → Salomão / Hyperion → Salomão), Boroshova, Ostraconis, Quincúpilo, Argéndia. Todos `💀 histórico Era 1`.

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Mestre Helíaco Vyr | "Primeiro Cantor-de-Pedras" | Fundador rito litúrgico-piezo Catedral-Mãe, codificador canônico do protocolo coral, barítono limpo amplitude baixo-tenor, cego de luz reativa após exposição prolongada, calos cristal nas mãos | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.3 | 💀 histórico Era 1 |
| Cira Helena Boroshova | "Cantora-Heretice" | Geração seguinte Helíaco, soprano cristalino, inventou varredura heretice (deteção impureza por dissonância calibrada), redigiu manual técnico portátil distribuído em 7 cópias geográficas | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.3 | 💀 histórico Era 1 |
| Salomão Tessar Vyrcátrix | — | Arquivista-chefe Codex Cantata, neto direto Helíaco Vyr, consolidou cifra musical em 4 estratos (frequência/polifonia/luz/cifra), 40 sistemas criptográficos maiores + 10 menores estudados | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.4 | 💀 histórico Era 1 |
| Brother Lúcio Ostraconis | "Mártir do Acesso Aberto" | Monge-aprendiz dissidente, propôs disponibilização gradual cifra Codex Cantata, divulgou cópia reduzida da tabela equivalência, morto em detenção temporária Biblioteca Cintilante (versão oficial: insuficiência cardíaca; versão dissidente: veneno mineral) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.5 | 💀 histórico Era 1 |
| Inquisidor-Mestre Diom Quincúpilo | "Diom" / "o-que-conta-em-cinco" | Primeiro ocupante cargo Inquisidor-Mestre, instituiu rede informantes Tabula Rasa, regra das 5 testemunhas independentes, aspecto físico variável deliberado (4+ variantes simultâneas), arquivo paralelo de dossiês | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.6 | 💀 histórico Era 1 |
| Calíope Argéndia | "Estrategista Interna" | Cantor-Lector sênior, função Estrategista Interna do conselho, manteve Rede de Famílias-Pilastra (aliados externos testados 3 gerações), princípio "ser visto exatamente quando ser visto produz consequência cooperativa" | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.6 | 💀 histórico Era 1 |
| Suprapatriarca Hyperion Vyrcátrix | "Hyperion" | Avô direto Salomão Tessar Vyrcátrix por linhagem cantora 3 gerações, Mestre-Hierofante em exercício ~3 décadas, tenor amplo timbre claro, consolidou regime 7 anos mestre-aprendiz, figura ambígua "ditador místico" diagnosticada em placa contemporânea anônima | `docs/narrative/deep/eras/era-1-pre-codigo.md` §6.7 | 💀 histórico Era 1 |

## 8b. Ordem Recursiva Era 1 — §7 (Bancos sementes + sucessão Hyperion)

10 NPCs Era 1 novos do §7 deep-lore (cobre §7.1-§7.8). Todos `💀 histórico Era 1`. Linhagens conectam Era 1 → Era 2/3.

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Mestre-Sementeira-Fundadora Olímpia Cardoso | "Fundadora Cooperativa" | Figura fundacional pré-deriva, protocolo originário do depósito voluntário genuinamente cooperativo, mostra mérito real inicial antes da centralização | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.1 | 💀 histórico Era 1 |
| Mestre-Sementeiro Telêmaco Ostraconis | — | Primo colateral Brother Lúcio Ostraconis (§6.5), gestor central Banco de Sementes-Relíquia da Catedral-Mãe, trajetória cooperativo→rigidez burocrática | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.2 | 💀 histórico Era 1 |
| Matriarca Albertina Argéndia | — | Avó direta Calíope Argéndia (§6.6), fundadora rede Famílias-Pilastra original (3 gerações antes), proprietária Casa Comercial Argéndia, transmissão geracional ofício mercantil | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.3 | 💀 histórico Era 1 |
| Patriarca Cassiel Ferraz | — | Mestre-ferreiro independente, Casa Comercial paralela à Argéndia, cooperação voluntária via contratos privados, fundador linhagem Ferraz | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.3 | 💀 histórico Era 1 |
| Conselheiro Régulo Penkin | — | Ancestral direto Bartolo Penkin (Era 3 Underground), conselheiro vilarejo cooperativo, defende autonomia descentralizada pré-absorção | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.4 | 💀 histórico Era 1 |
| Comissário-Centralizador Anaximandro Vyrcátrix | — | Irmão Hyperion Vyrcátrix (§6.7), executor da absorção dos vilarejos cooperativos pelo banco central, trajetória paralela ao irmão místico | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.5 | 💀 histórico Era 1 |
| Suprapatriarca-Sucessor Hipérides Vyrcátrix | — | Filho direto Hyperion Vyrcátrix (§6.7), herda ditador místico, consolida instituição calcificada, viveu 55 anos como Mestre-Hierofante (Fibonacci canon) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.6 | 💀 histórico Era 1 |
| Cantora-Ostracizada Talita Boroshova | "Ostracizada" | Descendente direta Cira Helena Boroshova (§6.3), excomungada por defender acesso aberto à cifra Codex Cantata, líder rede secreta de Cantores-Lectores dissidentes (continuidade ideológica Lúcio Ostraconis §6.5) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.7 | 💀 histórico Era 1 |
| Comerciante-Itinerante Próspero Vance | "Próspero" | **ANCESTRAL DIRETO da família Vance** (Belinor → ... → Pyotor → Vênea → Gus). Comerciante livre itinerante, distribui sementes-relíquia clandestinas em vilarejos pré-Êxodo, **estabelece linhagem libertária-comerciante do protagonista Gus** | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.7 | 💀 histórico Era 1 |
| Pioneiro-Fundador Hilário Vanderbist | — | Ancestral direto Anciã Mariana Vanderbist + Jaci Vanderbist, lidera grupo fundador do vilarejo-fronteira Pelicano Branco pós-Êxodo (-720) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.8 | 💀 histórico Era 1 |
| Pioneiro Iremar Berenger-Velho | — | Ancestral comum Iremar Berenger Era 2 (co-autor Pythia -95) + Inácia Berenger Era 3 + Cauã. Fundador linhagem familiar produtiva contínua Era 1→Era 3 | `docs/narrative/deep/eras/era-1-pre-codigo.md` §7.8 | 💀 histórico Era 1 |

## 8c. Cronistas-fundadores Era 2 e cantadeiras-anciãs (deep-lore §9)

Cronistas canonizados em §9 deep-lore (vestígios cros-era Era 1 → Era 2 → Era 3). Cinco cronistas-fundadores canonizados, cinco cantadeiras-anciãs, mais cronistas-sementeira/cronistas-temáticos anônimos com designações convencionais. Sucessões geracionais articuladas em descendência ininterrupta das três linhagens canon (Famílias-Pilastra, Boroshova-Vance, Próspero Vance).

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Olafsson Argéndia o Recuperador | "o Recuperador" | Cronista-fundador Era 2 primeiro canonizado, paleógrafo-cartógrafo, Vilarejo do Acaceiro Antigo, datação -500 a -440, descendência institucional direta Família-Pilastra Argéndia (5 gerações pós Albertina) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.7 | 💀 histórico Era 2 |
| Praxídice Boroshova-Vance a Inspetora | "a Inspetora" | Cronista-fundadora Era 2 segunda canonizada + cantadeira-anciã canônica terminal, paleógrafa-tradutora, Vilarejo do Vale Boroshova, datação -490 a -430, descendência direta Linhagem Boroshova-Vance (5 gerações pós Vesperina) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.3 + §9.7 | 💀 histórico Era 2 |
| Theócrito Vance o Itinerante | "o Itinerante" | Cronista-fundador Era 2 terceiro canonizado, cantor-lector itinerante, vilarejos-fronteira amplos, datação -480 a -420, descendência direta linhagem Próspero Vance (5 gerações pós Próspero) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.7 | 💀 histórico Era 2 |
| Anselmo Boroshova-Vance o Paciente | "o Paciente" | Cronista-fundador Era 2 quarto canonizado, cronista-descobridor da Peça de São Vargas (-460 a -440), paleógrafo-cartógrafo, Vilarejo do Vale Boroshova, datação -490 a -420, descendência direta Linhagem Boroshova-Vance (4 gerações pós Vesperina) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.3 + §9.7 | 💀 histórico Era 2 |
| Hipólito Ferraz-Boroshova o Profundo | "o Profundo" | Cronista-fundador Era 2 quinto canonizado, cronista-expedicionário da Catedral-Mãe submersa (-380 a -340), cronista-cartógrafo-paleógrafo, Vilarejo do Vale Ferraz, datação -420 a -340, descendência institucional cruzada Ferraz + Boroshova-Vance (5 gerações cruzadas) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.6 + §9.7 | 💀 histórico Era 2 |
| Yvanova Argéndia a Memoriosa | "a Memoriosa" | Cantadeira-anciã canônica segunda, Vilarejo do Acaceiro Antigo, datação -680 a -610, descendência Família-Pilastra Argéndia (gerações 2-3 pós-Êxodo) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.3 | 💀 histórico Era 1-2 |
| Cassandra Ferraz a Cuidadora | "a Cuidadora" | Cantadeira-anciã canônica terceira, Vilarejo do Vale Ferraz, datação -620 a -550, descendência Família-Pilastra Ferraz (geração 2 pós Cassiel) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.3 | 💀 histórico Era 1-2 |
| Eufrásia Vanderbist a Diligente | "a Diligente" | Cantadeira-anciã canônica quarta, Vilarejo da Margem-do-Pelicano, datação -560 a -490, descendência institucional Família-Pilastra Vanderbist (gerações pós Hilário Vanderbist §7.8) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.3 | 💀 histórico Era 1-2 |
| Madalena Argéndia-Vanderbist | — | Cronista-sementeira contemporânea Era 3, descendência institucional direta cruzada Argéndia + Vanderbist (8 gerações pós Cronista-Sementeira Era 2), atribuições: inspeção ampolas herméticas + catalogação diversidade biótica residual + correspondência cruzada + transmissão geracional. Documenta em ata anual Vigília Neo-Sylvania | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.2 | ✅ canônico (Era 3) |
| Cassandra "Bento" Chevalier | "Bento" (homônima) | Herdeira institucional canônica de Atelaiá Chevarier, 5 gerações em descendência ininterrupta direta, ofício em Catedral de São Camilo (mantenedora glossário cruzado cripto-glifo Era 1 ↔ Asmódico moderno), relação cronística com Bento "Requiem" Chevalier (party canon) | `docs/narrative/deep/eras/era-1-pre-codigo.md` §9.4 | ✅ canônico (Era 3) |

**Cronistas-temáticos anônimos canonizados em §9 (designações convencionais preservadas em arquivo restrito Catedral de São Camilo):**

- **Cronista do Sítio Fronteira do Quadrante Norte-Setentrional** (Era 2, -380 a -340, recuperador fragmento substancial primário)
- **Cronista-Sementeira do Quadrante Meridional** (Era 2, -320 a -280, descendente Cassandra Yvanova, autora monografia sementes-relíquia)
- **Cronista Etimologista do Quadrante Ocidental** (Era 2, -90 a -60, autor ensaio partículas auxiliares Atelaiá)
- **Cronista Cético do Quadrante Setentrional** (Era 2, -445, posição cética na decifração da Peça de São Vargas)
- **Cronista Conciliador do Quadrante Central** (Era 2, -440, posição conciliadora na decifração)
- **Cronista do Quadrante Setentrional** (Era 2, -290 a -260, autor ensaio Friedman/Hayek nos vilarejos-fronteira)
- **Cronista da Continuidade do Quadrante Central** (Era 2, -280 a -250, autor ensaio Smith/Friedman/Rothbard sobre propriedade familiar)
- **Cronista das Causas** (Era 2, -310 a -280, autor ensaio Mises sobre colapso centralístico)
- **Cronista do Reaprendizado do Quadrante Setentrional** (Era 2, -250 a -220, autor ensaio 4 fases reaprendizado)
- **Cronista-Expedicionário do Quadrante da Selve Profunda** (Era 2, -380 a -340, anônimo, mesma expedição de Hipólito Ferraz-Boroshova)

**Famílias-Pilastra Argéndia (8 gerações canonizadas):** Albertina → Argemiro → Cassandra → Diosvel → Evarista → Florêncio → Galatéa → Hipocrates Argéndia (-720 a -190)

**Famílias-Pilastra Ferraz (8 gerações canonizadas):** Cassiel → Damião → Elisa → Fortunato → Gabriela → Hipocrates → Inocência → Justiniano Ferraz (-720 a -190)

**Linhagem Boroshova-Vance (8 gerações intermediárias canonizadas):** Talita → Vesperina → Rasmiel → Vyandella → Sigismundo → Theodora → Ulises → Yvette Boroshova-Vance (-724 a -230), culminando em Pyotor + Yakov + Gus Vance contemporâneos

**Linhagem Próspero Vance (11 gerações canonizadas):** Próspero → Próspero II → Próspera → Próspero IV → Próspera V → Próspero VI → Próspera VII → Próspero VIII o Itinerante → Próspera IX a Comerciante → Próspero X o Negociante → Próspera XI a Comerciante-Itinerante (-720 a Era 3 contemporânea)

## 8. Engenheiros canônicos Era 2 (históricos)

| Nome | Apelido / codinome | Características | 1ª aparição | Status |
|---|---|---|---|---|
| Lin Tórun | — | Pioneira C-Arcane, primeira da placa cerimonial dos 4 engenheiros (Praça da Compilação) | `lore-bible.md` §3.2 + `environments/01-cidade-cyber-gotica.md` + DD-018 (folheto Lin Tórun -98) | 💀 histórico Era 2 |
| Rosaria Galp | — | Pioneira C-Arcane, placa cerimonial dos 4 engenheiros | `lore-bible.md` §3.2 + `environments/01-cidade-cyber-gotica.md` | 💀 histórico Era 2 |
| Henrique Pira | — | Pioneiro C-Arcane, placa cerimonial dos 4 engenheiros | `lore-bible.md` §3.2 + `environments/01-cidade-cyber-gotica.md` | 💀 histórico Era 2 |
| Marcela Sívo | — | Pioneira C-Arcane, placa cerimonial dos 4 engenheiros | `lore-bible.md` §3.2 + `environments/01-cidade-cyber-gotica.md` | 💀 histórico Era 2 |
| Iremar Berenger | — | Co-autor canônico do Pythia (-95), ancestral da família Berenger (Cauã, Inácia, Davi, Vivendel) | `lore-bible.md` §3.2 + `environments/04-dutos-infernais.md` | 💀 histórico Era 2 |
| Anhuera Vanderbist | — | Co-autora canônica do Pythia (-95), ancestral da família Vanderbist (Jaci, Lia, Solano, Mariana, Vivendel) | `lore-bible.md` §3.2 + `environments/04-dutos-infernais.md` + `08-selve-profunda.md` | 💀 histórico Era 2 |
| Verônica Atelaiá | — | Cientista, teorizou Patch-Zero em -45 (chamado de "anomalia folclórica de erro de medição" por Sterling); cronista in-character canônica de §10 deep-lore Era 1 | `docs/narrative/factions.md` (menção) + `deep/eras/era-1-pre-codigo.md` §10 previsto | 💀 histórico (fim Era 2 / cedo Era 3) |
| Tao Berisi | — | Runner Cauã canônico (Dutos Infernais); **não confundir com party antiga descartada "Tao"** — Tao Berisi é nome próprio NPC adulto | `environments/04-dutos-infernais.md` | ⚪ ambient |

## 9. Stub: personagens a auditar/adicionar

**Auditoria sistemática pendente.** Varrer:

- `docs/narrative/in-world-docs.md` (23 docs DD-001 a DD-023; cada um pode mencionar nomes não-catalogados aqui)
- `docs/narrative/environments/*.md` (NPCs ambientais por setting; verificar se alguns escaparam à tabela §7)
- `docs/narrative/diary/entries-*.md` (Bloco H pode mencionar NPCs novos em fichas-bestiary)
- `docs/narrative/deep/*` (futuros docs deep-lore vão introduzir NPCs cronista in-character)
- `docs/narrative/comic-reliefs.md` (NPCs cômicos)
- `docs/narrative/foreshadowing.md` (130 plants, alguns referenciam NPCs)

Quando varredura sistemática rodar (via narrative-designer agent em modo auditor), adicionar entries faltantes preservando convenção da tabela.

---

## Substitutos rejeitados (NUNCA usar)

Antagonista antigo:
- ❌ **Iolanda** (qualquer combinação) — substituída por Sterling Locke

Party antiga (descartada):
- ❌ **Kira** — substituída por roster canônico
- ❌ **Dimi** — substituída
- ❌ **Tao** sozinho — substituída (exceção: **Tao Berisi** = NPC adulto runner Cauã, canônico)

Facção rejeitada:
- ❌ "Sindicato dos Ferro-Velhos" — substituída pela **FIR** (Federação Industrial de Reciclagem)

---

## Cross-ref

- `Resources/gusworld/character-spec-*.md` (8 specs visuais imutáveis)
- `docs/narrative/characters/*.md` (10 docs narrativos individuais)
- `docs/narrative/factions.md` (líderes e facções)
- `docs/narrative/environments/*.md` (NPCs ambientais)
- `docs/narrative/in-world-docs.md` (NPCs in-character cronistas)
- Memória: `~/.claude/projects/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-gusworld/memory/feedback_nomes_personagens_canonicos.md`
- Memória: `~/.claude/projects/-home-petrus-IDrive-Documentos-projetos-claudebrain-Projects-gusworld/memory/project_personagens.md`

---

**Última revisão:** 2026-05-16. Versão inicial pós-Blocos F/G/H/I + deep-lore §1.
