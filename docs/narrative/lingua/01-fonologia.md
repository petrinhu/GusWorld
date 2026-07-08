# Sylvarin - Fonologia (esqueleto)

Status: sabor e esqueleto APROVADOS pelo lider 2026-06-23. Embasado no RAG-elfico (`rag_elvish`): Pedin Edhellen (Sindarin, Thorsten Renk), Quetin i lambe eldaiva (Quenya), artigos de acento e deriva historica (Tengwestie), cursos PT (licao1, licaosind3). Detalhe fino (inventario exato, regras completas de mutacao) sera fechado junto do lexico e da gramatica.

**Ajuste 2026-07-07 (auditoria AC-L5c):** o inventario consonantal listava 18 sons mas declarava contagem-alvo 13 (numero Fibonacci, canon). Correcao: reclassificados 5 sons (h, th, dh, lh, nh) de "fonema-base" para "alofone / realizacao de mutacao" (ver subsecao dedicada abaixo). Nenhum som foi cortado da lingua falada, nenhuma regra de mutacao mudou, nenhuma palavra ja gerada (*hylvi*, *davsi*) ou raiz do lexico-semente (`02-lexico-semente.md`) foi invalidada. So mudou o ESTATUTO desses 5 sons no inventario-base.

## Sabor sonoro
Equilibrio: a melodia do Quenya (vogais puras) na base + aspereza pontual do Sindarin (th/dh) + portugues forte por cima (nasais, lh/nh). Soa familiar a um brasileiro e exotico ao mesmo tempo: um "portugues elfico arcaico". (Nota: th/dh/lh/nh sao alofones no inventario-base, ver "Consoantes" abaixo - continuam soando na lingua, so nao contam como fonema independente.)

## Vogais
- Puras: **a e i o u** (como no portugues / Quenya), claras e abertas.
- Longas: **a e i o u** com macron ou acento agudo na escrita romanizada (a, e, i, o, u longas), contrastivas (mudam sentido).
- Nasais (marca brasileira): **a-til e o-til** (ex.: "Selvar-a-til-e").
- Ditongos (conjunto fechado): ai, au, ei, oi, eu (5).

## Consoantes

### Inventario-base: 13 fonemas (Fibonacci, fechado)
- Liquidas: **l, r**.
- Nasais: **m, n**.
- Aproximante: **v**.
- Fricativas surdas: **f, s**.
- Oclusivas surdas: **p, t, c/k**.
- Oclusivas sonoras: **b, d, g**.

(l r m n v f s p t c/k b d g = 13 fonemas-base. c/k conta como 1 fonema, duas grafias do mesmo som.)

### Alofones / realizacoes de mutacao (superficie - NAO contam no inventario-base)
Cinco sons continuam soando na lingua falada, mas cada um so aparece como SAIDA de um processo fonologico regular - mutacao gramatical (ver `03-gramatica-nucleo.md`) e/ou fricativizacao/palatalizacao lexicalizada -, nunca como consoante inicial independente de uma raiz nova. Por isso saem da contagem-base sem sumir do idioma:

| Alofone | Fonema-base | Processo | Onde aparece |
| :--- | :--- | :--- | :--- |
| **h** | /s/ | lenicao (s -> h) | *sylva* -> *hylvi* |
| **th** | /t/ | mutacao nasal (t -> th) | tabela de mutacao |
| **dh** | /d/ | lenicao (d -> dh) + fricativizacao lexicalizada medial | raiz *ondh-* |
| **lh** | /l/ | palatalizacao (digrafo herdado do portugues) | raiz *lhin-* |
| **nh** | /n/ | palatalizacao (digrafo herdado do portugues) | raizes *nenh-*, *anh-* |

A "aspereza pontual" (Sindarin: th/dh) e o sabor palatal (portugues: lh/nh) sobrevivem inteiros no som falado - so mudam de ESTATUTO, de fonema-base pra alofone/realizacao regular. E por isso a raiz *ondh-* (que tem "dh" no meio da palavra, nao no inicio mutavel) continua valida: ela usa o fonema-base /d/, so grafado/realizado como "dh" nessa posicao - exatamente o mesmo processo que a mutacao de lenicao usa pra transformar d -> dh no inicio de outras palavras. Mesma logica pra *lhin-*, *nenh-*, *anh-* (base /l/ e /n/, realizados lh/nh).

(Duas saidas adicionais da tabela de mutacao, g -> gh e c/k -> ch, ainda nao tem raiz nenhuma que as use; ficam so como regra em `03-gramatica-nucleo.md`, sem simbolo dedicado aqui, ate que alguma palavra as precise.)

## Estrutura silabica
- Padrao **(C)V(C)**, com forte preferencia por silaba ABERTA (consoante+vogal), o que da fluidez.
- Silabas fechadas (terminadas em consoante: l, r, n, s; tambem no alofone th quando a mutacao o produz) sao permitidas e dao o "peso de pedra" pontual (toque Sindarin).
- Encontros consonantais iniciais limitados (ex.: br, gl, th + liquida); sem clusters pesados.

## Acento
- **Paroxitono fixo**: o acento cai sempre na PENULTIMA silaba (como a maioria do portugues). Regra sem excecao (Pillar 2 = natureza e matematica rigida).
- Em palavra de 1 silaba, acento nela; o acento nunca cai numa ultima silaba de vogal curta (regra herdada do Quenya).

## Assinatura gramatical: mutacao da 1a consoante
O achado de ouro do Sindarin. A **primeira consoante da palavra muta conforme o contexto gramatical** (ex.: depois do artigo, ou em composicao), por uma TABELA FIXA e previsivel. E a "matematica viva" da lingua: sem excecao, decifravel por regra (Pillar 2; e por isso o cripto-glifo Era 1 e decifravel via C-Arcane).
- Mutacao suave (lenicao), ex.: p->b, t->d, c->g, s->h (a definir a tabela completa).
- Outras classes (nasal, mista) a especificar na gramatica.

## Pendente (proximas etapas)
- Lexico-semente (raizes ancoradas no canon: Sylva, Vyr, Tavus...).
- Tabela completa de mutacao.
- Morfologia (casos, numero, derivacao) e ordem das palavras.
- Deriva historica Era 1 (Sylvarin puro) -> Era 3 (substrato no portugues do jogo).
