
# Lente de fluxo: ordenação topológica e ondas (GusWorld)

Escopo desta lente: só ordenação e agrupamento em ondas, com foco em paralelismo real por arquivo disjunto e limite de WIP. Não pontuo WSJF, não estimo esforço, não escrevo a tabela final. Falta detectada vai na lista ao final, separada.

Convenção de marcação: **[FATO: lei]** quando a ordem vem direto de uma restrição citável; **[INFERÊNCIA]** quando é dedução minha a partir da leitura combinada das leis e do corpus, e o Cosmo/líder deve confirmar.

---

## 0. Premissa que organiza tudo (INFERÊNCIA)

O primeiro commit (A12) não deve conter o corpus (`docs/`, `resources/`) ainda, porque `docs/`/`resources/` só pode ir a público depois que o grupo B (limpeza) fechar, e a L-15 diz que corrigir versionamento depois do primeiro push custa reescrita de histórico. Logo assumo dois momentos de push distintos: (1) A12 sobe só a fundação do repositório (LICENSE, `.gitignore`, README etc); (2) um push posterior, não numerado na lista bruta, sobe o corpus já limpo por B. Isto não está escrito explicitamente nas leis; é a leitura mais segura dado L-15 combinada com o fato de o remoto já ser público. Trago para confirmação do Cosmo/líder.

---

## 1. Ondas

### W1: A1, A2, A5, A9, A10, A11
**Gatilhos de decisão surfaçados em paralelo (não consomem WIP de engenharia):** G1, G2, G3, G4, e uma quinta decisão não numerada que trato como gate: **B5** (a tensão `.dlg.txt`/Markdown é fonte de build ou formato de runtime, isso é decisão do líder, não execução, mesmo estando listada no grupo B). Chamo essa de "G5" só para referência interna, sem inventar item novo na tabela.

- **De fato paralelo:** os seis itens de trabalho tocam arquivos disjuntos e nenhum depende de outro: `.gitignore` (A1), `.gitattributes` (A2), `LICENSE` (A5), `OFFLINE-NOTICE.md` já redigido pelo Cláudio (A9), `THIRD-PARTY-LICENSES.md` (A10), e o template/convenção de cabeçalho SPDX (A11).
- **Só parece bloqueado, mas não é:** A11 é tratado no corpus como "um item da fundação", mas na verdade é uma **prática contínua** ("em todo arquivo novo, desde o primeiro", L-08) que se aplica também a C e D adiante. Não fecha nesta onda; o que fecha aqui é só a definição do template do cabeçalho.
- **Gates:** G1, G2, G3, G4 e B5/G5 custam tempo do líder, não WIP de engenharia. Surfaçá-los agora, mesmo que o trabalho que eles destravam só rode muito depois (G2 destrava E8, G4 destrava F1, G3 destrava um item que nem existe na lista, ver lacunas), evita que a fila trave lá na frente por decisão não pedida a tempo.
- **Limite de WIP:** 6 agentes em paralelo é tecnicamente sem colisão de arquivo, mas recomendo cap de **4 agentes simultâneos** por essa onda (coordenação de PR, revisão), deixando 2 itens (ex.: A9, A10, que já têm texto pronto) para logo em seguida sem custo de fluxo.

### W2: A6, A3, A8
- A6 (`LICENSES/`) depende de A5 (mesmo texto de licença). A3 (init do git-crypt) depende de A2 (a marcação em `.gitattributes` que diz o que ele cifra). A8 (`README.md`) depende de A5 e A9.
- **De fato paralelo:** os três tocam arquivos disjuntos (`LICENSES/*.txt`; init/chave do git-crypt; `README.md`).
- **WIP: 3.**

### W3: A4, A7
- A4 (corrigir a frase falsa nos dois arquivos de `docs/_secret/`) depende de A3 já configurado, para descrever o mecanismo real de proteção. A7 (`REUSE.toml`) depende de A6, A2 e A11.
- **De fato paralelo:** arquivos disjuntos (`docs/_secret/*.md` vs `REUSE.toml`).
- **WIP: 2.**

### W4: A12
**[FATO: A12 exige tudo de A1-A11 antes, por ordem direta do escopo desta lente.]** Item único, gate do primeiro commit. Sem paralelismo possível aqui: é literalmente o ato de configurar o remoto e commitar. **WIP: 1.**

