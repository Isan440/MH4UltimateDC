# Android native-app port plan

## Decision

Build an Android app around the existing MH4U PC-port architecture. The recompiled MH4U game code is the CPU execution path; Azahar supplies the 3DS platform pieces that remain necessary (kernel/services, PICA200 graphics and DSP audio). The Android app must present MH4U as its own app, not expose Azahar's emulator-oriented frontend as the product UI.

This is a hybrid native port, not a from-scratch 3DS emulator and not a complete replacement for Azahar's platform layer.

## Existing pieces verified in this repository

- `tools/recompile.py` generates C from the user's own MH4U v1.3.0 update dump. Generated game code stays out of Git.
- `port/src/arm_recomp.*` implements Azahar's `Core::ARM_Interface` backend for the recompiled code, with guest-thread fibers, memory page-table integration, SVC forwarding, cycle accounting and interpreter fallback for missing recompiled functions.
- `port/src/fiber_arm64.S` provides an AArch64 fiber switch for Android.
- `port/src/frontend.cpp` integrates the recompiled code library with the installed MH4U title and copies a user-provided library into the Android app's private directory when needed.
- `port/app/` already contains the PC port's own SDL2/ImGui UI, launcher, settings, input, overlays and multiplayer UI.
- `.github/workflows/android.yml` currently builds an Android APK based on Azahar's Android frontend with the recomp backend patched in. It does not yet build `port/app/` as the product's own Android UI.
- `native/runtime/` is a small standalone host-runtime experiment. It is not the chosen long-term architecture for this hybrid port and must not replace the production `port/src/arm_recomp.*` backend.

## Milestones

1. **Android UI feasibility audit**: inspect SDL2 Android lifecycle, existing `port/app/` assumptions, Azahar Android Gradle/CMake integration, renderer and input wiring. Keep the current Android workflow intact as a reference build.
2. **Minimal custom Android shell**: package the app's own launcher/window on Android and prove lifecycle, touch input, pause/resume and rotation without game code.
3. **Android renderer and input integration**: connect the custom UI to the existing Azahar video/audio/core components and map touchscreen/gamepad input.
4. **Recompiled-code loading**: connect the user-built ARM64 code library and the existing backend; validate title ID/code CRC and report actionable errors.
5. **Game boot and compatibility**: test boot, title, save/load, village and quest using the user's own legally obtained files.
6. **Performance and release**: profile on Android ARM64, optimize frame pacing/memory, and publish reproducible GitHub Actions artifacts.

## Constraints and acceptance criteria

- GitHub Actions is the primary build route; no local Gradle build in Termux is required.
- CI must never package copyrighted game code or assets. The user builds the code library from their own dump.
- Keep the current APK workflow working while the custom-app path is developed.
- A green CI build proves only what the workflow tests. Do not label the port playable until an actual device test reaches gameplay.
- First deliverable is the custom Android shell, not a speculative full gameplay rewrite.
