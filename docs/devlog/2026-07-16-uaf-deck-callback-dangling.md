# O bug que a suíte verde escondia

### Um heap-use-after-free no sistema de deck/mão, achado por uma sessão de IA que nem estava trabalhando nele

*16 de julho de 2026*

Toda suíte de testes verde carrega uma mentira pequena: "o que eu testei está certo". Ela nunca diz "o que eu não pensei em testar" está certo. Esse devlog é sobre o intervalo entre essas duas frases, um heap-use-after-free (UAF, carinhosamente chamado de "ler memória que já foi devolvida pro sistema") que viveu escondido dentro da fundação do sistema de deck/mão do GusWorld, com 2121 de 2121 testes passando, até ser desmascarado por um coredump que uma sessão de IA completamente diferente cruzou sem estar procurando.

## O sintoma

A onda `DECK-MAO-IMPL` construiu a fundação do sistema de cartas: um agregado `CardCollection` (o deck ativo e o cemitério de cartas descartadas), um `HandLoadout` (a mão como uma seleção de IDs dentro do deck) e as transações que movem carta de um lado pro outro (venda, upload). Em vários pontos desse código, alguém precisa saber o "tier" de uma carta (comum ou especial), e a resposta vem de um callback que o chamador injeta: `TierLookup tier_of(const std::string& card_id)`.

Parece trivial. E é, até você olhar de onde vem o `card_id` que se passa pra esse callback.

Em pontos como `HandLoadout::validate_candidate`, o código pegava um iterador `it` dentro do `std::vector` que guarda as cartas ativas de `CardCollection`, e passava `it->card_id` direto pro callback. Ou seja: não uma cópia da string, mas uma referência apontando pra dentro do buffer de memória do vector.

Isso é perigoso porque um `std::vector` não é uma casa fixa. Quando ele cresce além da capacidade reservada, o C++ realoca: pede um buffer novo, maior, copia tudo pra lá, e libera o buffer velho. Qualquer referência ou iterador que apontava pro buffer velho vira, no jargão, "pendurado" (dangling): aponta pra um endereço que já não é seu.

O problema é que `tier_of` é um callback opaco. Quem escreveu `validate_candidate` não controla o que esse callback faz por dentro. Se algum `tier_of` tocasse o próprio `CardCollection` (por exemplo, chamando algo como `add_to_active`, que cresce o vector), a referência que ainda estava sendo segurada por fora virava lixo no meio da própria chamada. Ler ela depois disso é comportamento indefinido: às vezes funciona por sorte, às vezes não.

## Por que os testes mentiam

A suíte inteira estava verde. Não "quase verde": **2121 de 2121**. E ainda assim o bug estava lá, plantado, esperando.

O motivo é desconfortavelmente simples: todo `tier_of` usado nos testes (e em todo o código de produção existente até aquele momento) era **puro**. Só consultava um mapa, nunca escrevia de volta no `CardCollection`. Então o gatilho do bug (um callback que realoca o vector no meio da leitura) nunca era exercitado. O contrato da API permitia o perigo, mas nada na base de código real o disparava ainda.

E tem uma segunda camada de disfarce, essa mais sutil: em build normal, sem instrumentação, ler uma referência pendurada nem sempre trava o programa. O alocador de memória do C++ geralmente não devolve o buffer liberado pro sistema operacional na hora; ele fica "livre mas ainda mapeado" por um tempo, então ler aquele endereço às vezes retorna o valor antigo, intacto, como se nada tivesse acontecido. É o tipo de bug que os programadores chamam de heisenbug: ele muda de comportamento dependendo de como a memória está organizada naquele instante, não da lógica do programa. Um dia funciona. Outro dia, sem nenhuma mudança de código, o mesmo teste (ou o mesmo save do jogador) estoura.

## O aviso que veio de fora

