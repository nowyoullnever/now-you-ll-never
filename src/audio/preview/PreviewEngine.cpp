#include "PreviewEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace tokkebi::audio::preview {
namespace {
constexpr float kGainSmoothing = .02f;
template <typename T> T ProtectedMix(T host, float preview) noexcept
{
  if (preview == 0.f) return host; // inactive preview leaves passthrough bit-exact.
  const float input = std::isfinite(static_cast<float>(host)) ? static_cast<float>(host) : 0.f;
  const float mixed = input + preview;
  return static_cast<T>(std::clamp(std::isfinite(mixed) ? mixed : 0.f, -1.f, 1.f));
}
}

bool PreviewEngine::Valid(const DecodedPcm& source) noexcept
{
  return source.channels != 0 && source.sampleRate != 0 &&
    source.frames <= std::numeric_limits<size_t>::max() / source.channels &&
    source.interleaved.size() == source.frames * source.channels;
}
float PreviewEngine::SanitizeGain(float gain) noexcept
{
  return std::isfinite(gain) ? std::clamp(gain, 0.f, 2.f) : 1.f;
}
bool PreviewEngine::Push(Command command) noexcept
{
  const uint32_t write = mWrite.load(std::memory_order_relaxed);
  const uint32_t next = (write + 1U) % kCommandCapacity;
  if (next == mRead.load(std::memory_order_acquire))
  {
    mLastFailure.store(CommandFailure::QueueFull, std::memory_order_release);
    return false;
  }
  mCommands[write] = command;
  mWrite.store(next, std::memory_order_release);
  mLastFailure.store(CommandFailure::None, std::memory_order_release);
  return true;
}
bool PreviewEngine::Pop(Command& command) noexcept
{
  const uint32_t read = mRead.load(std::memory_order_relaxed);
  if (read == mWrite.load(std::memory_order_acquire)) return false;
  command = mCommands[read];
  mRead.store((read + 1U) % kCommandCapacity, std::memory_order_release);
  return true;
}
bool PreviewEngine::Publish(std::shared_ptr<const DecodedPcm> source)
{
  if (!source || !Valid(*source)) { mLastFailure.store(CommandFailure::InvalidSource, std::memory_order_release); return Clear(); }
  const uint64_t generation = ++mNextGeneration;
  try
  {
    // The audio thread receives only a raw pointer. Establish its durable
    // control-thread owner before making that pointer visible in the queue.
    mSources.push_back({generation, source});
  }
  catch (...)
  {
    mLastFailure.store(CommandFailure::OwnershipUnavailable, std::memory_order_release);
    return false;
  }
  if (!Push({CommandType::InstallSource, source.get(), generation}))
  {
    mSources.pop_back();
    return false;
  }
  return true;
}
bool PreviewEngine::Clear() noexcept { return Push({CommandType::ClearSource, nullptr, ++mNextGeneration}); }
bool PreviewEngine::Play() noexcept { return Push({CommandType::Play}); }
bool PreviewEngine::Pause() noexcept { return Push({CommandType::Pause}); }
bool PreviewEngine::Stop() noexcept { return Push({CommandType::Stop}); }
bool PreviewEngine::ReturnToStart() noexcept { return Push({CommandType::ReturnToStart}); }
bool PreviewEngine::Seek(uint64_t frame) noexcept { Command c{CommandType::Seek}; c.frame=frame; return Push(c); }
bool PreviewEngine::SetGain(float gain) noexcept { Command c{CommandType::SetGain}; c.gain=SanitizeGain(gain); return Push(c); }
bool PreviewEngine::SetMuted(bool muted) noexcept { Command c{CommandType::SetMute}; c.muted=muted; return Push(c); }
void PreviewEngine::Maintain() noexcept
{
  const uint64_t acknowledged = mAcknowledgedGeneration.load(std::memory_order_acquire);
  while (!mSources.empty() && mSources.front().generation < acknowledged) mSources.pop_front();
}
void PreviewEngine::Apply(const Command& command) noexcept
{
  switch (command.type)
  {
    case CommandType::InstallSource: mActiveSource=command.source; mActiveGeneration=command.generation; mState=TransportState::Stopped; mCursor=0.; mRenderGain=mMuted?0.f:mTargetGain; mAcknowledgedGeneration.store(command.generation,std::memory_order_release); break;
    case CommandType::ClearSource: mActiveSource=nullptr; mActiveGeneration=command.generation; mState=TransportState::Unavailable; mCursor=0.; mRenderGain=0.f; mAcknowledgedGeneration.store(command.generation,std::memory_order_release); break;
    case CommandType::Play: if(mActiveSource){if(mCursor>=static_cast<double>(mActiveSource->frames))mCursor=0.;mState=TransportState::Playing;} break;
    case CommandType::Pause: if(mState==TransportState::Playing)mState=TransportState::Paused; break;
    case CommandType::Stop: if(mActiveSource){mState=TransportState::Stopped;mCursor=0.;} break;
    case CommandType::ReturnToStart: if(mActiveSource)mCursor=0.; break;
    case CommandType::Seek: if(mActiveSource)mCursor=static_cast<double>(std::min(command.frame,mActiveSource->frames)); break;
    case CommandType::SetGain: mTargetGain=command.gain;if(mTargetGain==0.f)mRenderGain=0.f;break;
    case CommandType::SetMute: mMuted=command.muted;mRenderGain=0.f;break;
  }
}
void PreviewEngine::PublishSnapshot() noexcept
{
  const uint32_t write = mSnapshotWrite.load(std::memory_order_relaxed);
  const uint32_t next = (write + 1U) % kSnapshotCapacity;
  if (next == mSnapshotRead.load(std::memory_order_acquire)) return;

  mSnapshots[write] = {mActiveGeneration, mState,
    mActiveSource ? std::min<uint64_t>(static_cast<uint64_t>(mCursor), mActiveSource->frames) : 0,
    mActiveSource ? mActiveSource->frames : 0, mTargetGain, mMuted, mActiveSource != nullptr};
  mSnapshotWrite.store(next, std::memory_order_release);
}
TransportSnapshot PreviewEngine::Snapshot() noexcept
{
  uint32_t read = mSnapshotRead.load(std::memory_order_relaxed);
  const uint32_t write = mSnapshotWrite.load(std::memory_order_acquire);
  while (read != write)
  {
    mLatestSnapshot = mSnapshots[read];
    read = (read + 1U) % kSnapshotCapacity;
  }
  mSnapshotRead.store(read, std::memory_order_release);
  return mLatestSnapshot;
}
template <typename T> void RenderImpl(PreviewEngine& engine,T** outputs,int outputChannels,int frames,double outputRate) noexcept
{
  PreviewEngine::Command command;while(engine.Pop(command))engine.Apply(command);
  if(!std::isfinite(outputRate)||outputRate<=0.)outputRate=48000.;const auto* source=engine.mActiveSource;
  if(!source||engine.mState!=TransportState::Playing||!outputs||outputChannels<=0||frames<=0){engine.PublishSnapshot();return;}
  const double ratio=static_cast<double>(source->sampleRate)/outputRate;
  for(int frame=0;frame<frames;++frame){if(engine.mCursor>=static_cast<double>(source->frames)){engine.mCursor=static_cast<double>(source->frames);engine.mState=TransportState::Stopped;break;}const uint64_t first=static_cast<uint64_t>(engine.mCursor);const uint64_t next=std::min(first+1,source->frames-1);const float fraction=static_cast<float>(engine.mCursor-static_cast<double>(first));const float gain=engine.mMuted?0.f:(engine.mRenderGain+=(engine.mTargetGain-engine.mRenderGain)*kGainSmoothing);for(int channel=0;channel<outputChannels;++channel){const uint32_t inputChannel=source->channels==1?0:static_cast<uint32_t>(std::min(channel,1));const float a=source->interleaved[static_cast<size_t>(first*source->channels+inputChannel)],b=source->interleaved[static_cast<size_t>(next*source->channels+inputChannel)];const float preview=std::isfinite(a)&&std::isfinite(b)?(a+(b-a)*fraction)*gain:0.f;outputs[channel][frame]=ProtectedMix(outputs[channel][frame],std::isfinite(preview)?preview:0.f);}engine.mCursor+=ratio;}
  if(engine.mCursor>=static_cast<double>(source->frames)){engine.mCursor=static_cast<double>(source->frames);engine.mState=TransportState::Stopped;}engine.PublishSnapshot();
}
void PreviewEngine::Render(float** outputs,int channels,int frames,double rate) noexcept { RenderImpl(*this,outputs,channels,frames,rate); }
void PreviewEngine::Render(double** outputs,int channels,int frames,double rate) noexcept { RenderImpl(*this,outputs,channels,frames,rate); }
}
