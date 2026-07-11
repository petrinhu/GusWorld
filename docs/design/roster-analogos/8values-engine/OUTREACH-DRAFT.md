# 8VALUES-OUTREACH: draft message (creator sends, not the agent)

**Status:** DRAFT for the creator (petrus) to review and send. External communication is sent by the creator, never by the agent. Suggested channel: a polite issue or discussion on `github.com/8values/8values.github.io`, or a direct message if the authors list a contact.

**Do not send as-is without the creator's review.** Adjust tone/wording to taste.

---

## Suggested subject

Thank you + a heads-up: 8values ported (MIT-credited) as a fiction game-design tool in GusWorld

## Suggested body (English)

Hi, and thank you for 8values.

I'm building a single-player indie game called **GusWorld**, and I used your quiz as the basis for an internal **game-design tool**: I ported the 70 questions, the per-axis scoring, and the 52-ideology table to a small Python script, purely to give fictional in-game characters (inspired by historical figures) a reproducible political-spectrum leaning that drives which kind of side-quest each character card gets.

A few things I want to be transparent about:

- **It's a faithful fork/port, credited under your MIT license.** The original questions and scoring math are unchanged; I kept your `LICENSE` verbatim and wrote an `ATTRIBUTION.md` that credits 8values as the source and lists exactly what GusWorld added on top (a `rightness` 0–1 scalar, a Python port, and a separate, clearly-labeled game-only weighted layer).
- **The weighted layer is game-only and does not modify your test.** It lives in a separate script and exists solely to encode how much each axis matters *inside the game's fictional world*. It is not presented as an improvement to 8values or as a statement about real-world politics.
- **It's a fiction project, not a political one.** Everything sits under a fiction disclaimer; the figures are reimagined as fictional characters.

Repository (Codeberg): https://codeberg.org/petrinhu/gusworld
The port and attribution: `docs/design/roster-analogos/8values-engine/`

If anything about the attribution or the way I've described 8values isn't to your liking, please let me know and I'll adjust it right away. Mostly I just wanted to say thanks: the quiz was a great, well-designed foundation to build a game mechanic on.

Best,
petrus (GusWorld)

## Suggested body (pt-br, if the creator prefers)

Olá, e obrigado pelo 8values.

Estou construindo um jogo indie single-player chamado **GusWorld** e usei o quiz de vocês como base de uma **ferramenta interna de design de jogo**: portei as 70 perguntas, o scoring por eixo e a tabela de 52 ideologias para um pequeno script Python, apenas para dar a personagens fictícios do jogo (inspirados em figuras históricas) uma inclinação de espectro político reproduzível que decide o tipo de sidequest de cada carta.

Pontos de transparência: é um fork/port fiel, creditado sob a licença MIT de vocês (perguntas e matemática originais intocadas, `LICENSE` verbatim, `ATTRIBUTION.md` creditando o 8values); a camada ponderada é exclusiva do jogo e não modifica o teste (script separado, sem alegação de melhoria nem sobre política do mundo real); e é um projeto de ficção, sob disclaimer. Se algo na atribuição não agradar, ajusto na hora. No fundo, é só um obrigado: o quiz foi uma base muito bem desenhada para construir uma mecânica de jogo.

Repositório: https://codeberg.org/petrinhu/gusworld
Port e atribuição: `docs/design/roster-analogos/8values-engine/`

Abraço,
petrus (GusWorld)
