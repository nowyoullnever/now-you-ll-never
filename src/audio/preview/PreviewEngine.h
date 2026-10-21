#pragma once

#include "../AudioTypes.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>

namespace tokkebi::audio::preview {
enum class TransportState : uint8_t { Unavailable, Stopped, Playing, Paused };
enum class CommandFailure : uint8_t { None, QueueFull, InvalidSource, OwnershipUnavailable };

struct TransportSnapshot {
  uint64_t generation{};
  TransportState state{TransportState::Unavailable};
  uint64_t position{};
  uint64_t frames{};
  float gain{1.f};
  bool muted{};
  bool sourceAvailable{};
};

// A single UI/control producer publishes fixed-size commands. ProcessBlock() is
// the sole consumer and owner of the raw PCM pointer and transport cursor.
class PreviewEngine final {
public:
  static constexpr uint32_t kCommandCapacity = 64;

  bool Publish(std::shared_ptr<const DecodedPcm> source);
  bool Clear() noexcept;
  bool Play() noexcept;
  bool Pause() noexcept;
  bool Stop() noexcept;
  bool ReturnToStart() noexcept;
  bool Seek(uint64_t frame) noexcept;
  bool SetGain(float linear) noexcept;
  bool SetMuted(bool muted) noexcept;

  // Call only from the control/UI thread. It releases acknowledged old PCM;
  // ProcessBlock never owns or destroys a shared_ptr.
  void Maintain() noexcept;
  CommandFailure LastCommandFailure() const noexcept { return mLastFailure.load(std::memory_order_acquire); }
  TransportSnapshot Snapshot() const noexcept;

  // Adds preview to existing outputs. The supplied rate is sampled by the
  // audio thread at a block boundary, so rate changes preserve source cursor.
  void Render(float** outputs, int outputChannels, int frames, double outputRate) noexcept;
  void Render(double** outputs, int outputChannels, int frames, double outputRate) noexcept;

private:
  template <typename T> friend void RenderImpl(PreviewEngine&, T**, int, int, double) noexcept;
  enum class CommandType : uint8_t { InstallSource, ClearSource, Play, Pause, Stop, ReturnToStart, Seek, SetGain, SetMute };
  struct Command { CommandType type{}; const DecodedPcm* source{}; uint64_t generation{}; uint64_t frame{}; float gain{}; bool muted{}; };
  struct SourceOwner { uint64_t generation{}; std::shared_ptr<const DecodedPcm> source; };
  struct SnapshotStorage {
    std::atomic<uint64_t> sequence{0};
    std::atomic<uint64_t> generation{0}, position{0}, frames{0};
    std::atomic<uint8_t> state{static_cast<uint8_t>(TransportState::Unavailable)};
    std::atomic<float> gain{1.f};
    std::atomic<bool> muted{false}, available{false};
  };

  bool Push(Command command) noexcept;
  bool Pop(Command& command) noexcept;
  void Apply(const Command& command) noexcept;
  void PublishSnapshot() noexcept;
  static bool Valid(const DecodedPcm& source) noexcept;
  static float SanitizeGain(float gain) noexcept;

  std::array<Command, kCommandCapacity> mCommands{};
  std::atomic<uint32_t> mWrite{0};
  std::atomic<uint32_t> mRead{0};
  std::deque<SourceOwner> mSources; // control thread only
  uint64_t mNextGeneration = 0;     // control thread only
  std::atomic<uint64_t> mAcknowledgedGeneration{0};
  std::atomic<CommandFailure> mLastFailure{CommandFailure::None};

  // Audio-thread canonical state. Never read directly from the control thread.
  const DecodedPcm* mActiveSource = nullptr;
  uint64_t mActiveGeneration = 0;
  TransportState mState = TransportState::Unavailable;
  double mCursor = 0.0;
  float mTargetGain = 1.f;
  float mRenderGain = 1.f;
  bool mMuted = false;
  SnapshotStorage mSnapshot;
};
}
