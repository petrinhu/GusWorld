# Receita: PDF-presente de personagem inspirado em pessoa real

**Status:** Registro histórico de uma pendência reutilizável do `gusworld_legacy`, trazido a pedido do líder em 25/08/2026. Não é item de tabela — é documento de referência para quando o líder mandar produzir um PDF-presente novo neste projeto. Nenhum código, template ou caminho descrito aqui foi executado ou verificado neste repositório: é o que o garimpo nos commits do projeto anterior encontrou, mais a forma de conversão vigente nesta máquina (ver "Ferramenta de conversão vigente" abaixo).
**Cross-ref:** `GODS_LAWS.md` L-16 (homenagem a pessoa real); `AI-DISCLOSURE.md` (créditos de inspiração); `CHARS.md` (inventário de personagem); `ASSETS-LICENSE.md` (regime de asset/lore).

---

## Divergência frente ao briefing desta tarefa

O achado apontado (`e562bad`, 25/06/2026) **existe**, mas é menor do que "a receita completa": esse commit só acrescenta **2 linhas ao `TODO.md`** do projeto anterior, um item de INBOX em prosa descrevendo o template (ver §1). Ele **não** traz HTML, CSS nem script.

A receita **executável de verdade** — com o HTML/CSS completo e o gerador em Python — nasceu um mês depois, no commit **`cced60c7`** (25/07/2026, `docs(narrative): 3 perfis canonicos (Gus, Pyotor, Yakov) + gerador de PDF reproduzivel`), que adiciona `tools/build_perfil_pdf.py`. **É esse script que este documento reproduz** nas seções abaixo, porque é o único lugar do repositório antigo onde a receita existe como código real, não só como descrição.

A segunda divergência importa mais: o `e562bad` (25/06) descreve a ferramenta de conversão como **weasyprint** ("HTML+CSS gerado por Python -> weasyprint -> PDF A4"). Mas o `cced60c7` (25/07) já **tinha trocado** para **Chromium headless `--print-to-pdf`**, por pedido explícito do próprio líder na época ("conforme o lider pediu", mensagem do commit) — ou seja, a troca para Chromium não é uma recomendação nova desta tarefa, é uma decisão que **já aconteceu** no projeto anterior, antes mesmo de qualquer regra desta máquina sobre o assunto. As duas seções abaixo (§2 e §3) documentam as duas eras separadamente, para não confundir "o que a receita dizia" com "o que ela virou".

---

## §1. O achado original (`e562bad`, 25/06/2026) — era weasyprint

Commit `docs(todo): pendencia reutilizavel - template de PDF-presente pra personagens inspirados em pessoas reais`, autor petrinhu, 25/06/2026. Texto do item de INBOX que ele acrescentou (traduzido de "tradução nenhuma" — é cópia do teor, não verbatim byte a byte, porque o original está em `snake_case`/sem acento de commit de terminal; o conteúdo semântico é preservado):

- **Gatilho:** o líder ia definir outros personagens inspirados em pessoas reais; cada um ganharia um PDF-presente em estilo **idêntico** ao do primeiro (Brunus Vetorial), para entregar a quem inspirou.
- **Pipeline descrito:** HTML+CSS gerado por Python → **weasyprint** → PDF A4.
- **Estrutura descrita:** HERO ciber-gótico (gradiente `#10131f` → `#22304d`, borda cyan `#22D3EE`) com a arte do personagem em 3 variantes no topo (drop-shadow, `image-rendering: pixelated`) + kicker monospace cyan "GUSWORLD · PERSONAGEM CANÔNICO" + título serif do conto + subtítulo com nome completo; corpo em papel creme `#f4f1ea`, fonte Georgia 11,6pt, dedicatória centralizada itálica ("Para \<nome real\>, que inspirou este homem/personagem"), conto justificado com recuo de 1ª linha (`text-indent: 7mm`, 1º parágrafo sem recuo), rodapé monospace citando o inspirador.
- **A armadilha registrada:** "NÃO usar drop-cap `float:left` (quebra o weasyprint — `AssertionError` em `float_layout`)". Esta é uma falha **específica do weasyprint** daquela época — o commit não afirma nem sugere que qualquer outro motor de renderização sofra do mesmo bug; ver §3.
- **Fonte do texto:** o conto-presente do personagem (`*-conto.md`), que o `narrative-writer` escreve sempre além da ficha canônica.
- **Destino do PDF:** `resources/sprites/personagens/<personagem>/`.
- **Contrato por personagem:** 1 ficha canônica + 1 conto-presente + 1 PDF.

O exemplo âncora era `resources/sprites/personagens/brunus_vetorial/Brunus_Vetorial_Solveckt.pdf`, gerado para Brunus "Vetorial" Solveckt, personagem inspirado no amigo real do líder, **Bruno Vettore**. O commit registra que o PDF ficava fora do git (`.gitignore`), a versionar só se o líder pedisse backup.

