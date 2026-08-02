# PROPOSTA: Mira do inimigo, intenção telegrafada e cartas de atração/repulsa

> **Status: PROPOSTA, não canon.** Nada aqui vira regra do jogo sem aprovação item a item do
> criador. Nasceu do playtest ao vivo de 2026-07-31, da queixa verbatim do líder: *"impossível
> para um ser humano entender como ocorre a ordem dos turnos... a mesma coisa sobre decisões da
> IA inimiga. Simplesmente vejo acontecer e parece bom"* e da observação *"quando fiquei apenas
> defendendo, o inimigo se concentrou em atacar apenas um (Cauã)"*.
>
> Decisão do líder em 2026-08-01: **peso + telegrafar + carta (provocação OU firewall) + momento
> de contexto narrativo**. Este doc detalha as quatro peças para aprovação.

---

## §0. Achado que muda o custo da entrega

Antes de propor qualquer coisa nova, a varredura do que já existe encontrou o seguinte:

1. **Telegrafar já é canon.** `battle-screen.md`, decisão 5: *"Ícone de intenção flutuando sobre o
   inimigo (estilo Slay the Spire). Padrão-ouro de legibilidade... Cada inimigo mostra um símbolo
   do plano (atacar quem + dano previsto / defender / aplicar status)."* Aprovado pelo criador em
   2026-06-24. Não é ideia nova; é canon **não construído**.
2. **O dado já existe no motor.** `IntentPreview` (`combat_records.hpp:80`) já carrega
   `predicted_action_id`, `predicted_damage`, `predicted_shape`, **`predicted_target_id`** e
   `is_chaotic`. O `ScriptedBrain` já sabe responder quem ele vai atacar.
3. **A tela já sabe pedir.** `BattleScene::intent_for()` (`battle_scene.cpp:658`) monta a
   `CombatState` e chama `preview_intent`. **Só que nada chama `intent_for` para desenhar**: a
   função existe e nunca é usada no render.

Conclusão: a peça cara (motor prever a intenção) está pronta. Falta **desenhar** e falta a mira
valer a pena ser lida. O Patch-Zero já tem tratamento canônico previsto (`is_chaotic` → ícone de
ruído embaralhado), então o caso especial também já está pensado.

---

## §1. O sistema de peso (quem o inimigo escolhe)

Hoje: `scripted_brain.cpp:26` faz `const CombatActor* target = players.front();`, sempre o
primeiro vivo da lista. Foi isso que fez o inimigo bater só no Cauã a batalha inteira.

Proposta: cada personagem da party carrega uma **nota de atração**, e o inimigo sorteia entre os
vivos usando essas notas como peso. Não é "o de maior nota sempre" (viraria decoreba) nem sorteio
puro (viraria ruído): é sorteio ponderado.

### Fatores propostos (aprovar um a um)

| # | Fator | Efeito na nota | Por quê |
|---|---|---|---|
| **F1** | **Dano causado nas últimas 2 rodadas** | sobe | quem está machucando vira ameaça. É o fator que faz o inimigo reagir ao seu jogo em vez de ignorá-lo. |
| **F2** | **HP baixo** | sobe | o inimigo tenta finalizar quem está caindo. Cria tensão real e dá função ao healer. |
| **F3** | **Papel de suporte (healer/buff)** | sobe | o clássico "matar o curandeiro primeiro". Dá peso estratégico à Jaci e obriga o jogador a protegê-la. |
| **F4** | **Defendendo / com Shield ativo** | ⏳ **em estudo** | a intuição diz "inimigo esperto não bate em parede", mas o líder recusou canonizar por intuição em 2026-08-01: *"não quero opinião, quero dados internos da nossa engine de luta"*. A pergunta real é **"é melhor derrubar a parede logo enquanto tenho força, ou deixar a parede para depois comigo já enfraquecido?"**, e o sinal deste fator (sobe ou desce) sai do estudo estatístico, ver `proposta-protocolo-simulacao-mira.md`. |
| **F5** | **Provocação ativa (carta, §2)** | sobe muito | escolha deliberada do jogador. |
| **F6** | **Firewall ativo (carta, §2)** | desce muito ou zera | escolha deliberada do jogador. |
| **F7** | **Vendeta declarada (§3)** | sobe muito | contexto narrativo pré-batalha. |
| **F8** | **Fraqueza elemental do alvo** | sobe | ✅ **aprovado 2026-08-01, só em chefes e inimigos de tier alto.** Capanga bate meio no escuro; chefe mira a fraqueza. Cria progressão de esperteza e faz o chefe parecer chefe. |

