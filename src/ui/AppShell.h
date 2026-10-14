#pragma once

#include "IControls.h"

#include "layout/ShellLayout.h"
#include "navigation/NavigationState.h"
#include "navigation/LocalAudioFocusRouter.h"
#include "components/VisualControls.h"
#include "LocalAudioDialogMailbox.h"
#include "../audio/AudioDocument.h"
#include "../audio/AudioLoadCoordinator.h"
#include "../audio/waveform/WaveformCoordinator.h"

namespace tokkebi::ui
{
class AppShell final : public iplug::igraphics::IControl
{
public:
  explicit AppShell(const iplug::igraphics::IRECT& bounds);
  ~AppShell() override;

  void Draw(iplug::igraphics::IGraphics& graphics) override;
  void OnMouseDown(float x, float y, const iplug::igraphics::IMouseMod& mod) override;
  void OnMouseOver(float x, float y, const iplug::igraphics::IMouseMod& mod) override;
  void OnMouseOut() override;
  void OnMouseWheel(float x, float y, const iplug::igraphics::IMouseMod& mod, float distance) override;
  bool OnKeyDown(float x, float y, const iplug::IKeyPress& key) override;

  bool HandleKey(const iplug::IKeyPress& key);
  void OnDisplayTick();
  const NavigationState& State() const { return mState; }

private:
  ShellLayout Layout() const;
  Rect TabBounds(const ShellLayout& layout, std::size_t index) const;
  Rect ThemeBounds(const ShellLayout& layout) const;
  TabId TabAt(float x, float y, const ShellLayout& layout) const;
  void DrawComponentDemo(iplug::igraphics::IGraphics& graphics, const ShellLayout& layout);
  bool HandleDemoKey(const iplug::IKeyPress& key);
  bool HandleLocalAudioKey(const iplug::IKeyPress& key);
  void ShowDemoNotification();
  void PromptForLocalAudio();
  Rect LocalAudioButtonBounds(const ShellLayout& layout) const;
  Rect WaveformBounds(const ShellLayout& layout) const;
  void Redraw();

  NavigationState mState;
  components::ButtonControl mNotificationButton {"TEST NOTIFICATION"};
  components::ButtonControl mOpenLocalAudioButton {"OPEN LOCAL AUDIO"};
  components::ButtonControl mClearButton {"CLEAR TEXT"};
  components::TextFieldControl mDemoText {64};
  components::ListViewControl mDemoList;
  components::ModalModel mDemoModal;
  components::NotificationControl mDemoNotification;
  components::ProgressControl mDemoProgress;
  components::FocusRouter mDemoFocus;
  components::ComponentFocusRouter mComponentFocus;
  components::ComponentFocus mFocusBeforeModal = components::ComponentFocus::None;
  bool mModalConfirmFocused = false;
  audio::AudioDocument mLocalAudioDocument;
  audio::AudioLoadCoordinator mLocalAudioLoads;
  audio::waveform::Coordinator mWaveforms;
  uint64_t mWaveformGeneration = 0;
  audio::waveform::Viewport mWaveformViewport;
  std::shared_ptr<LocalAudioDialogMailbox> mLocalAudioDialogMailbox = std::make_shared<LocalAudioDialogMailbox>();
  LocalAudioFocusRouter mLocalAudioFocus;
};
}
