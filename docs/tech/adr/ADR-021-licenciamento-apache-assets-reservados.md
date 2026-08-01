# ADR-021: Rotação para Apache License 2.0 (código) e regime de assets em duas zonas

| | |
|---|---|
| **Status** | Accepted (decidido pelo líder supremo em 2026-07-31; onda `LICENSE-APACHE`) |
| **Date** | 2026-07-31 |
| **Decisor** | petrinhu (criador supremo, titular único do copyright) |
| **Reversibilidade** | One-way door para o público, na mesma forma do ADR-005: titular único pode relicenciar o que vier a partir daqui; o que já foi publicado sob GPLv3/AGPL-3.0 (código) e CC-BY-SA 4.0 (assets) permanece naquela licença para quem já recebeu. |
| **Substitui** | [ADR-005](ADR-005-license-gpl3-assets-ccbysa.md) (GPLv3 + CC-BY-SA + livros reservados), que já reservava este movimento na própria linha 8. |

## Contexto

O repositório vinha sob GPLv3 (código, rotacionado de AGPL-3.0 pelo ADR-005) +
CC-BY-SA 4.0 (assets) + direitos reservados (livros-companheiros). Dois fatos
novos motivaram reabrir a decisão de licença de código:

1. **Autoria única, confirmada.** `git log --format=%an | sort -u` devolve um único
   nome (`petrinhu`) em todo o histórico do repositório. Não há copyright de
   terceiro no código próprio, e não há outro contribuidor cujo consentimento
   precise ser colhido para relicenciar.
2. **O próprio ADR-005 já previa o relicenciamento.** Linha 8 daquele ADR,
   campo Reversibilidade: *"Titular único pode relicenciar o que vier; releases
   já publicadas permanecem sob a licença vigente na época."* Esta ADR é
   exatamente esse relicenciamento, um mês depois.
3. **Steamworks.** A documentação oficial da Valve sobre distribuição de código
   aberto no Steam lista licenças permissivas como compatíveis e trata licenças
   copyleft como um problema estrutural: *"Common permissive and acceptable
   licenses includes MIT License, BSD 3-clause and 4-clause, Apache 2.0 and
   WTFPL"*, enquanto *"any license that has a so-called 'copyleft' element will
   be problematic when combining code with the Steamworks SDK. The best-known
   example is GPL."* (<https://partner.steamgames.com/doc/sdk/uploading/distributing_opensource>).
   Na prática: sob GPLv3, o SDK proprietário do Steamworks não pode ser
   linkado ao binário do jogo, e sem ele o jogo perderia a integração de
   conquistas, nuvem de save e lista de amigos caso um dia vá para a Steam.
   GusWorld hoje distribui via GitHub/itch/AppImage (não depende disso agora),
   mas fechar essa porta de saída sem necessidade seria custo aceito sem
   benefício, dado que o titular não tem motivo para preferir GPL aqui.

## Decisão

1. **Código-fonte = Apache License 2.0.** Rotacionado de GPLv3. Cobre
   `.cpp`/`.h`/`.hpp`, scripts de tooling e de build. Texto verbatim em
   `LICENSE` (imutável). SPDX `Apache-2.0` rotacionado nos 531 fontes que
   carregavam `GPL-3.0-or-later` (item `LIC-APACHE-3` da onda).
2. **Assets publicados até 2026-07-31 permanecem CC-BY-SA 4.0, irrevogavelmente.**
   Quem já recebeu esse conteúdo (clone, download, distribuição) continua com a
   licença que recebeu; a decisão de hoje não alcança o passado.
3. **Assets novos, a partir de 2026-08-01, ficam em todos os direitos
   reservados.** Fronteira detalhada em `ASSETS-LICENSE.md`.
4. **Livros-companheiros (Vol 1 + Vol 2) seguem em direitos reservados**, como já
   estavam desde o ADR-005; nenhuma mudança aqui.
5. **Marca (nome, logotipo, trade dress) fica fora de qualquer concessão**, de
   código ou de asset, em qualquer zona. Ver argumento na seção própria abaixo
   e o carve-out já registrado em `NOTICE`.
6. **Permissão explícita de conteúdo de fã** (vídeo, stream, captura de tela,
   fan art não-comercial) é devolvida por escrito em `ASSETS-LICENSE.md`, para
   que o fechamento dos assets novos não atinja quem só quer compartilhar que
   jogou.

## O argumento de marca

Entre as licenças permissivas avaliadas, a Apache License 2.0 é a única que
trata marca de forma expressa. Sua **seção 6 ("Trademarks")** diz:

> "This License does not grant permission to use the trade names, trademarks,
> service marks, or product names of the Licensor, except as required for
> reasonable and customary use in describing the origin of the Work..."

