# Native runtime foundation (no Azahar)

This directory is the first isolated host-runtime layer for the existing static recompiler ABI in `tools/recomp/runtime/recomp.h`.

## Included in this milestone

- A bounded byte-addressed guest-memory buffer with little-endian 8-, 16-, and 32-bit reads/writes.
- A minimal guest function dispatch table for unit testing.
- An `RHost` callback table compatible with `RC_HOST_VERSION`.
- Explicit error reporting for unresolved guest calls, SVCs, traps, bad returns, budget exhaustion, and invalid slow memory access.
- CMake host tests intended to run in GitHub Actions.

## Not implemented yet

This is **not** a playable port and does not yet load or run the full generated game-code shared library. The 1 MiB memory size is a test default, not a proposed final 3DS address-space layout. SVC handling currently reports unsupported calls instead of emulating them. There is no 3DS kernel/service layer, thread scheduler, NCCH/romfs loader, PICA200 GPU, DSP audio, input integration, or Android APK target in this workflow.

## CI

The `Native Android runtime foundation` workflow currently builds and tests the portable host-runtime skeleton on Ubuntu. Passing those tests validates only these initial primitives; it does not prove Android compatibility or game compatibility.
