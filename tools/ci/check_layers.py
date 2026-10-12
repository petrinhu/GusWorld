#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Layer gate (GODS_LAWS.md L-17): the compiler decides what the pure layers include.

Pure layers are core/, content/, domain/ and app/. present/ is outside this
gate on purpose: it is the only layer allowed to touch GlintFx (L-06, L-27).

No include directive is parsed as text. For every compile unit that lives in a
pure layer, taken from compile_commands.json (CMAKE_EXPORT_COMPILE_COMMANDS=ON),
the compiler runs its preprocessor in dependency mode (-M) and reports every
file it opened, transitively. Comments before '#', digraphs, line splices,
macros, symlinks and include paths are resolved by the compiler itself.

A dependency passes when it is inside a pure layer, or when it is inside a
directory the compiler lists in its own search path (asked with -v). Anything
else is a violation. Each pure-layer file is also read as bytes, and a file in
UTF-16 or with NUL bytes is a violation, because the compiler may be reading
something the text does not show.

Usage:
  python3 tools/ci/check_layers.py --build <dir>   gate; <dir> holds compile_commands.json
  python3 tools/ci/check_layers.py --selftest      runs tools/ci/test_check_layers.py

Scan floor (L-36): every pure layer needs at least one compile unit, and
analysed must equal found. Zero units in total means every layer is empty, so the
per-layer floor already fails it.
Exit codes: 0 clean | 2 violation, failure or floor | 1 execution error
"""

from __future__ import annotations

import json
import os
import shlex
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

REPO_ROOT = Path(os.path.realpath(Path(__file__).resolve().parents[2]))
PURE_LAYERS = ("core", "content", "domain", "app")
CPP_SUFFIXES = (".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".ipp", ".inl", ".tpp", ".ixx", ".cppm")
# Translation-unit suffixes: a source of these in a pure layer that CMake does not compile is an orphan.
SOURCE_SUFFIXES = (".c", ".cc", ".cpp", ".cxx", ".ixx", ".cppm")
# Variables that add directories to the compiler search path. Left set, a GlintFx
# directory named by one of them would look like toolchain and pass.
SEARCH_ENV = (
    "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "OBJC_INCLUDE_PATH",
    "GCC_EXEC_PREFIX", "COMPILER_PATH", "DEPENDENCIES_OUTPUT", "SUNPRO_DEPENDENCIES",
)
# Flags that belong to the build's output or module scanner, not to what is
# included. Dropped, so the run is a plain -M and nothing is written.
DROPPED_FLAGS = ("-c", "-M", "-MM", "-MD", "-MMD", "-MG", "-MP")
DROPPED_WITH_VALUE = ("-o", "-MF", "-MT", "-MQ")
DROPPED_PREFIXES = ("-fmodule", "-fdeps-", "-MF", "-MT", "-MQ")
UTF16_BOMS = (b"\xff\xfe", b"\xfe\xff")


@dataclass(frozen=True)
class Unit:
    layer: str
    source: str
    directory: str
    argv: tuple[str, ...]


@dataclass(frozen=True)
class Finding:
    path: str
    form: str
    detail: str


@dataclass
class Report:
    found: int = 0
    analysed: int = 0
    failed: list[tuple[str, str]] = field(default_factory=list)
    findings: list[Finding] = field(default_factory=list)
    layer_found: dict[str, int] = field(default_factory=dict)
    layer_analysed: dict[str, int] = field(default_factory=dict)
    layer_failed: dict[str, int] = field(default_factory=dict)


def inside(path: str, base: str) -> bool:
    path = os.path.normcase(os.path.realpath(path))
    base = os.path.normcase(os.path.realpath(base))
    return path == base or path.startswith(base + os.sep)


def relative(path: str, root: Path) -> str:
    # No realpath here: a symlink must be reported by its own name, not by its target.
    return os.path.relpath(path, root).replace(os.sep, "/")


def layer_of(path: str, root: Path) -> str | None:
    for layer in PURE_LAYERS:
        if inside(path, str(root / layer)):
            return layer
    return None


def clean_env() -> dict[str, str]:
    return {key: value for key, value in os.environ.items() if key not in SEARCH_ENV}


def load_units(root: Path, build_dir: Path) -> list[Unit]:
    database = json.loads((build_dir / "compile_commands.json").read_text(encoding="utf-8"))
    units = []
    for entry in database:
        directory = entry["directory"]
        argv = entry["arguments"] if "arguments" in entry else shlex.split(entry["command"])
        source = os.path.realpath(os.path.join(directory, entry["file"]))
        layer = layer_of(source, root)
        if layer is not None:
            units.append(Unit(layer, source, directory, tuple(argv)))
    return units


def toolchain_dirs(compiler: str, env: dict[str, str]) -> list[str]:
    # Asks the compiler itself which directories it searches for system headers.
    result = subprocess.run(
        [compiler, "-x", "c++", "-E", "-v", "-"],
        input=b"", capture_output=True, env=env,
    )
    if result.returncode != 0:
        raise RuntimeError(f"`{compiler} -v` saiu com codigo {result.returncode}")
    dirs, collecting = [], False
    for line in result.stderr.decode("utf-8", "replace").splitlines():
        if line.startswith("#include <...> search starts here:"):
            collecting = True
        elif line.startswith("End of search list."):
            break
        elif collecting:
            dirs.append(os.path.realpath(line.strip()))
    return dirs


def plain_command(unit: Unit) -> list[str]:
    """The unit's own compile command, minus what writes output or scans modules. The source stays: -M reads it."""
    out, skip_next = [], False
    for arg in unit.argv:
        if skip_next:
            skip_next = False
            continue
        if arg in DROPPED_WITH_VALUE:
            skip_next = True
            continue
        if arg in DROPPED_FLAGS or arg.startswith(DROPPED_PREFIXES):
            continue
        out.append(arg)
    return out + ["-M"]