Aqui a história fica interessante. O dev do GusWorld roda três sessões de IA simultâneas na mesma máquina: uma cuidando do jogo (gusworld), uma da lib de UI usada pelo jogo (glintfx) e uma do site que documenta tudo isso. Elas não compartilham contexto de conversa, mas trocam mensagens por um "bus" baseado em git (um repositório que serve só pra isso).

A sessão do glintfx estava trabalhando em outra coisa, sem nenhuma relação direta com o sistema de deck. No meio do trabalho, ela deu uma passada pelos logs do sistema (`journalctl`/`coredumpctl`, as ferramentas do Linux que registram quando um programa morre de forma anormal) e encontrou coredumps do binário `gusengine_domain_tests`: dois às 20:53 (um deles um `SIGABRT` de "double-free" detectado pela própria biblioteca padrão do C, o outro um `SIGSEGV`, segmentation fault) e mais quatro às 21:06.

Em vez de ignorar (não era o trabalho dela naquele momento), ela investigou a stack trace, desmangulou os símbolos e mandou uma mensagem pelo bus com um diagnóstico cirúrgico: cheirava a string pendurada, um bug de lifetime (o tempo de vida de um objeto na memória) que se manifestava de duas formas diferentes dependendo do estado do heap no momento do crash.

Ou seja: o bug que a suíte verde da sessão do gusworld escondia foi apontado por uma sessão de IA que nem estava olhando pra esse código, só porque ela topou com os cadáveres na máquina compartilhada. Se não fosse essa comunicação entre sessões, esses coredumps provavelmente teriam sido ignorados como "ruído de teste antigo" e o bug continuaria latente, esperando um `tier_of` real (um dia, inevitavelmente, alguém ia escrever um que consulta E atualiza o deck no mesmo passo).

## A caça (com ASan)

Com a pista em mãos, veio a parte de transformar um bug fantasma em um bug determinístico, porque é impossível consertar direito o que não se consegue reproduzir com confiança.

A ferramenta certa pra isso é o AddressSanitizer (ASan), um instrumentador de memória que se liga ao binário na hora da compilação (`-fsanitize=address,undefined`) e passa a rastrear cada alocação e liberação. Em vez de deixar a leitura de memória liberada passar batido (como o build normal faz), o ASan aborta na hora, aponta a linha exata e mostra o histórico completo: quem alocou aquele endereço, quem liberou, e onde está a leitura inválida.

Rodado sob ASan, o problema ficou cravado: `hand_loadout.cpp:59`, um "READ heap-use-after-free", com o rodapé mostrando que a memória tinha sido "freed by add_to_active" e "previously allocated by add_to_active". Ou seja, o próprio ASan confirmava a hipótese da sessão do glintfx palavra por palavra.

Só que havia uma pegadinha: os coredumps originais eram de binários compilados ANTES do commit mais recente (o build-id não batia com o binário atual), e o código já commitado, com callbacks puros, não reproduzia o crash sozinho. Faltava o gatilho. Então a investigação escreveu um `tier_of` adversarial de propósito, um callback de teste que força uma realocação do vector (chamando `add_to_active`) bem no meio da consulta, exatamente o cenário que nenhum código de produção fazia ainda mas que a API permitia. Com esse callback malicioso plugado, o ASan pegava o UAF de forma determinística, toda vez.

## O conserto

A correção na raiz é conceitualmente simples, embora exija disciplina pra aplicar em todo lugar: copiar o `card_id` por VALOR antes de chamar o callback, em vez de passar uma referência pra dentro do container. Uma cópia de string é barata e, mais importante, desacopla a corretude da chamada do lifetime interno do `CardCollection`. Não importa mais o que o callback opaco faz por dentro; a cópia já é dona da sua própria memória.

Mas o conserto não parou no ponto que crashou. Seguindo a prática de "auditoria-dominó" (quando se acha um problema de lifetime, nunca se assume que é isolado, se varre a superfície inteira atrás do mesmo padrão), a correção foi aplicada nos três pontos que compartilhavam o mesmo contrato perigoso:

