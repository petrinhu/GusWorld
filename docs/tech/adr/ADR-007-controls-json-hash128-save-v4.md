# ADR-007: Camadas anti-tamper do save (detect-and-respond, slot-id selado, KDF na chave)

Status: Accepted
Data: 2026-06-22
Decisores: lider supremo (petrus), software-architect, backend-engineer

## Decisao

O lider adicionou 3 camadas de defesa ao save:

- **T1.1 detect-and-respond.** O carregamento do save nunca lanca excecao nem crasha diante de corrupcao ou adulteracao: distingue selo invalido, payload corrompido, versao futura demais, dado invalido e arquivo no slot errado, e devolve esse resultado para a camada de aplicacao decidir como avisar o jogador.
- **T1.2 slot-id selado.** O identificador do slot viaja DENTRO do payload selado. Carregar um save comparando o slot selado com o slot de onde o arquivo foi lido detecta arquivo copiado entre slots (por exemplo, pelo gerenciador de arquivos do sistema).
- **T2.2 KDF na origem da chave.** A chave de integridade do selo do save passa a ser DERIVADA por uma funcao de derivacao de chave, nunca embutida crua. Muda o valor do selo, nao o layout do envelope; a fonte do codigo da derivacao e trocavel sem quebrar compatibilidade, so a chave em si e sensivel.
