# Android Port Adoption

## Source of truth

The Android application implementation is being adopted from
[`klark231/MH4U-Android`](https://github.com/klark231/MH4U-Android), branch `main`.

Do not replace this architecture with the earlier Azahar Android-app patch approach.
The target is the standalone Kotlin/Jetpack Compose application in `port/android/`,
with Azahar used as the emulation core.

## Target architecture

- Kotlin + Jetpack Compose launcher and in-game UI.
- JNI bridge in `port/android/jni/`.
- Azahar core built for Android ARM64 with Vulkan.
- Existing MH4U hooks and recompiled-code backend reused from `port/`.
- User-owned decrypted game data stays on-device and is never uploaded to CI.

## Work sequence

1. Import and reconcile the upstream Android source tree and Azahar Android patch.
2. Pin the AzaharPlus revision and make the GitHub Actions APK build reproducible.
3. Resolve the first-build/API-drift items in `port/android/NOTES.md`.
4. Validate Setup import, code-library loading, input, Vulkan rendering, and gameplay.
5. Replace the PC-only code-library build step with an in-app Android ARM64 builder.
6. Package or provision a legally distributable Android-host Python/compiler toolchain;
   never assume the Linux-host NDK compiler can execute on Android.
7. Add a bounded, cancellable preparation service with progress, validation, and
   resumable cache. Default to one worker for mid-range devices.

## Build policy

GitHub Actions builds and tests the APK only. Game dumps, extracted game code,
generated C sources, and the resulting `.so` are processed locally on the phone.

## First technical blockers

- The upstream Android README says the Android app has not yet had its first build.
- The current `build-code-library.sh` is a PC workflow and must not be presented as
  an on-device builder.
- The Android app's AzaharPlus dependency and patch must be pinned and checked for drift.
- Android-host Clang/LLD binaries and their transitive runtime dependencies need a
  reproducible source, ABI validation, and license review before packaging.
