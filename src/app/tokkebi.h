#pragma once

#include "config.h"
#include "IPlug_include_in_plug_hdr.h"
#include "../audio/preview/PreviewEngine.h"

using namespace iplug;

class Tokkebi final : public Plugin
{
public:
  explicit Tokkebi(const InstanceInfo& info);

#if IPLUG_DSP
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
private:
  tokkebi::audio::preview::PreviewEngine mPreview;
};
