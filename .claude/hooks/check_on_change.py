#!/usr/bin/env python3
"""PostToolUse hook: roda tools/check.sh (build + smoke + gate + suite) apos
edicoes que tocam codigo/i18n da engine. Espelha o automatismo do
PokemonTCGViewer, adaptado a stack C++/CMake/Catch2/Qt6 do GusWorld.

COMPLEMENTAR ao tdd_guard_cpp.py (PreToolUse): o guard cobra teste-primeiro
ANTES da escrita; este roda a suite DEPOIS. Os dois convivem no settings.json.

CUIDADO DE PERFORMANCE / RUIDO (por que nao incomoda):
  1. FILTRO DE PATH: so dispara se o arquivo editado casa GATILHO (codigo da
     engine, catalogos i18n, ou os proprios scripts de teste). Editar lore,
     docs, game/ (C# Godot congelado) ou engine/ (submodulo) NAO dispara.
  2. DEBOUNCE: numa rajada de N edits seguidos, roda no maximo 1x a cada
     DEBOUNCE_S segundos (marcador de tempo em build/.last_check). O ultimo
     edit da rajada sempre reflete o estado final (proxima chamada re-roda).
  3. LOCK: flock nao-bloqueante evita dois checks concorrentes (o 2o so
     anota "ja rodando" e sai).
  4. NAO-BLOQUEANTE: sempre exit 0. O resultado vai como additionalContext
     (transcript), nunca trava a tool. Build incremental e barato (~0.02s
     no-op, ~1.7s rebuild real); o custo so aparece quando ha recompilacao.

Para PULAR pontualmente (ex.: refactor longo em rajada): export
GUSWORLD_SKIP_CHECK=1 no ambiente da sessao.

Contrato PostToolUse: le JSON no stdin; imprime JSON com hookSpecificOutput.
"""
import json
import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CHECK = ROOT / "tools" / "check.sh"
STAMP = ROOT / "GusEngine" / "build" / ".last_check"
LOCK = ROOT / "GusEngine" / "build" / ".check.lock"

# Janela de debounce: no maximo 1 check a cada N segundos numa rajada de edits.
DEBOUNCE_S = 8

# Prefixos (relativos a raiz) que DISPARAM o check. Tudo fora disso e ignorado.
GATILHO_PREFIXOS = (
    "GusEngine/core/",
    "GusEngine/domain/",
    "GusEngine/platform/",
    "GusEngine/app/",
    "GusEngine/tests/",
    "GusEngine/CMakeLists.txt",
    "GusEngine/CMakePresets.json",
    "game/translations/",   # catalogos i18n (gate de paridade)
    "tools/check.sh",
    "tools/i18n_parity.py",
)
# Extensoes de codigo/dado que importam (dentro dos prefixos acima).
GATILHO_SUFIXOS = (".cpp", ".cc", ".cxx", ".hpp", ".hxx", ".h",
                   ".cmake", ".json", ".md", ".sh", ".py", ".txt")


def emite(msg: str) -> None:
    print(json.dumps({
        "hookSpecificOutput": {
            "hookEventName": "PostToolUse",
            "additionalContext": msg,
        }
    }))


def deve_disparar(rel: str) -> bool:
    if not rel.startswith(GATILHO_PREFIXOS):
        return False
    # CMakeLists/Presets/check.sh/i18n_parity casam por nome exato no prefixo;
    # para os diretorios, exige sufixo de codigo/dado conhecido.
    if rel.endswith(GATILHO_SUFIXOS):
        return True
    return False


def main() -> int:
    if os.environ.get("GUSWORLD_SKIP_CHECK") == "1":
        return 0
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0

    if payload.get("tool_name") not in ("Edit", "Write", "MultiEdit"):
        return 0

    fp = (payload.get("tool_input") or {}).get("file_path", "")
    if not fp:
        return 0

    try:
        rel = str(Path(fp).resolve().relative_to(ROOT)).replace("\\", "/")
    except ValueError:
        return 0  # fora do repo

    if not deve_disparar(rel):
        return 0

    if not CHECK.exists():
        return 0

    # Debounce: se rodou ha menos de DEBOUNCE_S, pula (rajada de edits).
    now = time.time()
    try:
        if STAMP.exists() and (now - STAMP.stat().st_mtime) < DEBOUNCE_S:
            return 0
    except OSError:
        pass

    # Lock nao-bloqueante: se outro check roda, sai quieto.
    STAMP.parent.mkdir(parents=True, exist_ok=True)
    try:
        import fcntl
        lock_fd = open(LOCK, "w")
        try:
            fcntl.flock(lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except OSError:
            lock_fd.close()
            return 0
    except ImportError:
        lock_fd = None  # plataforma sem fcntl: segue sem lock

    try:
        STAMP.write_text(str(now))
        env = dict(os.environ, CHECK_QUIET="1")
        proc = subprocess.run(
            ["bash", str(CHECK)],
            cwd=str(ROOT), env=env,
            capture_output=True, text=True, timeout=300,
        )
        saida = (proc.stdout or "").strip()
        # Resume so as linhas-marcador + tabela (a saida ja e enxuta em QUIET).
        msg = (f"[check.sh apos editar `{rel}`] rc={proc.returncode}\n{saida}")
        if proc.returncode != 0:
            err = (proc.stderr or "").strip()
            if err:
                msg += f"\n--- stderr ---\n{err[-1500:]}"
            msg += ("\nALGUM ESTAGIO FALHOU. Veja BUILD/SMOKE/GATE/SUITE acima "
                    "antes de prosseguir.")
        emite(msg)
    except subprocess.TimeoutExpired:
        emite(f"[check.sh apos editar `{rel}`] TIMEOUT (>300s). Rode "
              "`bash tools/check.sh` a mao para investigar.")
    except Exception as e:
        emite(f"[check.sh apos editar `{rel}`] erro ao rodar: {e}")
    finally:
        if lock_fd is not None:
            lock_fd.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())
