#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace tokkebi::audio::editor {
struct Selection { uint64_t inFrame{}, outFrame{}; bool operator==(const Selection&) const = default; };
struct TimeValue { uint64_t milliseconds{}; };
struct SelectionIntersection { uint64_t start{}, end{}; bool Empty() const { return start >= end; } };
std::optional<TimeValue> ParseTime(const std::string& text);
std::string FormatTime(uint64_t frames, uint32_t sampleRate);
std::optional<uint64_t> MillisecondsToFrame(uint64_t milliseconds, uint32_t sampleRate);
uint64_t FrameToMilliseconds(uint64_t frame, uint32_t sampleRate);
SelectionIntersection Intersect(Selection selection, uint64_t viewStart, uint64_t viewEnd);
class SelectionState {
public:
  void SetSource(uint64_t totalFrames, uint32_t sampleRate);
  uint64_t TotalFrames() const { return mTotalFrames; } uint32_t SampleRate() const { return mSampleRate; }
  const std::optional<Selection>& Current() const { return mCurrent; }
  bool Select(uint64_t first, uint64_t second); bool SelectAll(); void Clear();
  bool MoveIn(int64_t delta); bool MoveOut(int64_t delta);
  bool SetInText(const std::string& text); bool SetOutText(const std::string& text);
  bool Undo(); bool Redo();
private:
  void Commit(std::optional<Selection> next); void ResetHistory();
  uint64_t mTotalFrames{}; uint32_t mSampleRate{}; std::optional<Selection> mCurrent;
  std::vector<std::optional<Selection>> mUndo, mRedo; static constexpr size_t kHistoryCap=100;
};
}
