#pragma once
#include "Waveform.h"
#include <functional>
#include <memory>
#include <chrono>
namespace tokkebi::audio::waveform {
enum class BuildState { Empty, Building, Ready, Failed };
using BuildFunction = std::function<std::shared_ptr<const PeakCache>(const DecodedPcm&, std::string*)>;
class Coordinator { public:
  explicit Coordinator(BuildFunction build = {}); ~Coordinator();
  void Invalidate(uint64_t generation); void Begin(std::shared_ptr<const DecodedPcm> pcm, uint64_t generation);
  bool Apply(uint64_t generation); bool WaitForCompletion(uint64_t generation, std::chrono::milliseconds timeout) const;
  BuildState Status() const; std::shared_ptr<const PeakCache> Cache() const; std::string Diagnostic() const;
private: struct SharedState; std::shared_ptr<SharedState> m; };
}
