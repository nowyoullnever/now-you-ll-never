#include "SelectionState.h"
#include <algorithm>
#include <charconv>
#include <iomanip>
#include <limits>
#include <sstream>
namespace tokkebi::audio::editor {
namespace { bool Valid(const std::optional<Selection>& s,uint64_t total){return !s||(s->inFrame<s->outFrame&&s->outFrame<=total);} uint64_t Move(uint64_t value,int64_t delta){if(delta>=0)return static_cast<uint64_t>(delta)>std::numeric_limits<uint64_t>::max()-value?std::numeric_limits<uint64_t>::max():value+static_cast<uint64_t>(delta);uint64_t amount=static_cast<uint64_t>(-(delta+1))+1;return amount>value?0:value-amount;} }
std::optional<TimeValue> ParseTime(const std::string& text){if(text.size()<12)return{};size_t a=text.find(':'),b=a==std::string::npos?std::string::npos:text.find(':',a+1),c=b==std::string::npos?std::string::npos:text.find('.',b+1);if(a==std::string::npos||b==std::string::npos||c==std::string::npos||text.size()!=c+4)return{};uint64_t h{},m{},s{},ms{};auto parse=[](const std::string&v,uint64_t&out){return !v.empty()&&std::from_chars(v.data(),v.data()+v.size(),out).ec==std::errc{};};if(!parse(text.substr(0,a),h)||!parse(text.substr(a+1,b-a-1),m)||!parse(text.substr(b+1,c-b-1),s)||!parse(text.substr(c+1),ms)||m>=60||s>=60||ms>=1000||h>(std::numeric_limits<uint64_t>::max()/3600000))return{};return TimeValue{h*3600000+m*60000+s*1000+ms};}
std::string FormatTime(uint64_t f,uint32_t r){uint64_t seconds=r?f/r:0,rem=r?f%r:0,h=seconds/3600,m=(seconds/60)%60,s=seconds%60,ms=r?(rem*1000)/r:0;std::ostringstream o;o<<std::setfill('0')<<std::setw(2)<<h<<':'<<std::setw(2)<<m<<':'<<std::setw(2)<<s<<'.'<<std::setw(3)<<ms;return o.str();}
std::optional<uint64_t> MillisecondsToFrame(uint64_t ms,uint32_t rate){if(!rate||ms>std::numeric_limits<uint64_t>::max()/rate)return{};return (ms*rate+500)/1000;}
uint64_t FrameToMilliseconds(uint64_t frame,uint32_t rate){return rate?(frame/rate)*1000+((frame%rate)*1000)/rate:0;}
SelectionIntersection Intersect(Selection s,uint64_t start,uint64_t end){return{std::max(s.inFrame,start),std::min(s.outFrame,end)};}
void SelectionState::ResetHistory(){mUndo.clear();mRedo.clear();}
void SelectionState::SetSource(uint64_t total,uint32_t rate){mTotalFrames=total;mSampleRate=rate;mCurrent.reset();ResetHistory();}
void SelectionState::Commit(std::optional<Selection> next){if(!Valid(next,mTotalFrames)||next==mCurrent)return;mUndo.push_back(mCurrent);if(mUndo.size()>kHistoryCap)mUndo.erase(mUndo.begin());mCurrent=next;mRedo.clear();}
bool SelectionState::Select(uint64_t a,uint64_t b){if(!mTotalFrames)return false;auto in=std::min(a,b),out=std::max(a,b);if(in>=mTotalFrames)return false;out=std::min(out,mTotalFrames);if(in==out)return false;Commit(Selection{in,out});return true;}
bool SelectionState::SelectAll(){if(!mTotalFrames)return false;Commit(Selection{0,mTotalFrames});return true;} void SelectionState::Clear(){Commit({});}
bool SelectionState::MoveIn(int64_t d){if(!mCurrent)return false;auto n=Move(mCurrent->inFrame,d);if(n>=mCurrent->outFrame)return false;Commit(Selection{n,mCurrent->outFrame});return true;}
bool SelectionState::MoveOut(int64_t d){if(!mCurrent)return false;auto n=Move(mCurrent->outFrame,d);if(n<=mCurrent->inFrame||n>mTotalFrames)return false;Commit(Selection{mCurrent->inFrame,n});return true;}
bool SelectionState::SetInText(const std::string&t){auto time=ParseTime(t);auto frame=time?MillisecondsToFrame(time->milliseconds,mSampleRate):std::nullopt;return frame&&mCurrent&&*frame<mCurrent->outFrame?Select(*frame,mCurrent->outFrame):false;}
bool SelectionState::SetOutText(const std::string&t){auto time=ParseTime(t);auto frame=time?MillisecondsToFrame(time->milliseconds,mSampleRate):std::nullopt;return frame&&mCurrent&&*frame>mCurrent->inFrame&&*frame<=mTotalFrames?Select(mCurrent->inFrame,*frame):false;}
bool SelectionState::Undo(){if(mUndo.empty())return false;mRedo.push_back(mCurrent);mCurrent=mUndo.back();mUndo.pop_back();return true;} bool SelectionState::Redo(){if(mRedo.empty())return false;mUndo.push_back(mCurrent);mCurrent=mRedo.back();mRedo.pop_back();return true;}
}
