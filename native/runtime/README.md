# Native runtime foundation (no Azahar)

This directory is the first isolated host-runtime layer for the existing static recompiler ABI in `tools/recomp/runtime/recomp.h`.

## Included in this milestone

- A bounded byte-addressed guest-memory buffer with little-endian 8-, 16-, and 32-bit reads/writes.
- A minimal guest function dispatch table for unit testing.
- An `RHost` callback table compatible with `RC_HOST_VERSION`.
- A `CodeModule` loader that opens a recompiled shared library, resolves `rc_code_init`, validates the host ABI, registers its function table with `HostRuntime`, and unregisters those entries before unloading the library.
- Explicit error reporting for unresolved guest calls, SVCs, traps, bad returns, budget exhaustion, and invalid slow memory access.
- CI tests for empty module-path rejection, successful shared-library loading, metadata validation, function dispatch, and safe dispatch-table cleanup after unload.
- A shared-library fixture built with the production `tools/recomp/runtime/rc_dll.c` glue and a generated-table-shaped source, exercising the real `rc_code_init` ABI plus an indirect call that falls back from the module to the host runtime.
- CMake host tests intended to run in GitHub Actions.

## Not implemented yet

This is **not** a playable port. The tests now exercise the production shared-library glue against a small synthetic fixture, but the full generated MH4U game-code library has not yet been built or run. The 1 MiB memory size is a test default, not a proposed final 3DS address-space layout. SVC handling currently reports unsupported calls instead of emulating them. There is no 3DS kernel/service layer, thread scheduler, NCCH/romfs loader, PICA200 GPU, DSP audio, input integration, or Android APK target in this workflow.

## CI

The `Native Android runtime foundation` workflow currently builds and tests the portable host-runtime skeleton on Ubuntu. Passing those tests validates only these initial primitives; it does not prove Android compatibility or game compatibility.
