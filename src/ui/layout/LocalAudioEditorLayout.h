#pragma once

#include "ShellLayout.h"

#include <algorithm>
#include <array>

namespace tokkebi::ui
{
struct LocalAudioEditorLayout
{
  Rect openButton;
  Rect metadata;
  std::array<Rect, 4> transport;
  Rect volume;
  Rect elapsed;
  Rect waveform;
  Rect help;
};

inline LocalAudioEditorLayout CalculateLocalAudioEditorLayout(Rect content)
{
  constexpr float kTransportLeft = 200.f;
  constexpr float kTransportTop = 92.f;
  constexpr float kControlWidth = 68.f;
  constexpr float kControlGap = 4.f;
  constexpr float kWaveformTop = 278.f;
  constexpr float kHelpHeight = 18.f;
  constexpr float kBottomGap = 8.f;

  LocalAudioEditorLayout layout;
  layout.openButton = {content.x, content.y + kTransportTop, std::min(content.width, 190.f), 28.f};
  layout.metadata = {content.x, content.y + 128.f, std::min(content.width, 190.f), 140.f};
  for (std::size_t index = 0; index < layout.transport.size(); ++index)
    layout.transport[index] = {content.x + kTransportLeft + static_cast<float>(index) * (kControlWidth + kControlGap), content.y + kTransportTop, kControlWidth, 28.f};
  layout.volume = {content.x + kTransportLeft, content.y + 146.f, std::min(220.f, std::max(0.f, content.width - 210.f)), 18.f};
  layout.elapsed = {content.x + kTransportLeft, content.y + 126.f, std::max(0.f, content.width - kTransportLeft), 20.f};
  layout.waveform = {content.x, content.y + kWaveformTop, std::max(0.f, content.width),
                     std::max(0.f, std::min(170.f, content.height - kWaveformTop - kHelpHeight - kBottomGap))};
  layout.help = {content.x, layout.waveform.y + layout.waveform.height + 4.f, std::max(0.f, content.width), kHelpHeight};
  return layout;
}
}
