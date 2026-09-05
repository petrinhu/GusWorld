# E12: reavaliação de proporcionalidade de `E9` e `E10` (blindagem de save, L-25)

> Item da tabela: `E12` (`TODO.md:118`). Ponteiros (L-30): `GODS_LAWS.md` L-25 (`:158-169`), L-29 (`C-08`, `:195`), L-17 (`:126-128`); `docs/_processo/opcoes-protecao-save.md` (a fonte que fixa as ameaças e as defesas); `docs/design/mecanicas/modos-morte.md` §2.3 (a promessa do Hardcore); `docs/tech/convencao-formatos-gw.md:89-114` (o que o save guarda de conquista); `docs/design/mecanicas/conquistas.md` §0 e §1; `TODO.md` itens `E9` (`:234`), `E10` (`:236`), `E11` (`:223`), `E3` (`:229`), `D13` (`:221`), `G20` (`:130`).
> Autor: `security-engineer` (agente). Data: 05/09/2026. Decisão final é do líder (L-11, L-14): este documento avalia e devolve; não muda lei, tabela nem escopo.
> Convenção de leitura (L-18 global): **FATO** é citação com arquivo e linha, ou verbatim do líder; **INFERÊNCIA** é leitura deste agente e está marcada como tal.

## 1. A premissa nova, e o que ela muda de fato

**FATO.** Em 25/08/2026 o líder revogou a metade do `C-08` que proibia conquista (commit `6dca2ef`, verbatim do líder na mensagem do commit: *"inclusive quero usar conquistas steam"*). Decisão registrada no mesmo commit: conquista **interna** agora; integração com a Steam decidida quando a distribuição for real. Em 28/08/2026 o alcance foi corrigido: o `C-08` sobrevive estreitado, banindo só placar e ranking entre jogadores (`GODS_LAWS.md:195`; `docs/design/gdd.md:151`, que registra também o motivo do adiamento da Steam: o SDK do Steamworks é proprietário e colide com a AGPL herdada do GlintFx, L-08).

**FATO.** O que o save guarda de conquista está decidido: *"o conjunto dos identificadores (`id`) estáveis das conquistas já destravadas"*, dentro do envelope selado (`convencao-formatos-gw.md:102`; `:96`). O conjunto é monotônico por construção: um `id` está dentro ou fora, nada registra segunda vez, e reocorrência do gatilho depois de destravada é no-op (`conquistas.md:20-24`, regras 1 a 3 do §0).

**FATO.** Há sete conquistas especificadas (`conquistas.md:34-42`), todas internas, quatro com gatilho coberto no corpus e três com lacuna de definição levada ao líder. Nenhuma tem por condição vencer, sobreviver ou terminar alguma coisa no Hardcore; as condições cobertas são contadores (bestiário documentado, derrotas na mesma cena, erros seguidos num minigame) e uma cena.

**INFERÊNCIA (o que a premissa altera na conta).** A avaliação anterior partia de "não há o que fraudar". Agora há: o conjunto de `id` destravados é um **ativo novo** dentro do save, e um save forjado pode inflá-lo. O que a premissa **não** faz é criar uma **ameaça nova**: forjar um `id` no conjunto é uma instância das ameaças que a fonte já enumera (seção 2). A pergunta correta desta reavaliação, portanto, é: em qual coluna de ameaça a conquista cai, e a fase da L-25 que responde por essa coluna precisa mudar por causa dela?

## 2. O mapa que a fonte já fixa: cada fase responde por uma ameaça, e elas não se trocam

**FATO.** `opcoes-protecao-save.md:28-34` enumera cinco adversários, do mais fraco ao mais forte:

| # | Adversário (`opcoes-protecao-save.md`) | Defesa que a fonte nomeia | Linha |
|---|---|---|---|
| A1 | corrupção acidental | hash de integridade, gravação atômica, cadeia de backups | `:30` |
| A2 | editor casual (abre o arquivo, troca o número) | formato binário mais selo | `:31` |
| A3 | editor determinado (lê o fonte, deriva a chave, recompila, edita RAM) | *"NÃO. Inalcançável por construção"* em prevenção; só o replay semântico obriga a *"forjar história jogável"* | `:32`, `:110` |
| A4 | rollback (copia a pasta antes de arriscar, restaura se der errado) | *"Âncora out-of-band + machine-binding derrotam o rollback casual"*, sem menção a replay | `:33` |
| A5 | distribuição de save adulterado a terceiros | machine-binding no Hardcore; fora dele o dano é de quem baixou | `:34` |

