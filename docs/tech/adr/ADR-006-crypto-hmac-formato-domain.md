# ADR-006: Fonte do HMAC-SHA256 e formato de serializacao do domain/ (GusEngine)

Status: Accepted
Data: 2026-06-21
Decisores: lider supremo (petrus), software-architect, backend-engineer

## Contexto

A camada `domain/` do GusEngine (C++20 POCO, ZERO Qt, invariante auditada por grep no CI) porta de C# os subsistemas `templates` (TemplateSerializer) e `save` (o mais critico). Ambos usam HMAC-SHA256 anti-tamper + serializacao. O C# usa System.Security.Cryptography (HMACSHA256) + System.Text.Json. O projeto C++ nao tem OpenSSL, lib JSON nem crypto linkada (so Catch2; build via CMakePresets + FetchContent, sem vcpkg). QCryptographicHash esta FORA do domain pela invariante de camadas.

Fatos que delimitam a decisao:

- SHA-256 e determinístico e padronizado (FIPS 180-4 / RFC 2104/4231): qualquer implementacao correta produz o MESMO HMAC. A fonte do hash NAO afeta compatibilidade, so peso de build e soberania. O uso aqui e integridade anti-tamper CASUAL, nao sigilo (a chave mora no binario, extraivel por decompile; e o aceitavel/normal para save local single-player).
- Nao ha saves de jogador real; o C# de referencia e descontinuado no M8. O "oraculo byte-a-byte" (engine-design.md:80) cobre apenas a janela de migracao M3..M8.
- Os dois envelopes C# ja divergem entre si: template = binario (magic/length/payload/hmac, hmac raw); save = JSON {SaveVersion, Payload, IntegrityHmac} com payload indentado e 6 conversores custom (Vector3/Color/Quaternion/Basis/Transform3D). Replicar a formatacao exata do System.Text.Json em C++ e o trecho mais caro e fragil.

## Decisao

1. **HMAC-SHA256 proprio em `core/`**: implementacao de dominio publico (SHA-256 + HMAC), ZERO dependencia externa, validada TEST-FIRST contra os vetores oficiais FIPS 180-4 (SHA-256) e RFC 4231 (HMAC-SHA256). Nenhum subsistema (templates, save) porta antes da crypto estar verde contra os vetores. Chave de integridade FIXA embutida no binario.

2. **Formato de serializacao binario proprio** (envelope compacto: magic / length / payload / hmac), sem JSON nem dependencia externa. Rompe a compatibilidade byte-a-byte com os arquivos do C#/Godot (aceitavel: jogo em DEV, sem saves de jogador real; C# descontinuado no M8). Decidido AGORA porque romper o formato e barato em DEV e caro pos-lancamento.

3. **Oraculo de equivalencia semantica** no lugar do oraculo de formato byte-a-byte: congelar objetos de dominio canonicos e provar, em C# e em C++, o roundtrip identico (objeto -> bytes -> objeto identico) + a deteccao de tamper (byte-flip rejeitado). Prova nao-corrupcao com o mesmo rigor, sem acorrentar o C++ a formatacao do .NET depois do M8.

4. **Carimbo de data/hora (com milesimo) em cada SAVE**: campo de timestamp gravado em cada save de progresso, para listar/ordenar saves e exibir "salvo em X". Sem funcao de seguranca. (Decisao do lider 2026-06-21.)

5. **Semente de aleatoriedade do gameplay**: data + hora + milesimo sera a semente do gerador de numeros aleatorios das MECANICAS (loot, dano, sorteios), a implementar no subsistema de combate/mecanicas, com aprovacao do lider na ocasiao. NAO afeta o selo de seguranca (que segue com chave fixa). (Decisao do lider 2026-06-21.)

## Opcoes consideradas

Eixo fonte do HMAC: (1) OpenSSL via vcpkg/find_package: crypto auditada, mas arrasta dep pesada + vcpkg so para 1 HMAC, over-engineering pro escopo. (2) SHA-256/HMAC header-only FOSS vendorado: zero infra de dep, mas +1 codigo de terceiro no repo. (3) ESCOLHIDA: SHA-256/HMAC proprio em core/: zero dep, soberania total, validavel por vetores oficiais; risco de bug mitigado pelos vetores FIPS/RFC (uso so integridade, sem timing/sigilo).

Eixo formato: (A) JSON compativel com o C# (nlohmann/json): preserva o oraculo byte-a-byte, mas exige casar a formatacao exata do System.Text.Json (fragil, sobretudo no save indentado) + 1 dep. (B) ESCOLHIDA: binario proprio compacto: zero dep, controle total; rompe compat byte-a-byte, compensado pelo oraculo de equivalencia semantica.

## Consequencias

Positivas: build reprodutivel Linux+Windows sem vcpkg/OpenSSL/JSON externos; invariante zero-Qt e zero-dep externa em core/domain preservada; soberania total do codigo critico (crypto + formato) sob GPLv3, sem atribuicao extra; o oraculo semantico prova nao-corrupcao sem acorrentar o C++ a quirks do .NET pos-M8.

Negativas (aceitas como custo): crypto propria exige suite de teste com vetores oficiais (FIPS 180-4 + RFC 4231) como gate; romper o formato C# significa abrir mao do oraculo byte-a-byte literal (os fixtures C# viram referencia semantica, nao binaria); saves/templates do C# antigo nao sao lidos pelo C++ (aceitavel: sem base instalada real).

Riscos / atencao: bug sutil no SHA-256 proprio (mitigacao: test-first contra vetores oficiais antes de portar qualquer subsistema; nenhum subsistema porta sem a crypto verde); migrators forward-only do save passam a operar sobre structs versionadas (nao arvore JSON), mantendo a chain V1->V2->... testada com fixtures de cada versao.

## Reversibilidade

Two-way door no formato (trocavel enquanto em DEV; caro pos-lancamento, por isso decidido agora). A fonte do codigo do HMAC e trocavel sem quebrar compat (SHA-256 e determinístico); so a CHAVE e sensivel (trocar a chave muda todos os HMACs).
