# E12 — Reavaliação de proporcionalidade de `E9`/`E10` (blindagem de save, L-25)

> Item da tabela: `E12` (prioridade Alta, onda 0). Ponteiros: `GODS_LAWS.md` L-25, L-29 (`C-08`, `C-13`), L-17, L-18; `TODO.md` itens `E1`–`E11`, `D13`.
> Autor: `security-engineer` (agente), a pedido do orquestrador. Decisão final é do líder (L-11, L-14).

## 1. Premissa nova

A avaliação anterior (lente de produto) tinha esta cadeia de raciocínio: *"`C-08` corta placar e conquista → não há cenário competitivo a proteger → `E9`/`E10` (fases 2 e 3 da blindagem de save) são desproporcionais para um jogo single-player offline."*

Essa premissa caiu em duas etapas, registradas em `GODS_LAWS.md`:

- **25/08/2026:** o líder revogou a metade do `C-08` que proibia conquista. Verbatim registrado na lei: *"O jogo TEM conquistas."*
- **28/08/2026:** o alcance foi corrigido. O `C-08` **não foi apagado inteiro** — ele volta à L-29 **estreitado**, banindo só **placar e ranking entre jogadores**. O líder registrou também que quer **conquistas da Steam quando a distribuição for real**, hoje adiadas por incompatibilidade entre o SDK proprietário do Steamworks e a licença AGPL herdada do GlintFx (L-08), não por falta de vontade.

Ou seja: **existe hoje** o cenário que faltava — conquista destravada é uma reivindicação que um save forjado pode fraudar — e **não existe** o cenário de ranking entre jogadores, que continua cortado.

Esta reavaliação parte daqui, não repete a pergunta antiga, e devolve um veredicto por item, não um veredicto único — porque, como o próprio corpo do trabalho revela abaixo, a proporcionalidade de `E9` depende de um fato que ainda não aconteceu: a integração com a Steam.

## 2. Modelo de ameaça

**Ativo protegido:** o estado de progresso do jogador — em particular, a partir de 25/08/2026, os **sinalizadores de conquista desbloqueada** dentro do save, que antes não existiam como alvo de fraude porque a lei anterior os proibia.

**Atores e fronteira de confiança:** o jogo é single-player e offline (`C-01`), sem servidor, sem conta e sem telemetria (ressalva da L-08). **Não há fronteira de confiança entre "cliente" e "servidor"**, porque não existe servidor. O binário inteiro roda na máquina do único ator que importa: **o próprio jogador**, que é ao mesmo tempo o usuário legítimo e o único adversário possível hoje. Não há terceiro jogador para enganar, e não há juiz externo (plataforma, ranking, torneio) para iludir.

**O que o adversário ganha ao forjar uma conquista, hoje:**

- Um sinalizador local marcado como "desbloqueado" sem ter cumprido a condição.
- Ganho **puramente cosmético e não-atestado**: nenhuma plataforma confirma a alegação, nenhum outro jogador é comparado (`C-08` corta exatamente isso), e nenhum mecanismo do próprio jogo hoje amplifica a conquista fraudada (sem recompensa de gameplay atrelada, pelo que o corpus registra). É a mesma categoria de "trapaça" que um code cheat em jogo single-player sempre foi: o jogador engana só a si mesmo, e a comunidade não tem como verificar a alegação porque não há selo de terceiro.
- Se compartilhada (print, vídeo), a alegação **já nasce sem valor probatório**, porque qualquer print de "100% conquistas" num jogo FOSS sem atestação externa é, por definição, inverificável — com ou sem blindagem de save.

**O que o adversário ganharia ao forjar uma conquista, no dia em que a Steam entrar:**

- Aí sim a conquista passa a ser **atestada por terceiro**: aparece no perfil público do jogador, conta para estatística agregada da Valve ("X% dos jogadores desbloquearam"), e é comparável entre jogadores mesmo sem existir um "ranking" do GusWorld em si — o `C-08` corta o *nosso* placar, não impede que a Steam exponha estatística própria de conquista sobre qualquer jogo que a use. **Esse é o cenário genuinamente competitivo que faltava**, e ele está, por decisão do líder, **fora de escopo agora** (a integração de conquista com a Steam foi explicitamente adiada, não descartada).