**FATO.** A tabela comparativa da mesma fonte (`:107-110`) mantém A3 e A4 como **colunas distintas**, e na linha da Opção 2 (a escolhida, L-25) o replay aparece só na coluna A3 (*"Parcial (replay semântico obriga a forjar história jogável)"*), enquanto a coluna A4 traz *"Hardcore: sim, com teto TPM"*.

**FATO.** As três fases da L-25 (`GODS_LAWS.md:162`): envelope selado; catálogo compilado com save híbrido verificável por reexecução; âncora anti-rollback por chip de segurança do hardware, opcional. Os itens: `E9` é a fase 2 (`TODO.md:234`, *"save híbrido de estado mais registro de comandos com encadeamento, verificável por re-execução"*), `E10` é a fase 3 (`TODO.md:236`, *"âncora anti-rollback por TPM, com retorno à âncora em arquivo onde não houver; NUNCA requisito para jogar"*).

**INFERÊNCIA, a correspondência que sustenta tudo o que segue:**

- **`E9` responde por A3.** A pergunta que o replay faz ao save é *"existe uma história de comandos que, aplicada ao estado anterior sob as regras do jogo, chega neste estado?"* (`opcoes-protecao-save.md:93`). Um estado inteiro e internamente coerente, montado do zero, passa pelo selo (a chave está no fonte, `:83`) e passa pelo validador semântico (`E3`, que confere faixas e invariantes, não história); só cai no replay, porque a história não existe.
- **`E10` responde por A4.** Um backup restaurado é um estado **legítimo com história legítima**: ele foi gravado pelo próprio jogo. O replay o valida com sucesso, porque a pergunta que o replay faz tem resposta "sim". A única pergunta que pega o rollback é outra: *"este save é o mais recente que esta máquina gravou?"*, e ela só se responde com um contador que vive **fora** do save (`modos-morte.md:74`, Camada 1, *"a que de fato rejeita a cópia restaurada"*; `:78`, load rejeitado quando a sequência do save não bate com a âncora externa; `:79`, na morte a âncora marca a run como morta e qualquer cópia restaurada depois é recusada). O `E10` é o teto dessa âncora, movida para o TPM onde houver (`opcoes-protecao-save.md:94`).

É por isso que a L-25 tem uma fase 3 separada da fase 2: as duas perguntas são diferentes, e nenhum crescimento da fase 2 responde a pergunta da fase 3.

## 3. Onde a conquista cai em cada coluna

**Contra A1 e A2: fase 1, sem tocar em `E9` nem `E10`.** FATO: o selo (`E1`) detecta byte alterado; o validador semântico (`E3`, `TODO.md:229`) recusa estado internamente inconsistente. INFERÊNCIA: para conquista, o validador semântico pode e deve conferir a coerência entre o conjunto de `id` e os contadores que alimentam as condições (por exemplo, `achv_byte_collector` presente exige contador de documentação no limiar ou acima, `conquistas.md:66`). Isso barra o editor casual que insere um `id` sem mexer no resto. Não é trabalho de `E9`.

**Contra A3: é `E9`, e a conquista entra sem engenharia nova, com uma condição.** INFERÊNCIA: o forjador que monta um save coerente do zero (contadores no limiar, `id` no conjunto, missões marcadas) passa pela fase 1 inteira. Só o replay o pega. Para o replay cobrir a conquista, o destravamento tem de nascer **dentro** de `aplica(estado, comando)` como evento de domínio (L-17, `GODS_LAWS.md:128`), e não como campo gravado por fora do fluxo de comando. FATO: é exatamente o que o corpus já espera: `convencao-formatos-gw.md:114` diz que o destravamento *"deveria ser"* regra de domínio pela mesma lei que rege toda transição, e `conquistas.md:116` já amarra o incremento à identidade do evento de domínio por causa do replay. **Com essa condição satisfeita, a conquista é coberta pelo mesmo replay que cobre o resto do estado, e `E9` não precisa de peça adicional para ela.** Sem essa condição, `E9` não a cobre, por maior que seja.

