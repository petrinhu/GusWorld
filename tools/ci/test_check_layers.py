# SPDX-FileCopyrightText: 2026 Petrus Alves da Silva Costa
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Self-test of the layer gate (tools/ci/check_layers.py), run by `--selftest`.

Each control builds a small source tree under /var/tmp (never inside the project
and never under /tmp, which is tmpfs here), writes a compile_commands.json for
it, and lets the real compiler run its preprocessor (-M) over the hostile form.
Only the preprocessor runs: no object is linked, no binary is executed.

Controls c01 to c11 are the eleven forms from the layer gate brief. Controls
e01 to e06, i01, o01 and a01 to a02 are extras: a clean tree that must pass,
the per-layer floor, the zero floor, two reading traps, the toolchain policy,
the case-name rule, the orphan rule, and the environment guards.

Declared gap, not covered here: a branch of #if that is NOT taken in the
configuration the build uses is not seen by the compiler. c09 proves the taken
branch is caught; the untaken one is an open limit.

Usage: python3 tools/ci/test_check_layers.py -v
"""

from __future__ import annotations

import json
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_layers as gate  # noqa: E402

TEMP_BASE = "/var/tmp" if Path("/var/tmp").is_dir() else None
FRAMEWORK_HEADER = b"#pragma once\ninline int glintfx_value() { return 1; }\n"
CLEAN_LAYERS = {
    "core/a.cpp": b"#include <string_view>\n",
    "content/c.cpp": b"#include <array>\n",
    "domain/d.cpp": b"#include <vector>\n",
    "app/e.cpp": b"#include <optional>\n",
}


def find_compiler() -> str:
    """A real compiler, not a cache launcher: the preprocessor run must be direct."""
    for directory in os.environ.get("PATH", "").split(os.pathsep):
        for name in ("g++", "c++", "clang++"):
            candidate = os.path.join(directory, name)
            if os.path.isfile(candidate) and "ccache" not in os.path.realpath(candidate):
                return candidate
    raise RuntimeError("nenhum compilador C++ direto encontrado no PATH")


COMPILER = find_compiler()


class LayerGateTest(unittest.TestCase):
    def build(self, files, links=None, unlisted=(), flags=(), external=None, bare=False):
        if not bare:
            files = {**CLEAN_LAYERS, **files}  # every layer has a unit, so the floor never masks the case
        root = Path(tempfile.mkdtemp(prefix="check_layers_src_", dir=TEMP_BASE)).resolve()
        build_dir = Path(tempfile.mkdtemp(prefix="check_layers_build_", dir=TEMP_BASE)).resolve()
        self.addCleanup(shutil.rmtree, root, True)
        self.addCleanup(shutil.rmtree, build_dir, True)
        write(root / "present/glintfx/x.hpp", FRAMEWORK_HEADER)
        write(root / "present/glx.hpp", FRAMEWORK_HEADER)
        for relative_path, data in files.items():
            write(root / relative_path, data)
        for relative_path, target in (links or {}).items():
            (root / relative_path).parent.mkdir(parents=True, exist_ok=True)
            os.symlink(target, root / relative_path)
        for relative_path, data in (external or {}).items():
            write(build_dir / relative_path, data)
        extra = [flag.format(root=root, build=build_dir) for flag in flags]
        database = []
        for relative_path in files:
            if not relative_path.endswith(".cpp") or relative_path in unlisted:
                continue
            source = str(root / relative_path)
            obj = str(build_dir / (relative_path.replace("/", "_") + ".o"))
            command = [COMPILER, "-std=c++23", f"-I{root}/present", *extra, "-c", source, "-o", obj]
            database.append({"directory": str(build_dir), "file": source, "command": shlex.join(command)})
        (build_dir / "compile_commands.json").write_text(json.dumps(database), encoding="utf-8")
        return root, build_dir

    def assert_flagged(self, files, unit, fragment, **options):
        root, build_dir = self.build(files, **options)
        report = gate.scan(root, build_dir)
        self.assertEqual(gate.verdict(report), 2, f"deveria reprovar; findings={report.findings} failed={report.failed}")
        hits = [f for f in report.findings if f.path == unit and fragment in f"{f.form} {f.detail}"]
        self.assertTrue(hits, f"nenhum achado em {unit} com {fragment!r}; findings={report.findings}")

    def assert_clean(self, files, **options):
        root, build_dir = self.build(files, **options)
        report = gate.scan(root, build_dir)
        self.assertEqual(
            gate.verdict(report), 0,
            f"deveria passar; findings={report.findings} failed={report.failed}",
        )
        return report


def write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def utf16le_with_bom(text: str) -> bytes:
    return b"\xff\xfe" + text.encode("utf-16-le")


class TestHostileForms(LayerGateTest):
    def test_c01_comment_before_hash_include_rejected(self):
        self.assert_flagged({"core/a.cpp": b"/**/#include <glintfx/x.hpp>\n"}, "core/a.cpp", "glintfx/x.hpp")

    def test_c02_hash_split_by_comment_rejected(self):
        self.assert_flagged({"core/a.cpp": b"#/**/include <glintfx/x.hpp>\n"}, "core/a.cpp", "glintfx/x.hpp")

    def test_c03_digraph_percent_colon_rejected(self):
        self.assert_flagged({"core/a.cpp": b"%:include <glintfx/x.hpp>\n"}, "core/a.cpp", "glintfx/x.hpp")

    def test_c04_include_through_intermediate_impl_file_rejected(self):
        self.assert_flagged(
            {"core/a.cpp": b'#include "impl.tcc"\n', "core/impl.tcc": b"#include <glintfx/x.hpp>\n"},
            "core/a.cpp", "glintfx/x.hpp",
        )

    def test_c05_symlink_without_suffix_rejected(self):
        self.assert_flagged(
            {"core/a.cpp": b'#include "glx"\n'}, "core/glx", "atalho simbolico",
            links={"core/glx": "../present/glx.hpp"},
        )

    def test_c05_symlink_target_outside_layers_rejected_at_the_include(self):
        self.assert_flagged(
            {"core/a.cpp": b'#include "glx"\n'}, "core/a.cpp", "glx",
            links={"core/glx": "../present/glx.hpp"},
        )

    def test_c06_include_through_macro_rejected(self):
        self.assert_flagged(
            {"core/a.cpp": b"#define GLX_HDR <glintfx/x.hpp>\n#include GLX_HDR\n"},
            "core/a.cpp", "glintfx/x.hpp",
        )

    def test_c07_include_next_rejected(self):
        self.assert_flagged({"core/a.cpp": b"#include_next <glintfx/x.hpp>\n"}, "core/a.cpp", "glintfx/x.hpp")

    def test_c08_system_header_outside_toolchain_rejected(self):
        self.assert_flagged(
            {"core/a.cpp": b"#include <winsock_fake.h>\n"}, "core/a.cpp", "winsock_fake.h",
            flags=("-isystem{build}/sysvendor",),
            external={"sysvendor/winsock_fake.h": b"int osdep();\n"},
        )

    def test_c09_taken_branch_include_rejected(self):
        source = (
            b"#if defined(GUSWORLD_A)\n"
            b"#include <glintfx/x.hpp>\n"
            b"#else\n"
            b"#include <string_view>\n"
            b"#endif\n"
        )
        self.assert_flagged({"core/a.cpp": source}, "core/a.cpp", "glintfx/x.hpp", flags=("-DGUSWORLD_A",))

    def test_c10_utf16_source_not_passed_blind(self):
        self.assert_flagged(
            {"core/a.cpp": utf16le_with_bom("#include <glintfx/x.hpp>\n")},
            "core/a.cpp", "UTF-16",
        )

    def test_c11_broken_symlink_rejected_not_ignored(self):
        root, build_dir = self.build(
            {"core/a.cpp": b'#include "quebrado.hpp"\n'},
            links={"core/quebrado.hpp": "nao_existe.hpp"},
        )
        report = gate.scan(root, build_dir)
        self.assertEqual(gate.verdict(report), 2)
        self.assertIn("core/a.cpp", [path for path, _ in report.failed])
        self.assertTrue(
            [f for f in report.findings if f.path == "core/quebrado.hpp" and "atalho simbolico" in f.form],
            f"atalho quebrado nao foi achado; findings={report.findings}",
        )


class TestExtraCases(LayerGateTest):
    def test_e01_clean_four_layers_pass(self):
        report = self.assert_clean({**CLEAN_LAYERS, "core/b.hpp": b"#include <cstdint>\n",
                                    "core/a.cpp": b'#include <string_view>\n#include "b.hpp"\n'})
        self.assertEqual((report.found, report.analysed), (4, 4))

    def test_e02_missing_layer_hits_per_layer_floor(self):
        files = {key: value for key, value in CLEAN_LAYERS.items() if not key.startswith("app/")}
        root, build_dir = self.build(files, bare=True)
        report = gate.scan(root, build_dir)
        self.assertEqual(gate.verdict(report), 2)
        self.assertEqual(report.found, 3, "o total passa de zero: so o piso por camada pega a camada sumida")
        self.assertTrue([f for f in report.findings if f.path == "app" and "sem nenhuma" in f.form])

    def test_e03_zero_units_hits_floor(self):
        root, build_dir = self.build({"core/notes.txt": b"nada de codigo\n"}, bare=True)
        report = gate.scan(root, build_dir)
        self.assertEqual(report.found, 0)
        self.assertEqual(gate.verdict(report), 2)

    def test_e04_utf8_bom_does_not_hide_the_include(self):
        self.assert_flagged({"core/a.cpp": b"\xef\xbb\xbf#include <glintfx/x.hpp>\n"}, "core/a.cpp", "glintfx/x.hpp")

    def test_e05_line_continuation_does_not_hide_the_include(self):
        self.assert_flagged({"core/a.cpp": b"#include \\\n<glintfx/x.hpp>\n"}, "core/a.cpp", "glintfx/x.hpp")

    def test_e06_toolchain_os_header_passes_by_design(self):
        # LEI ZERO: o GusWorld liga em GlintFx e no sistema operacional. O SO passa.
        self.assert_clean({**CLEAN_LAYERS, "app/e.cpp": b"#include <unistd.h>\n"})


class TestStructureAndEnvironment(LayerGateTest):
    def test_i01_layer_name_with_other_case_rejected(self):
        self.assert_flagged(
            {**CLEAN_LAYERS, "Core/a.cpp": b"int b();\n"}, "Core", "caixa diferente",
        )

    def test_o01_orphan_source_outside_the_build_rejected(self):
        self.assert_flagged(
            {**CLEAN_LAYERS, "core/orphan.cpp": b"#include <glintfx/x.hpp>\n"},
            "core/orphan.cpp", "fonte C++ fora do gate", unlisted=("core/orphan.cpp",),
        )

    def test_a01_search_path_from_environment_does_not_make_glintfx_toolchain(self):
        external = {"extinc/extglx/x.hpp": b"int ext();\n"}
        root, build_dir = self.build(
            {"core/a.cpp": b"#include <extglx/x.hpp>\n"}, external=external,
        )
        # The directory is on the compiler search path only if the gate forgot to clean the environment.
        with mock.patch.dict(os.environ, {"CPLUS_INCLUDE_PATH": str(build_dir / "extinc")}):
            report = gate.scan(root, build_dir)
        self.assertEqual(gate.verdict(report), 2, f"o ambiente vazou para o toolchain; findings={report.findings}")
        self.assertIn("core/a.cpp", [path for path, _ in report.failed])

    def test_a02_git_variables_do_not_change_the_verdict(self):
        bogus = {"GIT_DIR": "/var/tmp/nao_existe_git", "GIT_INDEX_FILE": "/var/tmp/nao_existe_index"}
        with mock.patch.dict(os.environ, bogus):
            self.assert_flagged(
                {**CLEAN_LAYERS, "core/orphan.cpp": b"int o();\n"},
                "core/orphan.cpp", "fonte C++ fora do gate", unlisted=("core/orphan.cpp",),
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)