**Custo do ataque:** baixo a médio, hoje, independente de `E9`/`E10`. O jogo é FOSS sob AGPL (L-08): o adversário tem o código-fonte e o algoritmo de sempre (tensão que a própria L-18 já assume e proíbe negar em comunicação pública). O envelope selado da Fase 1 (`E1`–`E6`) já sobe o piso de quem edita save à mão; o validador semântico (`E3`) já sobe mais, recusando estado internamente inconsistente. Um forjador casual (editor de hex, ferramenta de terceiro) é barrado por aí. Um forjador que constrói um estado **inteiro e consistente** do zero (todas as missões marcadas concluídas, todos os flags coerentes entre si, sem nunca ter jogado) só é barrado por algo que prove **história**, não só **estado** — e essa é exatamente a lacuna que `E9` fecha.

**Custo da defesa:** `E9` é o item caro da tabela (dificuldade Alta, dependente de `D13`, bloqueado por seis módulos de domínio inteiros). `E10` é opcional por desenho e depende de `E9`.

## 3. Avaliação de `E9`

**O que `E9` entrega:** save híbrido de estado mais registro de comandos com encadeamento, verificável por **re-execução** — para forjar um save que passe, é preciso produzir uma história de comandos que legitimamente chegue àquele estado. A própria L-25 já registra que esta é "a única peça que não evapora quando o adversário lê o fonte", precisamente porque nasce da L-17 (determinismo por comando e evento), não de segredo.

**Achado central, que muda a conta de proporcionalidade:** a justificativa dominante de `E9` **nunca foi, sozinha, a fraude de conquista** — ela já existia antes da revogação do `C-08`, por dois motivos que continuam de pé independente de conquista:

1. **A promessa do modo Hardcore.** A L-25 já promete, para o próprio jogo (não para conquista): *"no Hardcore a morte apaga o save sem recuperação pelo jogo"*. Restaurar um backup de save para desfazer uma morte permanente **é rollback**, e rollback é exatamente o que `E9`/`E10` (encadeamento verificável, âncora anti-rollback) foram desenhados para impedir. Essa promessa existe desde a fase 2 original da L-25, antes de qualquer discussão de conquista, e sozinha já justifica o tamanho de `E9`.
2. **`E9` está amortizado por infraestrutura que o jogo precisa de qualquer forma.** O motor de re-execução determinística que `E9` usa para verificar save **é o mesmo** que a L-17 já exige para replay byte a byte (teste automatizado permanente, item `D13`) e para auto-resolve de balanceamento em massa. `E9` não pede um subsistema novo; ele reaproveita, para fins de segurança, um subsistema que o jogo constrói de qualquer forma para fins de design e QA. O custo **marginal** específico de segurança é menor do que o rótulo "fase cara" sugere.

**Onde a conquista entra, e por que ela não pede engenharia nova:** se a concessão de conquista for modelada como **evento de domínio** disparado por `aplica(estado, comando)` (a forma que a própria L-17 já manda), então o desbloqueio de conquista **já está** dentro do que o replay do log de comandos verifica — sem trabalho adicional. A conquista "pega carona" de graça no desenho que já existia. Isso é um argumento a favor de **não crescer**: o mecanismo certo para proteger conquista já está coberto pelo mecanismo que protege o resto do save, desde que a implementação trate conquista como evento e não como campo solto editável fora do fluxo de comando.

**A parte que a conquista, por si, NÃO justificaria, se fosse o único motivo:** um sistema de replay determinístico inteiro, com todo o custo de manter compatibilidade entre versões do jogo (patch de balanceamento pode invalidar retroativamente o replay de saves antigos — risco de engenharia real, já implícito no desenho, não novo desta reavaliação), seria desproporcional só para impedir que um jogador engane a si mesmo numa conquista que nenhuma plataforma atesta. Se `E9` não existisse já por causa do Hardcore, a fraude de conquista local sozinha **não pagaria** este preço.

