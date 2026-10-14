#pragma once
#include "AudioDecoder.h"
#include <memory>
namespace tokkebi::audio {
enum class AudioDocumentState { Empty, Loading, Ready, Failed };
class AudioDocument final {
public:
  AudioDocumentState State() const { return mState; } const AudioFileInfo& Info() const { return mInfo; }
  const DecodedPcm& Pcm() const;
  std::shared_ptr<const DecodedPcm> PcmSnapshot() const { return mPcm; }
  DecodeErrorCode Error() const { return mError; } const std::string& Diagnostic() const { return mDiagnostic; }
  void Clear(); void BeginLoad(); void Complete(AudioLoadResult result); void LoadLocalFile(const std::filesystem::path& path);
private:
  AudioDocumentState mState = AudioDocumentState::Empty; AudioFileInfo mInfo; std::shared_ptr<const DecodedPcm> mPcm; DecodeErrorCode mError = DecodeErrorCode::None; std::string mDiagnostic;
}; }