## §2. A receita executável (`cced60c7`, 25/07/2026) — era Chromium, vigente já no projeto anterior

Um mês depois, o líder pediu perfis canônicos para mais três personagens (**Gus**, **Pyotor Vance**, **Yakov Vance**) no mesmo template do Brunus, e o commit trouxe `tools/build_perfil_pdf.py` — um script Python que **já usava Chromium headless**, não mais weasyprint. O comentário do próprio script é explícito sobre a origem das cores: elas **não foram chutadas**, foram **amostradas do PDF original do Brunus** (`pdftoppm` a 150-200dpi + `magick %[pixel:p{x,y}]`), com a origem registrada em comentário no CSS, "para quem mexer depois re-amostrar em vez de adivinhar".

### 2.1 Pipeline

```
conto-presente (*-conto.md, formato canônico: # Título / *dedicatória* / --- / prosa)
        +
sprite do personagem (PNG, possivelmente com fundo branco opaco)
        ↓
tratamento de sprite (ImageMagick: floodfill de borda pra remover fundo branco
        sem comer branco interno + resize pra não inflar o PDF)
        ↓
template HTML5 + CSS (string Python, sprite embutido como data: URI base64)
        ↓
chromium-browser --headless --disable-gpu --no-sandbox
  --user-data-dir=<dir temporário próprio por execução>
  --no-pdf-header-footer --print-to-pdf=<destino.pdf> file://<pagina.html>
        ↓
PDF final (conferido por pdfinfo: contagem de páginas + tamanho mínimo de 5KB)
```

### 2.2 Estrutura visual (herdada do template do Brunus, cores reamostradas)

| Elemento | Valor | Origem da amostra |
|---|---|---|
| Fundo do corpo | `#f4f1ea` | amostrado, `srgb(244,241,234)` |
| Gradiente do hero | `linear-gradient(180deg, #101320 0%, #1c2439 100%)` | amostrado na coluna x=30 do PDF original: topo `srgb(16,19,32)` → base `srgb(28,36,57)` |
| Divisória do hero | `2.2pt solid #22d3ee` | cyan amostrado |
| Kicker | `#67e8f9`, monospace/sans, letter-spacing 0.30em, uppercase | — |
| Título (`h1`) | serif, 25pt, `#f6f3ec` | — |
| Dedicatória | itálica, centralizada, `#624f3c` (sépia amostrado) | `srgb(98,79,60)` |
| Corpo do conto | justificado, `text-indent: 5,2mm`, 1º parágrafo sem recuo, `orphans/widows: 2`, `hyphens: auto` | — |
| Rodapé | **monospace** espaçado (não itálico serif — defeito corrigido, ver §2.3), texto `#80735d`, nome do inspirador em **negrito azul-petróleo** `#2c5874` | amostrado |

### 2.3 Três defeitos que só a comparação visual pegou (registrados no commit, não inventados aqui)

A mensagem do commit é explícita sobre isto, e vale como lição operacional, não só como nota de produção: só a **comparação lado a lado** com o PDF original do Brunus revelou os três, nenhum apareceu por leitura de código:

1. A dedicatória saía **justificada à esquerda** — causa: especificidade de seletor CSS (`.corpo p` genérico vencia `.dedicatoria` até o script trocar o seletor para `.corpo p.dedicatoria`, classe+elemento, mais específico).
2. O kicker saía "CANÓNICO" com acento agudo (grafia pt-PT) em vez de "CANÔNICO" com circunflexo (pt-br).
3. O rodapé saía itálico serif quando o original era monospace espaçado com o nome do inspirador em negrito azul-petróleo.

### 2.4 O que o script faz que a descrição de §1 não previa

- **Trata o sprite no próprio script**: floodfill a partir da borda (não troca-branco-global, que comeria dentes/reflexos internos do desenho) + resize para não inflar o PDF — exemplo citado no commit: o sprite do Gus caiu de 3,5MB para 753KB.
- **Fonte do texto é parseada por convenção fixa**: `# Título` na primeira linha, dedicatória entre asteriscos (`*Para Fulano, que...*`), separador `---`, parágrafos separados por linha em branco. O script rejeita conto sem essa forma.
- **O rodapé deriva a atribuição da própria dedicatória** (regex sobre "Para \<nome\>, ..."), não é um campo redigitado à parte — reduz risco de o texto do rodapé divergir do texto da dedicatória.
- **PDFs não entram no git** — mesma política do Brunus, pasta ignorada; vivem em disco + backup do líder.
- **O PDF do Brunus foi propositalmente excluído do script** por ordem do líder em 25/07/2026 ("já está pronto, não refazer") — o script não sobrescreve o original, só gera os novos.

## §3. A armadilha do drop-cap, com o escopo correto

