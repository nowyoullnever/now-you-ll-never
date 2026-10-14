#pragma once

#include "../AudioTypes.h"
#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

namespace tokkebi::audio::preview {
enum class TransportState : uint8_t { Unavailable, Stopped, Playing, Paused };

// Control-side ownership retains every published snapshot until destruction. The audio
// thread only reads an atomically published raw pointer and never changes ownership.
// This is deliberately conservative: reclamation is outside ProcessBlock().
class PreviewEngine final {
public:
  void SetOutputSampleRate(double rate) noexcept;
  void Publish(std::shared_ptr<const DecodedPcm> source);
  void Clear() noexcept;
  void Play() noexcept; void Pause() noexcept; void Stop() noexcept; void ReturnToStart() noexcept;
  void Seek(uint64_t frame) noexcept; void SetGain(float linear) noexcept; void SetMuted(bool muted) noexcept;
  TransportState State() const noexcept { return mState.load(std::memory_order_acquire); }
  uint64_t Position() const noexcept { return mPosition.load(std::memory_order_acquire); }
  uint64_t Frames() const noexcept;
  bool Muted() const noexcept { return mMuted.load(std::memory_order_acquire); }
  float Gain() const noexcept { return mGain.load(std::memory_order_acquire); }
  const DecodedPcm* SourceForRender() const noexcept { return mSource.load(std::memory_order_acquire); }
  double OutputRateForRender() const noexcept { return mOutputRate.load(std::memory_order_acquire); }
  void FinishForRender() noexcept { mState.store(TransportState::Stopped, std::memory_order_release); }
  double CursorForRender() const noexcept { return mRenderCursor.load(std::memory_order_acquire); }
  void CommitRenderCursor(double cursor, uint64_t frames) noexcept { mRenderCursor.store(cursor, std::memory_order_release); mPosition.store(std::min<uint64_t>(static_cast<uint64_t>(cursor), frames), std::memory_order_release); }
  // Adds preview to outputs. No allocation, locking, ownership transition, I/O or UI work.
  void Render(float** outputs, int outputChannels, int frames) noexcept;
  void Render(double** outputs, int outputChannels, int frames) noexcept;
private:
  std::atomic<const DecodedPcm*> mSource{};
  std::vector<std::shared_ptr<const DecodedPcm>> mRetained;
  std::atomic<TransportState> mState{TransportState::Unavailable};
  std::atomic<uint64_t> mPosition{};
  std::atomic<double> mRenderCursor{};
  std::atomic<double> mOutputRate{48000.0};
  std::atomic<float> mGain{1.f}; std::atomic<bool> mMuted{false};
};
}
