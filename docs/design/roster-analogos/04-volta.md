# Análogo 04: Volta (Fase B do roster)

> **STATUS: PROPOSTA** (Fase B do roster de análogos, item `LORE-ORIGEM-MULTIVERSO`, cluster "Eletromagnetismo/energia"). Aguarda aprovação do criador. Segue o TEMPLATE validado no piloto `01-faraday.md`, de `docs/design/brainstorm-backlog.md` #1. Não é canon até aprovação; a prosa canônica final (quando entrar no deep-lore) vai via narrative-writer.

## 1. Figura real

**Alessandro Giuseppe Antonio Anastasio Volta** (18 de fevereiro de 1745, Como, Lombardia, 5 de março de 1827). Nome mantido igual (falecido no nosso mundo, regra de nomes). Físico e químico italiano, professor da Escola Real de Como a partir de 1774 e depois da cátedra de física experimental da Universidade de Pavia por quase 40 anos. Em 1776 isolou o **metano** (gás dos pântanos do Lago Maggiore). Sua contribuição maior: em 1799-1800 construiu a **pilha voltaica**, empilhando discos alternados de dois metais diferentes (cobre/prata e zinco) separados por pano ou papelão embebido em salmoura, ligando as pontas por um fio para gerar corrente contínua e sustentada. Foi a **primeira bateria elétrica da história**, e provou pela primeira vez que eletricidade podia ser gerada quimicamente por metais em contato, não só por seres vivos.

Isso resolveu (a favor de Volta) a disputa com **Luigi Galvani**, que nos anos 1780 tinha visto pernas de rã dissecadas se contraírem ao encostar em metais e concluiu existir uma "eletricidade animal" própria dos corpos vivos. Volta discordou: mostrou que a contração vinha do contato entre metais diferentes (o corpo da rã era só um detector sensível), e construiu a pilha justamente para provar isso sem precisar de tecido vivo nenhum. A controvérsia Galvani-Volta é considerada uma das discussões científicas mais importantes da história, e lançou tanto a bateria quanto a eletrofisiologia moderna.

Volta relatou os resultados à Royal Society de Londres (carta publicada em 1800), demonstrou a pilha a Napoleão Bonaparte em Paris em 1801 (que o fez conde e senador do Reino da Lombardia), e a unidade de força eletromotriz, o **volt**, recebeu seu nome em homenagem.

**Ângulo de ENSINO:** o princípio de **armazenar e depois liberar carga sob demanda**: a diferença entre gerar energia no instante (como um raio, ou como o campo de Faraday) e **guardá-la empilhada, pronta pra usar quando precisar**. E o lado de método científico: Volta não aceitou a explicação mística ("força vital") só porque era a mais popular; construiu um experimento que isolava a variável real (o contato metálico) e resolveu a disputa com evidência reprodutível, não com autoridade.

## 2. Identidade GusWorld

- **Nome no jogo:** Volta.
- **Era/lugar:** Era 2 (Era do Compilador), a era técnica, cronologicamente **anterior** ao período de Faraday dentro da própria história da rede técnica (mesma era, geração mais antiga). Cientista-artesão de um assentamento técnico primitivo, ainda mais rústico que o de Faraday; trabalha sozinho numa oficina de discos metálicos e panos salinizados.
- **Papel:** o pioneiro do **armazenamento** de carga. Na região onde Volta atua corre um mito popular (eco velado do real Galvani) de que só corpos vivos produzem faísca, que a "centelha" é força vital e não se guarda, só se sente. Volta refuta isso empilhando discos de metais diferentes separados por pano encharcado e provando que a carga se acumula e se libera sob controle, sem tecido vivo nenhum: um experimento, não uma crença. É esse princípio de empilhamento (camada sobre camada, carga retida e liberável) que gerações depois os técnicos da rede (Faraday entre eles) herdam e refinam em blindagem e em toda célula de energia portátil usada hoje.
- **Tece na timeline:** Era 2, sem tocar datas/eventos canônicos; o legado dele (o princípio da pilha) é o alicerce técnico que o presente herda, do mesmo jeito que a Gaiola de Faraday herda o campo.
- **Verdade enterrada (endgame):** como os demais análogos, Volta é um eco convergente do arquétipo "domador de carga estática" que recorre pela fratura (ver `cosmologia-origem-deep.md`).

## 3. A CARTA "Pilha Voltaica"

Carta-chave BÁSICA (obtida relativamente cedo, cluster eletromagnetismo/energia). Efeito ligado à contribuição real (armazenar e liberar carga sob demanda), mapeado ao recurso de **mana** (recurso de compilação do Codex, §6.2 do GDD) por ser o recurso que mais se parece com "energia acumulada que cresce e se gasta". **3 opções pro criador escolher:**

