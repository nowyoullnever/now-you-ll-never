#include "audio/preview/PreviewEngine.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using namespace tokkebi::audio;
using namespace tokkebi::audio::preview;

namespace
{
int failures{};
#define CHECK(x) do { if (!(x)) { ++failures; std::cerr << "FAIL: " #x "\n"; } } while (false)
constexpr float kTolerance = .0001f;

std::shared_ptr<const DecodedPcm> Pcm(uint32_t channels = 1, uint32_t rate = 4, std::vector<float> samples = {0.f, 1.f, .5f, -1.f})
{
  auto pcm = std::make_shared<DecodedPcm>();
  pcm->channels = channels;
  pcm->sampleRate = rate;
  pcm->frames = samples.size() / channels;
  pcm->interleaved = std::move(samples);
  return pcm;
}

void RenderRaw(PreviewEngine& engine, std::vector<std::vector<float>>& data, double rate = 4.)
{
  std::vector<float*> output;
  for (auto& channel : data) output.push_back(channel.data());
  engine.Render(output.data(), static_cast<int>(output.size()), static_cast<int>(data.empty() ? 0 : data[0].size()), rate);
}

TransportSnapshot Render(PreviewEngine& engine, std::vector<std::vector<float>>& data, double rate = 4.)
{
  RenderRaw(engine, data, rate);
  return engine.Snapshot();
}

void Zero(std::vector<std::vector<float>>& data)
{
  for (auto& channel : data)
    for (auto& sample : channel)
      sample = 0.f;
}

bool Silent(const std::vector<std::vector<float>>& data)
{
  for (const auto& channel : data)
    for (const float sample : channel)
      if (sample != 0.f) return false;
  return true;
}

std::vector<float> RunResample(uint32_t sourceRate, double outputRate, const std::vector<int>& blocks)
{
  std::vector<float> source(128);
  for (std::size_t index = 0; index < source.size(); ++index)
    source[index] = static_cast<float>(index % 17) / 17.f - .4f;

  PreviewEngine engine;
  auto pcm = Pcm(1, sourceRate, source);
  std::vector<std::vector<float>> bootstrap(1, std::vector<float>(1));
  CHECK(engine.Publish(pcm));
  Render(engine, bootstrap, outputRate);
  CHECK(engine.Play());

  std::vector<float> result;
  for (const int frames : blocks)
  {
    std::vector<std::vector<float>> block(1, std::vector<float>(static_cast<std::size_t>(frames)));
    Render(engine, block, outputRate);
    result.insert(result.end(), block[0].begin(), block[0].end());
  }
  return result;
}

void CheckEquivalent(const std::vector<float>& expected, const std::vector<float>& actual)
{
  CHECK(expected.size() == actual.size());
  for (std::size_t index = 0; index < std::min(expected.size(), actual.size()); ++index)
    CHECK(std::fabs(expected[index] - actual[index]) <= kTolerance);
}

void CheckResampleMatrix(uint32_t sourceRate, double outputRate)
{
  const std::vector<int> large {19};
  const std::vector<int> oneFrame(19, 1);
  const std::vector<int> irregular {1, 4, 2, 5, 3, 4};
  const auto expected = RunResample(sourceRate, outputRate, large);
  CheckEquivalent(expected, RunResample(sourceRate, outputRate, oneFrame));
  CheckEquivalent(expected, RunResample(sourceRate, outputRate, irregular));
}
}

