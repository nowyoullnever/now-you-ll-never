# Build (P02.9.1)

## Prerequisites

- Git with submodule support
- CMake 3.21 or later
- Windows: Visual Studio 2022 with Desktop development with C++
- macOS: Xcode Command Line Tools, CMake and Ninja
- VST3 SDK `v3.7.13_build_42` at pinned commit `8b59557d881bb0158ba08ff256b26f025f078314`

Initialize the tracked framework and then the intentionally untracked SDK checkout:

```powershell
git submodule update --init --recursive
pwsh -File scripts/setup-vst3-sdk.ps1
```

The setup script places the exact SDK revision at `third_party/iPlug2/Dependencies/IPlug/VST3_SDK`, the fixed location required by the pinned iPlug2 CMake module. It refuses a conflicting existing checkout rather than silently changing it. This directory is ignored and must not be committed.

## Windows x64

```powershell
cmake -S . -B build/windows-x64 -G "Visual Studio 17 2022" -A x64 -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build/windows-x64 --config Debug --target tokkebi-app tokkebi-vst3 tokkebi-audio-tests tokkebi-waveform-tests tokkebi-theme-tests tokkebi-ui-shell-tests tokkebi-ui-component-tests
cmake --build build/windows-x64 --config Release --target tokkebi-app tokkebi-vst3 tokkebi-audio-tests tokkebi-waveform-tests tokkebi-theme-tests tokkebi-ui-shell-tests tokkebi-ui-component-tests
ctest --test-dir build/windows-x64 -C Debug --output-on-failure
ctest --test-dir build/windows-x64 -C Release --output-on-failure
cmake --build build/windows-x64 --config Release --target tokkebi-verify-vst3-bundle
```

Outputs remain configuration-separated:

- `build/windows-x64/out/Debug/tokkebi.exe`
- `build/windows-x64/out/Release/tokkebi.exe`
- `build/windows-x64/out/Debug/tokkebi.vst3/Contents/x86_64-win/tokkebi.vst3`
- `build/windows-x64/out/Release/tokkebi.vst3/Contents/x86_64-win/tokkebi.vst3`

On Windows, `resources/main.rc` embeds the three font binaries in both the
Standalone executable and VST3 module. This is required because IGraphicsWin
loads the exact resource IDs (`TOKKEBI_PRIMARY_FONT`, `TOKKEBI_BODY_FONT`, and
`TOKKEBI_TECHNICAL_FONT`) rather than looking up installed fonts.
`tokkebi.FontAssets` validates the source asset hashes and license paths.

## macOS

Run Debug and Release in distinct build trees to keep bundle outputs separate:

```bash
git submodule update --init --recursive
pwsh -File scripts/setup-vst3-sdk.ps1
cmake -S . -B build/macos-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build/macos-debug --target tokkebi-app tokkebi-vst3 tokkebi-au tokkebi-audio-tests tokkebi-waveform-tests tokkebi-theme-tests tokkebi-ui-shell-tests tokkebi-ui-component-tests
ctest --test-dir build/macos-debug --output-on-failure
cmake -S . -B build/macos-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build/macos-release --target tokkebi-app tokkebi-vst3 tokkebi-au tokkebi-audio-tests tokkebi-waveform-tests tokkebi-theme-tests tokkebi-ui-shell-tests tokkebi-ui-component-tests
ctest --test-dir build/macos-release --output-on-failure
```

The macOS runner builds its native architecture only. Universal binaries require an explicit later run with `-DIPLUG2_UNIVERSAL=ON` or `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`; P00.3 does not claim a universal binary from a single-architecture build.

## Failure diagnostics

- `tokkebi requires the pinned VST3 SDK`: run `scripts/setup-vst3-sdk.ps1`; a Standalone-only configure is deliberately not treated as VST3 support.
- `iPlug2 is missing`: run `git submodule update --init --recursive`.
- A fresh Windows configure fetches WIL and WebView2 through pinned iPlug2 CMake. See [DEPENDENCIES.md](DEPENDENCIES.md); do not commit generated `_deps/` directories.
- No installer, signing or notarization is performed in P01.5.
