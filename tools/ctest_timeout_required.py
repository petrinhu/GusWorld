#!/usr/bin/env python3
"""ctest_timeout_required.py - GATE(ctest-timeout-required) do tools/check.sh.

ZERO-TOLERANCIA: TODO teste registrado no ctest precisa ter a propriedade
`TIMEOUT` (qualquer valor) - medido por INTROSPECAO REAL do ctest
(`ctest --preset <preset> --show-only=json-v1`), NUNCA por grep no
CMakeLists.txt.

Por que introspecao e nao grep: um grep por `PROPERTIES TIMEOUT` ou por
`set_tests_properties`/`set_property(TEST ...)` SO PROVA que a SINTAXE existe
em ALGUM LUGAR do arquivo - nao prova que aquele teste especifico, ou
qualquer teste, de fato recebeu a propriedade depois do ctest resolver
`catch_discover_tests`. Foi exatamente essa lacuna (achada por introspecao
real, GATES-S3, 2026-07-30) que revelou 3 dos 4 alvos de teste sem NENHUM
TIMEOUT enquanto um grep ingenuo diria "temos TIMEOUT" (achando so o de
app/tests) - o mesmo padrao apareceu do lado do glintfx com
`set_tests_properties(... ENVIRONMENT ...)`, que um grep confundiria com
timeout por acidente. Este gate fecha o buraco de forma PERMANENTE: no dia em
que nascer um 5o alvo de teste sem TIMEOUT, ele reprova na hora - em vez de
ficar destravado ate o proximo hang custar horas de maquina, do jeito que os
3 alvos ficaram destravados por 6 dias sem ninguem perceber.

REQUISITO DURO (nao e omissao): se a introspecao FALHAR por qualquer motivo
(sem build configurado, ctest ausente do PATH, JSON invalido, zero testes
devolvidos) o gate REPROVA com mensagem explicando o motivo - NUNCA reporta
OK sem ter conseguido medir de verdade. Um gate que passa quando nao
conseguiu medir e PIOR que nao ter gate (falso verde silencioso).
"""

import json
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(SCRIPT_DIR)
ENGINE = os.path.join(ROOT, "GusEngine")
PRESET = "linux-release"


def target_of(command):
    """Extrai o nome do binario (ultimo componente) do comando registrado no
    ctest - o mesmo dado que separou os 4 alvos na medicao original (GATES-S3)."""
    if not command:
        return "?"
    return os.path.basename(command[0])


def main() -> int:
    try:
        proc = subprocess.run(
            ["ctest", "--preset", PRESET, "--show-only=json-v1"],
            cwd=ENGINE,
            capture_output=True,
            text=True,
            timeout=60,
        )
    except FileNotFoundError:
        print("GATE(ctest-timeout-required): FALHA - binario 'ctest' nao encontrado no PATH.")
        print("  Nao consigo medir por introspecao - isto e FALHA, nao skip silencioso.")
        return 1
    except subprocess.TimeoutExpired:
        print("GATE(ctest-timeout-required): FALHA - 'ctest --show-only=json-v1' nao "
              "respondeu em 60s.")
        return 1

    if proc.returncode != 0:
        print(f"GATE(ctest-timeout-required): FALHA - 'ctest --preset {PRESET} "
              f"--show-only=json-v1' saiu com rc={proc.returncode} (sem build configurado? "
              f"preset '{PRESET}' ausente?).")
        print("  Nao consigo medir por introspecao - isto e FALHA, nao skip silencioso.")
        stderr_tail = proc.stderr.strip().splitlines()
        if stderr_tail:
            print(f"  stderr: {stderr_tail[-1]}")
        return 1

    try:
        data = json.loads(proc.stdout)
        tests = data["tests"]
    except (json.JSONDecodeError, KeyError) as exc:
        print("GATE(ctest-timeout-required): FALHA - saida de "
              f"'ctest --show-only=json-v1' nao e o JSON esperado ({exc}).")
        print("  Nao consigo medir por introspecao - isto e FALHA, nao skip silencioso.")
        return 1

    if not tests:
        print("GATE(ctest-timeout-required): FALHA - 'ctest --show-only=json-v1' devolveu "
              "ZERO testes (build vazio ou desatualizado?).")
        print("  Nao consigo medir por introspecao - isto e FALHA, nao skip silencioso.")
        return 1

    missing = []
    for t in tests:
        props = t.get("properties", [])
        has_timeout = any(p.get("name") == "TIMEOUT" for p in props)
        if not has_timeout:
            missing.append((target_of(t.get("command")), t.get("name", "?")))

    if missing:
        by_target = {}
        for tgt, name in missing:
            by_target.setdefault(tgt, []).append(name)
        print(f"GATE(ctest-timeout-required): FALHA - {len(missing)} teste(s) SEM propriedade "
              f"TIMEOUT (medido por introspecao real, {len(tests)} testes no total).")
        for tgt, names in sorted(by_target.items()):
            print(f"    - alvo {tgt}: {len(names)} teste(s) sem TIMEOUT")
            for name in names[:5]:
                print(f"        . {name}")
            if len(names) > 5:
                print(f"        ... e mais {len(names) - 5}")
        print("  Caminho certo: catch_discover_tests(<alvo> PROPERTIES TIMEOUT <N>) no "
              "CMakeLists.txt do alvo - meca o pior caso REAL daquele alvo (log do ctest ja da "
              "o tempo por teste) antes de escolher N, nao chute por simetria com outro alvo.")
        return 1

    print(f"GATE(ctest-timeout-required): OK ({len(tests)} testes, todos com TIMEOUT proprio, "
          "medido por introspecao real via ctest --show-only=json-v1).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