### Estado das aprovações (2026-08-01)

| Fator | Estado |
|---|---|
| F1 dano causado | ✅ aprovado |
| F2 HP baixo | ✅ aprovado |
| F3 papel de suporte | ✅ aprovado |
| F4 defendendo | ⏳ **sinal indefinido, depende do estudo estatístico** |
| F5 provocação | ✅ aprovado (§2.A) |
| F6 firewall | ✅ aprovado (§2.B) |
| F7 vendeta declarada | ✅ aprovado em conceito (§3); momentos e frases pendentes |
| F8 fraqueza elemental | ✅ aprovado, restrito a chefe/tier alto |

### Diegese (Pillar 1: magia é software)

A mira do inimigo não é "raiva", é **escalonamento de processo**. O inimigo roda um escalonador
que escolhe qual processo da party atender primeiro, por prioridade. Quem consome mais banda
(dano) sobe de prioridade; quem está travado atrás de um portão (Shield) desce; quem se anuncia
(provocação) sobe. Isso mantém o combate inteiro dentro da metáfora e dá vocabulário natural para
o log do terminal.

---

## §2. As cartas: as duas são a MESMA máquina

Observação de projeto que economiza trabalho: **provocação e firewall são o mesmo mecanismo com o
sinal trocado.** Uma soma peso em quem a joga, a outra subtrai peso de quem recebe. Se o sistema
de peso do §1 existir, as duas cartas custam quase nada a mais, e não é preciso escolher uma.

**Decisão do líder, 2026-08-01: as duas existem, e o uso é escolha do JOGADOR, não do motor.**
Verbatim: *"depende do momento do jogo, se tiver as duas cartas isso fica a critério do jogador,
não do engine do jogo. Se tiver só uma, ele decide se deixa ativa ou não. Decisão do jogador, não
nossa."* Consequência: o motor nunca escolhe por ele, nunca sugere a "carta certa", e nunca ativa
nenhuma das duas automaticamente. As duas competem por slot do Codex como qualquer outra carta.

### 2.A. Carta de PROVOCAÇÃO (conceito: *honeypot*)

- **O que é no mundo real:** um honeypot é um serviço que se anuncia deliberadamente como
  vulnerável para atrair o ataque e mantê-lo longe do que importa. É literalmente uma provocação
  em software. Encaixa em Pillar 1 sem esforço nenhum.
- **Efeito proposto:** o portador soma peso alto por N turnos. Não é uma trava (o inimigo *pode*
  escolher outro alvo, só é bem menos provável); trava absoluta mata a leitura do telegrafar.
- **Quem usa:** o Bento é o tanque da party e hoje **não tem função de tanque**, porque o inimigo
  ignora quem está na frente. Esta carta é o que dá sentido ao papel dele.
- **Contrapartida, DECIDIDA pelo líder em 2026-08-01:** o portador ganha bônus de defesa
  enquanto dura, **no máximo 10%**. O teto baixo é deliberado, e a justificativa dele é a peça
  de design mais importante desta seção:

  > *"sempre cria a dúvida no jogador do tradeoff da possibilidade de perder um tanque naquela
  > batalha mesmo com bônus de defesa (máx de 10%) ou poupar ele pra depois. Cria um suspense de
  > raciocínio e estratégia: enfraquecer um tanker numa batalha estúpida ou poupar ele e a
  > bateria das cartas para algo provavelmente difícil no futuro."*

  O eixo de decisão que ele quer manter permanentemente tenso: **utilidade do membro da party ×
  momento do jogo × posse da carta × bateria da carta × tipo de carta (pirata ou original)**. E o
  propósito declarado é didático: *"é um treinamento didático para o mundo real: devo investir meu
  dinheiro para o futuro com prudência prevendo problemas maiores ou gastar logo e ter prazer
  imediato? a dúvida da humanidade: prudência × yolo"*.

  Consequência de projeto: 10% **não pode** ser suficiente para tornar a provocação uma jogada
  obviamente boa. Se o balanceamento fizer a carta virar escolha automática, o número está errado,
  não a regra.

### 2.B. Carta FIREWALL (conceito: *DROP de pacote*)

- **O que é no mundo real:** um firewall que aplica política `DROP` não responde nada: o
  atacante varre e simplesmente não enxerga o alvo. Diferente de `REJECT`, que devolve um aviso.
  Essa distinção real vira mecânica de graça.
