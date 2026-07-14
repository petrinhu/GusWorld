# AI Disclosure / Divulgação de uso de IA

This document exists so anyone reading the GusWorld source, docs or lore knows exactly where AI tools were used, and where they weren't. It expands on the short note in [README.md](README.md), by role.

---

## English

### Summary, by role

GusWorld is directed, decided and reviewed by a human. The AI (Claude Code) was used as an execution tool, under my direction and review at every step: no design, narrative or architecture decision was delegated to the AI without my approval. Breaking down each part:

- **Creative direction, game design and narrative** (mechanics, world, characters, story, concept art): about 95% human. Every creative decision and every canonical direction was set and approved by me. The AI helped draft and organize text from decisions I had already made, always under my review; nothing becomes canon without my sign-off.
- **Architecture and technical choices** (stack, engineering patterns, the ADRs): near 100% attributed to me and the human contributors credited in the acknowledgments (in particular my brother, El Iagows, who guided me on architecture, SDL and spritesheets). The "how to build it" is a human decision; the AI contributed analysis and options, which I decided between.
- **Code (implementation):** this is where the AI had the heaviest role. Claude Code acted as a pair-programming tool, writing and refactoring code from my specifications and decisions, with tests and review. Mine is the direction, the specification and the final call; the AI's is the assisted typing.

In short: the author and director of this game is human. The AI is the tool.

### AI tools used, and what for

