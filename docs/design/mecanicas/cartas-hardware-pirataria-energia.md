# Cartas: hardware, energia e pirataria

> **Status:** BRAINSTORM EM ANDAMENTO (líder, 2026-07-18). Decisões fechadas via AskUserQuestion capturadas fielmente. Ainda ABERTO: efeitos exatos do vírus, o mercado negro (lugar/vendedor), números (delegar ao `economy-designer`), spec de implementação (delegar a `gameplay_engineer`/`backend-engineer`). NÃO é canon fechado até o líder revisar o doc consolidado.
>
> **Origem:** derivou do "vírus do Dante" (guarda-chuva Faraday, ver `docs/design/mundo-topologia.md` / brainstorm-backlog). O líder escalou o evento pontual (Dante injeta vírus na carta Faraday à noite) para um SISTEMA geral de cartas modificadas / piratas / infectadas / com bateria.
>
> **Cross-ref:** Pillar 1 (magia = software, "scripts rúnicos compilados"), Pillar 3 (triângulo hardware / Tavus-Drive), memórias `project_gus_eixo_compilado_interpretado` (compilado=rápido=BOM / interpretado=lento=RUIM), `reference_deck_mao_sistema` (especiais únicas × comuns bounded), `project_axiologia_canonica` (austríaco, anti-compadrio), `project_terminologia`, roster de análogos (Volta, Bastiat) em `docs/design/roster-analogos/`.

---

## 1. Enquadramento moral (one-way-door, decisão do líder)

**Mercado livre legítimo × a FRAUDE é que é o mal.** Comprar/modificar carta no mercado cinza NÃO é imoral — pode até ser resistência ao monopólio-compadrio do Sterling/DRE. O mal é:

- a **fraude** (vírus escondido, como o Dante; vender pirata como original);
- o **risco de fonte podre não avaliado** — o custo que **não se vê** (o **Bastiat** do roster é o professor diegético disso).

Lição diegética: **caveat emptor / avalie a fonte**, não "pirataria = pecado". Alinha com o austríaco anti-compadrio e com "faz-se dinheiro com trabalho".

## 2. Três tipos de carta fora do canal oficial

1. **Modificada (mod / homebrew):** carta base tunada (buff, efeito extra, custo menor). Legítima em si (você mexe no que é seu), mas **instável se mal "compilada"** — o mod porco roda mal e drena bateria mais rápido (custo do eixo compilado/interpretado).
2. **Clone pirata:** cópia não-autorizada de uma carta; degradada + o vetor mais comum de vírus.
3. **Infectada (vírus):** carrega payload oculto malicioso. É a **fraude** — o mal do enquadramento.

O **mercado negro** é o *lugar* onde os três circulam, misturados com usados legítimos honestos.

### Clone-falso de especial
As cartas especiais são **únicas** (uma no jogo). Mas existe **imitação pirata** de especial: parece o poder raro, é falsificação degradada/instável/possível-vírus, e **NÃO é a única real** (não conta como a especial). Não quebra a unicidade e ensina "a cópia não é o original". O Sterling pode usar falsos; um vendedor pode te enganar com um "Faraday" pirata.

## 3. Física da carta (o que prova original × pirata × homebrew)

| | **Original (oficial)** | **Vazia (homebrew)** | **Pirata (clone)** |
|---|---|---|---|
| Memória | ROM pura | **EPROM** de baixa qualidade | ROM |
| Chip | excelente | baixíssima qualidade | — |
| Capacidade | vários **MR** | poucos **kR** (só conjuro pequeno/comum) | — |
| Upload | de fábrica | **muito lento** | 1x definitivo |
| Conector | **nenhum** | **RSB** externo, visível | **RSB interno escondido = prova cabal de pirataria** |
| Mexer no software | **queima na hora** (anti-tamper físico) | regravável, mas **desgasta até queimar** | travado após gravar |
| Sinal externo | — | — | diferenças visuais sutis (poucos notam) |

**Amarras:** ROM de fábrica = compilado/rápido = BOM; EPROM lenta = lado interpretado/lento. O "queima se mexer" é anti-tamper físico, irmão do save-crypto. O conector interno é a evidência forense da fraude (a pirataria é cinza; a *ocultação* denuncia).

