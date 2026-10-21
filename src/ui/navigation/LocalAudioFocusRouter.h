#pragma once

#include "NavigationState.h"

namespace tokkebi::ui
{
enum class LocalAudioFocusTransition { None, EnterButton, EnterVolume, ExitToInbox, ExitToLibrary };

// Local-audio controls are reachable between INBOX and LIBRARY in the shell's
// tab sequence. They exist only on INBOX and release before another shell
// target takes ownership.
class LocalAudioFocusRouter final
{
public:
  bool ButtonFocused() const { return mButtonFocused; }
  bool PreviewVolumeFocused() const { return mPreviewVolumeFocused; }
  bool ConsumesArrowKeys() const { return mButtonFocused || mPreviewVolumeFocused; }

  LocalAudioFocusTransition TabTransition(const NavigationState& navigation, bool reverse) const
  {
    if (mButtonFocused)
      return reverse ? LocalAudioFocusTransition::ExitToInbox : LocalAudioFocusTransition::EnterVolume;
    if (mPreviewVolumeFocused)
      return reverse ? LocalAudioFocusTransition::EnterButton : LocalAudioFocusTransition::ExitToLibrary;
    if (navigation.Focus() != FocusTarget::TabRail)
      return LocalAudioFocusTransition::None;
    if (!reverse && navigation.Focused() == TabId::Inbox)
      return LocalAudioFocusTransition::EnterButton;
    if (reverse && navigation.Focused() == TabId::Library)
      return LocalAudioFocusTransition::EnterVolume;
    return LocalAudioFocusTransition::None;
  }

  bool EnterButton(NavigationState& navigation, bool reverse)
  {
    if (TabTransition(navigation, reverse) != LocalAudioFocusTransition::EnterButton)
      return false;
    navigation.Select(TabId::Inbox);
    mButtonFocused = true;
    mPreviewVolumeFocused = false;
    return true;
  }

  bool EnterVolume(NavigationState& navigation, bool reverse)
  {
    if (TabTransition(navigation, reverse) != LocalAudioFocusTransition::EnterVolume)
      return false;
    navigation.Select(TabId::Inbox);
    mButtonFocused = false;
    mPreviewVolumeFocused = true;
    return true;
  }

  bool ExitButton(NavigationState& navigation, bool reverse)
  {
    const auto transition = TabTransition(navigation, reverse);
    if (transition != LocalAudioFocusTransition::ExitToInbox && transition != LocalAudioFocusTransition::ExitToLibrary)
      return false;
    Release();
    navigation.Select(TabId::Inbox);
    if (transition == LocalAudioFocusTransition::ExitToLibrary)
      navigation.MoveFocus(1);
    return true;
  }

  void FocusButton(NavigationState& navigation) { navigation.Select(TabId::Inbox); mButtonFocused = true; mPreviewVolumeFocused = false; }
  void FocusVolume(NavigationState& navigation) { navigation.Select(TabId::Inbox); mButtonFocused = false; mPreviewVolumeFocused = true; }
  void SelectShellTab(NavigationState& navigation, TabId tab) { Release(); navigation.Select(tab); }
  void FocusTheme(NavigationState& navigation) { Release(); navigation.FocusTheme(); }
  void Release() { mButtonFocused = false; mPreviewVolumeFocused = false; }
  void EnforcePageContext(const NavigationState& navigation) { if (navigation.Selected() != TabId::Inbox) Release(); }

private:
  bool mButtonFocused = false;
  bool mPreviewVolumeFocused = false;
};
}
