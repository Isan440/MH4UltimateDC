"""Android entry point for the existing MH4U dump-to-ARM64 library pipeline.

This module is intentionally a thin adapter: it does not duplicate the game
extractor, version checks, recompilation logic, or ELF validation. It is called
from the embedded Python runtime by the Android preparation service.
"""
from __future__ import annotations

import os
import sys
from pathlib import Path
from typing import Any

PROGRAM_ID = 0x0004000000126300
TOOLS_DIR = Path(__file__).resolve().parents[1]


class AndroidBuildError(RuntimeError):
    """A user-actionable failure while preparing the local Android library."""


def _build_arguments(
    dump_path: Path,
    output_dir: Path,
    compiler_path: Path,
    jobs: int,
    optimization: str,
    mods_dir: Path | None,
) -> list[str]:
    if optimization not in {"O0", "O1"}:
        raise AndroidBuildError("Android builder supports O0 or O1 optimization.")
    if not 1 <= jobs <= 4:
        raise AndroidBuildError("Compiler worker count must be between 1 and 4.")

    args = [
        "--dump", str(dump_path),
        "--out", str(output_dir),
        "--android",
        "--cc", str(compiler_path),
        "--opt", optimization,
        "--jobs", str(jobs),
    ]
    if mods_dir is None:
        args.append("--no-mods")
    else:
        args.extend(["--mods", str(mods_dir)])
    return args


def build_android_library(
    dump_path: str,
    output_dir: str,
    compiler_path: str,
    jobs: int = 1,
    optimization: str = "O0",
    mods_dir: str | None = None,
) -> dict[str, Any]:
    """Build MH4U's Android ARM64 code library entirely in local app storage.

    The caller must copy the selected document URI into app-private storage and
    pass that local path here. The compiler must be a verified Android-host
    executable packaged with the APK (normally under nativeLibraryDir).
    """
    dump = Path(dump_path).resolve()
    output = Path(output_dir).resolve()
    compiler = Path(compiler_path).resolve()
    mods = Path(mods_dir).resolve() if mods_dir else None

    if not dump.is_file() or dump.stat().st_size == 0:
        raise AndroidBuildError("The selected game dump is missing or empty.")
    if not compiler.is_file() or not os.access(compiler, os.X_OK):
        raise AndroidBuildError("The bundled Android ARM64 compiler is missing or not executable.")
    if dump == output or dump.is_relative_to(output):
        raise AndroidBuildError("The output directory must not contain or replace the source dump.")
    if mods is not None and not mods.is_dir():
        raise AndroidBuildError("The selected MH4U mod directory does not exist.")

    output.mkdir(parents=True, exist_ok=True)
    if str(TOOLS_DIR) not in sys.path:
        sys.path.insert(0, str(TOOLS_DIR))

    try:
        import build_game
        from recomp.build import android_library_problems
    except Exception as exc:
        raise AndroidBuildError(f"Could not load the MH4U build pipeline: {exc}") from exc

    args = _build_arguments(dump, output, compiler, jobs, optimization, mods)
    try:
        result = build_game.main(args)
    except SystemExit as exc:
        code = exc.code if isinstance(exc.code, int) else 1
        if code:
            raise AndroidBuildError(f"MH4U recompilation failed (exit {code}).") from exc
        result = 0
    except Exception as exc:
        raise AndroidBuildError(f"MH4U recompilation failed: {exc}") from exc

    if result not in (None, 0):
        raise AndroidBuildError(f"MH4U recompilation returned exit code {result}.")

    library = output / "recomp" / f"{PROGRAM_ID:016X}.so"
    if not library.is_file() or library.stat().st_size == 0:
        raise AndroidBuildError("The build finished without producing the expected MH4U library.")

    problems = android_library_problems(str(library))
    if problems:
        raise AndroidBuildError("The generated library failed Android ELF validation: " + "; ".join(problems))

    return {
        "program_id": f"{PROGRAM_ID:016X}",
        "library_path": str(library),
        "library_bytes": library.stat().st_size,
        "optimization": optimization,
        "workers": jobs,
        "validated": True,
    }