**Homebrew COPIA, não cria (líder, 2026-07-18 — preserva o anti-pillar):** gravar uma carta vazia via RSB é **duplicar** um conjuro que já existe (pirataria = cópia ilegal), **não** inventar um efeito novo numa bancada. Isso mantém intacto o anti-pillar do `gdd.md`/`cartas-technomagik.md` ("cartas são obtidas, não craftadas; o jogador nunca monta carta nova numa bancada") e casa com **von Neumann = replicação**. O jogador nunca cria conjuro inédito — ele copia um existente para um meio pirata barato.

## 4. Unidade de memória (terminologia canônica)

- **runa (r)** = bit · **Runa (R)** = Byte · **1 R = 8 r** (base 8, igual ao real).
- Prefixos decimais **kR / MR / GR / TR** (×1000) e binários **kiR / MiR / GiR / TiR** (×1024) — o par kB/kiB do mundo real.
- Ancorada no Pillar 1 ("feitiços são scripts **rúnicos** compilados").

**Conector = RSB (Runic Serial Bus):** o análogo diegético do USB (Universal Serial Bus), trocando "Universal" por "Runic". A carta vazia expõe um RSB externo; a pirata esconde um RSB interno (§3).

**Regra de nomenclatura (líder, 2026-07-18):** **termos de TI são universais em inglês** (RSB, ROM, EPROM, bit, Byte, CPU…). O giro diegético entra no CONCEITO/adjetivo quando cabe (Universal→Runic; a unidade própria runa/Runa), nunca em traduzir a sigla pro pt-br.

**Gravador = terminal de bancada FIXO:** o conjuro é escrito/compilado na carta vazia num "computador do jogo" de bancada (interface de **terminal**, casa com o terminal-glitch canon e os logs de combate), conectado via **RSB**. Fixo (não em campo) — combina com o upload lento da EPROM ruim. Ficam **SÓ nas oficinas do mercado negro / ferro-velho** (§14): gravar homebrew é atividade de submundo maker; o Gus **não** tem bancada em casa. Reforça que homebrew é legítimo, mas vive no cinza.

## 5. Energia: baterias CR2032

- A carta tem **X usos** porque tem **bateria**; esgotada, a carta fica inerte até trocar. Diegese para o limite de usos.
- Um **recurso Y** da carta drena mais rápido (o efeito premium consome mais fundo).
- **CR2032** = a moeda de lítio, literalmente a bateria de BIOS/placa-mãe ("o coração que guarda o estado") — casa com carta = hardware.
- **Degradação:** a bateria degrada com uso e ciclos de carga.
- **Especiais:** bateria **selada / de maior capacidade** — mais confiável (condiz com "especiais protegidas"), mas ainda exige cuidado no endgame (amarra com o clímax sem-save/PEM).

### Troca e recarga
- **Cidade:** grátis — abrir inventário, pôr carregada no lugar da usada.
- **Estação de recarga:** te dá uma bateria **NOVA carregada** e cobra pela tua velha; o preço (**1,2x–2x** o de uma recarga) varia com a **degradação** da que você entrega.
- **Trade-off:** recarregar você mesmo custa *tempo de espera*; comprar já-carregada custa mais crédito.
- **In-battle (arena):** trocar (se tiver carregada) custa **1 turno + AP** (número → `economy-designer`).

### Descarte
- Bateria descarregada **ocupa espaço** no inventário e **não pode ser jogada fora** — **crime ambiental**, tratado com **sátira leve do excesso regulatório** (registro LucasArts; a megacidade te obriga a carregar teu próprio lixo tóxico). Só vender ou trocar.

### Itens de energia (novos)
- **Carregador solar** — recarrega baterias passivamente (evita depender de estação/compra).
- **Powerbank** com visor LED de carga própria restante.
- **Carregador rápido** de bateria.
- **Medidor (voltímetro)** — mostra carga atual e degradação **em volts**; comprável OU adquirido na **missão do Volta**.
- **Ghost do Volta** = o **tutor** de todo o sistema de energia (baterias, ciclos, degradação, recarga). Casa com o roster (cada análogo ensina sua área; Volta = a pilha).

### Bateria pirata / genérica
Existe: mais barata, mas **mente sobre a carga**, morre cedo, pode **danificar a carta** ou ser vetor de vírus. A fraude do enquadramento no nível do hardware — caveat emptor também nas pilhas.

## 6. Costuras entre software e hardware
- **Mod porco** → drena bateria mais rápido (custo do "compilado ruim").
- **Vírus** → pode sugar a bateria escondido (malware que consome energia).
- **Bateria pirata** → a fraude no nível do hardware.

