# 8VALUES-OUTREACH: draft message (creator sends, not the agent)

**Status: SENT (2026-07-11).** The creator sent this outreach as an issue on the 8values repository: **https://github.com/8values/8values.github.io/issues/185**. The text below is the draft that was used (kept as record). External communication was sent by the creator, not by the agent.

---

## Suggested subject

Thank you + a heads-up: 8values ported (MIT-credited) as a fiction game-design tool in GusWorld

## Suggested body (English)

Hi, and thank you for 8values.

First, a small apology: I'm Brazilian and my English isn't very good, so please forgive any awkward phrasing here. Also, a heads-up on how to reach me: I use **Codeberg** as my main repository and rarely check GitHub, so if you'd like to reply, the surest way is through the Codeberg project linked below (a message here on GitHub might sit unread for a long time).

I'm building a single-player indie game called **GusWorld** (it's **freeware**: free, non-commercial, no ads, no paid content), and I used your quiz as the basis for an internal **game-design tool**: I ported the 70 questions, the per-axis scoring, and the 52-ideology table to a small Python script, purely to give fictional in-game characters (inspired by historical figures) a reproducible political-spectrum leaning that decides which kind of side-quest each character card gets.

A few things I want to be transparent about:

- **It's a faithful fork/port, credited under your MIT license.** The original questions and scoring math are unchanged; I kept your `LICENSE` verbatim and wrote an `ATTRIBUTION.md` that credits 8values as the source and lists exactly what GusWorld added on top (a `rightness` 0-1 scalar, a Python port, and a separate, clearly-labeled game-only weighted layer).
- **The weighted layer is game-only and does not modify your test.** It lives in a separate script and exists solely to encode how much each axis matters *inside the game's fictional world*. It is not presented as an improvement to 8values or as a statement about real-world politics.
- **It's a fiction project, not a political one.** Everything sits under a fiction disclaimer. To quote it directly: "GusWorld is a work of fiction. [...] None of it is, or is intended to be read as, a factual, biographical or historical claim about the real person: not about what they actually believed, how they actually behaved, what their actual character or morality was, or how a rigorous historian, biographer or psychologist would assess them."

Repository (Codeberg): https://codeberg.org/petrinhu/gusworld
The port and attribution: `docs/design/roster-analogos/8values-engine/`

If anything about the attribution or the way I've described 8values isn't to your liking, please let me know and I'll adjust it right away. Mostly I just wanted to say thanks: the quiz was a great, well-designed foundation to build a game mechanic on.

Best,
petrus (GusWorld)

## Suggested body (pt-br, if the creator prefers)

Olá, e obrigado pelo 8values.

Primeiro, um pequeno pedido de desculpas: sou brasileiro e meu inglês não é muito bom, então perdoem qualquer frase estranha. Um aviso de como me alcançar: uso o **Codeberg** como repositório principal e quase não olho o GitHub, então, se quiserem responder, o caminho mais certo é pelo projeto no Codeberg linkado abaixo (uma mensagem aqui no GitHub pode ficar sem ser lida por muito tempo).

Estou construindo um jogo indie single-player chamado **GusWorld** (é **freeware**: gratuito, sem fins comerciais, sem anúncios e sem conteúdo pago) e usei o quiz de vocês como base de uma **ferramenta interna de design de jogo**: portei as 70 perguntas, o scoring por eixo e a tabela de 52 ideologias para um pequeno script Python, apenas para dar a personagens fictícios do jogo (inspirados em figuras históricas) uma inclinação de espectro político reproduzível que decide o tipo de sidequest de cada carta.

Pontos de transparência: é um fork/port fiel, creditado sob a licença MIT de vocês (perguntas e matemática originais intocadas, `LICENSE` verbatim, `ATTRIBUTION.md` creditando o 8values); a camada ponderada é exclusiva do jogo e não modifica o teste (script separado, sem alegação de melhoria nem sobre política do mundo real); e é um projeto de ficção, sob disclaimer, que cito diretamente: "GusWorld é uma obra de ficção. [...] Nada disso é, ou pretende ser lido como, uma afirmação factual, biográfica ou histórica sobre a pessoa real." Se algo na atribuição não agradar, ajusto na hora.

Repositório: https://codeberg.org/petrinhu/gusworld
Port e atribuição: `docs/design/roster-analogos/8values-engine/`

Abraço,
petrus (GusWorld)
