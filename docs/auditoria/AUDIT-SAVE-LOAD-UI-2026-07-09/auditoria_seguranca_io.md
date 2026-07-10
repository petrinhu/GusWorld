# Auditoria: Segurança do I/O de save (path/symlink/permissão)

- Subsistemas: `gus/platform/fs/save_file_store.cpp` (`resolve_saves_dir`,
  `ensure_saves_dir`, `FsSaveStore`), `gus/app/maestro.cpp`
  (`frozen_city_snapshot_path`).
- Critério: AUD-SEC (resolução de path/porteiro, symlink, permissão de arquivo do
  jogador — LGPD leve).

## Contexto e método

Leitura de `save_file_store.cpp` completo + a cadeia de resolução de diretório
(`resolve_saves_dir`) + o comentário histórico de symlink já registrado em `maestro.cpp`
(achado AC-E3 de uma auditoria anterior, `AUDITORIA-COMPLETA-2026-07-06`), para conferir
se o MESMO cuidado foi aplicado ao diretório de saves nesta onda.

## Achados

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| SEC-1 | (OK) | `resolve_saves_dir` segue a cadeia `GUSWORLD_HOME` (override de teste/CI) > `$HOME/.gusworld/saves` > fallback relativo ao CWD (defensivo, "nunca deveria acontecer num host real") — mesma política de `resolve_settings_dir`, já auditada | `save_file_store.cpp:120-134` | ✓ |
| SEC-2 | (OK) | `ensure_saves_dir` cria o diretório com `stdfs::perms::owner_all` (0700) sob demanda, e cada arquivo é gravado com `owner_read\|owner_write` (0600) — dado privado do jogador, mesma política LGPD leve já validada para `settings.json` | `save_file_store.cpp:31-49` (dir) + `save_file_store.cpp:87-96` (arquivo) | ✓ |
| SEC-3 | (OK) | Diretório do jogador (`~/.gusworld/`) é criado com 0700 ANTES de qualquer escrita — o risco clássico de symlink pré-plantado (a classe de vulnerabilidade documentada no achado AC-E3 anterior, sobre `/tmp` compartilhado) não se aplica aqui: o diretório é privado do dono desde a criação, e o padrão (`std::filesystem::create_directories`+`permissions`, sempre dentro de `$HOME`) é o MESMO já usado por `settings_file_store.cpp` e por `frozen_city_snapshot_path` (maestro.cpp) — nenhum caminho fixo em `/tmp` compartilhado é usado pelo save | `save_file_store.cpp:120-134` (nunca usa `/tmp`); comparação com `maestro.cpp:88-104` (comentário AC-E3 explica a MESMA técnica aplicada lá) | ✓ |
| SEC-4 | (OK) | Falha ao setar permissão (ex.: filesystem que não suporta POSIX perms) degrada com log e SEGUE gravando — não perde a gravação por causa disso, mas também não é uma condição de erro silenciosa (loga em stderr) | `save_file_store.cpp:43-47` (dir) + `save_file_store.cpp:92-96` (arquivo) | ✓ |
| SEC-5 | (OK) | Nomes lógicos (`primary_logical_name`/`backup_logical_name`) vêm de funções do domínio (`save_slots.hpp`/`save_backup.hpp`), nunca de entrada externa/do jogador diretamente — `FsSaveStore::path_for` faz `dir_ + "/" + name + ".sav"` sobre esses nomes controlados (ex.: `"autosave"`, `"save_3.backup2"`), sem concatenar nada vindo de fora do processo (sem risco de path traversal via nome de slot, já que `slot` é sempre um `int` validado por `is_valid_slot`) | `save_file_store.cpp:57-59` (`path_for`); `save_slots.hpp`/`save_backup.hpp` (nomes gerados só a partir de `int slot` fail-fast) | ✓ |
| SEC-6 | (OK) | `FsSaveStore::move`/`remove` degradam com segurança (ausência de origem = no-op pelo contrato do port; falha real de `remove()` não tem pra onde propagar no port, mas isso é aceitável — a camada acima, `delete_save`, VERIFICA via `exists()` se o arquivo de fato sumiu, não confia cegamente no retorno de `remove`) | `save_file_store.cpp:99-116` (`move`/`remove`); `save_file_store.cpp:165-177` (`delete_save` confirma via `!store.exists(primary)`) | ✓ |

## Conclusão

Nenhum achado de severidade neste capítulo. A fronteira de I/O do save segue a mesma
receita já validada para `settings.json` (0700/0600, resolução de path por `GUSWORLD_HOME`/
`HOME`, degradação segura de falha de permissão) e não introduz nenhum caminho fixo
compartilhado (tipo `/tmp`) nem concatenação de entrada externa não-controlada em paths.
O achado de integridade de dados (CRIT-1, capítulo anterior) não é uma falha de
segurança de I/O per se — é uma falha de lógica de UI sobre um `LoadOutcome` já
corretamente reportado pela camada de I/O.