**Contra A4: a conquista não muda nada, e o motivo de `E10` continua sendo o Hardcore.** INFERÊNCIA a partir de fatos citados: rollback move o estado **para trás**; o conjunto de `id` é monotônico (`conquistas.md:22`). Restaurar um backup **remove** conquistas do save, nunca acrescenta. Rollback só serviria a uma conquista cuja condição fosse "sobreviva a X" ou "termine Y sem morrer", e nenhuma das sete tem essa forma (`conquistas.md:34-42`). O que o rollback ataca é a promessa do Hardcore (`modos-morte.md:58`, o máximo de anti-rollback que dê para fazer offline, com servidor rejeitado; `:95`, o que se derrota é o cheater casual que copia a pasta antes de arriscar), e essa promessa existia antes da revogação do `C-08` e independe dela. Se um dia existir uma conquista ligada a completar o Hardcore, ela fica protegida por transitividade, porque o que ela atesta é justamente o que a âncora protege; `E10` não cresce por isso.

**Contra A5: nada muda.** FATO: fora do Hardcore não há amarra de máquina (`GODS_LAWS.md:164`; `E11`, `TODO.md:223`), e a fonte já aceita que um save baixado de fórum carrega no slot normal (`opcoes-protecao-save.md:34`). INFERÊNCIA: um save **legítimo** com todas as conquistas, baixado e carregado por terceiro, passa no replay de `E9`, porque a história é real. O replay distingue história forjada de história real; não distingue de quem é a história. Isso é o trade-off que a L-25 já assumiu ao restringir a amarra ao Hardcore, e a conquista interna não o reabre.

## 4. O que a Steam muda, e o que não muda

