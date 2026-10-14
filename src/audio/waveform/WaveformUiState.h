#pragma once
#include "../AudioDocument.h"
#include "WaveformCoordinator.h"
namespace tokkebi::audio::waveform {
enum class UiState { Empty, AudioLoading, AudioFailed, WaveformBuilding, WaveformFailed, Ready };
constexpr UiState SelectUiState(AudioDocumentState audio, BuildState waveform) {
  if (audio == AudioDocumentState::Loading) return UiState::AudioLoading;
  if (audio == AudioDocumentState::Failed) return UiState::AudioFailed;
  if (audio == AudioDocumentState::Ready && waveform == BuildState::Building) return UiState::WaveformBuilding;
  if (audio == AudioDocumentState::Ready && waveform == BuildState::Failed) return UiState::WaveformFailed;
  return audio == AudioDocumentState::Ready ? UiState::Ready : UiState::Empty;
}
}