## 7. Tutores diegéticos (análogos do roster, não NPCs novos)

Três análogos do roster ensinam o sistema, cada um na sua especialidade real (nenhum NPC criado só pra isso):

- **von Neumann** — o hardware da carta: **memória** (ROM/EPROM, capacidade em Runa, arquitetura stored-program) e **clonagem/pirataria** (a auto-replicação é a carta "Construtor Universal" dele; quem entende de cópia explica a pirataria). Reforça a amarra canon **von Neumann ↔ linhagem Neumann/Óxido** (Linda "Siren" Neumann) e a Óxido = "segurança opaca" (anti-tamper, o falso a detectar).
- **Turing** — a parte **forense/cripto**: detectar o vírus, ler o **conector interno escondido**, quebrar a cifra do falso (Enigma ↔ cripto-glifos).
- **Volta** — a **energia**: baterias, ciclos, degradação, recarga (ghost-tutor; ver §5).

## 8. Vírus de carta: payloads (efeitos)

Todo vírus é **oculto** — o jogador não sabe que pegou até se manifestar (ou até o Turing detectar). Tipos (podem coexistir, raridades/perigos diferentes):

- **Sabotador de combate (logic bomb):** a carta falha ou vira contra você no pior momento — o golpe do Dante generalizado. Dispara numa condição (turno crítico, chefe, HP baixo).
- **Backdoor / spyware:** a carta REPORTA ao inimigo; o Sterling "vê" tua mão/deck/posição e te antecipa. Explica diegéticamente como o vilão sempre sabe teus movimentos.
- **Worm de deck:** espalha pras outras cartas do deck em cadeia E **deixa a carta LENTA mesmo se ela for compilada/ROM** (nem a original escapa da lentidão — degrada performance por baixo). Cria pressão de contenção (isolar antes que contamine tudo).
- **Falso-benigno (isca Bastiat):** parece dar um BUFF ("carta turbinada!"), mas cobra um custo oculto retardado (dreno de bateria, HP, crédito ou debuff). O ensino do Bastiat puro.
- **Adware Sterling:** propaganda **indesejada e não solicitada** das indústrias Sterling. Fluxo ao acionar o feitiço: (1) a propaganda toma a tela (ex.: *"Os dispositivos Sterling são melhores pois têm mais qualidade. Conte conosco!"*); (2) espera alguns segundos; (3) o **X de fechar** aparece no topo direito, pequeno e quase invisível; (4) o personagem fecha; (5) só então o efeito da carta ocorre. Adware clássico — e sátira do corporativo-compadrio Sterling.
- **Zip-bomb (bomba de descompressão):** a carta parece pequena e inofensiva (poucos **kR**), mas ao soltar o feitiço ela **incha** e estoura a memória — trava/corrompe o deck, entope a memória (impede usar outras cartas no turno) ou consome a bateria de uma vez. Casa direto com o sistema de memória em Runa (§4). *(Ideia do Gus Dragon, playtester, 2026-07-18.)*

## 9. Como se pega vírus (vetores de contaminação)

- **Ao soltar o feitiço de uma carta contaminada:** probabilidade **X%** de contágio em TRÊS direções — (a) o **deck do inimigo no encontro** (o vírus vira **arma de duplo-gume**: pode sabotar o oponente, mas espalha nos dois sentidos); (b) o **ecossistema do mundo** (cartas em circulação — mercado negro, NPCs, trocas — vão ficando contaminadas; herda-se o vírus ao comprar/trocar de fonte suja); (c) o **próprio deck** (worm interno, §8). Single-player, mas com ecossistema de contágio.
- **Feitiço inimigo "sem efeito aparente":** certos ataques inimigos parecem não fazer nada — o personagem acha que nada aconteceu — mas infectaram (silencioso; casa com backdoor).
- **Databank pirata adulterado:** comprar databank pirata contaminado infecta.
- **Cartas grátis com adware** (§10): opt-in consciente.

**Risco de contaminação por tipo de carta** (`[calcular]` → `economy-designer`):

| Tipo | Risco |
|---|---|
| Especial (original) | **0** (não contamina) |
| Comum (original) | `[calcular]` — a ROM **não é reescrita**, mas o vírus age como **rootkit** (residente em runtime) |
| Pirata especial | `[calcular]` |
| Pirata comum | `[calcular]` |
| **Homebrew (EPROM)** | `[calcular]` — **a mais vulnerável** (mais fácil pegar, mais difícil não ser afetada) |

