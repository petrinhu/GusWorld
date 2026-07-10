# Auditoria: Paridade i18n + integridade da evidência visual (SAVE-LOAD-UI)

- Subsistemas: `game/translations/{pt_br,en_intl}.md` (chaves `SAVE_*`/`TITLE_*`),
  `tools/i18n_parity.py`, `docs/design/mockups/menu_capturas/{save_load_save,
  save_load_load}.png`.
- Critério: AUD-I18N (paridade estrutural, zero string hardcoded) + integridade da
  trilha de evidência do commit (achado IMP-1).

## Contexto e método

Verificação de paridade estrutural via o mesmo GATE do `tools/check.sh` (reproduzido no
capítulo 2 do índice mestre: 217 chaves, 0 faltando/0 extra em `en_intl`). Em seguida, a
auditoria abriu e conferiu VISUALMENTE os PNGs de "prova visual" já commitados no dossiê
de mockups (`docs/design/mockups/menu_capturas/save_load_save.png` e
`save_load_load.png`) — prática exigida pela convenção do projeto (memória
`feedback_verificacao_visual_print_headless`: nunca aceitar prova visual sem abrir e
olhar). Ao abrir, os PNGs mostravam texto suspeito (literais `"x"` em vez de frases) —
a auditoria então **reproduziu ao vivo** a captura real (binário de produção
`gusworld_app`, `GUSWORLD_SAVELOAD_SCREENSHOT_DIR`, Xvfb :99, `GUSWORLD_HOME` de scratch)
para comparar.

## Achados — paridade estrutural (OK)

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| I18N-1 | (OK) | Todas as ~30 chaves novas `SAVE_*` (slot/preview/telas/confirmação/exclusão) existem em AMBOS os locales, mesma ordem | `game/translations/pt_br.md:129-200` vs `en_intl.md:93-139` (23 chaves `SAVE_*` em cada) | ✓ |
| I18N-2 | (OK) | Todas as chaves novas `TITLE_*` (logo/subtítulo/rodapé/confirmação de Novo Jogo) existem em ambos os locales | `pt_br.md:225-243` vs `en_intl.md:157-169` (7 chaves `TITLE_*` em cada) | ✓ |
| I18N-3 | (OK) | GATE de paridade (`tools/i18n_parity.py`, reproduzido via `check.sh`) confirma **0 faltando / 0 extra** nas 217 chaves totais do catálogo (incluindo as desta feature) | reprodução do capítulo 2 do índice mestre | ✓ |
| I18N-4 | (OK) | `location_key_for_scene` (mapeamento `scene_path`→chave de local) tem fallback seguro (`LOCATION_UNKNOWN`) pra cenas desconhecidas, evitando string hardcoded | `save_load_menu_rml.cpp:64-66` | ✓ |
| I18N-5 | (OK) | `Translator::tr()` degrada retornando a PRÓPRIA CHAVE quando a tradução falta/arquivo não carrega — nunca uma string vazia ou um placeholder genérico tipo `"x"` | `app/src/i18n/translator.cpp:47-52` (`return key;` no fallback) | ✓ |

## Achado — integridade da evidência visual (IMP-1)

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| IMP-1 | 🟠 IMPORTANTE | Os 2 PNGs de "prova visual" commitados em `docs/design/mockups/menu_capturas/` (`save_load_save.png`/`save_load_load.png`, adicionados no commit `a24a699` e atualizados no `67a81db`) **não refletem** o catálogo real de i18n nem uma execução real do self-test `GUSWORLD_SAVELOAD_SCREENSHOT_DIR` como a mensagem do commit `67a81db` afirma ("2 PNGs headless (Xvfb :99) conferidos"). Os PNGs commitados mostram: subtítulo/rodapé/local como literal `"x"` (não "Praça da Compilação"/frases completas), timestamp `01/01/1970`-like e XP 340/Cap. 3 num slot que, numa captura real do self-test, nasce recém-semeado (XP 0, Cap. 1, timestamp de HOJE). Reproduzindo AO VIVO a captura real (binário `gusworld_app`, `GUSWORLD_HOME` de scratch, `GUSWORLD_SAVELOAD_SCREENSHOT_DIR` de scratch, Xvfb :99) o resultado é: subtítulo "7 espaços (lista rola) - espaço vazio não selecionável", local "Praça da Compilação", timestamp real `10/07/2026 03:05`, rodapé completo em pt-br. **Não é um bug de produção** — o i18n real funciona perfeitamente (confirmado) — mas a evidência commitada no dossiê está errada/obsoleta (provavelmente gerada por uma ferramenta de dev com um catálogo de teste reduzido, não pelo pipeline descrito no commit) | comparação lado a lado: PNGs commitados (`docs/design/mockups/menu_capturas/save_load_{save,load}.png`) vs PNGs gerados nesta auditoria (execução real documentada no capítulo 2 do índice mestre) | ⚠ aberto |

## Remediação proposta (NÃO aplicada)

Regenerar os 2 PNGs via o comando já confirmado nesta auditoria:

```bash
DISPLAY=:99 SDL_VIDEODRIVER=x11 \
  GUSWORLD_HOME=<dir de scratch vazio> \
  GUSWORLD_SAVELOAD_SCREENSHOT_DIR=<dir de saída> \
  ./build/linux-release/app/gusworld_app
```

e re-commitar por cima dos 2 arquivos existentes. Baixo custo (nenhuma mudança de
código), cabe no mesmo commit que resolve CRIT-1/COS-1.

## Conclusão

A paridade estrutural i18n está limpa (0 faltando/0 extra) e o comportamento de fallback
do `Translator` é seguro. O único achado deste capítulo é sobre a integridade da
EVIDÊNCIA já commitada (não do código): os 2 PNGs de prova visual do dossiê de mockups
não provam o que a mensagem do commit afirma que provam. Classificado 🟠 (não 🔴) porque
não há impacto funcional em produção — apenas risco de um auditor futuro confiar numa
evidência que não corresponde à realidade do i18n do jogo.