**FATO (documentação da Valve).** O destravamento de conquista na Steam é feito pelo **cliente**: o jogo chama `ISteamUserStats::SetAchievement` e depois `StoreStats` para enviar ao servidor da Steam ([Steamworks, Step by Step: Achievements](https://partner.steamgames.com/doc/features/achievements/ach_guide); [Stats and Achievements](https://partner.steamgames.com/doc/features/achievements)).

**FATO (ferramenta pública).** Existe ferramenta de código aberto que destrava ou retrava conquistas de qualquer jogo se passando pelo aplicativo perante a API da Steam, sem abrir o jogo nem tocar em save ([Steam Achievement Manager, repositório](https://github.com/mbwilding/steam-achievement-manager)). A única classe de conquista que ela não alcança é a **decidida em servidor do próprio jogo** ([guia da comunidade, jogos com conquistas server-side](https://steamcommunity.com/sharedfiles/filedetails/?id=3016956591)).

**FATO (deste projeto).** Servidor autoritativo foi rejeitado deliberadamente por soberania de dado (`modos-morte.md:58`, `:93`), e o jogo é single-player offline (`C-01`, `GODS_LAWS.md` L-29).

**INFERÊNCIA, e é o achado central desta reavaliação:** no dia em que a Steam entrar, a reivindicação passa a ser atestada por terceiro e visível em perfil público; esse é o cenário competitivo que faltava. Mas **o save deixa de ser o elo mais fraco antes mesmo de ser atacado**: a alegação à Steam é uma chamada de API feita na máquina do jogador, e o adversário com o fonte (A3) a emite sem save nenhum, seja recompilando o jogo, seja com a ferramenta pronta. Blindar o save com um `E9` maior, ou condicionar a chamada de destravamento ao replay bem-sucedido, cria fricção só contra o editor casual, que a fase 1 já barra. O ganho marginal para a conquista Steam é zero contra o adversário que importa. A conclusão da fonte, *"PREVENÇÃO é inalcançável; DETECÇÃO confiável é alcançável"* (`opcoes-protecao-save.md:56`), vale para a conquista Steam com força redobrada, porque ali nem a detecção é nossa: quem atesta é a Valve, e ela atesta o que o cliente mandar.

Consequência para a proporcionalidade: **a entrada da Steam não é motivo para `E9` crescer**. O que ela abre é uma decisão de **produto**, a tomar quando a distribuição for real: aceitar que a conquista Steam do GusWorld é tão falsificável quanto a de qualquer outro jogo sem servidor, ou não integrar. Nenhuma engenharia de save altera essa escolha, e este documento não a antecipa.

## 5. Custo: o que `D13` entrega e o que `E9` continua pagando sozinho

**FATO.** `D13` (`TODO.md:221`) é *"teste de replay determinístico permanente: semente mais lista de comandos reproduz o estado final byte a byte (L-17); detector das armadilhas de contêiner não ordenado e ponto flutuante em fórmula"*. É motor determinístico mais teste de CI.

**FATO.** A fonte marca a manutenção da Opção 2 como *"a mais alta das três, concentrada no replay versionado"* (`opcoes-protecao-save.md:96`) e nomeia o custo próprio: *"toda mudança de regra exige versionar o motor de replay ou invalidar logs antigos"* (`:93`; `:172`, risco E.6). A pergunta "replay contra qual ruleset" está aberta no `G20` (`TODO.md:130`), decisão do líder, sem onda, a tomar quando a fase 2 for desenhada.

**INFERÊNCIA (o que `D13` não amortiza):** o registro de comandos **dentro do envelope** (formato, política de ponto de verificação, tamanho do save); o **encadeamento por hash** entrada a entrada; a **reexecução no carregamento** (custo de load, que `D13` nunca paga porque roda em CI); o **versionamento do verificador** entre patches (`G20`); e a ferramenta de inspeção (`E6`) passando a entender o registro. `D13` barateia o motor; o resto do `E9` é custo específico de segurança e é onde a manutenção mora. Por isso a dificuldade **Alta** da tabela está correta, e **este documento não usa custo como argumento para o veredito**: a proporcionalidade de `E9` se sustenta pela ameaça que ele responde (A3, sobre todo o estado de progresso, com a conquista incluída) e pela verdade que a fonte fixa em `:56` e `:93`, de que é a única defesa que sobrevive ao adversário com o fonte na mão. Se um dia o líder decidir que A3 não merece defesa, `E9` cai inteiro; a conquista, sozinha, não o segura nem o derruba.

## 6. Veredito por item

| Item | Ameaça que resolve (fonte) | Veredito | Por quê, em uma linha |
|---|---|---|---|
| `E9` (fase 2) | A3, forjar história nunca jogada (`opcoes-protecao-save.md:32`, `:93`, `:110`) | **Mantém tamanho e desenho.** | A conquista é ativo novo dentro de uma ameaça já coberta; entra no replay sem peça extra, desde que destrave por evento de domínio dentro de `aplica()`. A Steam não muda a conta, porque a alegação à Steam contorna o save. |
| `E10` (fase 3, opcional) | A4, restaurar backup legítimo (`opcoes-protecao-save.md:33`, `:94`, `:110`; `modos-morte.md:74-79`) | **Mantém tamanho e desenho.** | Rollback só tira conquista do save, nunca põe. O motivo de `E10` é a promessa do Hardcore, anterior e independente do `C-08`; segue opcional e nunca requisito (L-25, `GODS_LAWS.md:162`). |

Nenhum dos dois cresce, nenhum muda de desenho, e nenhum é substituído pelo outro: um backup restaurado passa no replay de `E9` e só é recusado pela âncora de `E10`; um save forjado do zero passa pela âncora de `E10` (a sequência é forjada junto) e só cai no replay de `E9`.

## 7. O que volta ao líder (L-01 global, L-11)

1. **Ratificar o veredito da seção 6:** `E9` e `E10` no tamanho e desenho atuais, sem crescer por causa da revogação do `C-08` nem da futura Steam.
2. **Tornar explícita a condição de cobertura:** a conquista só fica protegida por `E9` se o destravamento nascer como evento de domínio dentro de `aplica(estado, comando)` (L-17), nunca como campo gravado fora do fluxo de comando. O corpus já aponta nessa direção (`convencao-formatos-gw.md:114`; `conquistas.md:116`), mas ninguém a fixou como requisito. A decisão é onde registrá-la: como nota no `D18` (especificação das conquistas), no `E9`, ou em ambos. Quem edita a tabela não é este agente.
3. **Nenhuma decisão nova sobre a Steam agora.** A que existe (integração decidida quando a distribuição for real, commit `6dca2ef`) fica como está. Este documento só registra, para a sessão que reabrir o tema, que a pergunta daquele dia será de produto (seção 4), e que nenhum crescimento de `E9` a responde.
4. **`G20` continua onde está** (sem onda, bloqueado pelo desenho da fase 2) e precisa ser decidido antes de `E9` ser desenhado; nada aqui o antecipa.
5. **Escopo:** esta reavaliação tocou só `E9` e `E10`. `C-08`, `E1` a `E8`, `E11` e o restante da tabela não são afetados.
