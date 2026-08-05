#!/usr/bin/env python3
"""callback_dtor_order.py - GATE(callback-dtor-order) do tools/check.sh.

ZERO-TOLERANCIA (nao ratchet). CONGELA a ordem de DECLARACAO dos membros nas
4 telas que registram um callback no glintfx::UiLayer: o dono do callback
(`ui_`) tem de ser declarado DEPOIS de tudo que a lambda registrada alcanca.

POR QUE ISTO E UM GATE, e nao so um comentario no fonte
-------------------------------------------------------
Membro de classe morre na ordem INVERSA da declaracao. Declarado por ULTIMO,
`ui_` e o PRIMEIRO a ser destruido; declarado por primeiro (como estava antes
da fatia CALLBACK-DTOR-ORDER), ele e o ULTIMO - e ai `~UiLayer()` roda com os
membros que a lambda toca JA DESTRUIDOS. Isso importa porque `~UiLayer()`
descarrega os documentos do RmlUi, o que RECALCULA a hover chain e pode emitir
UM ultimo evento de hover/clique; o glintfx levou exatamente esse
heap-use-after-free ao vivo (achado SO pelo ASan - passou na revisao, na suite
local e em 3 dos 4 jobs do CI pesado deles).

No caminho normal isto nunca dispara: `exit()` destroi `ui_` a mao com todos os
membros ainda vivos. A janela e o caminho em que `exit()` NAO roda - ele esta
solto na ultima linha de `run_screen_state()` (GusEngine/app/src/screen_state.cpp),
FORA de qualquer guard RAII, excecoes estao habilitadas (nao ha
-fno-exceptions) e as telas sao objetos de PILHA. Uma excecao escapando de
enter()/tick()/handle_event() desenrola a pilha, pula o `exit()` e cai direto
no destrutor - onde so a ordem de declaracao protege.

Como a ordem correta NAO tem sintoma observavel hoje (nenhum teste fica
vermelho se ela for desfeita, porque o cenario exige uma excecao que o codigo
atual nao lanca), o unico jeito de a proxima pessoa nao reordenar "por
organizacao" e um gate estatico. E este gate.

O QUE ELE MEDE
--------------
Para cada sitio da tabela SITES abaixo: a linha da declaracao de `owner`
(`ui_`) tem de ser MAIOR que a linha de declaracao de CADA `payload` (o que a
lambda alcanca). Comparacao por numero de linha, sobre a declaracao literal -
nao ha parser de C++ aqui, e nao precisa: cada literal e verificado como
OCORRENCIA UNICA no arquivo (auto-auditoria abaixo), entao um rename silencioso
REPROVA em vez de fazer o gate passar de graca.

AUTO-AUDITORIA (a parte que impede o gate de mentir): se qualquer literal da
tabela nao aparecer EXATAMENTE UMA vez no arquivo, o gate FALHA pedindo
atualizacao da tabela. Um gate que passa porque nao achou nada e pior que gate
nenhum - ele imprime OK e ninguem confere.

FORA DE ESCOPO (de proposito, NAO e omissao)
--------------------------------------------
- `app/tools/glintfx_input_e2e/glintfx_input_e2e_probe.cpp` registra um
  `App::set_frame_callback`, que NAO e da mesma familia: `frame_cb` e invocado
  de um unico sitio, `Impl::render_frame()` (glintfx `src/app.cpp`), alcancavel
  so por `App::render()`/`App::snapshot()` - nunca pelo teardown (`App::~App()`
  e `= default`). Nao ha evento de destruicao que o dispare, entao nao ha o que
  congelar ali. Se um dia aquele probe registrar hover/clique, ele entra nesta
  tabela - e ali a reordenacao NAO resolve (o `input` local captura `&app` no
  proprio construtor, logo `app` tem de nascer primeiro e morre por ultimo);
  o remedio seria desarmar o callback antes do fim do escopo.
- Desarmar o callback no destrutor (`set_*_callback(nullptr)`) e o conserto
  complementar, mais forte, que o proprio glintfx adotou. NAO foi aprovado
  nesta fatia; se um dia entrar, este gate continua valendo (os dois se somam,
  nenhum substitui o outro).
"""

import os
import sys

