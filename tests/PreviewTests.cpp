#include "audio/preview/PreviewEngine.h"
#include <cmath>
#include <iostream>
using namespace tokkebi::audio;
using namespace tokkebi::audio::preview;
static int failures{};
#define CHECK(x) do { if (!(x)) { ++failures; std::cerr << "FAIL: " #x "\n"; } } while(false)
static std::shared_ptr<const DecodedPcm> Pcm(uint32_t ch=1,uint32_t rate=4) { auto p=std::make_shared<DecodedPcm>();p->channels=ch;p->sampleRate=rate;p->frames=4;p->interleaved=ch==1?std::vector<float>{0.f,1.f,.5f,-1.f}:std::vector<float>{.1f,.2f,.3f,.4f,.5f,.6f,.7f,.8f};return p; }
int main(){
  PreviewEngine e; e.SetOutputSampleRate(4); float l[8]{},r[8]{};float* out[]={l,r};
  CHECK(e.State()==TransportState::Unavailable); e.Play(); CHECK(e.State()==TransportState::Unavailable);
  e.Publish(Pcm()); CHECK(e.State()==TransportState::Stopped&&e.Position()==0);e.Play();e.Render(out,2,2);CHECK(e.State()==TransportState::Playing&&e.Position()==2&&l[1]>0.9f&&r[1]>0.9f);
  e.Pause();const auto paused=e.Position();e.Render(out,2,2);CHECK(e.State()==TransportState::Paused&&e.Position()==paused);e.ReturnToStart();CHECK(e.State()==TransportState::Paused&&e.Position()==0);e.Play();e.Seek(4);CHECK(e.Position()==4);e.Play();CHECK(e.Position()==0&&e.State()==TransportState::Playing);
  e.SetGain(.5f);e.SetMuted(true);e.Render(out,2,1);CHECK(std::isfinite(l[0]));e.SetMuted(false);e.Stop();CHECK(e.State()==TransportState::Stopped&&e.Position()==0);
  e.SetGain(1.f); e.Publish(Pcm(2));e.Play();float a[2]{},b[2]{};float* stereo[]={a,b};e.Render(stereo,2,1);CHECK(std::fabs(a[0]-.1f)<.001f&&std::fabs(b[0]-.2f)<.001f);
  e.SetOutputSampleRate(8);e.ReturnToStart();e.Render(stereo,2,2);CHECK(e.Position()==1); e.Clear();CHECK(e.State()==TransportState::Unavailable&&e.Position()==0);
  return failures?1:0;
}
