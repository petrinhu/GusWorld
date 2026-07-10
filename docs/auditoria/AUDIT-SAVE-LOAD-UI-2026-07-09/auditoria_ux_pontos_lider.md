# Auditoria: UX — pontos sinalizados pro líder + gap conhecido + cosmético

- Subsistemas: `gus/app/screens/save_load_menu_loop.cpp` (SFX genérico, feature
  Apagar no Auto), `save_load_menu_loop.cpp`/`title_menu_loop.cpp` (avisos de load
  não implementados), `gus/domain/save/save_slots.hpp` (comentário stale).
- Critério: nenhum — este capítulo registra decisões já tomadas pelo implementador que
  o brief pediu explicitamente para **NÃO decidir** (o `internal-auditor` não promove
  decisão de UX sozinho), mais 1 gap já aceito e 1 achado cosmético trivial.

## 1. Ponto pro líder (a): som de clique "genérico"

`save_load_menu_loop.cpp` toca o SFX de clique em **qualquer** clique que acerte um alvo
real na lista — inclusive um clique num slot que devolve `None` (ex.: linha já
selecionada, ou uma tentativa que não muda nada). Isto é uma **simplificação deliberada**
em relação ao padrão do menu de pausa/config (`system_menu_loop.cpp`), que só toca o SFX
em ações que de fato "confirmam" algo — porque lá existe o caso do drag-de-slider (que
não deve soar a cada pixel arrastado), um caso que não existe na tela de save/load. O
comentário no código já documenta essa divergência como intencional:

> "SIMPLIFICAÇÃO deliberada vs. `system_menu_loop.cpp` (que só toca em ações
> 'confirming' por causa do caso do drag-de-slider, que não existe nesta tela) - clicar
> numa linha readonly/já-vazia (que devolve None) ainda soa um clique 'reconhecido',
> aceitável aqui." (`save_load_menu_loop.cpp:504-511`)

**Não é um bug** — é uma decisão de UX já tomada e justificada pelo implementador, mas
o brief pediu para registrar como ponto pro líder confirmar/revisar (não decidir aqui):
o líder pode preferir paridade estrita com o menu de pausa (só soar em ações que de fato
mudam algo) em vez desta simplificação.

## 2. Ponto pro líder (b): slot Auto apagável com o mesmo diálogo genérico

A feature "Apagar" (aprovada pelo líder, retoque ao vivo) inclui o ícone por-linha em
**todos** os slots ocupados, **inclusive o Auto** — usando o MESMO mini-diálogo Sim/Não
genérico dos slots manuais, sem nenhum aviso adicional específico de que apagar o Auto
tem uma implicação distinta (o autosave é regravado sozinho pelo jogo nos próximos
gatilhos — entrar em batalha, sair pro título, vitória — então "apagar o Auto" tem uma
janela de efeito diferente de apagar um slot manual, que só volta se o jogador salvar
manualmente ali de novo). O código já documenta que isso foi uma decisão explícita:

> "feature 'Apagar': ícone por-linha em slots OCUPADOS (Auto incluso - decisão: Auto
> também apagável) abre mini-diálogo Sim/Não próprio" (`TODO.md`, linha do item
> SAVE-LOAD-UI, "RETOQUE AO VIVO 2026-07-09")

**Não é um bug** — é uma decisão já tomada, mas o brief pediu para reconfirmar
explicitamente com o líder nesta auditoria (não decidir aqui) se o diálogo genérico é
suficiente ou se o Auto merece um texto de aviso diferenciado.

## 3. Gap conhecido (já aceito pelo líder, NÃO é achado)

Os 2 avisos de LOAD — (i) versão-incompatível/corrompido, (ii) controles-diferentes-do-
save — **não foram implementados** nesta onda, por decisão explícita do líder (fora de
escopo, ver `TODO.md`: "AINDA FALTA (fora do escopo desta onda por decisão do líder): os
2 AVISOS..."). Hoje, um save presente-mas-não-Ok degrada silenciosamente como slot vazio
tanto em `title_menu_loop.cpp` (`scan_saves`) quanto em `save_load_menu_loop.cpp`
(`build_previews_and_cache`) — ambos logam em stderr ("aviso: slot X tem arquivo mas NÃO
carregou Ok... degradando como vazio nesta onda") mas não mostram nada ao jogador.
Registrado aqui como **gap conhecido, não achado crítico**, conforme instruído — mas
note-se que este mesmo comportamento de degradação é o que ALIMENTA o achado CRIT-1
(`auditoria_integridade_dados_save.md`): a ausência do aviso de LOAD é aceitável por si
só (o jogador só não vê um texto explicativo), mas a MESMA degradação aplicada ao modo
SAVE tem uma consequência de perda de dado que vai além de "falta um aviso" — por isso
CRIT-1 é tratado como achado novo e distinto, não como parte deste gap já aceito.

## 4. Achado cosmético

| ID | Sev | Descrição | Evidência | Estado |
|---|---|---|---|---|
| COS-1 | 🟢 COSMÉTICO | Comentário desatualizado em `save_slots.hpp:53-54` ("Nome lógico do slot: `\"autosave\"` (0) ou `\"save_1\"..\"save_5\"` (1..5)") não foi atualizado após o bump aditivo `kManualSlotCount` 5→6 (decisão do líder, SAVE-LOAD-UI etapa 6, `ADR-006`) — o CONTRATO real (`slot_logical_name`) já gera corretamente `"save_1".."save_6"` para `1..6` (`kManualSlotCount=6` na linha 38 do mesmo arquivo), só o comentário-doc ficou desatualizado | `save_slots.hpp:38` (`kManualSlotCount = 6`) vs `save_slots.hpp:53-54` (comentário ainda cita "save_5"/"1..5") | ⚠ (trivial) |

## Conclusão

Nenhum destes 4 pontos bloqueia o milestone. Os 2 pontos de UX (§1, §2) já têm decisão
tomada e justificada pelo implementador — o `internal-auditor` os registra para o líder
confirmar/revisar, sem promover uma escolha. O gap dos avisos de load (§3) permanece
aceito como estava. O comentário stale (§4, COS-1) é um ajuste trivial de 1 linha.
