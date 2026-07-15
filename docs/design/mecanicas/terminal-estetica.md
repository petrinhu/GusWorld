# Terminal de combate: estética de "app levemente bugado"

**Status:** DESIGN em definição (decisões do criador 2026-07-15, via AskUserQuestion). Mecânica + conteúdo pendentes; parte depende de um pedido ao dev do glintfx (glifos especiais). Reversível / //PLAYTEST.

Cross-ref: `docs/design/mecanicas/cartas-technomagik.md` (frases pedagógicas das cartas), `project_repositorios_perdidos_canon` (dungeon = repositório corrompido), `reference_fonte_ui_fallback` (fonte PixelOperatorMono, sem fallback), `reference_glintfx_api`.

## 1. Conceito

O **log de combate** passa a se chamar **"terminal"** (estética software / cyber-gótico, casa com magia=software). O terminal é um **app in-world levemente bugado**: as linhas saem com cara de saída de sistema e, de vez em quando, aparece um caractere estranho aleatório, como um bug de aplicativo "a ser melhorado". É um defeito diegético proposital, não um bug real.

## 2. Framing de saída de sistema (por tipo de linha)

Interpretação das respostas do criador ("1+2+3: sistema usa `>`, fala de party usa `//`"): cada linha do terminal leva um **endereço hex fake** (sabor binário) + um **marcador por tipo**:

- **Saída de sistema** (eventos mecânicos: dano, status, ações, compilação de carta): marcador **`>`**.
  - Ex.: `0x2A > Gus compila tesla em e0 por 6.`
- **Fala de um membro da party** (bark / comentário de um companion): marcador **`//`**.
  - Ex.: `0x3F // Cauã: cuidado com o da direita.`

O endereço hex (`0x??`) é decorativo/aleatório por linha (reforça "binary as text"). Se ficar pesado no playtest, o hex vira opcional e sobra só `>` / `//`. **Decisão a confirmar com o criador** (registrada como interpretação; ver AMB no fim).

## 3. Glitch ambiente (o "bug levinho")

- **Frequência:** BAIXA chance por linha (poucas linhas recebem glitch), 1-2 glifos por vez.
- **Posição:** começo, meio (entre palavras) ou fim.
- **Regra de segurança (decisão do criador, opção 1):** o glitch **NUNCA** substitui dígitos (número de dano) nem nomes de ator. Só cai em posições seguras. "App bugadinho" sem atrapalhar a leitura da luta.
- **Sem caracteres de controle reais:** nada de `\n`/`\r`/`\0` de verdade (quebrariam o layout). O "return" é o SÍMBOLO visível (⏎), não uma quebra real.
- **Sem letras latinas comuns** (A-Z/a-z/dígitos), pra não confundir com texto legítimo.

### Pool aprovado (mock `docs/design/mockups/11-terminal-glitch-glyphs.html`, criador 2026-07-15)

Criador aprovou o mock e quer **TODOS os 100 glifos no pool**: os **60 da Fase 1** (a fonte já tem, usáveis já) + os **40 da Fase 2** (tofu/box/control-pictures/return/replacement, dependem do glintfx). O criador vai **relayar o pedido pro dev do glintfx** ele mesmo (prompt pronto no histórico do chat / abaixo) e volta quando os Fase-2 estiverem prontos. Até lá, o pool ativo = os 60 da Fase 1; os 40 entram quando o glintfx cobrir.

### Fonte dos glifos (decisão do criador: Latin-1 agora + pedir os especiais ao glintfx)

- **Fase 1 (agora, nosso lado, robusto):** pool só com símbolos que a `PixelOperatorMono` JÁ tem (Latin-1): `¶ § ° ± µ ¤ ¦ ¬ » ¿ × ÷ · ª º ¹ ½ ¼`. Renderizam como glifos reais esquisitos (mojibake), à prova do flip de fonte, zero dependência do glintfx.
- **Fase 2 (enriquecer, pós-relay ao glintfx):** os glifos que o criador citou por nome e que provavelmente estão FORA da fonte: **tofu** (caixinha □ de glifo faltante), **return** (⏎ U+23CE), **control pictures** de binário (␀ ␁ ␂ U+2400+). Precisa o dev do glintfx (a) garantir que glifo-faltante = caixinha visível ESTÁVEL nos dois motores de fonte (FreeType + motor próprio v0.10.0), e/ou (b) cobrir um punhado desses glifos-glitch de propósito.