int main()
{
  std::vector<std::vector<float>> mono(1, std::vector<float>(1));
  PreviewEngine transport;
  CHECK(transport.Snapshot().state == TransportState::Unavailable);
  CHECK(transport.Play());
  Render(transport, mono);
  CHECK(transport.Snapshot().state == TransportState::Unavailable);

  auto a = Pcm();
  std::weak_ptr<const DecodedPcm> weakA = a;
  CHECK(transport.Publish(a));
  a.reset();
  Render(transport, mono);
  auto snapshot = transport.Snapshot();
  CHECK(snapshot.state == TransportState::Stopped && snapshot.position == 0 && snapshot.frames == 4 && snapshot.sourceAvailable);
  CHECK(transport.Play());
  Zero(mono);
  snapshot = Render(transport, mono);
  CHECK(snapshot.state == TransportState::Playing && snapshot.position == 1);
  CHECK(transport.Pause());
  Render(transport, mono);
  const auto paused = transport.Snapshot().position;
  Render(transport, mono);
  CHECK(transport.Snapshot().state == TransportState::Paused && transport.Snapshot().position == paused);
  CHECK(transport.ReturnToStart());
  Render(transport, mono);
  CHECK(transport.Snapshot().state == TransportState::Paused && transport.Snapshot().position == 0);
  CHECK(transport.Stop());
  Render(transport, mono);
  CHECK(transport.Snapshot().state == TransportState::Stopped && transport.Snapshot().position == 0);

  // Frozen transport-state and same-block command-order matrix.
  PreviewEngine controls;
  CHECK(controls.Publish(Pcm(1, 4, {.1f, .2f, .3f, .4f})));
  Render(controls, mono);
  CHECK(controls.Play());
  Zero(mono);
  Render(controls, mono);
  CHECK(std::fabs(mono[0][0] - .1f) < kTolerance && controls.Snapshot().state == TransportState::Playing);
  CHECK(controls.ReturnToStart());
  std::vector<std::vector<float>> empty(1);
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Playing && snapshot.position == 0);
  Zero(mono);
  Render(controls, mono);
  CHECK(std::fabs(mono[0][0] - .1f) < kTolerance);
  CHECK(controls.Stop());
  Render(controls, empty);
  CHECK(controls.ReturnToStart());
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Stopped && snapshot.position == 0);
  CHECK(controls.Play());
  Render(controls, mono);
  CHECK(controls.Seek(2));
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Playing && snapshot.position == 2);
  Zero(mono);
  Render(controls, mono);
  CHECK(std::fabs(mono[0][0] - .3f) < kTolerance);
  CHECK(controls.Pause());
  Render(controls, empty);
  CHECK(controls.Seek(2));
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Paused && snapshot.position == 2);
  CHECK(controls.Stop());
  Render(controls, empty);
  CHECK(controls.Seek(2));
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Stopped && snapshot.position == 2);
  CHECK(controls.Seek(2));
  CHECK(controls.Stop());
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Stopped && snapshot.position == 0);
  CHECK(controls.Stop());
  CHECK(controls.Play());
  Zero(mono);
  snapshot = Render(controls, mono);
  CHECK(snapshot.state == TransportState::Playing && std::fabs(mono[0][0] - .1f) < kTolerance);
  CHECK(controls.Pause());
  CHECK(controls.Seek(2));
  snapshot = Render(controls, empty);
  CHECK(snapshot.state == TransportState::Paused && snapshot.position == 2);

  PreviewEngine eofRestart;
  CHECK(eofRestart.Publish(Pcm(1, 4, {.6f, -.4f})));
  Render(eofRestart, mono);
  CHECK(eofRestart.Play());
  std::vector<std::vector<float>> eofBlock(1, std::vector<float>(3));
  snapshot = Render(eofRestart, eofBlock);
  CHECK(snapshot.state == TransportState::Stopped && snapshot.position == 2);
  CHECK(eofRestart.Play());
  Zero(mono);
  snapshot = Render(eofRestart, mono);
  CHECK(snapshot.state == TransportState::Playing && std::fabs(mono[0][0] - .6f) < kTolerance);

  auto b = Pcm(1, 4, {.25f, .25f, .25f, .25f});
  std::weak_ptr<const DecodedPcm> weakB = b;
  CHECK(transport.Publish(b));
  b.reset();
  transport.Maintain();
  CHECK(!weakA.expired());
  Render(transport, mono);
  transport.Maintain();
  CHECK(weakA.expired() && !weakB.expired());
  CHECK(transport.Clear());
  Render(transport, mono);
  transport.Maintain();
  CHECK(weakB.expired());
  for (int index = 0; index < 8; ++index)
  {
    auto transient = Pcm();
    std::weak_ptr<const DecodedPcm> weak = transient;
    CHECK(transport.Publish(transient));
    transient.reset();
    Render(transport, mono);
    transport.Maintain();
    CHECK(!weak.expired());
    CHECK(transport.Clear());
    Render(transport, mono);
    transport.Maintain();
    CHECK(weak.expired());
  }

  // A cannot reappear after B adoption or a later clear boundary.
  PreviewEngine generations;
  auto sourceA = Pcm(1, 4, {.25f, .25f, .25f, .25f});
  auto sourceB = Pcm(1, 4, {.75f, .75f, .75f, .75f});
  CHECK(generations.Publish(sourceA));
  Render(generations, mono);
  CHECK(generations.Play());
  Zero(mono);
  Render(generations, mono);
  CHECK(std::fabs(mono[0][0] - .25f) < kTolerance && generations.Snapshot().position == 1);
  CHECK(generations.Publish(sourceB));
  Zero(mono);
  snapshot = Render(generations, mono);
  CHECK(Silent(mono) && snapshot.generation == 2 && snapshot.state == TransportState::Stopped && snapshot.position == 0);
  Zero(mono);
  snapshot = Render(generations, mono);
  CHECK(Silent(mono) && snapshot.generation == 2 && snapshot.state == TransportState::Stopped);
  CHECK(generations.Play());
  Zero(mono);
  Render(generations, mono);
  CHECK(std::fabs(mono[0][0] - .75f) < kTolerance);
  CHECK(generations.Clear());
  Zero(mono);
  snapshot = Render(generations, mono);
  CHECK(Silent(mono) && snapshot.state == TransportState::Unavailable && !snapshot.sourceAvailable);
  Zero(mono);
  snapshot = Render(generations, mono);
  CHECK(Silent(mono) && snapshot.state == TransportState::Unavailable && snapshot.position == 0);

  for (uint32_t index = 0; index < PreviewEngine::kCommandCapacity - 1; ++index) CHECK(transport.Play());
  CHECK(!transport.Play() && transport.LastCommandFailure() == CommandFailure::QueueFull);
  Render(transport, mono);

  PreviewEngine mix;
  CHECK(mix.Publish(Pcm()));
  Render(mix, mono);
  CHECK(mix.Play());
  std::vector<std::vector<float>> stereo(2, std::vector<float>(1));
  Render(mix, stereo);
  CHECK(stereo[0][0] == stereo[1][0]);
  CHECK(mix.Pause());
  std::vector<std::vector<float>> passthrough {{.2f}, {-.3f}};
  Render(mix, passthrough);
  CHECK(passthrough[0][0] == .2f && passthrough[1][0] == -.3f);

  // Every inactive preview state preserves known plugin host samples bit-exactly.
  PreviewEngine plugin;
  std::vector<std::vector<float>> host {{.2f}, {-.3f}};
  Render(plugin, host);
  CHECK(host[0][0] == .2f && host[1][0] == -.3f); // Unavailable
  CHECK(plugin.Publish(Pcm(1, 4, {.5f, .5f, .5f, .5f})));
  host = {{.2f}, {-.3f}};
  Render(plugin, host);
  CHECK(host[0][0] == .2f && host[1][0] == -.3f); // Stopped
  CHECK(plugin.Play());
  CHECK(plugin.Pause());
  host = {{.2f}, {-.3f}};
  Render(plugin, host);
  CHECK(host[0][0] == .2f && host[1][0] == -.3f); // Paused
  CHECK(plugin.Play());
  CHECK(plugin.SetMuted(true));
  host = {{.2f}, {-.3f}};
  Render(plugin, host);
  CHECK(host[0][0] == .2f && host[1][0] == -.3f); // Muted
  CHECK(plugin.SetMuted(false));
  CHECK(plugin.SetGain(0.f));
  host = {{.2f}, {-.3f}};
  Render(plugin, host);
  CHECK(host[0][0] == .2f && host[1][0] == -.3f); // Gain zero

  PreviewEngine protectedMix;
  CHECK(protectedMix.Publish(Pcm(1, 4, {1.f, -1.f, 1.f, -1.f})));
  Render(protectedMix, mono);
  CHECK(protectedMix.Play());
  host = {{1.f}, {1.f}};
  Render(protectedMix, host);
  CHECK(std::isfinite(host[0][0]) && std::isfinite(host[1][0]) && host[0][0] <= 1.f && host[1][0] <= 1.f);
  host = {{-1.f}, {-1.f}};
  Render(protectedMix, host);
  CHECK(std::isfinite(host[0][0]) && std::isfinite(host[1][0]) && host[0][0] >= -1.f && host[1][0] >= -1.f);
  PreviewEngine nonFinite;
  CHECK(nonFinite.Publish(Pcm(1, 4, {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()})));
  Render(nonFinite, mono);
  CHECK(nonFinite.Play());
  host = {{.25f}, {-.25f}};
  Render(nonFinite, host);
  CHECK(host[0][0] == .25f && host[1][0] == -.25f && std::isfinite(host[0][0]) && std::isfinite(host[1][0]));

  PreviewEngine stereoRouting;
  CHECK(stereoRouting.Publish(Pcm(2, 4, {.25f, -.5f, .75f, -.25f})));
  Render(stereoRouting, mono);
  CHECK(stereoRouting.Play());
  std::vector<std::vector<float>> routed(2, std::vector<float>(1));
  Render(stereoRouting, routed);
  CHECK(std::fabs(routed[0][0] - .25f) < kTolerance && std::fabs(routed[1][0] + .5f) < kTolerance);

  // Snapshot saturation only delays observation; playback continues and recovers after UI consumption.
  PreviewEngine snapshots;
  CHECK(snapshots.Publish(Pcm(1, 4, std::vector<float>(32, .25f))));
  Render(snapshots, mono);
  CHECK(snapshots.Play());
  for (int index = 0; index < 12; ++index)
  {
    Zero(mono);
    RenderRaw(snapshots, mono);
    CHECK(std::fabs(mono[0][0] - .25f) < kTolerance);
  }
  const auto delayed = snapshots.Snapshot();
  CHECK(delayed.sourceAvailable && delayed.state == TransportState::Playing && delayed.position > 0 && delayed.position < 12 && delayed.position <= delayed.frames);
  Zero(mono);
  RenderRaw(snapshots, mono);
  const auto recovered = snapshots.Snapshot();
  CHECK(recovered.sourceAvailable && recovered.state == TransportState::Playing && recovered.position > delayed.position && recovered.position <= recovered.frames);

  // Large, one-frame and irregular render blocks are equivalent for all frozen conversions.
  CheckResampleMatrix(44100, 44100.);
  CheckResampleMatrix(44100, 48000.);
  CheckResampleMatrix(48000, 44100.);
  PreviewEngine changingRate;
  CHECK(changingRate.Publish(Pcm(1, 44100, {0.f, .25f, .5f, .75f, 1.f, -.5f})));
  Render(changingRate, mono, 48000.);
  CHECK(changingRate.Play());
  std::vector<std::vector<float>> rateBlock(1, std::vector<float>(2));
  Render(changingRate, rateBlock, 48000.);
  const auto beforeRateChange = changingRate.Snapshot().position;
  Render(changingRate, rateBlock, 44100.);
  CHECK(changingRate.Snapshot().position >= beforeRateChange);

  PreviewEngine oneFrameSource;
  CHECK(oneFrameSource.Publish(Pcm(1, 4, {.5f})));
  Render(oneFrameSource, mono);
  CHECK(oneFrameSource.Play());
  Zero(mono);
  snapshot = Render(oneFrameSource, mono);
  CHECK(std::fabs(mono[0][0] - .5f) < kTolerance && snapshot.state == TransportState::Stopped && snapshot.position == 1);
  PreviewEngine twoFrameSource;
  CHECK(twoFrameSource.Publish(Pcm(1, 4, {.5f, -.5f})));
  Render(twoFrameSource, mono);
  CHECK(twoFrameSource.Play());
  Zero(mono);
  Render(twoFrameSource, mono);
  CHECK(std::fabs(mono[0][0] - .5f) < kTolerance && twoFrameSource.Snapshot().position == 1);
  Zero(mono);
  snapshot = Render(twoFrameSource, mono);
  CHECK(std::fabs(mono[0][0] + .5f) < kTolerance && snapshot.state == TransportState::Stopped && snapshot.position == 2);

  PreviewEngine gain;
  CHECK(gain.Publish(Pcm(1, 4, std::vector<float>(32, 1.f))));
  Render(gain, mono);
  CHECK(gain.Play());
  CHECK(gain.SetGain(.5f));
  Zero(mono);
  snapshot = Render(gain, mono);
  CHECK(snapshot.gain == .5f && mono[0][0] < 1.f && mono[0][0] > .5f);
  CHECK(gain.SetMuted(true));
  Zero(mono);
  snapshot = Render(gain, mono);
  CHECK(snapshot.muted && snapshot.gain == .5f && Silent(mono));
  CHECK(gain.SetMuted(false));
  Zero(mono);
  snapshot = Render(gain, mono);
  CHECK(!snapshot.muted && snapshot.gain == .5f && std::fabs(mono[0][0] - .01f) < kTolerance);
  CHECK(gain.SetGain(0.f));
  Zero(mono);
  Render(gain, mono);
  CHECK(Silent(mono));

  PreviewEngine standalone;
  std::vector<std::vector<float>> standaloneOut(1, std::vector<float>(1));
  Zero(standaloneOut);
  Render(standalone, standaloneOut);
  CHECK(Silent(standaloneOut)); // Unavailable
  CHECK(standalone.Publish(Pcm(1, 4, {.8f, .8f, .8f, .8f})));
  Zero(standaloneOut);
  Render(standalone, standaloneOut);
  CHECK(Silent(standaloneOut)); // Stopped
  CHECK(standalone.Play());
  Zero(standaloneOut);
  Render(standalone, standaloneOut);
  CHECK(standaloneOut[0][0] != 0.f); // Playing
  CHECK(standalone.Pause());
  Zero(standaloneOut);
  Render(standalone, standaloneOut);
  CHECK(Silent(standaloneOut)); // Paused
  CHECK(standalone.Play());
  CHECK(standalone.SetMuted(true));
  Zero(standaloneOut);
  Render(standalone, standaloneOut);
  CHECK(Silent(standaloneOut)); // Muted
  CHECK(standalone.SetMuted(false));
  CHECK(standalone.SetGain(0.f));
  Zero(standaloneOut);
  Render(standalone, standaloneOut);
  CHECK(Silent(standaloneOut)); // Gain zero

  return failures ? 1 : 0;
}
