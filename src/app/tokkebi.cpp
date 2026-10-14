#include "tokkebi.h"
#include "fonts.h"
#include "../ui/AppShell.h"
#include "../ui/theme/Theme.h"

#include "IControls.h"
#include "IPlug_include_in_plug_src.h"

#include <cstdio>

Tokkebi::Tokkebi(const iplug::InstanceInfo& info)
: Plugin(info, MakeConfig(0, 0))
{
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,
                        GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [this](iplug::igraphics::IGraphics* graphics) {
    using namespace iplug::igraphics;

    const bool fontsLoaded = tokkebi::fonts::Load(graphics);
    if (!fontsLoaded)
    {
      std::fputs("tokkebi: required bundled fonts could not be loaded\n", stderr);
      return;
    }
    const auto background = tokkebi::theme::Get(tokkebi::theme::ThemeMode::Dark).app;
    const IColor backgroundColor {background.a, background.r, background.g, background.b};
    graphics->AttachPanelBackground(backgroundColor);
    auto* shell = new tokkebi::ui::AppShell(graphics->GetBounds(), &mPreview);
    graphics->AttachControl(shell);
    graphics->SetDisplayTickFunc([shell] { shell->OnDisplayTick(); });
    graphics->EnableMouseOver(true);
    graphics->SetKeyHandlerFunc([shell](const iplug::IKeyPress& key, bool isUp) {
      return !isUp && shell->HandleKey(key);
    });
  };
#endif
}

#if IPLUG_DSP
void Tokkebi::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  mPreview.SetOutputSampleRate(GetSampleRate());
#if defined(APP_API)
  // P00.2 Standalone intentionally has no input bus and must remain silent.
  static_cast<void>(inputs);
  const int outputChannels = NOutChansConnected();
  for (int channel = 0; channel < outputChannels; ++channel)
  {
    for (int frame = 0; frame < nFrames; ++frame)
      outputs[channel][frame] = 0.0;
  }
#else
  const int inputChannels = NInChansConnected();
  const int outputChannels = NOutChansConnected();

  for (int channel = 0; channel < outputChannels; ++channel)
  {
    for (int frame = 0; frame < nFrames; ++frame)
      outputs[channel][frame] = channel < inputChannels ? inputs[channel][frame] : 0.0;
  }
#endif
  mPreview.Render(outputs, NOutChansConnected(), nFrames);
}
#endif
