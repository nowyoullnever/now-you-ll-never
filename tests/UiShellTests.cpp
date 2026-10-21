#include "ui/layout/ShellLayout.h"
#include "ui/navigation/LocalAudioFocusRouter.h"
#include "ui/navigation/NavigationState.h"
#include "ui/text/Strings.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string_view>

using namespace tokkebi::ui;

namespace
{
int failures = 0;

void Check(bool condition, std::string_view description)
{
  if (!condition)
  {
    std::cerr << "UiShellTests failure: " << description << '\n';
    ++failures;
  }
}

bool Bounded(Rect rect, Rect bounds)
{
  return rect.width >= 0.f && rect.height >= 0.f && rect.x >= bounds.x && rect.y >= bounds.y
      && rect.x + rect.width <= bounds.x + bounds.width && rect.y + rect.height <= bounds.y + bounds.height;
}

bool Overlap(Rect a, Rect b)
{
  return a.x < b.x + b.width && a.x + a.width > b.x && a.y < b.y + b.height && a.y + a.height > b.y;
}

void CheckLayout(float width, float height)
{
  const auto layout = CalculateShellLayout(width, height);
  Check(Bounded(layout.header, layout.bounds), "header stays within bounds");
  Check(Bounded(layout.tabRail, layout.bounds), "tab rail stays within bounds");
  Check(Bounded(layout.content, layout.bounds), "content stays within bounds");
  Check(Bounded(layout.statusBar, layout.bounds), "status bar stays within bounds");
  Check(Bounded(layout.contentPadding, layout.bounds), "content padding stays within bounds");
  Check(!Overlap(layout.header, layout.tabRail), "header and tab rail do not overlap");
  Check(!Overlap(layout.tabRail, layout.content), "tab rail and content do not overlap");
  Check(!Overlap(layout.content, layout.statusBar), "content and status bar do not overlap");
  Check(layout.statusBar.y + layout.statusBar.height == layout.bounds.height, "status bar reaches bottom edge");
}

void CheckShellContrast(tokkebi::theme::ThemeMode mode)
{
  const auto tokens = tokkebi::theme::Get(mode);
  Check(tokkebi::theme::Contrast(tokens.textPrimary, tokens.secondary) >= 4.5, "header and inactive tab text contrast");
  Check(tokkebi::theme::Contrast(tokens.actionText, tokens.actionDefault) >= 4.5, "selected tab label contrast");
  Check(tokkebi::theme::Contrast(tokens.textPrimary, tokens.primary) >= 4.5, "content text contrast");
  Check(tokkebi::theme::Contrast(tokens.technicalOnAudio, tokens.audio) >= 4.5, "status text contrast");
  Check(tokkebi::theme::Contrast(tokens.navigationHoverText, tokens.navigationHover) >= 4.5, "hovered tab label contrast");
  Check(tokkebi::theme::Contrast(tokens.actionText, tokens.actionDefault) >= 4.5, "focused theme button border contrast");
  Check(!(tokens.navigationHover == tokens.raised) && !(tokens.navigationHover == tokens.actionDefault), "hovered tab surface differs from inactive and active");
  Check(!(tokens.actionDefault == tokens.raised) && !(tokens.focusRing == tokens.primary), "active and focus navigation colors are distinguishable");
}
}