Ordem de vulnerabilidade: **especial (0) < comum < pirata especial < pirata comum ≪ homebrew.**

**Especiais são IMUNES a vírus comum** (risco 0 — ROM de qualidade suprema). **O vírus do Dante no clímax é a exceção que confirma a regra:** não é infecção acidental, é uma **arma ESPECIAL fabricada pelas indústrias Sterling sob medida** para furar e neutralizar a carta **Gaiola de Faraday** especificamente; o Dante (acesso logístico) só a instala à noite. Uma especial só cai por uma arma dedicada, nunca por descuido — o que reforça o peso da traição e do poder industrial da Sterling.

**Apelo dramático (líder, 2026-07-18):** a Gaiola de Faraday é provavelmente a **primeira carta especial** do jogador (obtida cedo, no interior-faraday) e o acompanha o **jogo inteiro** — a especial em que ele mais confia e a que mais se apega. É exatamente ELA que o vírus-arma da Sterling mata no clímax. A carta-companheira "morre" na hora mais crítica: a traição do Dante dói mais porque tira do jogador o que ele tinha de mais familiar. Reforça a decisão de obtê-la cedo (§ guarda-chuva Faraday).

**Destino da carta no clímax (líder, 2026-07-18 — resolve AMB-DADOS-02):** a arma da Sterling **QUEIMA** a Faraday (ela "morre" de verdade — `is_burned_out`, não uma infecção curável comum). O **único** jeito de recuperá-la é o branch de **redenção do Dante** (ele, por ação, remove o vírus que plantou e restaura a carta). Dá peso enorme à escolha de redimir: salva a carta-companheira E o traidor de uma vez. Reconcilia com o "a redenção = remover o vírus" do guarda-chuva Faraday.

## 10. Cartas grátis (bônus de compra) e adware

- **Cartas grátis de bônus de compra** vêm **SEMPRE com adware** (§8), **avisado** ao personagem — ele concorda ou não (opt-in consciente).
- Pra se livrar da carta: **só no ferro-velho**. Não pode no lixo normal — descartar eletrônico em qualquer lugar é **crime ambiental** (mesmo raciocínio das baterias, §5).

## 11. Cura / antivírus

- Cartas contaminadas **não têm antivírus** de prateleira.
- O **Turing** faz o **diagnóstico** (avisa que a carta está infectada / que o falso é falso, antes de você usar) e oferece uma **cura PARCIAL ARRISCADA**: tenta remover o vírus com **X% de sucesso** e **X% de queimar** o chipset na tentativa (`[calcular]` → balance). É uma aposta consciente do jogador.
- Fora dessa aposta, a carta infectada é praticamente irrecuperável — **conviver, arriscar a remoção (e talvez queimar), ou descartar no ferro-velho**.

## 12. Arco econômico = o Bastiat do jogo inteiro ("o que se vê e o que não se vê")

- **Começo do jogo** (pouco dinheiro): é mais fácil adquirir **pirata** que original — tentador, barato, acessível.
- **Ao longo do jogo** (mais dinheiro, ganho por trabalho): passa a poder comprar **produtos de mais qualidade** (original confiável).
- É a lição do **Bastiat** encarnada na progressão inteira: o barato sai caro (vírus, instabilidade, adware); conforme prospera pelo trabalho honesto, o jogador migra do pirata pro confiável. Ensino diegético central, alinhado à axiologia canônica.

## 13. Cartas e features específicas (desta onda)

### Carta `urandom` (pirata × original) — ideia do Gus Dragon
Carta-caos baseada no `/dev/random` do Linux:
- **Efeito:** ao usar, sorteia o efeito de **qualquer carta do teu deck (incluindo especiais)**, podendo ser bom ou ruim, no **caster ou no inimigo** — totalmente imprevisível.
- **Ponderação inversa:** quanto mais forte o efeito, **menor a probabilidade** de sair (a maioria é fraca; os fortes são jackpots raros; puxar o efeito de uma especial é o jackpot mais raro de todos).
- **Duas versões (pirata × original):**
  - **Pirata** — achada no **mercado negro** (caótica, instável, muito azar — mais chance de cair no próprio caster / efeito ruim). A instabilidade é temática (fonte duvidosa, "nem sei o que faz"). Casa com o sistema de pirataria (§1–3).
  - **Original** — carta extra-especial **ÚNICA**, o **prêmio de completar a RunaDex** (§13): mais poderosa e com **ponderação generosa** (a "de verdade"). O tema pirata×original que criamos hoje, aplicado à própria carta do Gus.
