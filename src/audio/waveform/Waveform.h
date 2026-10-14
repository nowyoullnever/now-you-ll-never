#pragma once
#include "../AudioTypes.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace tokkebi::audio::waveform {
struct Peak { float minimum{}, maximum{}; uint64_t firstFrame{}, endFrame{}; };
struct PeakLevel { uint64_t framesPerBucket{}; std::vector<std::vector<Peak>> channels; };
struct PeakCache { uint32_t channels{}; uint64_t frames{}; std::vector<PeakLevel> levels; };
std::shared_ptr<const PeakCache> BuildPeakCache(const DecodedPcm& pcm, uint64_t baseFrames = 64, std::string* error = nullptr);
std::size_t ChooseLevel(const PeakCache& cache, double framesPerPixel);
uint64_t MinimumViewportSpan(const PeakCache& cache);

struct Viewport { uint64_t totalFrames{}, start{}, end{}; bool Empty() const { return totalFrames == 0; } };
Viewport FullViewport(uint64_t frames);
Viewport Zoom(Viewport view, double factor, double pointerFraction, uint64_t minimumSpan = 16);
Viewport Scroll(Viewport view, int64_t frames);
double FrameToX(const Viewport& view, uint64_t frame, double width);
uint64_t XToFrame(const Viewport& view, double x, double width);
struct RenderColumn { double x{}; float minimum{}, maximum{}; uint32_t channel{}; uint64_t firstFrame{}, endFrame{}; };
struct RenderPlan { std::vector<RenderColumn> columns; uint64_t inspectedBuckets{}; };
RenderPlan BuildRenderPlan(const PeakCache& cache, const Viewport& view, uint32_t width);
struct RulerTick { uint64_t frame{}; double x{}; std::string label; };
std::vector<RulerTick> BuildTimeRuler(const Viewport& view, uint32_t sampleRate, uint32_t width, uint32_t minimumPixelSpacing = 72);
std::string FormatTime(uint64_t frame, uint32_t sampleRate);
}
