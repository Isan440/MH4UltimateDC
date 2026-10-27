# Android UI integration audit

Branch: `android-native-port`

## Findings from the current source

### 1. The current Android APK is still Azahar's Android frontend

`.github/workflows/android.yml` builds `azahar/src/android` with Gradle task
`assembleVanillaRelease`. The patched Android JNI target links `mh4u_backend` when that target
exists, but this does not build or launch `port/app/mh4u`.

**Implication:** the green Android workflow currently verifies the Azahar-based APK and recompilation
backend integration, not the planned standalone MH4U Android UI.

### 2. The port's own app is a desktop SDL executable

`port/CMakeLists.txt` adds `port/app` only when both `MH4U_APP` and `ENABLE_SDL2` are enabled.
`port/app/CMakeLists.txt` declares `mh4u` with `add_executable`, linking SDL2, Dear ImGui,
Azahar core libraries and the selected renderer.

`port/app/main.cpp` owns a conventional C++ `main` entry point. The launcher also expects desktop
process/tool flows for setup and updates (`port/app/launcher/process.cpp`,
`port/app/launcher/tools.cpp`, and `port/app/launcher/updater.cpp`).

**Implication:** enabling this target in Android CMake alone is not enough. Android needs an app
entry/lifecycle bridge and Android-appropriate storage, file selection, and launch flows.

### 3. Window and renderer code has desktop assumptions

`port/app/window.cpp` initializes SDL video/events and controller input. The Vulkan implementation
in `port/app/window_vk.cpp` obtains a desktop-native handle through `SDL_GetWindowWMInfo` and only
handles Windows, X11, Wayland and macOS window systems. That path does not provide an Android
`ANativeWindow` surface.

The OpenGL renderer may be a more practical first graphics path on Android, but it still needs to be
validated against Android's SDL lifecycle, GLES context creation, surface recreation, pause/resume,
rotation and touch input.

**Implication:** do not select the current Vulkan window path for the first Android shell. Establish
the Android activity/window lifecycle and prove an SDL/GLES surface first.

### 4. Existing Android JNI backend integration is reusable

The pinned Azahar patch adds `MH4U_PORT_DIR` support, links `mh4u_backend` into the Android JNI
library, restricts the APK to `arm64-v8a`, and increases the emulation thread stack for the
recompiled-code fiber path. The Android backend also includes `port/src/fiber_arm64.S`.

**Implication:** retain the known Android backend integration as a compatibility baseline while
building the app UI separately; avoid rewriting the production `port/src/arm_recomp.*` backend.

## Implementation sequence

1. Keep the existing Azahar APK workflow intact and verify its new branch trigger.
2. Add an Android-native app shell (Activity, lifecycle, storage paths and touch UI) as a separate
   target/module. The first shell must launch and survive pause/resume without loading a game.
3. Establish an Android surface and a minimal render loop before moving the desktop launcher.
4. Integrate the MH4U core/backend only after shell lifecycle and rendering are stable.
5. Add user-selected game/code-library paths and validate title ID/CRC before boot.
6. Test on a real ARM64 device; a successful CI build alone is not proof of playable gameplay.

## Explicit non-goals for the first shell

- Do not remove or replace the existing Android Azahar frontend.
- Do not bundle copyrighted game code, ROMs, update dumps or assets in CI artifacts.
- Do not claim native Android gameplay until a user-provided game dump boots and can be controlled
  on-device.
