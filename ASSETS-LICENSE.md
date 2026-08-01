# Licença dos assets (arte, música, lore in-game)

> Fronteira explícita entre **código** e **assets**. Regime original decidido pelo líder supremo em 2026-06-21 ([ADR-005](docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md), hoje registro histórico, ver bloco SUPERADO no topo daquele ADR). Regime vigente fechado em 2026-07-31 ([ADR-021](docs/tech/adr/ADR-021-licenciamento-apache-assets-reservados.md)): código em Apache License 2.0, assets divididos em duas zonas por data de corte.

GusWorld é **freeware** (grátis, pra sempre). Tem partes sob regimes diferentes. Este arquivo define qual regime cobre qual asset, e a partir de quando.

---

## Resumo

| Parte | Licença / regime | Onde |
|---|---|---|
| Código-fonte | Apache License 2.0 | ver [LICENSE](LICENSE) |
| Assets publicados até 2026-07-31 | Creative Commons BY-SA 4.0 (irrevogável) | Zona 1, abaixo |
| Assets novos a partir de 2026-08-01 | Direitos reservados | Zona 2, abaixo |
| Livros-companheiros (Vol 1 + Vol 2) | Direitos reservados (obra à parte) | Zona 3, abaixo |
| Marca (nome, logotipo, trade dress) | Fora de qualquer concessão de asset, em qualquer zona | ver "Marca" abaixo |

---

## Código = Apache License 2.0

Cobre todo o código-fonte vigente e os scripts de build:

- Fontes C++ (`.cpp`, `.h`, `.hpp`)
- Scripts de tooling/editor
- Scripts de build (CI, shell, empacotamento)

**Histórico:** o código foi GPLv3 até 2026-07-31, e AGPL-3.0 antes disso. Fontes C#
(`.cs`) e scripts GDScript (`.gd`) da era Godot não existem mais no repositório
atual (decommission no marco M8, 2026-07-22; preservados na tag
`pre-m8-godot-legacy`), mas **continuavam cobertos pela GPLv3 enquanto
existiram**; esta seção não remove nem enfraquece aquela cobertura retroativa.
A rotação para Apache License 2.0 (ADR-021) não retroage: releases já publicadas
sob GPLv3/AGPL-3.0 permanecem naquela licença para quem já as recebeu.

A licença completa está em [LICENSE](LICENSE) (texto Apache License 2.0 verbatim, imutável). Titular: petrinhu, 2026.

---

## Zona 1: assets publicados até 2026-07-31 (CC-BY-SA 4.0, irrevogável)

Todo asset que já estava publicado no repositório em ou antes de 2026-07-31
permanece sob **Creative Commons Atribuição-CompartilhaIgual 4.0 Internacional
(CC-BY-SA 4.0)**, para sempre, para quem já recebeu. Isto vale com todas as
letras, sem eufemismo: quem clonou, baixou ou recebeu esse conteúdo enquanto
ele estava sob CC-BY-SA continua com a licença que recebeu. Uma decisão do
titular sobre assets futuros não desfaz uma licença já concedida sobre o
passado.

Cobre, nesta zona:

- Sprites, modelos 3D, texturas, atlas, materiais visuais publicados até a data de corte
- Música, SFX, áudio publicados até a data de corte
- Textos in-game (diálogos, lore exibida no jogo, descrições, UI) publicados até a data de corte
- Conteúdo dentro de `assets/` e `resources/` presente no repositório em 2026-07-31

Licença: **Creative Commons Atribuição-CompartilhaIgual 4.0 Internacional (CC-BY-SA 4.0)**.
Texto e termos oficiais: <https://creativecommons.org/licenses/by-sa/4.0/>

Em resumo (não substitui o texto oficial): pode usar, copiar, modificar e redistribuir, **desde que** atribua o crédito (petrinhu, 2026) e **mantenha a mesma licença** (CC-BY-SA 4.0) na obra derivada.

Parte destes assets nasce de ferramentas de IA generativa de terceiro (PixelLab, Tripo3D, Suno, Gemini/Grok); nesses casos, o CC-BY-SA só se aplica na medida em que os Termos de Serviço daquela ferramenta cedem a titularidade do conteúdo gerado ao criador. Rastreabilidade por asset/lote em [`docs/tech/ai-assets-provenance.md`](docs/tech/ai-assets-provenance.md).

---

## Zona 2: assets novos a partir de 2026-08-01 (direitos reservados)

Todo asset que entrar no repositório a partir de 2026-08-01, seja arte, música,
SFX ou texto in-game novo, fica sob **todos os direitos reservados** ao autor
(petrinhu, 2026). Não é CC-BY-SA, não é copyleft, e não concede licença
automática de uso, cópia ou redistribuição a terceiros.

Esta é decisão do líder supremo, tomada em 2026-07-31 ([ADR-021](docs/tech/adr/ADR-021-licenciamento-apache-assets-reservados.md)), e vale a partir da data de corte porque assets já publicados sob o regime anterior não podem ser recolhidos (Zona 1 acima).

