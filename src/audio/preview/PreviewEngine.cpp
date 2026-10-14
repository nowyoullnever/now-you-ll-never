#include "PreviewEngine.h"
#include <algorithm>
#include <cmath>

namespace tokkebi::audio::preview {
void PreviewEngine::SetOutputSampleRate(double rate) noexcept { if (std::isfinite(rate) && rate > 0.) mOutputRate.store(rate, std::memory_order_release); }
void PreviewEngine::Publish(std::shared_ptr<const DecodedPcm> source) { if (!source || !source->channels || !source->sampleRate || source->frames > std::numeric_limits<size_t>::max()/source->channels || source->interleaved.size() != source->frames * source->channels) { Clear(); return; } mRetained.push_back(source); mSource.store(source.get(), std::memory_order_release); mPosition.store(0, std::memory_order_release); mRenderCursor.store(0., std::memory_order_release); mState.store(TransportState::Stopped, std::memory_order_release); }
void PreviewEngine::Clear() noexcept { mSource.store(nullptr, std::memory_order_release); mPosition.store(0, std::memory_order_release); mRenderCursor.store(0., std::memory_order_release); mState.store(TransportState::Unavailable, std::memory_order_release); }
uint64_t PreviewEngine::Frames() const noexcept { const auto* s=mSource.load(std::memory_order_acquire); return s?s->frames:0; }
void PreviewEngine::Play() noexcept { const auto* s=mSource.load(std::memory_order_acquire); if(!s) return; if(mPosition.load(std::memory_order_acquire)>=s->frames)mPosition.store(0,std::memory_order_release); mState.store(TransportState::Playing,std::memory_order_release); }
void PreviewEngine::Pause() noexcept { if(mState.load(std::memory_order_acquire)==TransportState::Playing)mState.store(TransportState::Paused,std::memory_order_release); }
void PreviewEngine::Stop() noexcept { if(mSource.load(std::memory_order_acquire)){mPosition.store(0,std::memory_order_release);mState.store(TransportState::Stopped,std::memory_order_release);} }
void PreviewEngine::ReturnToStart() noexcept { if(mSource.load(std::memory_order_acquire)){mPosition.store(0,std::memory_order_release);mRenderCursor.store(0.,std::memory_order_release);} }
void PreviewEngine::Seek(uint64_t frame) noexcept { const auto* s=mSource.load(std::memory_order_acquire); if(s){const auto v=std::min(frame,s->frames);mPosition.store(v,std::memory_order_release);mRenderCursor.store(static_cast<double>(v),std::memory_order_release);} }
void PreviewEngine::SetGain(float gain) noexcept { mGain.store(std::isfinite(gain)?std::clamp(gain,0.f,2.f):1.f,std::memory_order_release); }
void PreviewEngine::SetMuted(bool muted) noexcept { mMuted.store(muted,std::memory_order_release); }
template <typename T> static void RenderImpl(PreviewEngine& e,T** outputs,int outputChannels,int frames) noexcept { const auto* s=e.SourceForRender(); if(!s||e.State()!=TransportState::Playing||!outputs||frames<=0)return; const double ratio=static_cast<double>(s->sampleRate)/e.OutputRateForRender(); const float gain=e.Muted()?0.f:e.Gain(); double cursor=e.CursorForRender(); for(int f=0;f<frames;++f){const uint64_t i=static_cast<uint64_t>(cursor);if(i>=s->frames){e.Seek(s->frames);e.FinishForRender();return;}const uint64_t next=std::min(i+1,s->frames-1);const float frac=static_cast<float>(cursor-static_cast<double>(i));for(int ch=0;ch<outputChannels;++ch){const uint32_t sourceCh=s->channels==1?0:static_cast<uint32_t>(std::min(ch,1));const float a=s->interleaved[static_cast<size_t>(i*s->channels+sourceCh)],b=s->interleaved[static_cast<size_t>(next*s->channels+sourceCh)];const float v=(a+(b-a)*frac)*gain;if(std::isfinite(v))outputs[ch][f]+=static_cast<T>(v);}cursor+=ratio;}e.CommitRenderCursor(cursor,s->frames); }
void PreviewEngine::Render(float** o,int c,int f) noexcept { RenderImpl(*this,o,c,f); }
void PreviewEngine::Render(double** o,int c,int f) noexcept { RenderImpl(*this,o,c,f); }
}