- **Opção A (fiel ao seed): PASSIVA pura: "Carga Armazenada".** No fim de cada turno, o mana não gasto não se perde por inteiro: até um teto (metade do mana máximo atual), o excedente fica retido como reserva e soma ao mana disponível no turno seguinte. Simples, é o item-chave básico "guarda-e-libera" limpo; ensina o princípio sem overhead de gerenciamento extra.
- **Opção B: PASSIVA + ATIVA.** Reserva passiva (como A) + ativa "Descarga da Pilha" (custo 1 AP): consome toda a carga armazenada de uma vez, convertendo em dano elétrico de área OU enchendo instantaneamente o mana de toda a party. Mais jogável em combate, dá ao jogador o momento "descarregar tudo agora".
- **Opção C: PASSIVA + FAILSAFE.** Reserva passiva (como A) + gatilho automático: se um companion vai sofrer dano fatal, a carga armazenada dispara sozinha como escudo de emergência de 1 uso por combate (eco de nobreak/UPS: a pilha "segura a queda"). Sem custo de AP, mas limitado e imprevisível pelo jogador (é reativo, não escolhido).
> Recomendo **A** pra manter a carta como chave básica limpa, no mesmo espírito do Faraday; B como upgrade posterior natural (o "big button" de descarga), C como variante mais defensiva se o criador preferir uma carta de suporte a uma de utilidade pura.

## 4. Descoberta (missão/puzzle)

Numa oficina abandonada da rede técnica mais antiga que a de Faraday (ruína rústica, sem os arcos elétricos "civilizados" do assentamento mais recente), o Gus encontra discos soltos de dois metais diferentes e retalhos de pano ressecado, mais um diário fragmentado que registra a disputa com um rival local (eco velado do Galvani: alguém convencido de que a centelha só vem de tecido vivo). **Puzzle de empilhamento:** o jogador precisa reconstituir a pilha, alternando corretamente os discos dos dois metais e intercalando o pano embebido (reidratado numa fonte salina próxima) na ordem certa: errar a alternância zera a carga e é preciso recomeçar a sequência daquele bloco. Ao completar a pilha e fechar o circuito nas duas pontas, o mecanismo trancado do núcleo da oficina libera a carga guardada e abre a passagem. Ensina o princípio de armazenamento ao resolver. Gate cedo (carta básica), mais mecânico/manual que o puzzle de blindagem do Faraday (que é sobre fechar uma malha; este é sobre empilhar na ordem certa).

## 5. Dica-pro-Tusk (gravada no diário do Gus ao descobrir Volta)

Enigma parcial, aponta em pista pro local de início da missão-capstone do Helion Tusk (1 das 20 dicas): *"Onde a carga se acumula em silêncio, camada sobre camada, sem nunca escapar antes da hora, ali espera o primeiro degrau do circuito que só se fecha inteiro."* (Reforça a mecânica: só com as 20 cartas o feixe/circuito fecha por completo.)

## 6. Arte da carta (figura interior: prompt PixelLab; moldura comum separada)

Prompt (figura interior, NÃO gerar ainda): "retrato pixel-art cel-shaded de um nobre-cientista italiano do século 18, casaca de gola alta e laço no pescoço, cabelo preso curto, expressão serena e confiante, segurando com as duas mãos uma pequena pilha de discos metálicos alternados (cobre e zinco) separados por panos escuros encharcados, com faíscas douradas-azuladas suaves saindo do topo da pilha; ao fundo, retalhos de pano pendurados e um fio de cobre enrolado sutil; paleta fria com toques de cobre quente (azul elétrico + dourado antigo), fundo simples pra encaixar na moldura comum de carta; sem texto." (Moldura comum reutilizável = decisão de design própria, gerada/salva à parte, mesma do Faraday.)

**Imagem-referência (web):** https://commons.wikimedia.org/wiki/File:Alessandro_Volta.jpeg (direto: https://upload.wikimedia.org/wikipedia/commons/5/52/Alessandro_Volta.jpeg) — busto, diagonal (pintura/gravura de época, reprodução fotográfica fiel de obra de domínio público), fonte Wikimedia Commons (domínio público). Segunda opção: https://commons.wikimedia.org/wiki/File:Alessandro_Volta_01.jpg (direto: https://upload.wikimedia.org/wikipedia/commons/9/9f/Alessandro_Volta_01.jpg) — busto/meio-corpo, diagonal, mostra Volta ao lado de suas invenções (pilha e eletróforo), fonte Wikimedia Commons (domínio público). (Alimenta o PixelLab image-to-pixelart; o orquestrador confere a imagem antes de gerar.)