- **[Claude Code](https://claude.com/claude-code) (Anthropic):** AI pair programming used throughout development, the heaviest AI role in the project. It writes and refactors code from my specifications and decisions, under test and review.
- **[Gemini](https://gemini.google.com/) ("nano banana") and [Grok](https://grok.com/) (xAI):** 2D concept image generation, from prompts derived from the game's lore and my decisions.
- **[Tripo3D](https://www.tripo3d.ai/):** 3D concept art (image-to-3D), a production tool only, feeding a bake-to-sprite pipeline. The game itself is 2D at runtime.
- **[PixelLab](https://www.pixellab.ai/):** AI-assisted pixel art and sprite generation and animation (multi-direction characters, animation from a single reference image).

All prompts used for image and sprite generation were derived from the game's lore and from decisions I had already made; nothing was generated blind or left unsupervised.

### Human contributors

Alongside the AI tools, several people shaped this project directly. Full detail lives in the README's Credits and acknowledgments section and in [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md); in summary:

- **Gus Dragon (my son):** inspiration for the protagonist's look and tastes, my partner in game decisions, and lead tester.
- **El Iagows (my brother):** a computer engineer and a personal inspiration to me in IT, whose training shaped how I think about tech; he gave me plenty of architecture and stack advice, and guided me on using SDL and spritesheets for movement.
- **Od Fuinha Minduim, Thiago MadDog and Thiago Arcanjo:** IT professionals who gave countless tips on testing, architecture, QA, security, CI and RAG.
- **The #metaleiros-PE crew:** a longstanding friendship and a source of ongoing feedback and camaraderie.
- **Bruno Vettore:** suggested creating a language for the game (Sylvarin was born from that conversation), and inspired the character Brunus "Vetorial" Solveckt.

### Method: who decides, what the AI does

The working method, in practice:

1. I decide every point of design, narrative and architecture. Nothing becomes canon without my approval.
2. The AI (mainly Claude Code) proposes options, drafts text or code, and executes, always under my direction and review.
3. Extensive deep-lore worldbuilding prose was drafted with AI assistance, but under my direction and review at every step. The decisions, what is canon, what a character is like, what a place means, are mine.
4. Code is where the AI carries the heaviest load, acting as a pair-programming tool. I write the specification and make the final call; the AI writes a first pass, which I review, adjust and approve, with automated tests as a gate.
5. Architecture and stack choices, including the ADRs behind the project's two stack pivots, were decided by me and by the human contributors credited above. The AI contributed analysis and options, not decisions.

### The AI architecture that worked best

The highest-value way I used AI on this project was a **layered, multi-model architecture with the human on top**, rather than a single assistant doing everything:

- **Coordination and verification (Claude Opus), on the main thread.** The strongest model is the layer that talks to me, holds the plan, hands out the work and, above all, **re-verifies every deliverable before accepting it** (the project's internal rule is "an agent's report is not proof": builds, tests and claims are re-checked).
- **Orchestration of work-fronts (Claude Fable).** A dedicated model acts as the "C-level" of each front: it **plans** how to break the work into tasks and which specialists to call, without doing the hands-on work itself.
- **Hands-on execution (Claude Sonnet).** The specialist agents that **do the actual work** (writing and refactoring code, reviewing, testing), always from a specification and under review by the layer above.

The gain was not a "smarter" model but the **separation of roles with cross-checking**: I decide and approve at the top, the coordination layer re-verifies, and whoever executes is never the one who reviews.

And this direction **only works because I have enough grounding in IT and AI** to make the technical decisions that matter: choosing the stack, designing the architecture, and judging what the AI delivers, accepting what fits and rejecting what doesn't. That grounding comes from my (unfinished) **electronic engineering** studies, where I studied algorithms, programming logic and the principles of programming languages, plus years of **self-teaching in IT and AI**, which today lead me to teach AI and technology to physicians. And, in all honesty: many of my C and C++ questions I cleared up with **Gus** himself, my 11-year-old son, who already programs better than I do, and who inspired this game's protagonist. The AI amplifies competent human judgment; it does not replace it. Without that grounding, coordinating models this way would not work.

### In short

The author and director of this game is human. The AI is a tool: a fast typist and a tireless drafting assistant, not a co-author and not a decision-maker.

---

## 🇧🇷 Português

### Resumo, por papel

O GusWorld é dirigido, decidido e revisado por um humano. A IA (Claude Code) foi usada como ferramenta de execução, sob minha direção e revisão em cada etapa: nenhuma decisão de design, narrativa ou arquitetura foi delegada à IA sem minha aprovação. Detalhando o papel de cada parte:

- **Direção criativa, game design e narrativa** (mecânicas, mundo, personagens, história, arte conceitual): ~95% humana. Toda decisão criativa e todo rumo canônico foram definidos e aprovados por mim. A IA ajudou a rascunhar e organizar texto a partir das minhas decisões, sempre sob revisão minha; nada vira cânone sem meu aval.
- **Arquitetura e escolhas técnicas** (stack, padrões de engenharia, os ADRs): quase 100% atribuídas a mim e aos contribuidores humanos creditados nos agradecimentos (em especial ao meu irmão El Iagows, que me orientou em arquitetura, SDL e spritesheets). O "como construir" é decisão humana; a IA contribuiu com análise e opções, que eu decidi.
- **Código (implementação):** é onde a IA teve o papel mais pesado. O Claude Code atuou como par de programação, escrevendo e refatorando código a partir das minhas especificações e decisões, com testes e revisão. O meu é a direção, a especificação e a decisão final; o da IA é a digitação assistida.

Em resumo: o autor e diretor do jogo é humano; a IA é a ferramenta.

### Ferramentas de IA usadas, e pra que cada uma serviu

- **[Claude Code](https://claude.com/claude-code) (Anthropic):** par de programação por IA usado ao longo de todo o desenvolvimento, o papel mais pesado de IA no projeto. Escreve e refatora código a partir das minhas especificações e decisões, com testes e revisão.
- **[Gemini](https://gemini.google.com/) ("nano banana") e [Grok](https://grok.com/) (xAI):** geração de imagem conceitual 2D, a partir de prompts derivados da lore do jogo e das minhas decisões.
- **[Tripo3D](https://www.tripo3d.ai/):** arte conceitual 3D (image-to-3D), só ferramenta de produção, alimentando um pipeline de bake-para-sprite. O jogo em si é 2D em runtime.
- **[PixelLab](https://www.pixellab.ai/):** geração e animação de pixel art assistida por IA (personagens multi-direção, animação a partir de uma única imagem de referência).

Todos os prompts usados na geração de imagem e sprite foram derivados da lore do jogo e de decisões que eu já tinha tomado; nada foi gerado às cegas ou sem supervisão.

### Contribuidores humanos

Além das ferramentas de IA, várias pessoas moldaram este projeto diretamente. O detalhe completo vive na seção de Créditos e agradecimentos do README e em [ACKNOWLEDGMENTS.md](ACKNOWLEDGMENTS.md); em resumo:

- **Gus Dragon (meu filho):** inspiração da aparência e dos gostos do protagonista, parceiro nas decisões sobre o jogo, e meu tester principal.
- **El Iagows (meu irmão):** engenheiro de computação e uma inspiração pessoal minha em TI, cuja formação moldou meu jeito de pensar tecnologia; me deu várias dicas de arquitetura e stack, e me orientou no uso de SDL e de spritesheets para o movimento.
- **Od Fuinha Minduim, Thiago MadDog e Thiago Arcanjo:** profissionais de TI que deram inúmeras dicas de testes, arquitetura, QA, segurança, CI e RAG.
- **A galera do #metaleiros-PE:** uma amizade antiga e uma fonte constante de feedback e parceria.
- **Bruno Vettore:** deu a sugestão de criar uma língua para o jogo (o Sylvarin nasceu dessa conversa), e inspirou o personagem Brunus "Vetorial" Solveckt.

### Método: quem decide, o que a IA faz

O método de trabalho, na prática:

1. Eu decido cada ponto de design, narrativa e arquitetura. Nada vira cânone sem minha aprovação.
2. A IA (principalmente o Claude Code) propõe opções, rascunha texto ou código, e executa, sempre sob minha direção e revisão.
3. Deep-lore extensa de worldbuilding foi rascunhada com assistência de IA, mas sob minha direção e revisão em cada etapa. As decisões, o que é cânone, como é um personagem, o que um lugar significa, são minhas.
4. Código é onde a IA carrega o peso maior, atuando como ferramenta de par de programação. Eu escrevo a especificação e tomo a decisão final; a IA escreve uma primeira versão, que eu reviso, ajusto e aprovo, com testes automatizados como portão de qualidade.
5. Escolhas de arquitetura e stack, incluindo os ADRs por trás dos dois pivôs de stack do projeto, foram decididas por mim e pelos contribuidores humanos creditados acima. A IA contribuiu com análise e opções, não com decisões.

### A arquitetura de IA que mais rendeu

A adoção de IA de maior valor neste projeto foi uma **arquitetura em camadas, com múltiplos modelos e o humano no topo**, em vez de um único assistente fazendo tudo:

- **Coordenação e verificação (Claude Opus), na thread principal.** O modelo mais forte é a camada que conversa comigo, guarda o plano, distribui o trabalho e, sobretudo, **re-verifica cada entrega antes de aceitar** (o princípio interno do projeto é "relatório de agente não é prova": build, testes e afirmações são reconferidos).
- **Orquestração das frentes de trabalho (Claude Fable).** Um modelo dedicado atua como "C-level" de cada frente: **planeja** como quebrar o trabalho em tarefas e quais especialistas acionar, sem pôr a mão na massa.
- **Execução braçal (Claude Sonnet).** Os agentes especialistas que **fazem o trabalho de fato** (escrever e refatorar código, revisar, testar), sempre a partir da especificação e sob revisão da camada de cima.

O ganho não foi um modelo "mais inteligente", e sim a **separação de papéis com verificação cruzada**: eu decido e aprovo no topo, a coordenação re-verifica, e quem executa nunca é quem revisa.

E essa direção **só funciona porque tenho base suficiente de TI e de IA** para tomar as decisões técnicas que importam: escolher a stack, desenhar a arquitetura e julgar o que a IA entrega, aceitando o que serve e recusando o que não serve. Essa base vem da minha formação em **engenharia eletrônica** (não concluída), na qual estudei algoritmos, lógica de programação e princípios de linguagens de programação, somada a anos de **autodidatismo em TI e IA**, que hoje me levam a dar aulas de IA e tecnologia para médicos. E, com toda a sinceridade: muitas das minhas dúvidas de C e C++ eu tirei com o próprio **Gus**, meu filho de 11 anos, que já programa melhor do que eu, e que inspirou o protagonista deste jogo. A IA amplifica um julgamento humano competente, não o substitui; sem essa base, coordenar modelos dessa forma não daria certo.

### Em resumo

O autor e diretor deste jogo é humano. A IA é uma ferramenta: uma digitadora rápida e uma assistente incansável de rascunho, não uma coautora e não quem decide.

---

*Last updated / última atualização: 2026-07-14.*
