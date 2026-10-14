cmake_minimum_required(VERSION 3.21)

project(tokkebi VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)

set(IPLUG2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/iPlug2" CACHE PATH "Path to the pinned iPlug2 checkout")
if(NOT EXISTS "${IPLUG2_DIR}/iPlug2.cmake")
  message(FATAL_ERROR "iPlug2 is missing. Run: git submodule update --init --recursive")
endif()

set(IPLUG2_CXX_STANDARD 20 CACHE STRING "C++ standard used by iPlug2 and tokkebi" FORCE)
include("${IPLUG2_DIR}/iPlug2.cmake")
find_package(iPlug2 REQUIRED)
find_package(Threads REQUIRED)

add_library(tokkebi-audio-core
  src/audio/AudioTypes.h
  src/audio/AudioDecoder.h
  src/audio/AudioDecoder.cpp
  src/audio/AudioDocument.h
  src/audio/AudioDocument.cpp
  src/audio/AudioLoadCoordinator.h
  src/audio/AudioLoadCoordinator.cpp
  src/audio/waveform/Waveform.h
  src/audio/waveform/Waveform.cpp
  src/audio/waveform/WaveformCoordinator.h
  src/audio/waveform/WaveformCoordinator.cpp
)
target_include_directories(tokkebi-audio-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(tokkebi-audio-core PUBLIC Threads::Threads)
target_compile_features(tokkebi-audio-core PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(tokkebi-audio-core PRIVATE /W4 /permissive- /utf-8)
else()
  target_compile_options(tokkebi-audio-core PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(TOKKEBI_VST3_SDK_DIR "${IPLUG2_DIR}/Dependencies/IPlug/VST3_SDK")
if(NOT EXISTS "${TOKKEBI_VST3_SDK_DIR}/pluginterfaces/vst/ivstaudioprocessor.h")
  message(FATAL_ERROR
    "tokkebi requires the pinned VST3 SDK at ${TOKKEBI_VST3_SDK_DIR}. "
    "Run: pwsh -File scripts/setup-vst3-sdk.ps1")
endif()

set(TOKKEBI_FORMATS APP VST3)
if(APPLE)
  list(APPEND TOKKEBI_FORMATS AU)
endif()

iplug_add_plugin(tokkebi
  SOURCES
    src/app/tokkebi.cpp
    src/app/tokkebi.h
    src/app/config.h
    src/app/fonts.h
    src/ui/AppShell.cpp
    src/ui/AppShell.h
    src/ui/components/Components.h
    src/ui/layout/ShellLayout.h
    src/ui/navigation/NavigationState.h
    src/ui/navigation/LocalAudioFocusRouter.h
    src/ui/LocalAudioDialogMailbox.h
    src/ui/navigation/TabId.h
    src/ui/text/Strings.h
    src/ui/theme/Theme.cpp
    src/ui/theme/Theme.h
    src/app/resource.h
  RESOURCES
    assets/fonts/dunggeunmo/dunggeunmo.ttf
    assets/fonts/ridibatang/ridibatang.otf
    assets/fonts/galmuri/galmuri11-bitmap-regular-2.40.4.ttf
  FORMATS ${TOKKEBI_FORMATS}
)

foreach(tokkebi_target IN ITEMS tokkebi-app tokkebi-vst3 tokkebi-au)
  if(TARGET ${tokkebi_target})
    target_link_libraries(${tokkebi_target} PRIVATE tokkebi-audio-core)
    target_include_directories(${tokkebi_target} PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
      "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/dunggeunmo"
      "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/ridibatang"
      "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/galmuri"
    )
  endif()
endforeach()

# iPlug2's AUv2 CMake target omits this wrapper support source. Keep the
# workaround local to tokkebi rather than modifying the pinned submodule.
if(APPLE AND TARGET tokkebi-au)
  target_sources(tokkebi-au PRIVATE
    "${IPLUG2_DIR}/IPlug/AUv2/dfx-au-utilities.c"
  )
endif()

if(WIN32 AND TARGET tokkebi-app)
  set_target_properties(tokkebi-app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/out/RelWithDebInfo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/out/MinSizeRel"
  )
endif()

if(WIN32 AND TARGET tokkebi-vst3)
  # iPlug2 adds main.rc automatically only for APP. Add the same binary font
  # resources to the Windows VST3 module so no installed-font lookup is needed.
  target_sources(tokkebi-vst3 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/main.rc")
  set_target_properties(tokkebi-vst3 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/tokkebi.vst3/Contents/x86_64-win"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/tokkebi.vst3/Contents/x86_64-win"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/out/RelWithDebInfo/tokkebi.vst3/Contents/x86_64-win"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/out/MinSizeRel/tokkebi.vst3/Contents/x86_64-win"
  )
  add_custom_command(TARGET tokkebi-vst3 POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:tokkebi-vst3>/../Resources"
    COMMENT "Creating configuration-specific VST3 Resources directory"
  )
endif()

foreach(tokkebi_target IN ITEMS tokkebi-app tokkebi-vst3 tokkebi-au)
  if(TARGET ${tokkebi_target})
    if(MSVC)
      target_compile_options(${tokkebi_target} PRIVATE /W4 /permissive- /utf-8)
    else()
      target_compile_options(${tokkebi_target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
  endif()
endforeach()

if(TARGET tokkebi-vst3)
  add_custom_target(tokkebi-verify-vst3-bundle
    COMMAND "${CMAKE_COMMAND}"
      "-DBUNDLE_ROOT=${CMAKE_BINARY_DIR}/out/$<CONFIG>"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyWindowsVST3Bundle.cmake"
    DEPENDS tokkebi-vst3
    VERBATIM
  )
endif()

if(BUILD_TESTING)
  add_executable(tokkebi-audio-tests tests/AudioTests.cpp)
  target_link_libraries(tokkebi-audio-tests PRIVATE tokkebi-audio-core)
  target_compile_features(tokkebi-audio-tests PRIVATE cxx_std_20)
  if(MSVC)
    target_compile_options(tokkebi-audio-tests PRIVATE /utf-8)
  endif()
  add_test(NAME tokkebi.AudioDecoder COMMAND tokkebi-audio-tests)
  add_executable(tokkebi-waveform-tests tests/WaveformTests.cpp)
  target_link_libraries(tokkebi-waveform-tests PRIVATE tokkebi-audio-core)
  target_compile_features(tokkebi-waveform-tests PRIVATE cxx_std_20)
  add_test(NAME tokkebi.Waveform COMMAND tokkebi-waveform-tests)

  add_executable(tokkebi-theme-tests tests/ThemeTests.cpp)
  target_include_directories(tokkebi-theme-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_compile_features(tokkebi-theme-tests PRIVATE cxx_std_20)
  add_test(NAME tokkebi.ThemeTokens COMMAND tokkebi-theme-tests)
  add_test(NAME tokkebi.ThemeTokensNegative COMMAND tokkebi-theme-tests --negative)
  set_tests_properties(tokkebi.ThemeTokensNegative PROPERTIES WILL_FAIL TRUE)

  add_executable(tokkebi-ui-shell-tests tests/UiShellTests.cpp)
  target_include_directories(tokkebi-ui-shell-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_compile_features(tokkebi-ui-shell-tests PRIVATE cxx_std_20)
  if(MSVC)
    target_compile_options(tokkebi-ui-shell-tests PRIVATE /utf-8)
  endif()
  add_test(NAME tokkebi.UiShell COMMAND tokkebi-ui-shell-tests)

  add_executable(tokkebi-ui-component-tests tests/UiComponentTests.cpp)
  target_include_directories(tokkebi-ui-component-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${IPLUG2_DIR}/IGraphics" "${IPLUG2_DIR}/IGraphics/Controls" "${IPLUG2_DIR}/IPlug" "${IPLUG2_DIR}/WDL")
  target_link_libraries(tokkebi-ui-component-tests PRIVATE iPlug2::IGraphics::NanoVG)
  target_compile_features(tokkebi-ui-component-tests PRIVATE cxx_std_20)
  if(MSVC)
    target_compile_options(tokkebi-ui-component-tests PRIVATE /utf-8)
  endif()
  add_test(NAME tokkebi.UiComponents COMMAND tokkebi-ui-component-tests)
endif()

if(BUILD_TESTING)
  add_test(
    NAME tokkebi.IdentityAndVersionConsistency
    COMMAND "${CMAKE_COMMAND}"
      "-DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyVersion.cmake"
  )
  add_test(
    NAME tokkebi.IdentityValidatorSelfTest
    COMMAND "${CMAKE_COMMAND}"
      "-DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DVALIDATOR_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyVersion.cmake"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyVersionSelfTest.cmake"
  )
  add_test(
    NAME tokkebi.FontAssets
    COMMAND "${CMAKE_COMMAND}"
      "-DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyFontAssets.cmake"
  )
  add_test(
    NAME tokkebi.FontAssetsSelfTest
    COMMAND "${CMAKE_COMMAND}"
      "-DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DVALIDATOR_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyFontAssets.cmake"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyFontAssetsSelfTest.cmake"
  )
endif()
