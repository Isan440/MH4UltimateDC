# Android reference audit: klark231/MH4U-Android

Date: 2026-10-29

## Decision

Use `klark231/MH4U-Android` as the implementation reference for the long-term Android app, but do not copy its Android tree blindly or replace the current APK workflow until a compatibility build passes.

Reference:
- Repository: https://github.com/klark231/MH4U-Android
- Android guide: `port/android/README.md`
- First-build notes: `port/android/NOTES.md`
- Azahar core patch: `port/azahar/azahar-android.patch`

## What the reference actually implements

- A standalone Kotlin/Jetpack Compose Android application (`org.mh4u.port`), rather than Azahar's Android app/frontend.
- A native `libmh4u-android.so` JNI layer that owns the Android Surface, emulation thread, input, storage and keyboard.
- Reuse of the MH4U port's core backend and SDL-free PC-port modules (configuration, multiplayer, invite/lobby).
- A launcher/setup/settings/online UI and an in-game menu, plus touch controls and gamepad mapping.
- Vulkan-only Android rendering according to its README.

## Compatibility findings

1. The reference README explicitly says its Android implementation was written against AzaharPlus and had **not yet been compiled or run** when documented. Its NOTES list likely first-build issues around `System::Load`, performance-stat fields, `Frontend::EmuWindow` virtuals, initialization callbacks, Android storage translation and audio/resolution settings. Treat it as unverified source, not a working drop-in APK.
2. Our current workflow pins upstream Azahar commit `5f3306d13cab40bbd5cf38f2cf3ae8b6e6eedca6` and applies `port/azahar/azahar.patch` plus `mh4u-home.patch`. The reference patch targets an AzaharPlus tree. Some patch contexts share the same upstream file-index hashes, which is encouraging, but does not prove every changed API and build target is compatible.
3. Our current APK is still Azahar's Android app with an MH4U home activity. The reference is a larger architectural change: it builds a separate Compose app and a custom JNI library, and suppresses building Azahar's stock Android JNI app when `MH4U_PORT_DIR` is set.
4. The reference's `build-code-library.sh` requires an Android NDK compiler and a user's own game dump. Its README says this step is done on a PC. Our Android-only workflow must separately solve or document the ARM64 code-library build path; CI must not receive or package the ROM or copyrighted generated game code.
5. Therefore, merging the full reference now would risk breaking the known APK workflow and could leave us with an unbuildable app.

## Current integration strategy

- Keep `android-native-port` and the current APK workflow as a known fallback.
- Adapt the reference architecture in a separate, reviewable migration step: first bring in the Compose app/JNI source and required assets, then align CMake/Gradle and the Azahar patch.
- Keep the existing production backend files `port/src/arm_recomp.*`, `port/src/fiber_arm64.S`, and frontend integration unless a concrete compatibility issue requires a change.
- Preserve the ARM64-only target and use GitHub Actions as the primary build route. Do not require local Gradle builds in Termux.
- Build without game data; verify the APK packages the expected native libraries and does not bundle a ROM or generated game code.
- Do not call the migration complete until the app installs on the user's iQOO Z7 5G and the user verifies launch, game selection, boot into gameplay, controls, audio and settings.

## Next technical step

Prepare a separate integration branch or staged commits that add the reference's `port/android/` sources and assets, then update the workflow to build that app against our pinned Azahar source. Before changing the default launcher, get the first CI build green and retain the existing APK workflow as rollback. Resolve the reference's documented Azahar API drift from actual CI errors rather than guessing.
