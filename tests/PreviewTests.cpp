#include "audio/preview/PreviewEngine.h"

#include <atomic>
#include <cmath>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

using namespace tokkebi::audio;
using namespace tokkebi::audio::preview;
namespace {
int failures{};
#define CHECK(x) do { if (!(x)) { ++failures; std::cerr << "FAIL: " #x "\n"; } } while (false)
std::shared_ptr<const DecodedPcm> Pcm(uint32_t channels=1, uint32_t rate=4, std::vector<float> samples={0.f,1.f,.5f,-1.f})
{ auto p=std::make_shared<DecodedPcm>(); p->channels=channels;p->sampleRate=rate;p->frames=samples.size()/channels;p->interleaved=std::move(samples);return p; }
TransportSnapshot Render(PreviewEngine& engine, std::vector<std::vector<float>>& data, double rate=4.)
{ std::vector<float*> output;for(auto& channel:data)output.push_back(channel.data());engine.Render(output.data(),static_cast<int>(output.size()),static_cast<int>(data.empty()?0:data[0].size()),rate);return engine.Snapshot(); }
void Zero(std::vector<std::vector<float>>& data){for(auto& channel:data)for(auto& sample:channel)sample=0.f;}
}
int main()
{
  PreviewEngine e; std::vector<std::vector<float>> mono(1,std::vector<float>(1));
  CHECK(e.Snapshot().state==TransportState::Unavailable); CHECK(e.Play()); Render(e,mono); CHECK(e.Snapshot().state==TransportState::Unavailable);
  auto a=Pcm(); std::weak_ptr<const DecodedPcm> weakA=a; CHECK(e.Publish(a)); a.reset(); Render(e,mono); auto s=e.Snapshot(); CHECK(s.state==TransportState::Stopped&&s.position==0&&s.frames==4&&s.sourceAvailable);
  CHECK(e.Play()); Zero(mono); s=Render(e,mono); CHECK(s.state==TransportState::Playing&&s.position==1);
  CHECK(e.Play()); Render(e,mono); CHECK(e.Pause()); Render(e,mono); const auto paused=e.Snapshot().position; Render(e,mono); CHECK(e.Snapshot().state==TransportState::Paused&&e.Snapshot().position==paused);
  CHECK(e.ReturnToStart());Render(e,mono);CHECK(e.Snapshot().state==TransportState::Paused&&e.Snapshot().position==0);
  CHECK(e.Play());Render(e,mono);CHECK(e.Seek(3));Render(e,mono);CHECK(e.Snapshot().position==4||e.Snapshot().position==3);
  CHECK(e.Stop());Render(e,mono);CHECK(e.Snapshot().state==TransportState::Stopped&&e.Snapshot().position==0);
  CHECK(e.ReturnToStart());Render(e,mono);CHECK(e.Snapshot().state==TransportState::Stopped&&e.Snapshot().position==0);

  // Command order is consumed only at block boundaries; later commands win deterministically.
  CHECK(e.Play());CHECK(e.Seek(3));CHECK(e.Stop());Render(e,mono);CHECK(e.Snapshot().state==TransportState::Stopped&&e.Snapshot().position==0);
  CHECK(e.Stop());CHECK(e.Play());Render(e,mono);CHECK(e.Snapshot().position>0);
  CHECK(e.Pause());CHECK(e.Seek(2));Render(e,mono);CHECK(e.Snapshot().state==TransportState::Paused&&e.Snapshot().position==2);

  auto b=Pcm(1,4,{.25f,.25f,.25f,.25f});std::weak_ptr<const DecodedPcm> weakB=b;CHECK(e.Publish(b));b.reset();Render(e,mono);CHECK(e.Snapshot().generation==2&&e.Snapshot().position==0&&e.Snapshot().state==TransportState::Stopped);e.Maintain();CHECK(weakA.expired());
  CHECK(e.Clear());Render(e,mono);CHECK(e.Snapshot().state==TransportState::Unavailable&&!e.Snapshot().sourceAvailable);e.Maintain();CHECK(weakB.expired());

  // Owners outlive queued raw pointers and retire only after their generation is acknowledged.
  PreviewEngine ownership; auto oa=Pcm(),ob=Pcm(),oc=Pcm(),od=Pcm(); std::weak_ptr<const DecodedPcm> wa=oa,wb=ob,wc=oc,wd=od;
  CHECK(ownership.Publish(oa)); oa.reset(); ownership.Maintain(); CHECK(!wa.expired());
  CHECK(ownership.Publish(ob)); CHECK(ownership.Clear()); CHECK(ownership.Publish(oc)); CHECK(ownership.Publish(od)); ob.reset(); oc.reset(); od.reset();
  Render(ownership,mono); CHECK(ownership.Snapshot().sourceAvailable && ownership.Snapshot().generation==5); ownership.Maintain();
  CHECK(wa.expired() && wb.expired() && wc.expired() && !wd.expired());
  CHECK(ownership.Clear()); Render(ownership,mono); ownership.Maintain(); CHECK(wd.expired());
  for(int i=0;i<8;++i){auto transient=Pcm();std::weak_ptr<const DecodedPcm> weak=transient;CHECK(ownership.Publish(transient));transient.reset();Render(ownership,mono);ownership.Maintain();CHECK(!weak.expired());CHECK(ownership.Clear());Render(ownership,mono);ownership.Maintain();CHECK(weak.expired());}

  // 63 queued commands fill a 64-slot ring; the failure is observable rather than dropped silently.
  for(uint32_t i=0;i<PreviewEngine::kCommandCapacity-1;++i)CHECK(e.Play());CHECK(!e.Play()&&e.LastCommandFailure()==CommandFailure::QueueFull);Render(e,mono);

  // Mono duplication, stereo routing and exact inactive passthrough.
  auto source=Pcm();CHECK(e.Publish(source));Render(e,mono);CHECK(e.Play());std::vector<std::vector<float>> stereo(2,std::vector<float>(1));Render(e,stereo);CHECK(stereo[0][0]==stereo[1][0]);
  CHECK(e.Pause());std::vector<std::vector<float>> passthrough{{.2f},{-.3f}};Render(e,passthrough);CHECK(passthrough[0][0]==.2f&&passthrough[1][0]==-.3f);
  CHECK(e.SetGain(0.f));CHECK(e.Play());passthrough={{.2f},{-.3f}};Render(e,passthrough);CHECK(passthrough[0][0]==.2f&&passthrough[1][0]==-.3f);
  CHECK(e.SetGain(std::numeric_limits<float>::quiet_NaN()));CHECK(e.SetMuted(true));passthrough={{.2f},{-.3f}};Render(e,passthrough);CHECK(passthrough[0][0]==.2f&&passthrough[1][0]==-.3f&&e.Snapshot().gain==1.f&&e.Snapshot().muted);
  CHECK(e.SetMuted(false));CHECK(e.ReturnToStart());passthrough={{.9f},{.9f}};Render(e,passthrough);CHECK(std::isfinite(passthrough[0][0])&&passthrough[0][0]<=1.f);

  // Fractional cursor is equivalent for one large block and irregular block splits.
  auto resample=Pcm(1,44100,{0.f,.25f,.5f,.75f,1.f,-.5f}); PreviewEngine large, split;CHECK(large.Publish(resample));CHECK(split.Publish(resample));std::vector<std::vector<float>> once(1,std::vector<float>(7)),pieces(1);Render(large,once,48000.);CHECK(large.Play());Render(large,once,48000.);CHECK(split.Play());for(int n:std::vector<int>{1,3,1,2}){std::vector<std::vector<float>> part(1,std::vector<float>(n));Render(split,part,48000.);pieces[0].insert(pieces[0].end(),part[0].begin(),part[0].end());}CHECK(large.Snapshot().position==split.Snapshot().position);for(size_t i=0;i<once[0].size();++i)CHECK(std::fabs(once[0][i]-pieces[0][i])<.0001f);
  PreviewEngine changing;CHECK(changing.Publish(resample));Render(changing,mono);CHECK(changing.Play());std::vector<std::vector<float>> first(1,std::vector<float>(2)),second(1,std::vector<float>(2));Render(changing,first,48000.);const auto before=changing.Snapshot().position;Render(changing,second,44100.);CHECK(changing.Snapshot().position>=before);

  // Same-rate, 1/2-frame blocks, EOF and repeated snapshots retain the same transport facts.
  PreviewEngine exact;CHECK(exact.Publish(Pcm(1,4,{.5f,-.5f})));Render(exact,mono,4.);CHECK(exact.Play());std::vector<std::vector<float>> one(1,std::vector<float>(1));Render(exact,one,4.);CHECK(std::fabs(one[0][0]-.5f)<.0001f&&exact.Snapshot().position==1);Zero(one);Render(exact,one,4.);CHECK(std::fabs(one[0][0]+.5f)<.0001f&&exact.Snapshot().state==TransportState::Stopped&&exact.Snapshot().position==2);Zero(one);Render(exact,one,4.);CHECK(one[0][0]==0.f&&exact.Snapshot().position==2);
  for(int i=0;i<32;++i){const auto coherent=exact.Snapshot();CHECK((!coherent.sourceAvailable&&coherent.state==TransportState::Unavailable&&coherent.position==0&&coherent.frames==0)||(coherent.sourceAvailable&&coherent.position<=coherent.frames));}
  PreviewEngine concurrent;CHECK(concurrent.Publish(Pcm(1,4,std::vector<float>(512,.25f))));Render(concurrent,mono,4.);CHECK(concurrent.Play());std::atomic<bool> rendering{true};std::thread callback([&]{std::vector<std::vector<float>> block(1,std::vector<float>(1));for(int i=0;i<1024;++i){Zero(block);Render(concurrent,block,4.);}rendering.store(false,std::memory_order_release);});while(rendering.load(std::memory_order_acquire)){const auto coherent=concurrent.Snapshot();CHECK(coherent.sourceAvailable&&coherent.position<=coherent.frames&&coherent.frames==512);}callback.join();

  // Standalone-style zero buffers remain silent when stopped, paused, muted, or gain zero.
  PreviewEngine silence;CHECK(silence.Publish(Pcm()));Render(silence,mono);Zero(mono);Render(silence,mono);for(float v:mono[0])CHECK(v==0.f);CHECK(silence.Play());CHECK(silence.SetMuted(true));Zero(mono);Render(silence,mono);for(float v:mono[0])CHECK(v==0.f);
  return failures?1:0;
}
