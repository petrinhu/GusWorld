# Lições aprendidas

> As lições sobre teste e gate de qualidade já viraram lei e vivem em `GODS_LAWS.md`, seção **L-19**: a da asserção pensada pelo bug, a do conserto sem gate, a do gate que não mede, a da métrica sem régua, a do autoteste que cabe no próprio limite de tempo, a de distinguir defeito do teste e do produto, e a do prefixo comum. Não repetidas aqui. Consulte a L-19 antes de escrever qualquer teste ou gate novo.

Este documento reúne as demais lições recuperadas da mineração do projeto anterior (21/08/2026), ordenadas da mais generalizável para a mais específica.

## 1. Comentário desatualizado que afirma o que não é mais verdade é bug, mesmo sem efeito funcional

Um comentário dizia "NÃO commitado" num arquivo que estava commitado havia meses, sem consequência prática nenhuma no comportamento do programa. Ainda assim é defeito: comentário que mente engana quem lê depois, inclusive um agente que decide com base nele.

**Fazer diferente:** corrigir o comentário assim que a divergência for encontrada, no mesmo commit, sem abrir item para "depois". Isto é o mesmo princípio da L-24 aplicado a comentário: texto errado que sobrevive é texto que alguém vai obedecer por engano.

## 2. Comando que deveria mudar o sistema de arquivos se confere pelo estado resultante, nunca pelo código de saída relatado

Um rótulo de asset trocado (leste/oeste) foi corrigido, o comando de troca reportou sucesso, e a sessão aceitou o relato. Na prática a primeira tentativa tinha sido bloqueada silenciosamente por um hook de proteção, e o arquivo continuou errado; só o líder, olhando ao vivo, percebeu. A verificação automatizada da sessão já tinha falhado em pegar o mesmo erro duas vezes antes disso.

**Fazer diferente:** depois de qualquer comando que deveria alterar um arquivo, ler o arquivo de novo e comparar o conteúdo com o que se esperava. Sucesso relatado por quem invocou o comando não é prova; o estado do arquivo é a prova. É a mesma família de "relatório de agente não é prova", aplicada a operações de arquivo.

## 3. "Zero ocorrências de X" pode ser o alvo errado de uma limpeza

Depois de rotacionar a licença do código inteiro, um QA verificou que sobravam 11 arquivos citando a licença antiga e apontou como pendência. Todos os 11 eram preservação deliberada de contexto histórico: o registro da decisão superada, o catálogo do que foi vendorizado antes, a licença de uma dependência de terceiro que é dela mesma, não do projeto.

**Fazer diferente:** ao caçar "zero ocorrências de X" como critério de limpeza, classificar cada ocorrência encontrada entre "resíduo a apagar" e "registro do porquê da decisão", antes de apagar qualquer uma. Apagar a segunda destrói a rastreabilidade da própria decisão. Vale junto com a L-24 (revogado se apaga) e a L-14 (nada é declarado morto por agente): a classificação em si não dá licença para apagar sozinho o que parecer resíduo.

## 4. Preferir a correção arquitetural a uma sequência de remendos pontuais da mesma classe de bug

Uma rotina de ativação de diálogo/combate foi corrigida três vezes seguidas para personagens diferentes (um capturado só pela largura do sprite, não pela altura; depois um NPC inteiro escondido atrás de outro por falta de colisão física além do gatilho de ativação), até alguém propor usar o contorno visual inteiro do sprite como base da colisão, resolvendo a classe inteira de uma vez.

**Fazer diferente:** quando o segundo remendo pontual da mesma classe de bug aparece, isso já é o sinal para trocar de tática e resolver a causa estrutural, sem esperar o terceiro caso aparecer sozinho.

## 5. Resiliência de infraestrutura só se mede de verdade quando o caminho normal cai

Durante uma pane real de cerca de 4 horas no provedor de CI, o disparo automático por evento (`push`) parou de criar execução, e só o disparo manual funcionou. Descobriu-se então que 3 dos 6 workflows do projeto nem tinham esse gatilho manual configurado, e ninguém sabia até a pane acontecer.