**Conclusão de `E9`:** **continua do tamanho que está.** A premissa nova (conquistas existem) não abre um buraco que faltava fechar — o buraco já estava coberto pela promessa de Hardcore, e a conquista entra de graça por desenho. `E9` **não cresce hoje**.

**Mas a proporcionalidade não é permanente, e o gatilho tem de ficar escrito:** no dia em que a integração de conquista com a **Steam** entrar em escopo (hoje adiada por licença, não descartada — L-29 `C-08`), a conta muda. Nesse momento a fraude de conquista passa a ter alvo real (a Valve, a estatística pública, outros jogadores comparando perfis), e vale reabrir esta avaliação para decidir se o subconjunto de estado que alimenta a chamada da API do Steamworks precisa de um canal de verificação adicional (por exemplo, o replay se tornando pré-condição obrigatória, não só disponível, antes de qualquer chamada de desbloqueio à API externa). **Gatilho explícito: entrada da integração Steam em escopo do roadmap**, não a mera existência de conquista local.

## 4. Avaliação de `E10`

**O que `E10` entrega:** âncora anti-rollback por TPM, opcional, com retorno à âncora em arquivo onde não houver hardware; nunca requisito para jogar (a própria L-25 fixa isso).

**Achado:** `E10` está ligado à mesma promessa do Hardcore mencionada acima — impedir que o jogador restaure um backup de save mais antigo para desfazer uma morte permanente ou reverter uma decisão de consequência marcada como definitiva. Esta é uma ameaça de **rollback de arquivo local**, não de fraude de conquista. A revogação do `C-08` não introduz nenhum novo motivo para `E10` existir ou crescer: conquista local, sem atestação externa, não muda em nada o cálculo de "o jogador restaurou um backup para escapar de uma morte permanente" — essa ameaça já existia e já era o motivo de `E10`.

**Conclusão de `E10`:** **continua do tamanho que está**, e a decisão de mantê-lo **opcional e nunca obrigatório** (já fixada na L-25) segue correta sem alteração. A premissa nova não o toca.

## 5. Veredicto explícito por item

| Item | Veredicto | Justificativa curta |
|---|---|---|
| `E9` | **Mantém o tamanho atual.** | A conquista local não é o motivo dominante (já era o Hardcore + amortização via L-17/`D13`); conquista entra de graça se modelada como evento de domínio. Reabrir **somente** se a integração Steam entrar em escopo — aí a fraude passa a ter atestação de terceiro e a conta muda de fato. |
| `E10` | **Mantém o tamanho atual.** | Ligado exclusivamente à promessa de rollback do Hardcore, que é anterior e independente da questão de conquista. A revogação do `C-08` não introduz ameaça nova a este item. |

**Nenhum dos dois muda de desenho.** A recomendação é **não tocar** a especificação da L-25 nem o texto de `E9`/`E10` no `TODO.md` por causa desta premissa — a única mudança recomendada é de **registro**, não de escopo: anotar, junto de `E9`, o gatilho de reavaliação futura (entrada da Steam em escopo) para que a próxima sessão que olhar este item não repita a mesma pergunta do zero pela terceira vez.

## 6. O que fica como decisão do líder

1. **Confirmar ou rejeitar o veredicto acima** — `E9` e `E10` mantidos no tamanho atual, sem crescer por causa da revogação do `C-08`.
2. **Decidir se o gatilho de reavaliação futura (integração Steam) entra no `TODO.md`** como nota junto ao item `E9`, ou fica só registrado aqui — quem edita o `TODO.md` não é este agente (fora de escopo desta tarefa).
3. **Decidir se quer que a modelagem de conquista-como-evento-de-domínio** (a forma que faz a proteção "pegar carona" de graça em `E9`) vire um requisito explícito de arquitetura para quando o sistema de conquista for de fato implementado, ou se prefere deixar isso para o desenho detalhado daquele item quando chegar a vez dele na fila.
4. **Nenhuma ação deste documento afeta `C-08`, `E1`–`E8`, `E11` ou qualquer outro item da tabela** — o escopo desta reavaliação foi estritamente `E9` e `E10`, por ordem da tarefa.