def parse_make_deps(text: str) -> list[str]:
    # -M prints "target: dep dep \\\n dep". Splices and escaped spaces are undone here.
    body = text.replace("\\\r\n", " ").replace("\\\n", " ")
    _, _, listed = body.partition(":")
    protected = listed.replace("\\ ", "\x00")
    return [name.replace("\x00", " ") for name in protected.split()]


def violation_for(real: str, root: Path, search: list[str]) -> str | None:
    if any(inside(real, str(root / layer)) for layer in PURE_LAYERS):
        return None
    if inside(real, str(root)):
        return "dependencia fora das camadas puras"
    if any(inside(real, directory) for directory in search):
        return None
    return "dependencia fora do repositorio e do toolchain"


def encoding_violation(real: str) -> str | None:
    with open(real, "rb") as handle:
        data = handle.read()
    if b"\x00" in data or data[:2] in UTF16_BOMS:
        return "arquivo em UTF-16 ou com byte nulo: o conteudo lido pelo compilador nao e verificavel"
    return None


def analyse_unit(unit: Unit, root: Path, env: dict[str, str], search: list[str]) -> tuple[list[Finding], str | None]:
    name = relative(unit.source, root)
    if os.path.basename(unit.argv[0]).startswith(("ccache", "sccache")):
        return [], "launcher de cache na linha de compilacao: nao suportado pelo portao"
    result = subprocess.run(plain_command(unit), cwd=unit.directory, capture_output=True, env=env)
    if result.returncode != 0:
        first = result.stderr.decode("utf-8", "replace").strip().splitlines()
        return [], f"o pre-processador falhou: {first[0] if first else 'sem mensagem'}"
    findings = []
    for listed in parse_make_deps(result.stdout.decode("utf-8", "replace")):
        real = os.path.realpath(os.path.join(unit.directory, listed))
        reason = violation_for(real, root, search)
        if reason:
            findings.append(Finding(name, reason, listed))
            continue
        if layer_of(real, root) is not None and os.path.isfile(real):
            encoding = encoding_violation(real)
            if encoding:
                findings.append(Finding(name, encoding, relative(real, root)))
    return findings, None