### W5: B1, B2, B6 | C1 (duas trilhas paralelas)
- **Trilha B:** B1 (reparo do `__DEP_REMOVIDA__`) só começa depois de G1 resolvido (aprovação em bloco da lista). B2 (remover nome/bio/foto do Elon Musk de dois arquivos específicos) e B6 (conferência visual amostral das 1266 imagens) não dependem de G1 e tocam arquivos totalmente diferentes de B1.
- **Trilha C:** C1 (`CMakeLists.txt` raiz) é o primeiro item de C, sem dependência.
- **De fato paralelo:** B (trilha de corpus/texto) e C (trilha de build/CI) não compartilham nenhum arquivo, então correm em paralelo verdadeiro entre si. Dentro de B, B1/B2/B6 tocam arquivos diferentes (corpus geral vs os dois arquivos do Elon Musk vs imagens), paralelo real.
- **WIP: 4** (B1, B2, B6, C1).

### W6: B4 | C2, C3, C4, C5, C6, C8
- **Trilha B:** B4, as "sete pontas soltas". **Atenção:** duas das sete tocam o **mesmo arquivo** (`plano_vs.md`: uma vez pelo vocabulário .NET, outra pela contradição de plataforma), então dentro de B4 há uma colisão interna; as outras cinco (bloco histórico em `_INDEX.md`, títulos "Especificação Técnica de Asset 3D" nas oito specs, linha órfã em `style-guide.md`, citações órfãs em `en_intl.md`, e a ponta em `raid-log.md`) são de fato disjuntas entre si e paralelas.
- **Trilha C:** C1 já fechou. C2 (gate de CI de camadas), C4 (matriz de 5 plataformas) e C5 (os quatro portões de qualidade) provavelmente editam o **mesmo arquivo de workflow de CI**, então são apenas aparentemente paralelos: recomendo sequenciar C2 -> C4 -> C5 dentro da mesma onda, ou separar em arquivos de workflow distintos desde já. C3 (harness de teste, código C++) e C6 (`clang-format`) e C8 (hook de TDD) são genuinamente disjuntos e paralelos a tudo.
- **WIP: 4** (as duas sub-sequências de C contam como 1 "fatia" cada mesmo tendo 3 itens dentro; some com C3/C6/C8 e a parte não colidente de B4, o número real de agentes simultâneos fica entre 4 e 5).

### W7: B3 | C7
- B3 (trocar caminhos absolutos por genérico nos arquivos públicos) fica para depois de B1 e B4 de propósito: é uma varredura ampla, e rodar antes arrisca colidir com as mesmas linhas que B1/B4 já tocaram. C7 (script local espelhando o CI) só faz sentido depois que C2/C4/C5/C6 estiverem definidos.
- **De fato paralelo:** B3 (docs) e C7 (script local) são arquivos disjuntos.
- Neste ponto o corpus está limpo e pode ser commitado/pushado (ver premissa 0), e o grupo C fecha por inteiro, cumprindo L-20 (fundação de build e CI como primeira fatia, antes do primeiro módulo de comportamento).
- **WIP: 2.**

### W8: D1
Primeiro módulo de comportamento real, agora sim sob TDD estrito (L-19). Sem dependência dentro de D. **WIP: 1.**

### W9: D2, D3
D2 (sorteio determinístico contado) e D3 (POCO de carta/instância/estado físico) dependem só de D1. **De fato paralelo** se os arquivos forem `core/rng.*` vs `domain/card.*` (arquivos disjuntos). **WIP: 2.**

### W10: D4
D4 (deck, mão, coleção) depende de D3 (precisa do tipo de instância de carta já existir). Item único. **WIP: 1.**

### W11: D5, D7, D8, D9, D10
Cinco módulos que dependem só de D1-D4, não uns dos outros: combate (D5), economia (D7), progressão (D8), diálogo como dado (D9, mas só depois de B5/G5 resolvido quanto ao formato), mapa como dado (D10). **De fato paralelo** por arquivo (`combat.*`, `economy.*`, `progression.*`, `dialogue.*`, `map.*`).
- **Risco de excesso de WIP:** cinco frentes simultâneas de domínio é tecnicamente sem colisão de arquivo, mas fere o princípio "time terminando > time começando" (fluxo > paralelismo teórico). **Recomendo WIP cap de 3** nesta onda, com D9 e D10 esperando vaga (D9 já depende de decisão externa de qualquer forma; D10 tem um risco de canon não resolvido, ver seção de riscos).
- **WIP recomendado: 3, com fila de espera para os outros 2.**

