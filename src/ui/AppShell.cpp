#include "AppShell.h"

#include "../app/fonts.h"
#include "../audio/waveform/WaveformUiState.h"
#include "text/Strings.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace tokkebi::ui
{
namespace
{
using namespace iplug::igraphics;

IColor ToIColor(theme::Color color) { return {color.a, color.r, color.g, color.b}; }
IRECT ToIRECT(const IRECT& origin, Rect rect) { return {origin.L + rect.x, origin.T + rect.y, origin.L + rect.x + rect.width, origin.T + rect.y + rect.height}; }
const char* ContainerName(audio::AudioContainer value) { return value == audio::AudioContainer::Wav ? "WAV" : value == audio::AudioContainer::Aiff ? "AIFF" : value == audio::AudioContainer::Flac ? "FLAC" : "UNKNOWN"; }
const char* CodecName(audio::AudioCodec value) { return value == audio::AudioCodec::Pcm ? "PCM" : value == audio::AudioCodec::IeeeFloat ? "IEEE FLOAT" : "UNKNOWN"; }
const char* SourceName(audio::SampleFormat value) { return value == audio::SampleFormat::Int16 ? "16-bit PCM" : value == audio::SampleFormat::Int24 ? "24-bit PCM" : value == audio::SampleFormat::Int32 ? "32-bit PCM" : value == audio::SampleFormat::Float32 ? "32-bit float" : "UNKNOWN"; }
std::string Duration(double seconds) { const auto millis = static_cast<uint64_t>(std::llround(seconds * 1000.0)); std::ostringstream text; text << std::setfill('0') << std::setw(2) << millis / 60000 << ':' << std::setw(2) << (millis / 1000) % 60 << '.' << std::setw(3) << millis % 1000; return text.str(); }
}

AppShell::AppShell(const IRECT& bounds)
: IControl(bounds)
{
  mDemoList.Model().SetRows({{"demo-a", "DEMO ROW A"}, {"demo-b", "DEMO ROW B"}, {"demo-c", "DEMO ROW C"}, {"demo-disabled", "DEMO ROW (DISABLED)", true}, {"demo-d", "DEMO ROW D"}});
  mDemoList.Model().SetViewportRows(3);
  mDemoProgress.Model().Set(components::ProgressState::Determinate, .42);
}
AppShell::~AppShell()
{
  mLocalAudioDialogMailbox->Close();
  if (GetUI())
    GetUI()->SetDisplayTickFunc(nullptr);
}

void AppShell::OnDisplayTick()
{
  bool changed = false;
  if (const auto selected = mLocalAudioDialogMailbox->Consume())
  {
    mWaveforms.Invalidate(++mWaveformGeneration);
    mWaveformViewport = {};
    mLocalAudioLoads.Begin(*selected, mLocalAudioDocument);
    changed = true;
  }
  if (mLocalAudioLoads.ApplyCompleted(mLocalAudioDocument))
  { if (mLocalAudioDocument.State() == audio::AudioDocumentState::Ready) { mWaveforms.Begin(mLocalAudioDocument.PcmSnapshot(), mWaveformGeneration); mWaveformViewport = audio::waveform::FullViewport(mLocalAudioDocument.Pcm().frames); } changed = true; }
  if (mWaveforms.Apply(mWaveformGeneration)) changed = true;
  if (changed)
    SetDirty(false);
}

ShellLayout AppShell::Layout() const { return CalculateShellLayout(mRECT.W(), mRECT.H()); }

Rect AppShell::TabBounds(const ShellLayout& layout, std::size_t index) const
{
  const std::size_t columns = layout.compactTabs ? 3 : kTabs.size();
  const std::size_t row = layout.compactTabs ? index / columns : 0;
  const std::size_t column = layout.compactTabs ? index % columns : index;
  const std::size_t rows = layout.compactTabs ? 2 : 1;
  const float width = columns == 0 ? 0.f : layout.tabRail.width / static_cast<float>(columns);
  const float height = rows == 0 ? 0.f : layout.tabRail.height / static_cast<float>(rows);
  return {layout.tabRail.x + width * static_cast<float>(column), layout.tabRail.y + height * static_cast<float>(row), width, height};
}

Rect AppShell::ThemeBounds(const ShellLayout& layout) const
{
  const float width = layout.header.width < 132.f ? layout.header.width : 132.f;
  const float inset = layout.header.height < 16.f ? layout.header.height / 2.f : 8.f;
  return {layout.header.x + layout.header.width - width + inset, layout.header.y + inset,
          ClampNonNegative(width - inset * 2.f), ClampNonNegative(layout.header.height - inset * 2.f)};
}

Rect AppShell::LocalAudioButtonBounds(const ShellLayout& layout) const
{
  const auto content = layout.contentPadding;
  return {content.x, content.y + 92.f, std::min(content.width, 190.f), 28.f};
}
Rect AppShell::WaveformBounds(const ShellLayout& layout) const
{
  const auto content = layout.contentPadding;
  return {content.x + 210.f, content.y + 92.f, std::max(0.f, content.width - 210.f), 170.f};
}

TabId AppShell::TabAt(float x, float y, const ShellLayout& layout) const
{
  const float localX = x - mRECT.L;
  const float localY = y - mRECT.T;
  for (std::size_t index = 0; index < kTabs.size(); ++index)
    if (TabBounds(layout, index).Contains(localX, localY))
      return kTabs[index];
  return TabId::Count;
}

void AppShell::Redraw() { SetDirty(false); }

void AppShell::Draw(IGraphics& graphics)
{
  const auto layout = Layout();
  const auto tokens = theme::Get(mState.Theme());
  const auto color = [](theme::Color value) { return ToIColor(value); };
  const auto local = [this](Rect rect) { return ToIRECT(mRECT, rect); };

  graphics.FillRect(color(tokens.app), mRECT);
  graphics.FillRect(color(tokens.secondary), local(layout.header));
  graphics.FillRect(color(tokens.raised), local(layout.tabRail));
  graphics.FillRect(color(tokens.primary), local(layout.content));
  graphics.FillRect(color(tokens.audio), local(layout.statusBar));
  graphics.DrawRect(color(tokens.borderStrong), mRECT, nullptr, theme::kBorderStructural);
  graphics.DrawLine(color(tokens.separator), mRECT.L, mRECT.T + layout.tabRail.y, mRECT.R, mRECT.T + layout.tabRail.y, nullptr, theme::kBorderThin);
  graphics.DrawLine(color(tokens.separator), mRECT.L, mRECT.T + layout.statusBar.y, mRECT.R, mRECT.T + layout.statusBar.y, nullptr, theme::kBorderThin);

  graphics.DrawText({24.f, color(tokens.textPrimary), fonts::kPrimary}, strings::kProductName.data(), local({16.f, 8.f, 180.f, layout.header.height - 16.f}));
  graphics.DrawText({13.f, color(tokens.textPrimary), fonts::kTechnical}, strings::kVersion.data(), local({194.f, 8.f, 80.f, layout.header.height - 16.f}));
  const auto themeBounds = ThemeBounds(layout);
  graphics.FillRect(color(tokens.actionDefault), local(themeBounds));
  graphics.DrawRect(color(mState.ThemeFocused() ? tokens.actionText : tokens.borderDefault), local(themeBounds), nullptr,
                    mState.ThemeFocused() ? theme::kBorderStandard : theme::kBorderThin);
  graphics.DrawText({13.f, color(tokens.actionText), fonts::kPrimary}, mState.Theme() == theme::ThemeMode::Dark ? strings::kThemeDark.data() : strings::kThemeLight.data(), local(themeBounds));

  for (std::size_t index = 0; index < kTabs.size(); ++index)
  {
    const auto tab = kTabs[index];
    const auto bounds = TabBounds(layout, index);
    const bool active = tab == mState.Selected();
    const bool focused = mState.Focus() == FocusTarget::TabRail && tab == mState.Focused();
    const bool hovered = tab == mState.Hovered();
    if (active)
      graphics.FillRect(color(tokens.actionDefault), local(bounds));
    else if (hovered)
      graphics.FillRect(color(tokens.navigationHover), local(bounds));
    if (focused)
      graphics.DrawRect(color(active ? tokens.actionText : tokens.focusRing), local(bounds).GetPadded(-2.f), nullptr, theme::kBorderStandard);
    graphics.DrawText({14.f, color(active ? tokens.actionText : (hovered ? tokens.navigationHoverText : tokens.textPrimary)), fonts::kPrimary}, strings::TabLabel(tab).data(), local(bounds));
  }

  const auto content = layout.contentPadding;
  graphics.DrawText({24.f, color(tokens.textPrimary), fonts::kPrimary}, strings::TabLabel(mState.Selected()).data(), local({content.x, content.y, content.width, 34.f}));
  graphics.DrawLine(color(tokens.separator), mRECT.L + content.x, mRECT.T + content.y + 42.f, mRECT.L + content.x + content.width, mRECT.T + content.y + 42.f, nullptr, theme::kBorderThin);
  graphics.DrawText({14.f, color(tokens.textPrimary), fonts::kBody}, strings::SectionMessage(mState.Selected()).data(), local({content.x, content.y + 58.f, content.width, content.height > 58.f ? content.height - 58.f : 0.f}));
  if (mState.Selected() == TabId::Inbox)
  {
    const auto button = LocalAudioButtonBounds(layout);
    mOpenLocalAudioButton.SetBounds(button);
    mOpenLocalAudioButton.SetEnabled(true);
    mOpenLocalAudioButton.Draw(graphics, mRECT, tokens, mLocalAudioFocus.ButtonFocused());
    std::string status = "NO LOCAL AUDIO SELECTED.";
    const auto waveformState = mWaveforms.Status();
    const auto uiState = audio::waveform::SelectUiState(mLocalAudioDocument.State(), waveformState);
    if (mLocalAudioDocument.State() == audio::AudioDocumentState::Ready)
    {
      const auto& info = mLocalAudioDocument.Info();
      const auto filename = info.path.filename().u8string();
      status = "FILE      " + std::string(filename.begin(), filename.end()) + "\nCONTAINER " + ContainerName(info.container) + "\nCODEC     " + CodecName(info.codec) + "\nSOURCE    " + SourceName(info.sourceFormat) + "\nCHANNELS  " + std::to_string(info.channels) + (info.channels == 2 ? " (Stereo)" : "") + "\nRATE      " + std::to_string(info.sampleRate) + " Hz\nFRAMES    " + std::to_string(info.frames) + "\nDURATION  " + Duration(info.durationSeconds);
    }
    if (uiState == audio::waveform::UiState::AudioLoading)
      status = "LOADING LOCAL AUDIO…";
    else if (uiState == audio::waveform::UiState::AudioFailed)
      status = "LOAD FAILED\n" + mLocalAudioDocument.Diagnostic();
    else if (uiState == audio::waveform::UiState::WaveformBuilding)
      status += "\nBUILDING WAVEFORM…";
    else if (uiState == audio::waveform::UiState::WaveformFailed)
      status += "\nWAVEFORM UNAVAILABLE\n" + mWaveforms.Diagnostic();
    graphics.DrawText({12.f, color(tokens.textSecondary), fonts::kTechnical}, status.c_str(), local({content.x, content.y + 128.f, content.width, 154.f}));
    if (const auto cache = mWaveforms.Cache()) { const Rect canvas=WaveformBounds(layout); graphics.FillRect(color(tokens.waveBackground), local(canvas)); const auto view=mWaveformViewport.Empty()?audio::waveform::FullViewport(cache->frames):mWaveformViewport; const auto plan=audio::waveform::BuildRenderPlan(*cache,view,static_cast<uint32_t>(canvas.width)); for(uint32_t ch=0;ch<cache->channels;++ch){const float mid=canvas.y+canvas.height*(ch+.5f)/cache->channels;graphics.DrawLine(color(tokens.separator),mRECT.L+canvas.x,mRECT.T+mid,mRECT.L+canvas.x+canvas.width,mRECT.T+mid,nullptr,1.f);graphics.DrawText({10.f,color(tokens.technicalOnAudio),fonts::kTechnical},cache->channels==1?"MONO":(ch==0?"L":"R"),local({canvas.x+3.f,mid-9.f,25.f,16.f}));} for(const auto&p:plan.columns){const float mid=canvas.y+canvas.height*(p.channel+.5f)/cache->channels,half=canvas.height/cache->channels/2.f;graphics.DrawLine(color(tokens.wavePrimary),mRECT.L+canvas.x+static_cast<float>(p.x),mRECT.T+mid-p.maximum*half,mRECT.L+canvas.x+static_cast<float>(p.x),mRECT.T+mid-p.minimum*half,nullptr,1.f);} for(const auto&tick:audio::waveform::BuildTimeRuler(view,mLocalAudioDocument.Info().sampleRate,static_cast<uint32_t>(canvas.width))){graphics.DrawLine(color(tokens.separator),mRECT.L+canvas.x+static_cast<float>(tick.x),mRECT.T+canvas.y+canvas.height-8.f,mRECT.L+canvas.x+static_cast<float>(tick.x),mRECT.T+canvas.y+canvas.height,nullptr,1.f);graphics.DrawText({9.f,color(tokens.technicalOnAudio),fonts::kTechnical},tick.label.c_str(),local({canvas.x+static_cast<float>(tick.x),canvas.y+canvas.height-20.f,78.f,12.f}));} }
    graphics.DrawText({11.f, color(tokens.textSecondary), fonts::kTechnical}, "WAV/WAVE local waveform only. Wheel: zoom, Shift+wheel: scroll, F: full view. No playback, selection, trim or export.", local({content.x, content.y + 286.f, content.width, 24.f}));
  }
  if (mState.Selected() == TabId::Settings)
    DrawComponentDemo(graphics, layout);

  const float third = layout.statusBar.width / 3.f;
  graphics.DrawText({12.f, color(tokens.technicalOnAudio), fonts::kTechnical}, strings::kStatusJobs.data(), local({8.f, layout.statusBar.y, third - 8.f, layout.statusBar.height}));
  graphics.DrawText({12.f, color(tokens.technicalOnAudio), fonts::kTechnical}, strings::kStatusHelper.data(), local({third + 8.f, layout.statusBar.y, third - 8.f, layout.statusBar.height}));
  graphics.DrawText({12.f, color(tokens.technicalOnAudio), fonts::kTechnical}, strings::kStatusLibrary.data(), local({third * 2.f + 8.f, layout.statusBar.y, third - 8.f, layout.statusBar.height}));
}

void AppShell::DrawComponentDemo(IGraphics& graphics, const ShellLayout& layout)
{
  const auto tokens = theme::Get(mState.Theme());
  const auto color = [](theme::Color value) { return ToIColor(value); };
  const auto local = [this](Rect rect) { return ToIRECT(mRECT, rect); };
  const auto c = layout.contentPadding;
  const float top = c.y + 92.f;
  const float width = std::min(c.width, 510.f);
  if (width < 180.f || c.height < 130.f) return;
  const auto demo = Rect {c.x, top, width, std::max(0.f, c.height - 96.f)};
  graphics.FillRect(color(tokens.raised), local(demo));
  graphics.DrawRect(color(tokens.borderDefault), local(demo), nullptr, theme::kBorderThin);
  graphics.DrawText({13.f, color(tokens.textAccent), fonts::kPrimary}, "TEMPORARY UI COMPONENT DEMO — NOT SAVED", local({c.x + 8.f, top + 4.f, width - 16.f, 22.f}));
  const auto input = Rect {c.x + 8.f, top + 32.f, width - 16.f, 26.f};
  const bool textFocus = mDemoFocus.Owner() == components::FocusOwner::Text;
  mDemoText.SetBounds(input); mDemoText.Draw(graphics, mRECT, tokens, textFocus);
  const auto action = Rect {c.x + 8.f, top + 66.f, 136.f, 25.f};
  mNotificationButton.SetBounds(action); mNotificationButton.Draw(graphics, mRECT, tokens, mComponentFocus.Current() == components::ComponentFocus::NotificationButton);
  const auto clear = Rect {c.x + 152.f, top + 66.f, 110.f, 25.f};
  mClearButton.SetBounds(clear); mClearButton.SetEnabled(!mDemoText.Model().Text().empty()); mClearButton.Draw(graphics, mRECT, tokens, mComponentFocus.Current() == components::ComponentFocus::ClearButton);
  const auto list = Rect {c.x + 8.f, top + 98.f, width - 16.f, 70.f};
  mDemoList.SetBounds(list); mDemoList.Draw(graphics, mRECT, tokens, mComponentFocus.Current() == components::ComponentFocus::List);
  const auto progress = Rect {c.x + 8.f, top + 176.f, width - 16.f, 12.f};
  mDemoProgress.SetBounds(progress); mDemoProgress.Draw(graphics, mRECT, tokens);
  graphics.DrawText({11.f, color(tokens.textSecondary), fonts::kTechnical}, "DEMO PROGRESS: 42% (not a job)", local({progress.x, progress.y + 12.f, progress.width, 18.f}));
  const auto note = Rect {c.x + 8.f, top + 208.f, width - 16.f, 24.f}; mDemoNotification.SetBounds(note); mDemoNotification.Draw(graphics, mRECT, tokens, mComponentFocus.Current() == components::ComponentFocus::NotificationAction);
  if (mDemoModal.Open())
  {
    const auto modal = Rect {c.x + 18.f, top + 48.f, width - 36.f, 110.f};
    graphics.FillRect(color(tokens.overlay), local(demo));
    graphics.FillRect(color(tokens.secondary), local(modal));
    graphics.DrawRect(color(tokens.focusRing), local(modal), nullptr, theme::kBorderStandard);
    graphics.DrawText({15.f, color(tokens.textPrimary), fonts::kPrimary}, "Discard temporary text?", local({modal.x + 10.f, modal.y + 8.f, modal.width - 20.f, 25.f}));
    graphics.DrawText({12.f, color(tokens.textSecondary), fonts::kBody}, "This affects only this in-memory demo.", local({modal.x + 10.f, modal.y + 34.f, modal.width - 20.f, 22.f}));
    const Rect confirm {modal.x + 10.f, modal.y + 66.f, 125.f, 26.f}; const Rect cancel {modal.x + 145.f, modal.y + 66.f, 120.f, 26.f};
    graphics.FillRect(color(tokens.actionDefault), local(confirm)); graphics.DrawRect(color(mModalConfirmFocused ? tokens.focusRing : tokens.borderDefault), local(confirm), nullptr, theme::kBorderThin); graphics.DrawText({12.f, color(tokens.actionText), fonts::kPrimary}, "CONFIRM", local(confirm));
    graphics.FillRect(color(tokens.raised), local(cancel)); graphics.DrawRect(color(!mModalConfirmFocused ? tokens.focusRing : tokens.borderDefault), local(cancel), nullptr, theme::kBorderThin); graphics.DrawText({12.f, color(tokens.textPrimary), fonts::kPrimary}, "CANCEL", local(cancel));
  }
}

void AppShell::OnMouseDown(float x, float y, const IMouseMod&)
{
  const auto layout = Layout();
  const float localX = x - mRECT.L;
  const float localY = y - mRECT.T;
  if (mState.Selected() == TabId::Inbox && LocalAudioButtonBounds(layout).Contains(localX, localY))
  {
    mLocalAudioFocus.FocusButton(mState);
    if (mOpenLocalAudioButton.OnMouseDown(localX, localY)) PromptForLocalAudio();
    return;
  }
  if (mState.Selected() == TabId::Settings)
  {
    const auto c = layout.contentPadding; const float top = c.y + 92.f; const float width = std::min(c.width, 510.f);
    if (mDemoModal.Open())
    {
      const Rect modal {c.x + 18.f, top + 48.f, width - 36.f, 110.f};
      const Rect confirm {modal.x + 10.f, modal.y + 66.f, 125.f, 26.f}; const Rect cancel {modal.x + 145.f, modal.y + 66.f, 120.f, 26.f};
      if (confirm.Contains(localX, localY)) { mDemoText.Model().SetText({}); mDemoModal.Confirm(); mComponentFocus.Set(components::ComponentFocus::NotificationButton); mDemoFocus.Set(components::FocusOwner::Component); }
      else if (cancel.Contains(localX, localY)) { mDemoModal.Cancel(); mComponentFocus.Set(mFocusBeforeModal); mDemoFocus.Set(components::FocusOwner::Component); }
      Redraw(); return;
    }
    const Rect input {c.x + 8.f, top + 32.f, width - 16.f, 26.f};
    const Rect action {c.x + 8.f, top + 66.f, 136.f, 25.f}; const Rect clear {c.x + 152.f, top + 66.f, 110.f, 25.f}; const Rect list {c.x + 8.f, top + 98.f, width - 16.f, 70.f};
    if (mDemoText.Hit(localX, localY)) { mDemoFocus.Set(components::FocusOwner::Text); mComponentFocus.Set(components::ComponentFocus::TextField); Redraw(); return; }
    if (mNotificationButton.OnMouseDown(localX, localY)) { ShowDemoNotification(); mDemoFocus.Set(components::FocusOwner::Component); mComponentFocus.Set(components::ComponentFocus::NotificationButton); Redraw(); return; }
    if (mClearButton.OnMouseDown(localX, localY)) { mFocusBeforeModal = components::ComponentFocus::ClearButton; mDemoModal.Show(); mDemoFocus.Set(components::FocusOwner::Modal); mModalConfirmFocused = false; Redraw(); return; }
    if (mDemoList.OnMouseDown(localX, localY)) { mDemoFocus.Set(components::FocusOwner::Component); mComponentFocus.Set(components::ComponentFocus::List); Redraw(); return; }
    if (mDemoNotification.OnMouseDown(localX, localY)) { Redraw(); return; }
  }
  if (ThemeBounds(layout).Contains(localX, localY))
  {
    mLocalAudioFocus.FocusTheme(mState);
    mState.ToggleTheme();
  }
  else
  {
    const auto tab = TabAt(x, y, layout);
    if (IsValid(tab))
      mLocalAudioFocus.SelectShellTab(mState, tab);
  }
  Redraw();
}

void AppShell::OnMouseWheel(float x, float y, const IMouseMod& mod, float distance)
{
  IControl::OnMouseWheel(x, y, mod, distance);
  if (mState.Selected() == TabId::Inbox && WaveformBounds(Layout()).Contains(x - mRECT.L, y - mRECT.T) && !mWaveformViewport.Empty())
  {
    if (mod.S) mWaveformViewport = audio::waveform::Scroll(mWaveformViewport, static_cast<int64_t>(-distance * static_cast<float>(mWaveformViewport.end - mWaveformViewport.start) / 8.f));
    else { const auto cache=mWaveforms.Cache(); const uint64_t minimum=cache?audio::waveform::MinimumViewportSpan(*cache):mWaveformViewport.totalFrames; mWaveformViewport = audio::waveform::Zoom(mWaveformViewport, distance > 0.f ? 1.25 : .8, (x - mRECT.L - WaveformBounds(Layout()).x) / WaveformBounds(Layout()).width, minimum); }
    Redraw(); return;
  }
  if (mState.Selected() != TabId::Settings || mDemoModal.Open()) return;
  const auto c = Layout().contentPadding; const auto list = Rect {c.x + 8.f, c.y + 190.f, std::min(c.width, 510.f) - 16.f, 70.f};
  if (list.Contains(x - mRECT.L, y - mRECT.T)) { mDemoList.OnWheel(distance); mDemoFocus.Set(components::FocusOwner::Component); mComponentFocus.Set(components::ComponentFocus::List); Redraw(); }
}

void AppShell::OnMouseOver(float x, float y, const IMouseMod& mod)
{
  IControl::OnMouseOver(x, y, mod);
  const auto next = TabAt(x, y, Layout());
  if (mState.SetHovered(next))
    Redraw();
}

void AppShell::OnMouseOut()
{
  IControl::OnMouseOut();
  if (mState.SetHovered(TabId::Count))
    Redraw();
}

bool AppShell::OnKeyDown(float, float, const iplug::IKeyPress& key) { return HandleKey(key); }

bool AppShell::HandleKey(const iplug::IKeyPress& key)
{
  mLocalAudioFocus.EnforcePageContext(mState);
  if (HandleLocalAudioKey(key)) return true;
  if (mState.Selected() == TabId::Settings && HandleDemoKey(key)) return true;
  if (key.C || key.A)
    return false;
  if (key.VK == iplug::kVK_TAB)
  {
    mState.MoveFocus(key.S ? -1 : 1);
    Redraw();
    return true;
  }
  if (key.VK == iplug::kVK_LEFT || key.VK == iplug::kVK_UP)
  {
    mState.MoveTabFocus(-1);
    Redraw();
    return true;
  }
  if (key.VK == iplug::kVK_RIGHT || key.VK == iplug::kVK_DOWN)
  {
    mState.MoveTabFocus(1);
    Redraw();
    return true;
  }
  if (key.VK == iplug::kVK_RETURN || key.VK == iplug::kVK_SPACE)
  {
    mState.ActivateFocused();
    Redraw();
    return true;
  }
  return false;
}

bool AppShell::HandleLocalAudioKey(const iplug::IKeyPress& key)
{
  if (mState.Selected() == TabId::Inbox && (key.VK == 'F' || key.VK == 'f') && !mWaveformViewport.Empty()) { mWaveformViewport = audio::waveform::FullViewport(mWaveformViewport.totalFrames); Redraw(); return true; }
  if (mLocalAudioFocus.ButtonFocused())
  {
    if (key.VK == iplug::kVK_RETURN || key.VK == iplug::kVK_SPACE) { if (mOpenLocalAudioButton.OnKey(key)) PromptForLocalAudio(); Redraw(); return true; }
    if (key.VK == iplug::kVK_TAB)
    {
      mLocalAudioFocus.ExitButton(mState, key.S);
      Redraw();
      return true;
    }
    if (key.VK == iplug::kVK_LEFT || key.VK == iplug::kVK_RIGHT || key.VK == iplug::kVK_UP || key.VK == iplug::kVK_DOWN)
      return true;
    return false;
  }
  if (key.VK == iplug::kVK_TAB && mLocalAudioFocus.EnterButton(mState, key.S))
  {
    Redraw();
    return true;
  }
  return false;
}

bool AppShell::HandleDemoKey(const iplug::IKeyPress& key)
{
  if (mDemoFocus.Owner() == components::FocusOwner::Shell && !(key.VK == iplug::kVK_TAB && mState.ThemeFocused())) return false;
  if (mDemoModal.Open())
  {
    if (key.VK == iplug::kVK_TAB) { mModalConfirmFocused = !mModalConfirmFocused; Redraw(); return true; }
    if (key.VK == iplug::kVK_ESCAPE) { mDemoModal.Cancel(); mComponentFocus.Set(mFocusBeforeModal); mDemoFocus.Set(components::FocusOwner::Component); Redraw(); return true; }
    if (key.VK == iplug::kVK_RETURN || key.VK == iplug::kVK_SPACE) { if (mModalConfirmFocused) { mDemoText.Model().SetText({}); mDemoModal.Confirm(); mComponentFocus.Set(components::ComponentFocus::NotificationButton); } else { mDemoModal.Cancel(); mComponentFocus.Set(mFocusBeforeModal); } mDemoFocus.Set(components::FocusOwner::Component); Redraw(); return true; }
    return true;
  }
  if (key.VK == iplug::kVK_TAB)
  {
    if (mDemoFocus.Owner() == components::FocusOwner::Shell) { const auto entry = key.S ? (mDemoNotification.HasAction() ? components::ComponentFocus::NotificationAction : components::ComponentFocus::List) : components::ComponentFocus::TextField; mComponentFocus.Set(entry); mDemoFocus.Set(entry == components::ComponentFocus::TextField ? components::FocusOwner::Text : components::FocusOwner::Component); Redraw(); return true; }
    const auto next = mComponentFocus.Advance(key.S, mClearButton.Enabled(), mDemoNotification.HasAction());
    mComponentFocus.Set(next);
    if (next == components::ComponentFocus::None) mDemoFocus.Set(components::FocusOwner::Shell);
    else mDemoFocus.Set(next == components::ComponentFocus::TextField ? components::FocusOwner::Text : components::FocusOwner::Component);
    Redraw(); return true;
  }
  if (mDemoFocus.Owner() == components::FocusOwner::Component && mComponentFocus.Current() == components::ComponentFocus::List && (key.VK == iplug::kVK_UP || key.VK == iplug::kVK_DOWN))
  {
    mDemoList.OnKey(key); Redraw(); return true;
  }
  if (mDemoFocus.Owner() == components::FocusOwner::Component && (mComponentFocus.Current() == components::ComponentFocus::NotificationButton || mComponentFocus.Current() == components::ComponentFocus::ClearButton) && (key.VK == iplug::kVK_RETURN || key.VK == iplug::kVK_SPACE))
  {
    if (mComponentFocus.Current() == components::ComponentFocus::NotificationButton) { mNotificationButton.OnKey(key); ShowDemoNotification(); }
    else if (mClearButton.OnKey(key)) { mFocusBeforeModal = components::ComponentFocus::ClearButton; mDemoModal.Show(); mDemoFocus.Set(components::FocusOwner::Modal); mModalConfirmFocused = false; }
    Redraw(); return true;
  }
  if (mDemoFocus.Owner() == components::FocusOwner::Component && mComponentFocus.Current() == components::ComponentFocus::NotificationAction && mDemoNotification.OnKey(key)) { Redraw(); return true; }
  if (mDemoFocus.Owner() != components::FocusOwner::Text || key.A) return false;
  if (key.C && (key.VK == 'C' || key.VK == 'c'))
  {
    GetUI()->SetTextInClipboard(mDemoText.Model().SelectedText().c_str()); return true;
  }
  if (key.C && (key.VK == 'V' || key.VK == 'v'))
  {
    WDL_String clipboard; if (GetUI()->GetTextFromClipboard(clipboard)) mDemoText.Model().Insert(clipboard.Get()); Redraw(); return true;
  }
  if (key.C) return false;
  bool handled = true;
  if (key.VK == iplug::kVK_BACK) mDemoText.Model().Backspace();
  else if (key.VK == iplug::kVK_DELETE) mDemoText.Model().Delete();
  else if (key.VK == iplug::kVK_LEFT) mDemoText.Model().MoveLeft(key.S);
  else if (key.VK == iplug::kVK_RIGHT) mDemoText.Model().MoveRight(key.S);
  else if (key.VK == iplug::kVK_HOME) mDemoText.Model().Home(key.S);
  else if (key.VK == iplug::kVK_END) mDemoText.Model().End(key.S);
  else if (key.VK == iplug::kVK_ESCAPE) { mDemoFocus.Set(components::FocusOwner::Component); }
  else if (key.VK == iplug::kVK_RETURN) { mDemoFocus.Set(components::FocusOwner::Component); }
  else if (key.utf8 && key.utf8[0] != '\0') mDemoText.Model().Insert(key.utf8);
  else handled = false;
  if (handled) Redraw();
  return handled;
}

void AppShell::ShowDemoNotification()
{
  mDemoNotification.Model().Show({components::NotificationSeverity::Success, "Controlled demo notification", "CHANGE MESSAGE", [this] { mDemoNotification.Model().Show({components::NotificationSeverity::Information, "Action completed in memory", {}, {}}); }});
}

void AppShell::PromptForLocalAudio()
{
  WDL_String fileName;
  WDL_String path;
  const auto mailbox = mLocalAudioDialogMailbox;
  GetUI()->PromptForFile(fileName, path, EFileAction::Open, "wav wave", [mailbox](const WDL_String& selected, const WDL_String&) {
    if (selected.GetLength() == 0)
      return;
    mailbox->Publish(selected.Get());
  });
}
}