O texto do achado original (§1) é preciso ao nomear a causa: `float:left` como técnica de capitular (drop-cap) quebrava especificamente o **weasyprint** daquela época, com `AssertionError` dentro do módulo `float_layout` dele. **Isto não é uma propriedade geral de "drop-cap em PDF"** — é um bug de uma versão de uma biblioteca específica, e o achado nunca afirmou o contrário.

O script de `cced60c7` (era Chromium) **não usa float nem drop-cap**: o recuo da primeira linha de cada parágrafo é feito por `text-indent`, com o primeiro parágrafo do conto explicitamente sem recuo (`.corpo p:first-of-type { text-indent: 0; }`). Ou seja, a receita atual **evita a técnica que causava o bug**, não porque o Chromium tenha o mesmo problema comprovado (isso não foi testado em nenhum dos dois commits), mas porque o efeito visual desejado (recuo de primeira linha do parágrafo, não uma letra capitular grande) nunca precisou de drop-cap para começo de conversa. Registrar aqui para quem for implementar de novo: **se algum dia a receita quiser drop-cap de verdade** (letra grande caindo por duas ou três linhas), isso é uma técnica CSS diferente de recuo de primeira linha, e precisa ser testada contra o motor de conversão vigente antes de assumir que funciona.

## §4. Ferramenta de conversão vigente nesta máquina

Preferência permanente do líder, independente do que a receita antiga fazia: conversão de HTML para PDF nesta máquina usa o **Chromium instalado**, nunca WeasyPrint, wkhtmltopdf ou qualquer outra ferramenta:

```bash
chromium-browser --headless --disable-gpu --no-sandbox \
  --user-data-dir=<diretório temporário próprio, descartável> \
  --no-pdf-header-footer \
  --print-to-pdf=<saída.pdf> \
  file://<página.html>
```

Como registrado em §2, **isto já era a forma em uso** no `gusworld_legacy` desde `cced60c7` (25/07/2026) — a receita neste projeto não precisa "migrar" de weasyprint para Chromium, porque essa migração já tinha acontecido no projeto anterior antes mesmo dele ser encerrado. O que muda, se e quando o líder pedir um PDF-presente neste projeto, é reescrever o script do zero (LEI ZERO, L-01: nenhum arquivo do `gusworld_legacy` é reaproveitável como base de código aqui), usando esta receita só como referência de forma — cores a reamostrar do PDF original do Brunus (se ele for trazido), estrutura de template, e a lista de três defeitos de §2.3 como checklist de comparação visual antes de aprovar qualquer PDF novo.

Embutir imagem como `data:` URI base64 (em vez de referência de caminho de arquivo) evita problema de resolução de caminho relativo entre o HTML temporário e o sprite — prática já usada no script antigo e coerente com a orientação geral desta máquina para conversão HTML → PDF.

## §5. L-16 — homenagem a pessoa real só existe com aceite

Este é o ponto que torna este documento sensível, e não um mero registro técnico de CSS: **a receita descreve exatamente o artefato físico/digital que se entrega à pessoa homenageada.** A L-16 do `GODS_LAWS.md` (21/08/2026) é explícita: homenagem a pessoa real só existe **com aceite prévio dela**, e o aceite é fato que o **líder confirma**, não que um agente presume.

Consequências diretas para qualquer uso futuro desta receita:

- **Nenhum PDF-presente se gera para um personagem inspirado em pessoa real sem o líder confirmar que essa pessoa já aceitou a homenagem.** O `gusworld_legacy` só gerou PDFs para quatro casos, e em todos os quatro a dedicatória e o aceite eram fatos que o líder tinha estabelecido antes: Brunus → Bruno Vettore (crédito registrado, à época, no `README.md` do projeto anterior; neste projeto o crédito equivalente vive em `AI-DISCLOSURE.md` e `CHARS.md`), Gus → "Gus Dragon" (nunca o nome de batismo — regra do menor, também consolidada na L-16), Pyotor → Petrus (o próprio líder), Yakov → Iago.
- **O nome de batismo de menor nunca é versionado.** No caso do Gus, a dedicatória do PDF era para "Gus Dragon", nunca para o nome real do filho do líder — o mesmo apelido que a L-16 fixa como o único citável em público.
- **Este documento não autoriza a geração de nenhum PDF-presente novo por conta própria.** Ele registra a forma, para quando o líder pedir; o pedido e a confirmação de aceite continuam sendo decisão exclusiva dele (L-11 também se aplica: agente não decide, pergunta).

## §6. O que este documento não é

- Não é uma spec pronta para implementação neste projeto — os caminhos (`resources/sprites/personagens/...`, `resources/sprites/personagens_inspirados/...`) são do `gusworld_legacy` e podem não bater com a estrutura de `resources/` e `assets/` deste repositório; quem for implementar confere a estrutura atual antes de escrever caminho novo.
- Não é ordem de gerar PDF nenhum agora. É a receita, preservada porque o líder decidiu que ela merece virar documento em vez de linha de tabela.