def structural_findings(root: Path, units: list[Unit]) -> list[Finding]:
    sources = {unit.source for unit in units}
    findings = []
    for entry in root.iterdir():
        if entry.name.lower() in PURE_LAYERS and entry.name not in PURE_LAYERS:
            findings.append(Finding(entry.name, "nome de camada com caixa diferente",
                                    "em sistema sem distincao de caixa vira a mesma pasta"))
    for layer in PURE_LAYERS:
        base = root / layer
        if not base.is_dir():
            continue
        for dirpath, dirnames, filenames in os.walk(base):
            for name in dirnames + filenames:
                full = os.path.join(dirpath, name)
                if os.path.islink(full):
                    findings.append(Finding(relative(full, root), "atalho simbolico dentro de camada pura",
                                            "nao e seguido; reprovado"))
                elif name.endswith(SOURCE_SUFFIXES) and os.path.realpath(full) not in sources:
                    findings.append(Finding(relative(full, root), "fonte C++ fora do gate",
                                            "nao esta no compile_commands: nunca compilada nem verificada"))
    return findings


def scan(root: Path, build_dir: Path) -> Report:
    root = Path(os.path.realpath(root))
    units = load_units(root, build_dir)
    env = clean_env()
    search = {compiler: toolchain_dirs(compiler, env)
              for compiler in sorted({unit.argv[0] for unit in units})}
    report = Report(found=len(units))
    for layer in PURE_LAYERS:
        report.layer_found[layer] = sum(1 for unit in units if unit.layer == layer)
        report.layer_analysed[layer] = 0
        report.layer_failed[layer] = 0
    for unit in units:
        findings, failure = analyse_unit(unit, root, env, search[unit.argv[0]])
        if failure:
            report.failed.append((relative(unit.source, root), failure))
            report.layer_failed[unit.layer] += 1
            continue
        report.analysed += 1
        report.layer_analysed[unit.layer] += 1
        report.findings.extend(findings)
    for layer in PURE_LAYERS:
        if report.layer_found[layer] == 0:
            report.findings.append(Finding(layer, "camada sem nenhuma unidade de compilacao",
                                           "o piso de varredura exige ao menos uma por camada (L-36)"))
    report.findings.extend(structural_findings(root, units))
    return report


def verdict(report: Report) -> int:
    if report.failed or report.analysed != report.found or report.findings:
        return 2
    return 0


def print_report(report: Report) -> int:
    print(
        f"check_layers: encontrados={report.found} analisados={report.analysed} "
        f"falharam={len(report.failed)} violacoes={len(report.findings)}"
    )
    for layer in PURE_LAYERS:
        print(
            f"  camada {layer}: encontrados={report.layer_found[layer]} "
            f"analisados={report.layer_analysed[layer]} falharam={report.layer_failed[layer]}"
        )
    for path, reason in report.failed:
        print(f"  FALHA {path}: {reason}", file=sys.stderr)
    for finding in report.findings:
        print(f"  {finding.path}  [{finding.form}]  {finding.detail}", file=sys.stderr)
    if verdict(report) == 0:
        print("check_layers: LIMPO. As camadas puras so incluem o que o compilador admite.")
        return 0
    print("check_layers: REPROVADO. Camada pura depende de algo fora da lista, "
          "ou nao foi possivel verificar.", file=sys.stderr)
    return 2


def main(argv: list[str]) -> int:
    if "--selftest" in argv:
        test_file = Path(__file__).with_name("test_check_layers.py")
        return subprocess.run([sys.executable, str(test_file), "-v"]).returncode
    if "--build" not in argv or argv.index("--build") + 1 >= len(argv):
        print("uso: check_layers.py --build <dir> | --selftest", file=sys.stderr)
        return 1
    build_dir = Path(argv[argv.index("--build") + 1]).resolve()
    try:
        report = scan(REPO_ROOT, build_dir)
    except (RuntimeError, OSError, ValueError, KeyError) as error:
        print(f"check_layers: erro de execucao: {error}", file=sys.stderr)
        return 1
    return print_report(report)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