MIT, BSD e zlib são silenciosas quanto a marca: elas não a concedem (marca não
é direito autoral e não nasce de silêncio de licença), mas também não a
mencionam, o que deixa a questão para ser resolvida fora do texto da licença.
A Apache a nomeia, o que é mais claro para quem lê o repositório sem ser
advogado.

Há uma sinergia com outra cláusula da mesma licença: a **seção 4(d)** obriga
quem redistribui, com ou sem modificação, a manter um arquivo `NOTICE`
legível caso o trabalho original o inclua. Como o `NOTICE` do GusWorld já
registra a política de marca (nome, logo e nomes de personagem não cobertos
pela licença de código; forks devem se apresentar sob outro nome), essa
política **viaja com cada fork por força da própria licença**, não por boa
vontade de quem copia.

⚠️ Cuidado com os números de seção, confirmados contra o `LICENSE` do
repositório antes de escrever este ADR: patente é a **seção 3** ("Grant of
Patent License"), dispensa de CLA é a **seção 5** ("Submission of
Contributions"), e marca é a **seção 6** ("Trademarks"). Não confundir as três.

## Alternativas consideradas

### A. Manter GPLv3

**Pros:**
- Já era a licença vigente; zero trabalho.
- Copyleft forte, que era o desejo original do criador em 2026-06-21.

**Cons:**
- Incompatível com o Steamworks SDK por natureza copyleft (ver Contexto), o
  que fecha uma porta de distribuição futura sem necessidade real hoje.
- O criador reavaliou a preferência: não há mais razão declarada para exigir
  que forks do jogo permaneçam abertos.

**Decidida:** REJEITADA.

### B. MIT / BSD / zlib

**Pros:**
- Mais enxutas, mais conhecidas, zero atrito para quem só quer ler.

**Cons:**
- Nenhuma delas tem **concessão de patente** (a Apache tem, seção 3).
- Nenhuma delas tem **cláusula de marca expressa** (a Apache tem, seção 6).
- Nenhuma exige propagação de `NOTICE`, então a política de marca não viaja
  com o fork por força de licença; dependeria de o forker copiar o arquivo
  por conta própria.

**Decidida:** REJEITADA. Mais enxuta não compensa perder duas cláusulas que o
projeto usa de verdade (marca, e a propagação do `NOTICE` que a carrega).

### C. Apache License 2.0 (escolhida)

**Pros:**
- Aceita pelo Steamworks (ver Contexto).
- Trata marca de forma expressa (seção 6) e propaga o `NOTICE` que carrega essa
  política (seção 4(d)).
- Concede patente a terceiros (seção 3), o que facilita adoção por quem se
  preocupa com risco de patente ao redistribuir.

**Cons:**
- Mais longa e mais formal que MIT/BSD/zlib; exige manter `NOTICE` e cabeçalhos.
- A concessão de patente aqui não protege o autor (ver ressalva abaixo).

**Decidida:** ACEITA.

## Ressalva honesta sobre a concessão de patente

A seção 3 da Apache License 2.0 é, nesta situação concreta, sobretudo **o
autor concedendo direitos de patente a terceiros que usem o código**, não o
autor se protegendo de patente alheia. Não há patente registrada neste
projeto, e não há outro contribuidor cujas contribuições precisem de uma
licença de patente cruzada. A cláusula existe e é real, mas não deve ser
vendida como blindagem que ela não é neste caso: ela facilita a vida de quem
adota o código (reduz o risco de reivindicação de patente contra quem usa),
não a do titular.

## Irreversibilidade

O que já saiu sob GPLv3/AGPL-3.0 (código) e CC-BY-SA 4.0 (assets) continua
assim para quem recebeu. A rotação de hoje vale só para o que vem a partir de
2026-07-31 (código) e 2026-08-01 (assets novos). Ver `ASSETS-LICENSE.md` para
a fronteira operacional dos assets.

## Método e limite da auditoria de proveniência

Antes de trocar qualquer cabeçalho, foi feita uma auditoria de proveniência
sobre o código (item `LIC-APACHE-1-PROVENIENCIA` da onda), que **voltou
limpa**:

- **Varredura de copyright alheio nos 531 arquivos** que carregavam SPDX
  `GPL-3.0-or-later`: zero achados de comentário de copyright de terceiro,
  cabeçalho de licença estranha, ou bloco de código com atribuição alheia.
- **Caça a irmãos do caso `glad`**: o vendor do loader OpenGL
  (`GusEngine/platform/rmlui/RmlUi_Include_GL3.h`, achado em 2026-07-28 fora de
  `third_party/`, documentado em `THIRD-PARTY-LICENSES.md`) foi o único caso
  conhecido de código de terceiro vendorizado fora do diretório esperado.
  A varredura repetiu essa busca especificamente e não achou nenhum novo caso.
- **Cross-check do inventário de terceiros** em `THIRD-PARTY-LICENSES.md`
  contra o que está de fato vendorizado ou linkado no binário: nenhuma
  divergência nova.
- **Revisão dos empréstimos de constantes do Godot**, remanescentes da era
  Godot/C# (pré-M8): são valores de interface (enums, constantes numéricas de
  configuração), não expressão criativa protegida por copyright; e o próprio
  Godot é MIT, permissivo, sem exigência que impeça o uso desses valores.