- `HandLoadout::validate_candidate` (o ponto original do coredump)
- a transação de venda/upload em `deck_transactions.cpp::remove_and_credit`
- o descarte e a venda dentro do próprio `CardCollection` (`discard_to_dead`/`remove_for_sale`), que além de copiar o valor também precisaram reancorar o iterador antes de apagar o elemento, porque `erase` também invalida iteradores

Pra garantir que o bug nunca mais volte sem ser notado, nasceu um teste de regressão novo, `deck_tier_callback_lifetime_test.cpp`, com seis casos usando o mesmo callback adversarial que expôs o problema. A prova ficou de mão dupla: sem o fix, sob ASan, três testes falhavam com UAF; com o fix, os seis casos passam limpos e a suíte de deck inteira (43 testes) segue limpa sob ASan também. A suíte normal, que já estava verde antes, continuou verde depois (foi de 2121 pra 2127 testes com a adição dos seis casos novos).

O fix ficou registrado no commit `d2601c1`.

## O que aprendemos

Cinco lições saem dessa caça, e valem pra qualquer código C++ que passe referências pra dentro de containers através de callbacks opacos:

1. **Suíte verde não prova ausência de UAF.** Prova ausência dos cenários que ela pensou em testar. Pra pegar lifetime bug de verdade, é preciso ASan rodando de forma disciplinada, e callbacks adversariais que forcem o pior caso (o callback que faz a coisa "errada" que ninguém escreveu ainda, mas que o contrato da API permite).
2. **Passar referência pra dentro do buffer de um container pra um callback opaco é um contrato perigoso.** Quem não controla o que o callback faz por dentro não controla o lifetime da referência que está segurando. A defesa é copiar por valor na fronteira, antes de entregar o controle pra fora.
3. **A comunicação entre sessões se pagou.** Um bug invisível pra quem estava trabalhando diretamente no código foi caçado por outra sessão que só passou pelos coredumps por acaso, e teve a curiosidade de investigar em vez de ignorar.
4. **Auditoria-dominó importa.** O achado de lifetime nunca é isolado: o mesmo padrão perigoso apareceu em três lugares diferentes do mesmo subsistema. Consertar só o que crashou seria deixar duas bombas-relógio armadas.
5. **Pra depurar lifetime sutil, o modelo mais forte se paga.** A investigação subiu o modelo do agente de Sonnet pra Opus especificamente pra essa caça: raciocínio de ownership e ciclo de vida de memória é exatamente onde a diferença de capacidade de raciocínio rende no fim.

---

### Ficha técnica

| Item | Detalhe |
|---|---|
| **Tipo de bug** | Heap-use-after-free (UAF) de referência pendurada, latente |
| **Subsistema** | Deck/mão (`CardCollection`, `HandLoadout`, transações), camada de domínio POCO |
| **Ferramenta de diagnóstico** | AddressSanitizer (ASan, `-fsanitize=address,undefined`) |
| **Achado por** | Coredump cruzado pela sessão do glintfx, via `journalctl`/`coredumpctl`, comunicado pelo bus autocomm |
| **Sintomas originais** | 2 crashes às 20:53 (SIGABRT double-free + SIGSEGV), 4 crashes às 21:06 |
| **Ponto cravado pelo ASan** | `hand_loadout.cpp:59`, READ heap-use-after-free |
| **Arquivos corrigidos** | `GusEngine/domain/src/deck/card_collection.cpp`, `GusEngine/domain/src/deck/deck_transactions.cpp`, `GusEngine/domain/src/deck/hand_loadout.cpp` |
| **Teste de regressão** | `GusEngine/domain/tests/deck_tier_callback_lifetime_test.cpp` (6 casos, callback adversarial) |
| **Verificação** | ASan sem fix: 3 falhas UAF. ASan com fix: 6/6 + 43/43 limpo. Suíte normal: 2121 → 2127 verde |
| **Commit** | `d2601c1` |
