# GusWorld

Jogo 2D pixel-art, single-player e offline, escrito em C++23, assentado sobre o framework [GlintFx](https://github.com/petrinhu/GlintFx).

> RPG de combate por turnos, com exploração e puzzle. Você joga como Gus, um garoto analítico de 11 anos, num mundo dividido entre a megacidade ciber-gótica GusWorld City e a Selve Sombria: uma floresta tecnorgânica onde a natureza segue matemática rígida, não caos.

## Sumário

- [Sobre](#sobre)
- [Status](#status)
- [Pilares criativos](#pilares-criativos)
- [Stack técnica](#stack-técnica)
- [Build e execução](#build-e-execução)
- [Documentação](#documentação)
- [Contribuindo](#contribuindo)
- [Apoie o projeto](#apoie-o-projeto)
- [Licença](#licença)
- [Governança do projeto](#governança-do-projeto)
- [Créditos](#créditos)

## Sobre

GusWorld é um RPG single-player, de combate por turnos, com elementos de exploração e puzzle. Combate, exploração e puzzle giram em torno da mesma ideia: o mundo é um sistema para entender e raciocinar, não para vencer na força bruta.

É um projeto de uma pessoa só: design, código, direção de arte e roteiro são feitos por um único desenvolvedor solo, com um pipeline de produção de arte assistido por IA e muita ajuda do ecossistema open-source (veja [Créditos](#créditos)).

**Figuras históricas:** GusWorld reimagina figuras históricas reais e falecidas (Faraday, Ada Lovelace, Hayek, entre outras) como personagens ficcionais in-game, mais um personagem fictício inspirado numa figura contemporânea viva. Veja o aviso de [ficção e metodologia](docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md) para entender o que isso é, e o que não é.

## Status

O código está sendo escrito do zero. **O jogo ainda não roda**: não há build funcional, não há tela, não há demo. O que existe hoje é a fundação do repositório (licenças, configuração de versionamento) e o corpus de design, lore e ativos que alimentam o desenvolvimento.

O núcleo de regra de jogo nasce primeiro, em TDD estrito; a camada que desenha só entra quando o [GlintFx](https://github.com/petrinhu/GlintFx) tiver janela, contexto gráfico, entrada e texto prontos, e liga direto nele: o GusWorld não cria janela própria, não tem contexto gráfico próprio e não roda laço de quadro próprio.

O andamento real, item a item, vive na tabela de pendências: [`TODO.md`](TODO.md).

## Pilares criativos

Cinco pilares mantêm o design coerente:

1. **Lógica vence força.** Combate e exploração são resolvidos por análise, predição e combinação, nunca por reflexo, dano bruto ou grind.
2. **Magia é sistema formal computável, natureza é matemática.** Magia funciona como sistema formal (linguagens diegéticas como C-Arcane e Óxido); a Selve Sombria segue padrões matemáticos observáveis, não caos.
3. **O triângulo de hardware é a interface.** Toda habilidade do Gus passa por uma de três peças de equipamento: óculos táticos, aparelho ortodôntico ou o executor de pulso Tavus-Drive. Nada de poder "vindo do nada".
4. **Criança analítica de 11 anos, não herói adulto.** Gus resolve por inteligência, curiosidade e otimização, com tom analítico, não fantasia de poder adulta.
5. **Contraste multipolar com duas âncoras.** O mundo se organiza em torno do contraste entre a megacidade ciber-gótica e a Selve Sombria tecnorgânica.

Detalhes completos em [`docs/design/pillars.md`](docs/design/pillars.md).

## Stack técnica

GusWorld é escrito em **C++23** ([L-03](GODS_LAWS.md#l-03)). A única dependência de framework é o **[GlintFx](https://github.com/petrinhu/GlintFx)**: janela, laço principal, entrada, áudio e desenho são dele, e o GusWorld escreve só a lógica do jogo ([LEI ZERO](GODS_LAWS.md)). Fora do GlintFx e do sistema operacional, o jogo não linka em mais nada.

## Build e execução

**Não existe hoje build funcional.** Não há `CMakeLists.txt`, não há `src/`, não há `include/`, não há `tests/`, não há verificação automática configurada; confirmado por busca no repositório, não por suposição. A fundação de build é a primeira fatia de código do projeto, e esta seção ganha os comandos reais assim que ela existir.

## Documentação

- [`Standards.md`](Standards.md): índice dos manuais de processo do projeto (código, testes, metodologia ágil, deploy, auditorias, ferramentas).
- [`docs/design/pillars.md`](docs/design/pillars.md): os cinco pilares criativos, por completo.
- [`sinopse.md`](sinopse.md), [`CHARS.md`](CHARS.md), [`PLACES.md`](PLACES.md): o panorama canônico do mundo, o inventário de personagens e o inventário de lugares.
- [`docs/tech/adr/`](docs/tech/adr/): os registros de decisão de arquitetura.
- [`TODO.md`](TODO.md): a tabela de pendências, onde o projeto está agora e o que vem a seguir.

## Contribuindo

GusWorld é um **projeto solo** e não aceita pull requests externos nesta fase de desenvolvimento. Relatos de bug e feedback são sempre bem-vindos via [issues do GitHub](https://github.com/petrinhu/GusWorld/issues).

**Política de issues:** somente issues **técnicos** são aceitos (bugs, problemas de build, crashes, erros de documentação). Issues de fundo político, econômico, filosófico, social ou de natureza similar serão **fechados ou excluídos sem resposta**. GusWorld é uma obra de ficção (veja o [aviso de ficção](docs/design/roster-analogos/OBRA-DE-FICCAO-E-METODOLOGIA.md)); seus documentos de design não são um convite a debate de mundo real.

## Apoie o projeto

GusWorld é **freeware**: de graça pra jogar, para sempre, sem intenção comercial alguma. Se você curte o projeto e quer ajudar a manter o desenvolvimento (inclusive os tokens de IA que ajudam a construir o jogo):

[![Buy me a coffee and some AI tokens](resources/buymecoffe.png)](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL)

**Buy me a coffee and some AI tokens.** Via [PayPal](https://www.paypal.com/donate/?business=9XNZQ4RND67KL&no_recurring=0&currency_code=BRL) *(totalmente opcional, nunca obrigatório)*.

Ou aponte a câmera do celular no QR Code:

![QR Code de doação PayPal](resources/QRCode.png)

## Licença

- **Código-fonte**: [AGPL-3.0-or-later](LICENSE).
- **Assets** (arte, música, som, sprite, texto de sabor e prosa in-game): todos os direitos reservados. Ver [ASSETS-LICENSE.md](ASSETS-LICENSE.md).
- **Marca** (nome, logotipo, trade dress, nomes de personagem): fora de qualquer concessão de código ou de asset. Ver [NOTICE](NOTICE).
- **Jogo offline**: o GusWorld não tem servidor, conta nem telemetria. Uma nota em linguagem simples sobre o que isso significa para a licença está em [OFFLINE-NOTICE.md](OFFLINE-NOTICE.md).

## Governança do projeto

As ordens do líder do projeto que regem decisões técnicas e de produto estão em [GODS_LAWS.md](GODS_LAWS.md).

## Créditos

A lista completa de pessoas, ferramentas de IA e agradecimentos técnicos vive em [AI-DISCLOSURE.md](AI-DISCLOSURE.md), atualizada, e não duplicada aqui.