⚠️ **Limite declarado do método, não bloqueio da decisão:** existe risco
residual de regurgitação de material de treino, porque este código foi escrito
majoritariamente por agentes de IA, e **nenhuma ferramenta local disponível
detecta isso** (não existe um scanner de similaridade contra um corpus GPL
mundial com base de referência licenciada e confiável ao alcance deste
projeto). Encenar uma varredura contra esse corpus sem uma base de referência
real não produziria resultado confiável, então **não foi tentado**. Isto fica
registrado como limite conhecido do método de auditoria adotado, não como algo
que bloqueia a decisão: a varredura real que era possível de fazer (leitura
linha a linha em busca de atribuição alheia, checagem de vendors fora de
lugar, cross-check de inventário, revisão de empréstimos conhecidos) foi
feita e voltou limpa.

## Consequências

### Positivas

- **Compatibilidade com Steamworks preservada** para uma distribuição futura na
  Steam, sem custo hoje (o projeto não depende disso agora).
- **Marca protegida por cláusula expressa de licença**, que viaja com cada
  fork via propagação obrigatória do `NOTICE`.
- **Conteúdo de fã explicitamente permitido**, mitigando o efeito colateral do
  fechamento dos assets novos sobre a comunidade.
- **Liberdade futura preservada**, na mesma lógica do ADR-005: titular único
  pode relicenciar de novo, se um dia quiser.

### Negativas (custos aceitos)

- **Assets novos deixam de ser copyleft**: quem quiser reusar arte/áudio/texto
  publicado a partir de 2026-08-01 precisa de autorização explícita do autor.
  Custo aceito pelo líder; o acervo já publicado continua livre sob CC-BY-SA.
- **Três regimes de asset no mesmo repositório** (CC-BY-SA histórico, direitos
  reservados novo, livros reservados) exigem disciplina de fronteira, mitigada
  por `ASSETS-LICENSE.md` e pelo comando de verificação por data de commit.
- **A concessão de patente da Apache não protege o autor** neste caso concreto
  (ver ressalva acima): é benefício para quem adota o código, não blindagem
  própria.

## Ações

1. ✅ `LICENSE` trocado para o texto verbatim da Apache License 2.0 (`LIC-APACHE-2`).
2. ✅ `NOTICE` criado com copyright e carve-out de marca (`LIC-APACHE-2`).
3. ✅ SPDX `Apache-2.0` rotacionado nos 531 fontes C++ (`LIC-APACHE-3`), com SHA
   registrado em `.git-blame-ignore-revs`.
4. ✅ Documentação não-código atualizada (README, `THIRD-PARTY-LICENSES.md`,
   `AUDITORIAS.md`, `TESTES.md`, `ROADMAP.md`, `CHANGELOG.md`,
   `docs/PROJECT-TREE.md`) (`LIC-APACHE-4`).
5. ✅ `ASSETS-LICENSE.md` reescrito com o regime de duas zonas + livros +
   marca + permissão de conteúdo de fã (`LIC-APACHE-5`, esta ADR).
6. ✅ `ADR-005` marcado `SUPERADO` (`LIC-APACHE-5`, esta ADR).
7. ⏳ Verificação independente do implementer (`LIC-APACHE-6`): contagem final
   de `GPL` fora de `_deps`/`build` deve ser zero em código, com `git grep -i`.

## Cross-refs

- [ADR-005](ADR-005-license-gpl3-assets-ccbysa.md) (decisão superada por esta;
  já previa o relicenciamento na própria linha 8).
- `LICENSE` (texto Apache License 2.0 verbatim, imutável).
- `NOTICE` (copyright + carve-out de marca, propagado pela seção 4(d)).
- `ASSETS-LICENSE.md` (regime de assets em duas zonas + livros + marca + fã).
- `THIRD-PARTY-LICENSES.md` (atribuição de terceiros, inclusive o caso `glad`).
- Apache License 2.0: <https://www.apache.org/licenses/LICENSE-2.0> · Steamworks
  e licenças open source: <https://partner.steamgames.com/doc/sdk/uploading/distributing_opensource>
  · CC-BY-SA 4.0: <https://creativecommons.org/licenses/by-sa/4.0/>

*Recomendação técnica de rotação de licença; a validação jurídica formal cabe
ao titular, inclusive: classes de registro de marca a proteger, nomes de
personagem cobertos pelo carve-out, e a redação final do parágrafo de
conteúdo de fã em `ASSETS-LICENSE.md`.*