### W12: D6
D6 (efeitos de status + contrato de evento) depende de D5 fechado (o combate é onde o status se resolve). Coordenar a interface entre `status.*` e `combat.*` (arquivos vizinhos, não totalmente disjuntos). **WIP: 1.**

### W13: F1, F2, F3 (execução das ideias do Gus, depois de D5-D10 fecharem)
- F1 (carta glitch) depende de D10 (mapa/bloqueio) e D3/D4 (catálogo), e adicionalmente de **G4** resolvido (os dois pontos em aberto devolvidos a ele na issue 3). F2 (carta urandom) depende de D3 (catálogo) e D8 (RunaDex/progressão). F3 (efeito de zip-bomb em batalha) depende de D5/D6 (a ficha técnica já existe no design; falta só o efeito).
- **F1 e F2 tocam o mesmo arquivo de catálogo de cartas** (`content/cards.*` ou equivalente): não são paralelos entre si, mesmo sem dependência lógica direta. Sequenciar F1 -> F2 (ou F2 -> F1, ordem indiferente entre eles).
- **F3 é de fato paralelo** a F1/F2, por tocar o arquivo de efeitos de combate, disjunto do catálogo.
- **Registro vs execução (L-07):** as ideias já entraram na tabela no ato da aprovação; esta onda é só a execução, e ela entra "sem atropelar o que está rodando", ou seja, depois de D5-D10 fechados, não antes.
- **WIP: 2** (F3 em paralelo com a dupla sequencial F1->F2).

### W14: D11
Save model como estado canônico serializável. Depende de **todo** D3-D10 (é o agregador: o save precisa capturar o estado de cada sistema). Também depende de F1/F2/F3 já terem entrado (senão o save nasce incompleto e precisa retrabalho). Item único, WIP: 1.

### W15: D13
Teste de replay determinístico (semente + lista de comandos reproduz o estado final byte a byte). Depende de D11 fechado e do contrato comando/evento (D12) já aplicado em cada sistema. **WIP: 1.**

### W16: D14
Auto-resolve de combate para balanceamento em massa. Depende de D5 (combate) e da infraestrutura de replay/determinismo de D13 para rodar simulações confiáveis em lote. **WIP: 1.**

### Trilha E, em paralelo a partir de W11 (correndo ao lado de D6, F, D11, D13, D14)

- **E2** (serialização binária dos POCOs), **E3** (validador semântico) e **E4** (gravação atômica + backups) podem começar assim que D3/D4/D10 existirem (final de W9/W11), sem precisar de criptografia. Arquivos disjuntos entre si, e disjuntos das frentes de D6/F/D11 que rodam ao lado. **De fato paralelo** a essas.
- **E1** (envelope binário com selo autenticado) é o ponto onde a trilha esbarra na falta de criptografia no GlintFx. Pela L-05 (proibido dublê) e pela L-25 (as primitivas vêm do GlintFx, nada de biblioteca de terceiro nem código caseiro), o selo autenticado **não pode ser escrito de verdade** até o GlintFx entregar as primitivas. O fluxo correto, por desenho da própria L-07/L-25, é: começar E1, esbarrar de fato na falta, registrar **E7** no bus imediatamente nesse ponto (não depois de E6, como a ordem bruta sugere, ver correções abaixo), e **parar, esperando a resposta** (Lei Zero).
- **E6** (ferramenta de inspeção/re-selo) nasce junto com o formato (L-25 explícito: "nasce junto com o formato, não depois"), então trava exatamente junto com E1.
- **E5** (config selada nunca impede o boot) tem uma parte que dá para escrever agora (o comportamento de fallback para padrão de fábrica), mas a checagem real do selo depende de E1. Fica parcialmente bloqueado.
- **E8** (gerador de build com tabelas compiladas + pacote binário selado), **E9** (fase 2, save híbrido com cadeia de comandos) e **E11** (amarra de máquina no Hardcore) dependem todos, em cascata, de E1 fechado. Ficam **bloqueados sem data**, no mesmo padrão do `present/`.
- **E10** (fase 3, âncora TPM) é opcional por lei ("nunca requisito para jogar") e nunca bloqueia nada; fica no fim da fila, sem pressão de agenda.

### Bloqueado, sem onda, sem data
**present/ inteiro** (L-06): não entra em nenhuma onda com data implícita. Só começa quando o GlintFx tiver janela, contexto gráfico, entrada e texto, hoje inexistentes (o GlintFx tem um header e nenhuma data prometida). Junto dele, pela cascata acima, ficam **E1, E6, E5 (parcial), E8, E9, E11**: uma segunda frente bloqueada pela mesma causa raiz (dependência externa no GlintFx, aqui criptografia em vez de UI).