int main()
{
  Check(kTabs.size() == 6, "exactly six tab IDs exist");
  const std::array<TabId, 6> expected {{TabId::Web, TabId::P2P, TabId::Inbox, TabId::Library, TabId::History, TabId::Settings}};
  for (std::size_t index = 0; index < expected.size(); ++index)
  {
    Check(kTabs[index] == expected[index], "tab order is stable");
    Check(!strings::TabLabel(kTabs[index]).empty(), "tab label is nonempty");
    Check(!strings::SectionMessage(kTabs[index]).empty(), "Korean section message is nonempty");
  }
  Check(strings::TabLabel(TabId::Web) == "WEB" && strings::TabLabel(TabId::P2P) == "P2P"
        && strings::TabLabel(TabId::Inbox) == "INBOX" && strings::TabLabel(TabId::Library) == "LIBRARY"
        && strings::TabLabel(TabId::History) == "HISTORY" && strings::TabLabel(TabId::Settings) == "SETTINGS",
        "exact uppercase English tab labels");

  NavigationState first;
  NavigationState second;
  Check(first.Selected() == TabId::Web && first.Focused() == TabId::Web, "WEB is the default tab");
  for (const auto tab : kTabs)
  {
    first.Select(tab);
    Check(first.Selected() == tab, "valid tab selection updates page identity");
    Check(!first.Select(tab), "selecting active tab has no reset side effect");
  }
  Check(!first.Select(TabId::Count), "invalid tab selection is rejected");
  Check(second.Selected() == TabId::Web, "separate navigation state is isolated");
  first.Select(TabId::Library);
  first.ToggleTheme();
  Check(first.Selected() == TabId::Library && first.Theme() == tokkebi::theme::ThemeMode::Light,
        "theme switching preserves selected tab");
  first.MoveFocus(1);
  Check(first.Focused() == TabId::History, "forward keyboard focus navigation");
  first.MoveFocus(-1);
  Check(first.Focused() == TabId::Library, "reverse keyboard focus navigation");
  first.SetHovered(TabId::P2P);
  Check(first.Hovered() == TabId::P2P && first.Selected() == TabId::Library, "hover does not select a tab");
  first.SetHovered(TabId::History);
  Check(first.Hovered() == TabId::History, "new hover replaces previous hover");
  first.SetHovered(TabId::Count);
  Check(first.Hovered() == TabId::Count && first.Selected() == TabId::Library, "leaving tab rail clears hover only");
  first.Select(TabId::Settings);
  first.MoveFocus(1);
  Check(first.ThemeFocused(), "Tab reaches the theme button after SETTINGS");
  const auto selectedBeforeThemeActivation = first.Selected();
  const auto themeBeforeActivation = first.Theme();
  Check(first.ActivateFocused() && first.Theme() != themeBeforeActivation && first.Selected() == selectedBeforeThemeActivation,
        "Enter or Space activation can toggle theme without changing tab");
  first.MoveFocus(-1);
  Check(first.Focus() == FocusTarget::TabRail && first.Focused() == TabId::Settings, "Shift+Tab returns focus to SETTINGS");
  first.Select(TabId::Web);
  first.MoveFocus(-1);
  Check(first.ThemeFocused(), "Shift+Tab reaches theme button from WEB");
  first.MoveFocus(1);
  Check(first.Focus() == FocusTarget::TabRail && first.Focused() == TabId::Web, "Tab returns from theme button to WEB");
  first.Select(TabId::Library);
  first.MoveTabFocus(1);
  Check(first.Focused() == TabId::History && first.Selected() == TabId::Library, "arrow-key tab focus does not activate a page");
  first.ActivateFocused();
  Check(first.Selected() == TabId::History, "Enter or Space activates focused tab");

  NavigationState localAudioNavigation;
  LocalAudioFocusRouter localAudioFocus;
  localAudioNavigation.Select(TabId::Inbox);
  Check(localAudioFocus.EnterButton(localAudioNavigation, false) && localAudioFocus.ButtonFocused(),
        "INBOX plus Tab enters OPEN LOCAL AUDIO");
  Check(localAudioFocus.EnterVolume(localAudioNavigation, false) && localAudioFocus.PreviewVolumeFocused(),
        "OPEN LOCAL AUDIO plus Tab enters PREVIEW VOLUME");
  Check(localAudioFocus.ExitButton(localAudioNavigation, false) && !localAudioFocus.PreviewVolumeFocused()
        && localAudioNavigation.Focused() == TabId::Library,
        "PREVIEW VOLUME plus Tab enters LIBRARY");
  localAudioNavigation.Select(TabId::Library);
  Check(localAudioFocus.EnterVolume(localAudioNavigation, true) && localAudioFocus.PreviewVolumeFocused(),
        "LIBRARY plus Shift+Tab enters PREVIEW VOLUME");
  Check(localAudioFocus.EnterButton(localAudioNavigation, true) && localAudioFocus.ButtonFocused(),
        "PREVIEW VOLUME plus Shift+Tab enters OPEN LOCAL AUDIO");
  Check(localAudioFocus.ExitButton(localAudioNavigation, true) && !localAudioFocus.ButtonFocused() && !localAudioFocus.PreviewVolumeFocused()
        && localAudioNavigation.Focused() == TabId::Inbox,
        "OPEN LOCAL AUDIO plus Shift+Tab enters INBOX");
  localAudioFocus.FocusButton(localAudioNavigation);
  localAudioFocus.SelectShellTab(localAudioNavigation, TabId::Settings);
  Check(!localAudioFocus.ButtonFocused() && localAudioNavigation.Selected() == TabId::Settings,
        "shell tab selection releases local-audio focus");
  localAudioNavigation.Select(TabId::Inbox); localAudioFocus.FocusButton(localAudioNavigation);
  localAudioFocus.FocusTheme(localAudioNavigation);
  Check(!localAudioFocus.ButtonFocused() && localAudioNavigation.ThemeFocused(),
        "theme focus releases local-audio focus");
  localAudioNavigation.Select(TabId::Inbox); localAudioFocus.FocusButton(localAudioNavigation);
  const auto focusedBeforeArrow = localAudioNavigation.Focused();
  Check(localAudioFocus.ConsumesArrowKeys() && localAudioFocus.ButtonFocused()
        && localAudioNavigation.Focused() == focusedBeforeArrow,
        "local-audio focus consumes arrows without moving the tab rail");

  CheckLayout(760.f, 540.f);
  CheckLayout(1024.f, 680.f);
  CheckLayout(1160.f, 760.f);
  CheckLayout(1440.f, 900.f);
  CheckLayout(320.f, 220.f);
  Check(CalculateShellLayout(1160.f, 760.f).widthClass == WidthClass::Wide, "wide layout breakpoint");
  Check(CalculateShellLayout(1024.f, 680.f).widthClass == WidthClass::Intermediate, "intermediate layout breakpoint");
  Check(CalculateShellLayout(320.f, 220.f).widthClass == WidthClass::Compact && CalculateShellLayout(320.f, 220.f).compactTabs,
        "compact layout uses reachable two-row tabs");
  CheckShellContrast(tokkebi::theme::ThemeMode::Dark);
  CheckShellContrast(tokkebi::theme::ThemeMode::Light);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