- **Efeito proposto (`DROP`):** o alvo aliado sai da lista de mira por N turnos (peso zero). O
  inimigo mira outro. Se **todos** estiverem protegidos, a política cai para o default e alguém é
  escolhido mesmo assim (senão vira invulnerabilidade).
- **Variante (`REJECT`):** em vez de sumir, o ataque volta com aviso, mas isso já é o `Reflect`
  do Newton (canon, `cartas-technomagik.md` §5.5 e a nota N-3). **Recomendo `DROP`** justamente
  para não pisar em cima do Reflect que já existe.
- **Não confundir com o que já existe:** `Shield` é pool de absorção (aguenta o golpe),
  `BlindagemEM` do Faraday filtra debuff elétrico, `Reflect` devolve percentual. O Firewall age
  **antes de tudo isso**, na escolha do alvo: o golpe não chega a ser dirigido a você.

### 2.C. Nomes

O canon nomeia cartas em Sylvarin (`cartas-technomagik.md` §: Escudo = **Ondhesse**). Os nomes
destas duas ficam **pendentes**, a serem cunhados pelo `narrative-writer` sobre o conlang
(`project_conlang_sylvarin`, 13 raízes com mutação consonantal) **depois** que o criador aprovar
os efeitos. Não invento raiz de conlang aqui.

---

## §3. O momento de contexto: a vendeta declarada

Pedido do líder, verbatim: *"se for um inimigo declarado do personagem, isso acrescenta peso de
escolha de ataque - deve ser declarado antes de iniciar a batalha, ainda no diálogo da tela de
cidade/dungeon"*, com o exemplo: *"...então [personagem], por conta disso, não é possível te
poupar! Espero que seus amigos te ajudem, mas meu alvo hoje é você."*

### Como funciona

O diálogo pré-batalha marca um alvo. A marca entra na batalha como o fator **F7** e o telegrafar
**confirma na tela** que ele está cumprindo o que prometeu. É a combinação que resolve a queixa
de origem: o jogador ouve a promessa, vê a intenção, e entende a IA sem precisar auditar código.

### Momentos propostos (aprovar um a um)

| # | Momento | Quem declara | Alvo | Âncora canônica |
|---|---|---|---|---|
| **M1** | Confronto com o antagonista adulto | Sterling Locke | Gus | `sterling-locke.md`; ele é predador corporativo e o conflito é pessoal |
| **M2** | Emboscada de facção a uma desertora | Cult | Iara | `iara-lumen.md`: ela é **desertora do Cult**; desertor é alvo prioritário por definição |
| **M3** | Após a traição | Dante | quem ele conhece melhor | `dante-grid.md`: ele foi da party, sabe as fraquezas, a vendeta é dele *contra* vocês |
| **M4** | Facção ofendida por escolha do jogador | facção da quest | quem tomou a decisão | consequência de escolha, não roteiro fixo |
| **M5** | Boss revisitado | o boss derrotado antes | quem deu o golpe final | rancor acumulado; premia memória de jogo |
| **M6** | Anomalia / Patch-Zero | ninguém | — | **exceção deliberada**: o Patch-Zero é caótico por canon (`is_chaotic`), não declara nem cumpre promessa. O contraste com todos os outros é o que faz ele assustar. |

### A frase

O padrão que o líder desenhou tem três partes, e sugiro fixar isso como forma:

1. **o motivo** (por que este alvo, e não outro),
2. **a declaração** (o alvo nomeado, explícito),
3. **a brecha** (o reconhecimento de que os amigos podem interferir, é o que transforma a
   ameaça em convite tático, e não em sentença).

Exemplo do líder, anotado: *"...então **[personagem]**, por conta disso ⟨motivo⟩, não é possível
te poupar! ⟨declaração⟩ Espero que seus amigos te ajudem, mas meu alvo hoje é você ⟨brecha⟩."*

**As frases finais por momento são escritas pelo `narrative-writer`**, com a voz de cada
antagonista, depois da aprovação. O main thread não escreve prosa canônica.

---

## §4. O que fica de fora desta proposta

- **Nomes em Sylvarin** das duas cartas (pendente do `narrative-writer` pós-aprovação).
- **Números** (quanto cada fator pesa, quantos turnos dura cada carta): dependem de playtest;
  entram como `//PLAYTEST` no código, como o resto do balanceamento do motor de cartas.
- **Statline e custo de mana** das cartas: seguem a regra de `cartas-technomagik.md` §2.3.
- **A prosa dos diálogos** dos momentos M1-M5.
