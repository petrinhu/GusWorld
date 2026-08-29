# Plano de Build e Portão de Release — GusWorld

**Status:** Canônico. Escrito para o stack atual do projeto (C++23 + GlintFx).

Documento leve de delivery solo (anti-over-engineering).

---

## Princípio reitor

O portão não é "pipeline de CI verde". O portão é: **o artefato roda de verdade, em
cada uma das cinco plataformas da matriz, sem passo manual e sem dependência faltante**.
CI verde é pré-condição necessária, não suficiente — um binário pode compilar e passar
nos testes e ainda assim não abrir numa máquina limpa.

---

## Fase 0 — Fundação de build e matriz de CI

A fundação de build e a matriz de CI existem **desde o primeiro commit**, como
**primeira fatia do projeto**, antes de qualquer módulo de comportamento
(GODS_LAWS.md, L-20). Fundação de build não tem comportamento a especificar, então
nasce legitimamente sem TDD; a partir do primeiro módulo de verdade, essa saída acaba
(GODS_LAWS.md, L-19).

A matriz tem **cinco entradas distintas, nenhuma não bloqueante** (GODS_LAWS.md, L-09
e L-20):

| Plataforma | Papel | Observação |
|---|---|---|
| Fedora 44 | Alvo primário | Imagem pinada em `fedora:44`, nunca `:latest`: é o sistema do líder, e o CI tem de falhar quando a máquina dele falharia. |
| Ubuntu | Alvo próprio | Entrada própria na matriz, não confirmação secundária do Fedora. |
| Arch | Alvo próprio | Entrada própria. |
| CachyOS | Alvo próprio | **Não é Arch renomeado.** Job, imagem e toolchain próprios; verde no Arch não autoriza declarar CachyOS suportado. |
| Windows | Alvo próprio | Entrada própria, no gate desde o primeiro commit, nunca deferida. |

O que cada entrada da matriz precisa provar, no mínimo: toolchain com suporte real ao
padrão de linguagem do projeto (não só a versão que o compilador reporta — compilar e
rodar um fragmento que exercite um recurso do padrão), build completo, suíte de teste
executada.

---

## Fase 1 — Build local que espelha o CI

Script local reproduzível, de um comando, rodado **antes do push**, espelhando
exatamente os passos que o CI roda (GODS_LAWS.md, L-19). O objetivo é achar
divergência de plataforma antes do push, não depois.

---

## Fase 2 — Portão de release: smoke em ambiente limpo

Esta é a barra que define "pronto para liberar" um artefato — distinta do CI de cada
commit, que prova build e teste, não instalação em máquina alheia. Critério, por
plataforma da matriz:

| Critério (mensurável) | Método |
|---|---|
| O artefato instala ou extrai e abre em ambiente limpo daquela plataforma | Ambiente limpo (VM ou container) da distro/SO → instalar ou extrair → chegar à tela inicial, sem passo manual e sem dependência faltante |
| Reprodutível pelo wrapper da Fase 1, não por passo manual | Rerrodar o script do zero produz artefatos equivalentes |

**Nenhuma das cinco plataformas é dispensada ou tratada como confirmação não
bloqueante** (GODS_LAWS.md, L-20): verde numa plataforma não substitui o smoke de
outra — o mesmo princípio que já impede o CachyOS de herdar o resultado do Arch.

---

## Fase 3 — Reforço incremental de qualidade em CI

Os quatro portões de qualidade já são lei (GODS_LAWS.md, L-19) e se somam à matriz da
Fase 0 de forma incremental, plugados um de cada vez, não como decisão em aberto:

1. Zero aviso de compilação em todo commit.
2. ASan e UBSan a cada fatia fechada, em build separado.
3. Análise estática no CI.
4. Scan de segredo no CI.

---

## Em aberto (decisão do líder, não inventada aqui)

- Gerador de build e sistema de empacotamento concreto.
- Formato final do artefato distribuível por plataforma (ex.: pacote nativo Linux,
  arquivo compactado universal, instalador Windows) e limiar de tamanho que aciona
  alerta de inchaço.
- Assinatura de binário (Linux/Windows).

---
*Este plano deriva o portão e a matriz de GODS_LAWS.md (L-09, L-19, L-20); não decide
ferramenta além do que a lei já fixa.*