## 4. Hazard de dungeon: corrupção do terminal (feature nova)

Decisão do criador: criar um **efeito danoso dentro de uma dungeon** que **inunda o terminal só com caracteres estranhos aleatórios** enquanto o desafio da dungeon não é resolvido. Quando o desafio é resolvido, o terminal volta ao normal.

- Diegético perfeito: dungeon = repositório corrompido (`project_repositorios_perdidos_canon`); a corrupção do dado SANGRA pro HUD/terminal do Gus. A poluição visual É a pressão do desafio.
- Diferente do glitch ambiente (raro, leve): aqui é **ILEGÍVEL TOTAL** (decisão do criador 2026-07-15) — o terminal vira uma parede de lixo estilo hexdump (offset + glifos aleatórios), ZERO palavra identificável, igual abrir um binário num leitor de texto. Não é "texto legível com glitch"; é substituição total por ruído. Persiste até a condição de resolução.
- Interage com a leitura de combate: o jogador precisa lutar/resolver com o terminal degradado, o que aumenta a tensão. Balancear pra não virar injogável (talvez números/nomes ainda protegidos, ou um item/carta que "limpa" temporariamente, ex.: gancho pro Faraday/anti-PEM ou Turing/decrypt).
- Provável casamento com um puzzle/mecânica de dungeon específica (não toda dungeon). Design detalhado quando a dungeon-alvo for definida.

## 5. Conteúdo: as 12 frases do Gambito (aprovadas pelo criador 2026-07-15)

Pool aprovado (rascunho; a redação final + os velados bem escondidos passam pelo `narrative-writer`/`ux-writer` na hora de fiar a mecânica). Regra de exibição: **20% de chance** de UMA aparecer no terminal a cada uso do Gambito; 80% silêncio. Voz do Gus (prodígio analítico, xadrez/TCG/teoria dos jogos). Saem com framing de sistema (`>`).

1. Gambito: entregar uma peça agora pra dominar o tabuleiro depois.
2. Todo inimigo roda um padrão. Acha a função, prevê a saída.
3. Tempo vale mais que dano. Quem controla a ordem, controla a luta.
4. Prever não é chutar: é cortar a incerteza pela metade.
5. Minimax: suponha que ele joga perfeito, e jogue melhor.
6. No xadrez o sacrifício abre linha. Aqui, abre o próximo turno.
7. Iniciativa é recurso. Reordenar a fila é gastar ela bem.
8. Blefe só funciona uma vez. Depois vira estatística a seu favor.
9. Reduz o problema: um oponente previsível já tá resolvido.
10. Cada rodada é um estado. Escolhe a transição, não o acaso.
11. O padrão cresce somando o que veio antes. *(Fibonacci velado, não rotular no jogo)*
12. Três passos pra ler o tabuleiro: observar, prever, agir. *(3 degraus, maçônico velado)*

## 6. Pendências de implementação

- **Mecânica (gameplay/app):** injetor de glitch (chance + pool + posição segura) na camada que renderiza o terminal; framing por tipo de linha (`>` sistema / `//` party + hex); roll 20% do pool de frases no `resolve_gambit_*`.
- **Conteúdo:** finalizar as 12 frases via writer; frases pedagógicas das cartas (primo, CARTAS-FRASES-PEDAGOGICAS).
- **glintfx (relay):** cobertura/estabilidade dos glifos tofu □ / return ⏎ / control-pictures ␀ nos dois motores de fonte.
- **Dungeon hazard:** design detalhado + condição de resolução, quando a dungeon-alvo for definida.

## AMB (ambiguidades registradas, a confirmar com o criador)
- **AMB-T1:** framing "1+2+3" interpretado como hex-address por linha + `>` (sistema) / `//` (party). Confirmar se o hex entra mesmo ou se fica só `>`/`//`.
- **AMB-T2:** fonte dos glifos: seguindo opção 1 (Latin-1 agora + relay glintfx pros especiais). Confirmar que não é "esperar o glintfx pra tudo".