### TST-*/AUD-* (a skill injeta os IDs; aqui só a regra de posição)
- Teste unitário nasce com o próprio código, dentro de cada item de D/E/C, não é item à parte.
- Teste não unitário (`TST-*`) entra na fila **imediatamente após a onda que fecha o que ele cobre**: por exemplo, o teste de integração de combate entra logo depois de D5/D6 fecharem, não antes.
- Auditoria (`AUD-*`) é downstream de código + teste, e o dossiê formal (`AUDITORIAS.md`, pelo `internal-auditor`) é **portão de release antes da 1.0** (L-19): fica perto do fim da fila, antes da tag. Os quatro portões automáticos (zero aviso com `-Werror`, ASan/UBSan, `clang-tidy`/`cppcheck`, `gitleaks`) já rodam em toda fatia fechada desde que C existe, não esperam o fim.

### Onda final: H1, H2
**[FATO: regra permanente da casa, citada no prompt.]** Wiki (H1) e documentação para iniciante (H2) são sempre os últimos itens, com pré-requisito de **tag de versão**. Tag exige aval explícito do líder (L-23) e, na prática, uma tag que signifique algo para o jogador pressupõe pelo menos um loop jogável, o que exige `present/`. **Logo H1/H2 herdam, por transitividade, o mesmo bloqueio sem data do `present/` e da trilha de criptografia**: não é só "são os últimos da fila", é "a fila inteira pode ficar esperando o GlintFx antes de chegar neles".

---

## 2. Riscos de fluxo

1. **O maior risco do projeto inteiro é o convergir de três caudas na mesma dependência externa sem data**: `present/` (L-06), a trilha de criptografia de E (E1/E6/E8/E9/E11), e H1/H2 (via pré-requisito de tag). Isso significa que, depois que D (domínio) e a parte não-criptográfica de E se esgotarem, **o time pode ficar sem trabalho desbloqueado nenhum**, esperando resposta do GlintFx em duas frentes diferentes (UI e criptografia) ao mesmo tempo. Vale antecipar ao líder: o que a equipe faz nesse vácuo (mais profundidade de domínio via D14, mais conteúdo tipo F, documentação, ou ociosidade honesta) é uma decisão dele, não deve ser resolvida com dublê disfarçado (L-05).
2. **Aging WIP previsível em E1**: pelo próprio desenho da lei (começar, esbarrar, registrar no bus, esperar parado), E1 é um item que entra "em progresso" e fica lá indefinidamente até resposta externa. Se não for tratado como bloqueado explicitamente na tabela (e não como "em andamento"), ele vai poluir a métrica de aging WIP com um item que na verdade está parado por decisão de arquitetura, não por atraso do time.
3. **G1 é o gate mais urgente**: ele trava B1, que por sua vez trava o corpus poder ser commitado, que por sua vez atrasa qualquer trabalho de D que dependa da leitura correta do corpus (L-01). Recomendo que o Cosmo cobre essa decisão do líder antes mesmo de W1 fechar.
4. **G3 trava um item que não existe na lista bruta** (ver lacunas): o risco de fluxo aqui é o gate ficar aberto sem nada visivelmente esperando por ele, e ninguém perceber que há um buraco de escopo até alguém precisar mexer em "modos de morte".
5. **W11 (cinco domínios em paralelo) é o ponto de maior tentação de estourar WIP.** Recomendo o cap de 3 citado ali, mesmo sendo tecnicamente livre de colisão de arquivo: paralelismo teórico sem limite de WIP é multitasking, não fluxo.
6. **B1/B3/B4 competem por texto no mesmo corpus** (risco de colisão de arquivo maior do que o número de itens sugere, em especial `plano_vs.md` tocado por dois sub-itens de B4 e possivelmente de novo por B3). Recomendo escopo por arquivo explícito na ordem de serviço de cada agente dessa trilha, não só por ID.
7. **C2/C4/C5 provavelmente disputam o mesmo arquivo de workflow de CI.** Se ninguém separar em arquivos de workflow distintos desde já, a onda W6 vira sequencial onde parecia paralela.

---

## 3. Violações de ordem que corrigi ao montar