**Fazer diferente:** todo pipeline crítico precisa do caminho alternativo testado antes da pane, não descoberto durante ela. E ao adicionar um gatilho manual novo, lembrar que ele só existe no servidor depois que o commit que o cria for enviado; testar antes do push dá erro esperado, não falha do conserto.

## 6. Ler a árvore de trabalho durante o commit de outro agente acusa defeito que não existe

Um agente leu arquivos no meio do processo de commit de outro agente e reportou que o trabalho tinha parado pela metade, quando na verdade o commit estava correto e só ainda não tinha terminado de ser gravado. A regra de ler o blob commitado, e não a árvore de trabalho em voo, já existia havia semanas quando isso aconteceu de novo.

**Fazer diferente:** verificar sempre contra o commit (`git show <sha>:<arquivo>`), nunca contra a árvore de trabalho, quando outro agente pode estar com edição em andamento. E ao relatar um defeito no trabalho de outro agente, dizer contra qual SHA a verificação foi feita, para o relato ser reproduzível.

## 7. Grep ou leitura de configuração que "acha a propriedade" pode estar vendo só parte dos alvos

Um `TIMEOUT` de teste existia em 1 de 4 alvos de build, e faltava exatamente no alvo onde um teste travou por quase 9 horas. Um `grep TIMEOUT` no repositório teria respondido "temos" errando em três quartos dos casos, porque a busca encontra a propriedade em algum lugar, não em todo lugar onde ela precisa estar.

**Fazer diferente:** ao confirmar que uma proteção de configuração existe, enumerar todos os alvos que deveriam tê-la e conferir cada um, não parar na primeira ocorrência encontrada. Vale também o corolário: nenhuma proteção de timeout de um executor de teste protege a execução direta do binário fora dele.

## 8. Um checklist de playtest humano só deve pedir o que é conferível por observação; o resto é teste automatizado

Um checklist de playtest entregue ao líder pedia que ele avaliasse ordem de turnos, decisão de IA inimiga e consequência de morte por dificuldade, coisas estruturalmente impossíveis de perceber só jogando. O próprio líder apontou o limite, e a resposta correta reconhecida depois foi: tinha sido entregue uma lista de conferência quando devia ter sido entregue uma lista de coisas para sentir.

**Fazer diferente:** ao montar um checklist para humano, separar antes o que é da categoria "sensação de jogo" (ritmo, dificuldade percebida, clareza de UI) do que é da categoria "correção interna" (ordenação, decisão de IA, resultado de sorteio). A segunda categoria vai sempre para teste automatizado, nunca para observação humana.

## 9. Vírgula em nome de teste corta o filtro de teste em pedaços que não casam nada

Um harness de caça a teste instável rodou 55 amostras e reportou 20 falhas (36%), todas espúrias: os nomes de teste tinham vírgula não escapada, o framework de teste da época tratava vírgula como separador de filtro, e o nome fatiado não casava com nenhum teste real, produzindo "No tests ran" indistinguível de falha de verdade sem olhar a saída completa.

**Fazer diferente:** não usar vírgula em nome de teste, e sempre filtrar por curinga quando o harness suportar filtro por nome. E, ao investigar uma suíte com falhas em massa e súbitas, conferir primeiro se o nome do teste tem caractere especial de filtro antes de assumir regressão real.

## 10. Loop modal que descarta "tecla solta" silenciosamente deixa o personagem andando sozinho depois

Em duas ocasiões distintas o líder relatou, jogando ao vivo, o personagem continuando a se mover sozinho depois de fechar um diálogo. Nas duas vezes a causa raiz foi a mesma: um evento de tecla solta descartado enquanto o modal (diálogo, menu) estava aberto, deixando o estado de "tecla pressionada" pendurado depois que o modal fechava.

**Fazer diferente:** todo sistema de input com modo modal (que pausa o jogo mas não pausa a leitura de eventos do sistema operacional) precisa de uma limpeza explícita do estado de tecla ao entrar e ao sair do modal. Esta é uma classe de bug, não um caso isolado, e merece teste de regressão dedicado quando a camada `present/` nascer (L-06).

---

Este documento cresce. Lição nova entra aqui, na posição que sua generalidade indicar.