- **Nome:** `urandom` (variante non-blocking do `/dev/random`; nome próprio arcano, velado o bastante pra não gritar "Linux"). Pesos/probabilidades das duas versões → `economy-designer`/balance.

### RunaDex (índice de coleção, estilo Pokédex) — ideia do líder
Álbum/índice inspirado no Pokédex (o público-alvo ama; casa com "Pokémon na apresentação de batalha"). Nome amarra com a terminologia **runa** (§4): RunaDex = índice de Runas/cartas. **Três estados de entrada:**
- **Vista** (só viu, ex.: um inimigo usou): **silhueta / "???"** — dá senso de caça e mostra o que falta descobrir.
- **Tida** (já possuiu): **face revelada**.
- **No deck ativo:** face revelada + **moldura de neon azul**.

**Acesso:** pelos **óculos táticos** do Gus (diegético — HUD/scan, hardware-âncora do Pillar 3) **E** atalho no **menu de pausa** (conveniência).
**Completude:** mostra contagem de coleção (X/Y descobertas) e **completar tudo dá a `urandom` ORIGINAL** (carta extra-especial única, suprema — ver §13; ideia do Gus Dragon). A pirata você acha no mercado negro; a original é o troféu da coleção completa.

## 14. Mercado negro

### Estrutura (três camadas, todas)
- **Coração nos Ferrovelhos** (Periferia Industrial, setting #4, à sombra da **FIR**): o ecossistema de sucata onde convive a **tríade** — FIR oficial (monopólio-compadrio, caro) × **Sucata Honesta** (comércio livre honesto) × mercado negro (barato/livre/arriscado). Amarra Periferia + FIR + Dante + o descarte legal (§5/§10).
- **Rede oculta espalhada:** vendedores clandestinos em várias áreas do mundo, acessados por senha/reputação/contato — o mercado negro não é um lugar só.
- **Bazar noturno dedicado:** um hub concentrado e memorável.

### Vendedores
- **Vários, com REPUTAÇÕES diferentes** (confiáveis, golpistas, caóticos): a lição do **Bastiat** virada mecânica — escolher de quem comprar É o jogo. Cabe um vendedor-âncora recorrente entre eles pra dar personalidade/humor.
- **Um contato ligado ao Dante / FIR:** fonte perigosa com peso narrativo (amarra o traidor); pode destravar mercadoria específica ou ser uma armadilha.

### Enquadramento (recap §1)
O mercado negro **não** é o vilão — é resistência ao monopólio-compadrio da FIR/Sterling. O mal é a **fraude** (vírus, vender pirata como original) e a fonte podre não avaliada. O ferro-velho é também o **único descarte legal** de eletrônico/bateria (§5/§10).

### Avaliar a fonte (a lição Bastiat jogável)
Duas camadas que se combinam:
- **Reputação (macro, o vendedor):** a *taxa de confiabilidade* dele, **não binária** — um pode ser ~90% honesto, outro ~40%, outro golpista assumido. **Descoberta por:** (a) **experiência própria** (comprou ruim → o jogo lembra que aquele vendedor te passou a perna); (b) **fofoca de NPCs** (comentam quem é de confiança / quem é golpista → dá pra evitar golpe ANTES de cair). Orgânico e social, sem tela de estrelas.
- **Sinais/preço (micro, o item):** mesmo num vendedor de boa fama, *aquele item* pode ter preço bom demais / sinal sutil de pirata. Caveat emptor — "o que não se vê" mora no bom-demais-pra-ser-verdade.
- As **ferramentas (voltímetro/Turing) NÃO revelam antes de comprar** (senão matariam o risco) — são diagnóstico/cura **depois** (§11).

---

## Pontos ABERTOS (retomar aqui)
- [ ] **Números** → `economy-designer` (preço de recarga, AP da troca in-battle, drain rate, capacidade kR/MR, vida de ciclos, **X% e a tabela de risco de contaminação da §9**, preço pirata vs original ao longo do arco).
- [ ] **Spec de implementação** → `gameplay_engineer` (usos/bateria, estados de carta, vírus/adware) + `backend-engineer` (modelo de dados: tipo, memória, bateria, integridade, flag de infecção).
- [ ] Canonizar no doc de mecânicas + refletir em `cartas-technomagik.md` / terminologia quando o líder aprovar.
