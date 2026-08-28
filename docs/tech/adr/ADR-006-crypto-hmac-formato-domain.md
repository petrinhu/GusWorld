# ADR-006: Formato de serializacao do domain/ (GusEngine), timestamp e semente de RNG

Status: Accepted
Data: 2026-06-21
Decisores: lider supremo (petrus), software-architect, backend-engineer

## Decisao

1. **Formato de serializacao binario proprio** (envelope compacto: magic / length / payload / hmac), sem JSON nem dependencia externa.

2. **Carimbo de data/hora (com milesimo) em cada SAVE**: campo de timestamp gravado em cada save de progresso, para listar/ordenar saves e exibir "salvo em X". Sem funcao de seguranca. (Decisao do lider 2026-06-21.)

3. **Semente de aleatoriedade do gameplay**: data + hora + milesimo sera a semente do gerador de numeros aleatorios das MECANICAS (loot, dano, sorteios), a implementar no subsistema de combate/mecanicas, com aprovacao do lider na ocasiao. NAO afeta o selo de seguranca (que segue com chave fixa). (Decisao do lider 2026-06-21.)

## Consequencias

Riscos / atencao: migrators forward-only do save operam sobre structs versionadas (nao arvore JSON), mantendo a chain V1->V2->... testada com fixtures de cada versao.

## Reversibilidade

Two-way door no formato (trocavel enquanto em DEV; caro pos-lancamento, por isso decidido agora). A fonte do codigo do HMAC e trocavel sem quebrar compat (SHA-256 e determinístico); so a CHAVE e sensivel (trocar a chave muda todos os HMACs).