_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Dono do callback -> o que a lambda registrada nele alcanca. `owner` TEM de ser
# declarado DEPOIS de todo `payload` (= destruido ANTES de todos eles).
#
# Os payloads sao os membros que a lambda toca de fato, conferidos no fonte:
#   - as 3 telas de menu: a lambda chama `native_hover_callback_`, que le/escreve
#     `last_hover_sfx_id_` e le `state_` (via is_navigable_hover_id);
#   - battle_preview: a lambda derreferencia `*scene_` e `audio_ptr_` (que
#     aponta pra `local_audio_engine_` no modo standalone).
# `audio_` e referencia injetada (o dono e externo, sobrevive a tela) e os
# SoundId sao PODs - nao entram na tabela porque nao ha destrutor a ordenar.
SITES = {
    "GusEngine/app/src/screens/title_menu_loop.cpp": {
        "owner": "std::optional<glintfx::UiLayer> ui_;",
        "payload": [
            "std::string last_hover_sfx_id_;",
            "TitleMenuState state_;",
        ],
    },
    "GusEngine/app/src/screens/difficulty_menu_loop.cpp": {
        "owner": "std::optional<glintfx::UiLayer> ui_;",
        "payload": [
            "std::string last_hover_sfx_id_;",
            "DifficultyMenuState state_;",
        ],
    },
    "GusEngine/app/src/screens/system_menu_loop.cpp": {
        "owner": "std::optional<glintfx::UiLayer> ui_;",
        "payload": [
            "std::string last_hover_sfx_id_;",
            "SystemMenuState state_;",
        ],
    },
    "GusEngine/app/src/screens/battle_preview.cpp": {
        "owner": "std::optional<glintfx::UiLayer> ui_;",
        "payload": [
            "std::optional<BattleScene> scene_;",
            "std::optional<gus::platform::audio::AudioEngine> local_audio_engine_;",
            "gus::platform::audio::AudioEngine* audio_ptr_ = nullptr;",
        ],
    },
}


def _decl_lines(lines, literal):
    """Linhas (1-based) em que `literal` aparece como DECLARACAO de membro.

    Declaracao de membro neste codebase e sempre uma linha cujo conteudo
    strippado E o literal (indentacao de 4 espacos dentro da classe). Casar a
    linha inteira, e nao substring, evita contar o mesmo nome dentro de um
    comentario ou de uma expressao.
    """
    return [i + 1 for i, ln in enumerate(lines) if ln.strip() == literal]


def check_site(rel_path, spec):
    """Devolve lista de mensagens de erro (vazia = sitio OK)."""
    errors = []
    abs_path = os.path.join(_ROOT, rel_path)
    if not os.path.isfile(abs_path):
        return [f"{rel_path}: arquivo NAO existe (tabela SITES desatualizada?)."]

    with open(abs_path, "r", encoding="utf-8") as fh:
        lines = fh.read().splitlines()

    # --- auto-auditoria: cada literal tem de existir EXATAMENTE uma vez ------
    literals = [spec["owner"]] + list(spec["payload"])
    found = {}
    for literal in literals:
        hits = _decl_lines(lines, literal)
        found[literal] = hits
        if len(hits) != 1:
            errors.append(
                f"{rel_path}: a declaracao `{literal}` foi encontrada "
                f"{len(hits)}x (esperado: exatamente 1). A tabela SITES de "
                "tools/callback_dtor_order.py ficou desatualizada - atualize-a "
                "no MESMO commit que renomeou/moveu o membro, senao este gate "
                "passa sem medir nada."
            )
    if errors:
        return errors

    # --- a medida em si -----------------------------------------------------
    owner_line = found[spec["owner"]][0]
    for literal in spec["payload"]:
        payload_line = found[literal][0]
        if owner_line < payload_line:
            errors.append(
                f"{rel_path}:{owner_line}: `{spec['owner']}` esta declarado "
                f"ANTES de `{literal}` (linha {payload_line}), logo sera "
                "destruido DEPOIS dele - use-after-free se ~UiLayer() emitir o "
                "ultimo evento de hover/clique (CALLBACK-DTOR-ORDER). Mova a "
                "declaracao de `ui_` pro FIM da lista de membros."
            )
    return errors


def main() -> int:
    all_errors = []
    for rel_path, spec in sorted(SITES.items()):
        all_errors.extend(check_site(rel_path, spec))

    if all_errors:
        print("GATE(callback-dtor-order): FALHA - a ordem de declaracao que "
              "protege o teardown do UiLayer foi quebrada:")
        for err in all_errors:
            print(f"    - {err}")
        print("  Racional completo: o comentario ATENCAO - CALLBACK-DTOR-ORDER "
              "no fim da lista de membros de cada tela, e o docstring deste "
              "script.")
        return 1

    n_payload = sum(len(s["payload"]) for s in SITES.values())
    print(f"GATE(callback-dtor-order): OK ({len(SITES)} tela(s) com callback no "
          f"UiLayer, {n_payload} membro(s) alcancado(s) por lambda; em todas "
          "`ui_` e declarado DEPOIS de tudo que a lambda toca, logo destruido "
          "ANTES).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
