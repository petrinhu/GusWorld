# Sylvarin - Nucleo de gramatica (mutacao + morfologia)

Status: PROPOSTA aguardando validacao do lider (2026-06-23). Principio reitor: tudo REGULAR, sem excecao (Pillar 2, natureza = matematica rigida). O Sindarin real tem irregularidades historicas; o Sylvarin as remove de proposito - e por isso que e decifravel por regra (C-Arcane le a lingua como quem le codigo).

## 1. Mutacao consonantal (a assinatura)
A primeira consoante da palavra muta conforme o contexto gramatical, por TABELA FIXA.

### Mutacao suave (lenicao) - a mais comum
Gatilhos: depois do artigo definido, adjetivo posposto, objeto direto, 2o elemento de composto.

| Base | Muta para |
| :--- | :--- |
| p | b |
| t | d |
| c / k | g |
| b | v |
| d | dh |
| g | gh |
| m | v |
| s | h |

(8 mapeamentos no total.) As demais consoantes (l, r, n, v, lh, nh, th, dh, f, h) nao mutam na lenicao.

**Nota de reconciliacao (AC-L5c, 2026-07-07):** dh, th e h nesta tabela sao ALOFONES (realizacoes de superficie de /d/, /t/ e /s/), nao fonemas-base independentes; lh e nh tambem sao alofones (de /l/ e /n/, palatalizacao lexicalizada). O inventario-base fechado em 13 fonemas esta em `01-fonologia.md` ("Consoantes" -> "Alofones / realizacoes de mutacao"). Isso nao muda nenhum mapeamento desta tabela nem das palavras ja geradas (*sylva* -> *hylvi*, *tavus* -> *davsi*) - so formaliza que as SAIDAS de mutacao (b, d, g, v, dh, gh, h / f, th, ch) podem ser alofone ou fonema-base, sem restricao.

### Outras classes (resumo; detalhar depois)
- **Mutacao nasal** (depois de prefixo/preposicao nasal *an-*, *in-*): p->f, t->th, c->ch.
- **Mutacao mista** (depois de preposicoes terminadas em consoante): combina as duas.

## 2. Numero (substantivo)
- **Singular**: forma-base (*sylva* = floresta).
- **Plural**: lenicao da 1a consoante + **-i**. Ex.: *sylva* -> *hylvi* (s->h); *tavus* -> *davsi* (t->d); *nenha* -> *nenhi* (n nao muta).
- **Coletivo** (o todo, nao a soma): **-rim**. Ex.: *sylvarim* = a Selve inteira como entidade.

## 3. Caso e posse
Ordem-base **SVO** (sujeito-verbo-objeto, como o portugues falado).
- **Genitivo (posse)**: possuido + particula **en** + possuidor; o possuido sofre lenicao. Ex.: *cala en elen* = "a luz da estrela" (*cale* -> *cala* por lenicao).
- **Locativo** ("em"): sufixo **-sse** (emprestado do Quenya, regular). Ex.: *sylvasse* = na floresta.
- **Alativo** ("para, em direcao a"): sufixo **-nna**. Ex.: *sylvanna* = para a floresta.

### Reversibilidade (resolve colisoes) - FECHADO 2026-06-23
Tres garantias tornam a lenicao SEMPRE reversivel, e por isso decifravel (Pillar 2; o C-Arcane le a lingua revertendo a mutacao):
1. A lenicao muda APENAS a consoante inicial, nunca as vogais. As vogais sao contrastivas: formas com a mesma raiz mas sufixos diferentes seguem distintas pela vogal final (ex.: *cale* verbo vs *cala* substantivo, e vs a).
2. **g leniza para gh** (fricativa), nunca para zero. Assim nenhuma raiz "desaparece" virando vogal-inicial: *glyfa* (glifo) -> *ghlyfa*, distinto de qualquer palavra de vogal inicial.
3. Os gatilhos de lenicao sao um conjunto FIXO e enumerado (artigo, adjetivo posposto, objeto direto, 2o elemento de composto). A sintaxe sempre sinaliza que houve mutacao, entao a forma-base e recuperavel por regra. Homofonia residual se resolve pelo contexto sintatico, como em qualquer lingua natural.

## 4. Verbo (tempos)
Raiz verbal + sufixo. Apenas 3 tempos + infinitivo (economia regular).
- **Infinitivo**: **-e** (*tave* = executar, *lhine* = falar/cantar, *glyfe* = compilar).
- **Presente**: **-a** (*tava* = executa).
- **Passado**: **-ne** (*tavne* = executou).
- **Futuro**: **-uva** (emprestado do Quenya; *tavuva* = executara).

Exemplo conjugado (`tave`, executar um cartao - liga ao Tavus-Drive):
*tava* (executa) / *tavne* (executou) / *tavuva* (executara).

## 5. Adjetivo
Posposto ao nome (como em portugues e Sindarin) e sofre lenicao. Ex.: *dragna vyrin* = "dragao igneo" (adjetivo *vyrin* depois do nome).

## 6. Pronomes
### Pessoais (sujeito)
| Pessoa | Sylvarin |
| :--- | :--- |
| eu | **ni** |
| tu / voce | **le** |
| ele / ela | **se** |
| nos | **men** |
| vos | **lle** |
| eles / elas | **te** |

### Possessivos (sufixo no nome possuido)
| Posse | Sufixo | Exemplo (*sylva* = floresta) |
| :--- | :--- | :--- |
| meu | **-nya** | *sylvanya* |
| teu | **-lya** | *sylvalya* |
| dele / dela | **-rya** | *sylvarya* |
| nosso | **-mma** | *sylvamma* |

Para possuidor explicito, usa-se o genitivo com *en*: *sylva en Gus* (a floresta do Gus).

## Itens abertos (proxima etapa)
- ~~Colisao de lenicao (cale->gala vs raiz gala): fechar regra de desambiguacao.~~ RESOLVIDA 2026-07-12: a raiz 11 foi trocada de *gala-* para *eryn-* (vogal-inicial, correcao canonica por conflito com palavrao pt-br); como raizes vogal-iniciais nao sofrem lenicao de consoante inicial, a colisao cale->gala vs raiz gala deixou de existir.
- Tabelas completas das mutacoes nasal e mista.
- Pronomes pessoais e possessivos.
- Deriva historica Era 1 -> Era 3 (como o Sylvarin puro virou substrato no portugues do jogo).
- Sistema de escrita cifrada (cripto-glifo da Era 1).
