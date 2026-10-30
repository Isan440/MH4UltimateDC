# MH4U Android In-App Code Library Builder

Status: architecture plan; no ROM-derived data or generated game code is committed.

## Goal

Let the Android app prepare a user's own supported, decrypted MH4U update dump locally and produce the ARM64 Android code library consumed by the existing MH4U/Azahar integration. GitHub Actions builds and tests the APK only; it must never receive a ROM, extracted game code, generated recompilation sources, or the resulting game-code library.

## Verified starting points

- `tools/build_game.py` owns the dump-to-library pipeline: NCCH/ExeFS extraction, supported-version validation, optional code patches, C recompilation, and shared-library build.
- `tools/recomp/build.py` already has an Android AArch64 target, a minimal Android C header set, linker stubs for Android `libc.so`/`libm.so`, a 16-KiB page-alignment setting, and ELF/import validation.
- `port/android-shell/` is a minimal Android app shell, not yet the production game UI.
- `.github/workflows/android.yml` builds the current Azahar-based APK.
- `.github/workflows/build-mh4u-android-library.yml` currently accepts a dump URL in CI and builds the private game library remotely. This is not the target flow for the in-app builder and must remain untouched until a replacement is proven.
- Ocarina ReTuned demonstrates the Android pattern: a separate preparation service, embedded Python, compiler/linker binaries packaged in the APK's ARM64 native-library directory, a hash-verified preparation kit, progress/cancellation, bounded parallel compilation, and local cache.

## Component mapping

| Ocarina pattern | MH4U adaptation |
| --- | --- |
| ROM selection via Android document picker | Select the user's decrypted MH4U update dump via Storage Access Framework |
| `RomPreparationService` in a separate process | `Mh4uBuildService` in a separate `:builder` process |
| Chaquopy embedded Python entrypoint | Embedded Python entrypoint wrapping the existing `tools/build_game.py` pipeline |
| `libretuned_clang.so` and `libretuned_lld.so` in APK native libs | Android ARM64-hosted Clang/LLD, proven by an on-device compile/link smoke test |
| Verified preparation kit + SDK manifest | Versioned MH4U builder kit containing only scripts, headers, and toolchain support files |
| Incremental object/module cache | Cache keyed by compiler/toolchain identity, source/header hashes, dump identity and build options |
| Progress/cancel/session recovery | Persistent stage status, cancellation marker, atomic output activation and recovery after process death |

## Implementation sequence

1. **Toolchain proof of concept**
   - Package a minimal Android ARM64-hosted compiler and linker as APK native libraries.
   - From an Android service, invoke them to compile a tiny C source for `aarch64-linux-android`, link a shared object, and validate it with ELF inspection.
   - Do not start the full game pipeline until this passes on a physical Android device.

2. **Python dependency audit**
   - Inventory imports used by `build_game.py`, `recompile.py`, `recomp.build`, NCCH extraction, Capstone and ELF helpers.
   - Verify Android-compatible Chaquopy wheels/native modules for each required dependency.
   - Replace or isolate desktop-only process assumptions without changing recompilation semantics.

3. **Builder kit and local pipeline**
   - Stage only the MH4U build scripts, runtime headers, and compiler support files.
   - Verify every staged file with SHA-256 and a pinned manifest.
   - Refactor the pipeline behind a Python API callable from the embedded runtime; preserve the existing CLI for desktop builds.
   - Read the ROM from app-private storage; never upload it or derived game code.

4. **Android service and UI**
   - Add document-picker import, foreground notification, progress, cancellation, free-space checks, and a single-builder lock.
   - Bound compiler workers using available memory and device limits; start conservatively at one worker on mid-range devices.
   - Keep generated files in a private staging directory and atomically activate the finished library only after validation.

5. **Runtime integration**
   - Install/copy the generated `0004000000126300.so` into the location expected by the existing MH4U loader.
   - Validate title ID, supported game version/code hash, ELF machine, SONAME/dependencies, symbols, and 16-KiB alignment before activation.

6. **CI and device acceptance**
   - GitHub Actions builds the APK and runs host-side tests; it does not handle the user's dump or output library.
   - Test on-device on ARM64: toolchain smoke test, dump validation, full build, cancellation/retry, cache reuse, and game-library loading.
   - Keep the existing APK workflow as rollback until the new path passes.

## Non-goals and safety constraints

- Do not copy Ocarina's title-specific translator, A32 runtime, or game module into MH4U.
- Do not include a ROM, extracted `code.bin`, generated C sources, or a game-derived `.so` in public APKs or public CI artifacts.
- Do not depend on Termux, a PC, an external compiler app, or remote upload of the user's dump.
- A green APK build is not proof that the in-app compiler works; physical-device validation is required.

## Audit update — 2026-10-30

### Confirmed MH4U runtime dependencies

The production import path from `tools/build_game.py` through `recompile.py`,
`recomp.analysis`, `recomp.emit`, `recomp.build`, NCCH extraction, BLZ
decompression and code-mod handling uses the Python standard library and this
repository's own Python modules. The production decoder is the project's own
ARM/Thumb implementation (`tools/recomp/arm.py`, `thumb.py`, `insn.py`),
not Capstone or Unicorn. The current `requirements.txt` includes Capstone,
Unicorn and pyelftools for development/test tooling; they are not direct
requirements of the dump-to-library runtime import path inspected so far.
This makes an embedded Python runtime substantially simpler: no native Python
wheels are currently required for the core build path.

### First implementation commit

Added `tools/android_builder/entry.py`, a thin embedded-Python API that:
- accepts only local filesystem paths supplied by the Android service;
- requires a present, executable Android-host compiler;
- calls the existing `build_game.main()` with `--android` and the supplied compiler;
- defaults to one worker and `O0`, limits the worker count to 1–4, and allows
  only `O0` or `O1`;
- uses `--no-mods` unless a specific local mod directory is provided;
- checks that the expected MH4U title library exists and passes the existing
  Android ELF/import/alignment validator.

Added unit tests for argument construction and bounds in
`tests/test_android_builder_entry.py`. The CI workflow now also runs on this
feature branch. These changes do not yet provide an Android-host compiler,
service, UI, or a working on-device build.

### Toolchain provenance caveat

Ocarina's Android setup code expects `libretuned_clang.so` and
`libretuned_lld.so` under Android's `nativeLibraryDir`. However, the public
`stage_android_host.py` and `android-native-host.yml` inspected here stage
the neutral game/runtime libraries and `libc++_shared.so`; they do not create
those two compiler executables. Therefore we must not assume that the visible
Ocarina host workflow itself supplies the compiler. The exact provenance,
build recipe, license, and executable packaging path for those compiler
artifacts still need to be established before adapting or redistributing them.

### Immediate next gate

Before implementing the Android service, build a tiny ARM64 Android-host
executable, package it under `jniLibs/arm64-v8a`, and run it from
`ApplicationInfo.nativeLibraryDir` in a physical-device smoke test. This
isolates Android's executable/packaging behavior from the much larger Clang
toolchain. It is only an execution probe, not a compiler and not proof that
MH4U can yet be built on-device.
