# DEPLOY_CHECKLIST.md - Checklist de Release Irreversível do GusWorld

> **Não há banco de dados, servidor, ambiente de produção, blue-green ou canary neste projeto.** LEI ZERO do `GODS_LAWS.md`: o jogo liga só no GlintFx e no sistema operacional, e roda inteiro na máquina do jogador. O que é **irreversível** aqui é **publicar**: criar tag, publicar release pública, ou quebrar compatibilidade de save. Este checklist substitui o deploy de serviço web pelo equivalente real deste projeto. Hub: `Standards.md`.

> Antes de executar qualquer operação marcada como irreversível, percorra **todos** os itens abaixo.

---

## FASE 0 - Classificação da mudança

- Quebra de compatibilidade de save sentida pelo jogador (sobe o componente **A** da versão, L-23)
- Mudança no formato do envelope binário de save, configuração, mapa ou catálogo (L-25) sem caminho de migração
- Criação de tag e publicação de release pública
- Merge em `main`, push ao remoto público, ou qualquer ação visível em remoto: exige **autorização explícita do líder naquele contexto**, sempre - aprovação anterior não vale para sempre

---

## FASE 1 - Pré-condições de qualidade (antes de tocar em tag)

- CI verde nas **cinco entradas** da matriz (L-09, L-20): Fedora 44 pinado, Ubuntu, Arch, CachyOS, Windows
- Os **cinco portões** da L-19 fechados: zero aviso de compilação (`-Werror`), ASan e UBSan em build separado, `clang-tidy`/`cppcheck` no CI, `gitleaks` na árvore **e** no histórico (`git log --all -p | grep -ci <termo>`, com `-i`), e o **gate de mensagem de commit** (`tools/security/commit_gate.py`) instalado e com o teste próprio verde (`python3 tools/security/test_commit_gate.py`)
- ⚠️ O quinto portão é **gancho local**, não etapa de CI: ele roda na máquina de quem commita. Um clone sem `tools/git-hooks/install.py` executado **não tem o portão**, e o CI verde não prova o contrário. Antes de taggear, confirme que o gancho está instalado no clone de onde a tag vai sair
- Suíte de **replay determinístico** (L-17: mesma semente + mesma lista de comandos reproduz o mesmo estado final) verde
- Marcação REUSE/SPDX íntegra em todo arquivo tocado (L-08)
- `resources/livros/` e `resources/glb/` seguem fora do git; Git LFS íntegro no restante do binário pesado (L-15)

---

## FASE 2 - Validação de compatibilidade

- Se a mudança quebra o formato de save: aviso explícito nas release notes, e o componente **A** da versão sobe (L-23)
- Se não quebra: componente **B** (recurso novo, compatível para trás) ou **C** (correção, sem recurso novo e sem quebra), conforme a tabela da L-23
- Quando a fatia toca save, configuração, mapa ou catálogo: o `internal-auditor` confirma que `AUDITORIAS.md` cobre a mudança antes de seguir

---

## FASE 3 - Aprovação e publicação (passo irreversível)

- **Aval explícito do líder no contexto**, tanto para a tag quanto para a release: a L-23 fixa o formato da versão, mas não autoriza taggear por si só
- Tag no formato **`vA.B.C.D`**, componente certo conforme a Fase 2
- Release notes agrupadas por tema, escritas para o jogador (destaques, corrigido, quebra de compatibilidade), nunca changelog cru de commit

---

## FASE 4 - Pós-publicação

- Confirmar a tag no remoto por `git ls-remote <url> <tag>` - **nunca só pela mensagem do `git push`**, que já mentiu antes em cenário de remoto duplo
- **Nenhuma reescrita de tag já publicada.** Erro grave descoberto depois vira tag corretiva nova, nunca `--force` sobre a já publicada
- Registro no `TODO.md` do que mudou, quando, e quem aprovou (o líder, com timestamp real, L-12)