**Onde a fronteira passa, na prática:** qualquer arquivo dentro de `resources/`
ou `assets/` cuja primeira aparição no histórico do git seja datada de
2026-08-01 ou depois está nesta zona. Em caso de dúvida sobre um arquivo
específico, confira a data do primeiro commit que o adicionou:

```bash
git log --follow --diff-filter=A --format=%ad --date=short -- <caminho-do-arquivo>
```

Um arquivo revisado ou substituído depois de 2026-08-01, cuja versão original já
estava publicada até 2026-07-31, mantém CC-BY-SA para a versão antiga (quem já a
recebeu); a versão nova, se for criação substancialmente diferente, entra na
Zona 2. Casos limítrofes (retrabalho pesado de um asset antigo) são decisão do
titular, registrada como nota neste arquivo quando ocorrerem.

---

## Zona 3: livros-companheiros = direitos reservados (inalterado)

Os **dois livros-companheiros** NÃO entram no CC-BY-SA, em nenhuma zona. São obra literária à parte, com **todos os direitos reservados** ao autor (petrinhu, 2026), desde antes desta reorganização:

- **Vol 1**: bíblia de worldbuilding
- **Vol 2**: antologia de contos

Ficam (no todo ou em parte) em `docs/book/`. Reprodução, distribuição ou obra derivada exigem autorização escrita do autor. O fato de a lore aparecer no jogo (sob CC-BY-SA na Zona 1, ou reservada na Zona 2) NÃO estende licença nenhuma ao texto dos livros: são suportes distintos, com regimes distintos.

---

## Marca (carve-out, vale para as três zonas)

Identidade de marca (nome do jogo, logotipo do produto, trade dress) fica
**fora de qualquer concessão de asset**, em qualquer zona, inclusive a Zona 1.
A própria CC-BY-SA 4.0 já deixa isso claro por conta própria: a seção 2(b)(2)
do texto legal da licença exclui direitos de marca do que ela concede. Ou
seja: quem recebeu um asset da Zona 1 sob CC-BY-SA pode usar, modificar e
redistribuir aquele asset, mas isso não concede o direito de usar o nome
"GusWorld", o logotipo do jogo, ou apresentar um fork ou derivado como se fosse
o produto oficial.

Ver também [NOTICE](NOTICE), que registra este carve-out para o código, e
[ADR-021](docs/tech/adr/ADR-021-licenciamento-apache-assets-reservados.md) para o argumento completo.

---

## Permissão para conteúdo de fã (vídeos, streams, fan art)

O regime de direitos reservados da Zona 2 não tira da comunidade o direito de
jogar e compartilhar. Por isso, independente da zona do asset envolvido, o
autor concede permissão explícita para:

- Gravar e publicar vídeos de gameplay (Let's Plays, reviews, tutoriais, compilações)
- Fazer streaming ao vivo do jogo
- Publicar capturas de tela e clipes curtos
- Criar e compartilhar fan art, fan fiction e outro conteúdo derivado **não-comercial**, inspirado nos personagens e no mundo do jogo

Monetização padrão de plataforma (anúncios do YouTube/Twitch no vídeo do
criador de conteúdo) é permitida. Isso não cobre: vender o asset original como
se fosse próprio, republicar o jogo, ou uso comercial dos assets fora do
contexto de conteúdo de fã acima descrito. Fan art e fan fiction seguem
sujeitas ao carve-out de marca acima (não usar o nome/logo do jogo como se o
conteúdo fosse produto oficial).

---

## Regra de fronteira (histórica): arquivos de cena Godot (.tscn / .tres)

> **Nota (2026-07-22):** o stack Godot/C# foi decommissionado no marco M8; arquivos
> `.tscn` e `.tres` não existem mais no repositório (histórico preservado na tag
> `pre-m8-godot-legacy`). A regra abaixo fica como registro de como a fronteira
> código/asset foi resolvida naquela era; não se aplica a nenhum arquivo do
> repositório atual.

Arquivos `.tscn` e `.tres` podiam ser **código** ou **asset** conforme o conteúdo:

- **Com lógica** (script anexado, expressões, máquina de estado, configuração de comportamento) = **código**, sob GPLv3 (licença vigente naquela era).
- **Dado ou arte puro** (apenas valores, referências de recurso, posições, paleta, dado de balanceamento sem lógica) = **asset**, sob CC-BY-SA 4.0.

Na dúvida sobre um `.tscn`/`.tres` específico, valia a natureza dominante do arquivo. Se carregava comportamento, tratava-se como código.

---

## Atribuições de terceiros

Bibliotecas e fontes de terceiros têm licenças próprias, listadas em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

---

*Recomendação técnica de fronteira de licença; a validação jurídica formal cabe
ao titular, inclusive: classes de registro de marca a proteger, se o carve-out
precisa de menção nominal aos nomes de personagem, e a redação final do
parágrafo de conteúdo de fã. Releases de código já publicadas sob
GPLv3/AGPL-3.0 permanecem sob aquela licença (ver ADR-005). Assets já
publicados sob CC-BY-SA 4.0 permanecem sob aquela licença para sempre (Zona 1
acima, ver ADR-021).*