1. **E7 (registro no bus) aparece na posição 7 da lista bruta de E, sugerindo que roda depois de E1-E6.** Pela própria L-25/L-07 ("o pedido ao bus é registrado quando a fatia do envelope começar"), E7 dispara **no início** da tentativa de E1, não depois de E6. Recoloquei E7 grudado no início da tentativa de E1, não como item posterior.
2. **E6 aparece na posição 6, depois de E2-E5 na lista bruta.** A lei diz explicitamente que a ferramenta de inspeção "nasce junto com o formato, não depois". Recoloquei E6 pareado com E1, não numa sequência posterior a E2-E5.
3. **D12 (contrato comando/evento) aparece como item 12, depois de D1-D11 na lista bruta**, dando a impressão de ser um passo posterior de "aplicar o contrato" no fim. Na leitura da L-17, o contrato é aplicado **na fronteira de cada sistema conforme ele é escrito** (D5, D6, D7 etc., cada um já nasce como `aplica(estado, comando) -> (estado, eventos)`), não como uma tarefa separada rodando depois de tudo. Tratei D12 como transversal, embutido em cada item de D5 em diante, não como onda própria.
4. **B5 (a tensão `.dlg.txt`/Markdown) está no meio da lista bruta do grupo B**, dando a impressão de ser trabalho de limpeza como os outros. Na leitura da própria frase ("decidir e tratar"), a parte "decidir" é gate do líder, não execução, e portanto deveria estar junto com G1-G4 no início, não no meio de B. Recoloquei como quinta decisão a surfaçar em W1.
5. **A11 (cabeçalhos SPDX) está listado como item da fundação (A), dando a impressão de tarefa que fecha em W1-W4.** Na verdade é prática contínua "em todo arquivo novo, desde o primeiro" (L-08), que atravessa C, D, E e F inteiros. Tratei como convenção que nasce em W1 mas nunca "fecha" sozinha.
6. **A ordem bruta não distingue F1 (registro, já feito) de F1 (execução, pendente)**, o que na leitura literal da lista parece um item só. Segui a L-07 e separei: a entrada na tabela já aconteceu (é por isso que o item existe na lista bruta); o que eu ordenei aqui foi só a execução, na onda W13, depois de D5-D10.

---

## 4. Lacunas detectadas (NÃO virei item, só reporto)

1. **Não existe, em nenhum grupo da lista bruta, um item para a camada `app/`** (fluxo do jogo, cenas, avanço de turno, coordenação entre domínios), que a L-17 exige como uma das cinco camadas. Sem ela, F4 (missão com relógio correndo, inclusive durante batalhas) não tem onde pousar: falta o item, não só a onda.
2. **G3 ("confirmar o que sobrevive do desenho antigo dos modos de morte") não tem nenhum item de execução correspondente em D, E ou F.** Ou o conteúdo de "modos de morte" já está implicitamente coberto por D5/D6/E11 e ninguém nomeou, ou é um item que falta criar depois que o líder decidir o que sobrevive.
3. **As quatro contradições de canon listadas na L-13** (`pillars.md` cel-shaded 3D; `core-loop-exploracao.md` câmera orbital, já resolvida pela L-26 mas o texto ainda não foi apagado; `knowledge-gates.md` como esquema de save em GDScript, a re-derivar; `docs/design/producao/*` descrevendo pipeline Godot/C#) **não aparecem como itens em nenhum grupo da lista bruta**, nem em B nem em D. Isso é particularmente sério para **D10** (mapa, que depende da correção de `core-loop-exploracao.md`) e **D11** (save, que depende da re-derivação do esquema de `knowledge-gates.md`): pela L-13, trabalho que depende de canon desatualizado está bloqueado até o canon ser corrigido, e hoje não há item nenhum cobrindo essa correção.
4. **A "ideia da bateria acabando dentro da parede", citada em G4, não aparece como item F próprio** (só a carta glitch em batalha aparece, em F1). Ou ela é a mesma coisa que F1 sob outro nome, ou é uma ideia do Gus ainda sem entrada própria na tabela, o que seria uma violação silenciosa da regra "as ideias do Gus entram na tabela de imediato" (L-07) se realmente for distinta.
5. **Não há item explícito para "commitar o corpus depois de B fechar"** (ver premissa 0): a lista bruta trata A12 como "o" primeiro commit e não prevê um segundo commit/push para o corpus limpo. Se a intenção do líder for um único commit grande incluindo tudo, isso muda a ordem entre A12 e B (B teria que fechar inteiro antes de A12, não depois).
