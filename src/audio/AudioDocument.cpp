#include "AudioDocument.h"
#include <utility>
namespace tokkebi::audio {
namespace { const DecodedPcm kEmptyPcm {}; }
const DecodedPcm& AudioDocument::Pcm() const { return mPcm ? *mPcm : kEmptyPcm; }
void AudioDocument::Clear() { mState = AudioDocumentState::Empty; mInfo = {}; mPcm.reset(); mError = DecodeErrorCode::None; mDiagnostic.clear(); }
void AudioDocument::BeginLoad() { mState = AudioDocumentState::Loading; mInfo = {}; mPcm.reset(); mError = DecodeErrorCode::None; mDiagnostic.clear(); }
void AudioDocument::Complete(AudioLoadResult result)
{
  mInfo = std::move(result.info);
  mError = result.error;
  mDiagnostic = std::move(result.diagnostic);
  if (result.Success())
  {
    mPcm = std::make_shared<DecodedPcm>(std::move(result.audio));
    mState = AudioDocumentState::Ready;
  }
  else
  {
    // Keep failure diagnostics and metadata, never decoded sample storage.
    mPcm.reset();
    mState = AudioDocumentState::Failed;
  }
}
void AudioDocument::LoadLocalFile(const std::filesystem::path& path) { BeginLoad(); Complete(DecodeLocalAudio(path)); }
}
