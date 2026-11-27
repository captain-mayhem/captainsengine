#include "stateframes.h"

namespace sd{

namespace{

const size_t kStates = 10;
const size_t kCharacterStates = 36;

template<typename T>
void ensure(std::vector<T>& list, size_t index){
  if (list.size() <= index)
    list.resize(index+1);
}

const char* const kCharacterStateNames[16] = {
  "Stand front", "Stand back", "Stand right",
  "Walk front", "Walk back", "Walk right",
  "Talk, Stand front", "Talk, Stand back", "Talk, Stand right",
  "Talk, Walk front", "Talk, Walk back", "Talk, Walk right",
  "Bored 1", "Bored 2", "Pickup front", "Pickup right",
};

}

ItemFrames::ItemFrames(Item& item) : item_(item){
  if (item_.states.size() < kStates)
    item_.states.resize(kStates);
}

bool ItemFrames::frameExists(int state, int frame) const{
  return frame < (int)item_.states[state].frames.size();
}

std::vector<std::string> ItemFrames::frame(int state, int frame) const{
  const ItemState& st = item_.states[state];
  if (frame >= (int)st.frames.size() || st.frames[frame].empty())
    return {};
  return {st.frames[frame]};
}

void ItemFrames::setFramePart(int state, int frame, int part, const std::string& name){
  ensure(item_.states[state].frames, frame);
  item_.states[state].frames[frame] = name;
}

std::string ItemFrames::script(int state, int frame) const{
  const ItemState& st = item_.states[state];
  return frame < (int)st.scripts.size() ? st.scripts[frame] : "";
}

void ItemFrames::setScript(int state, int frame, const std::string& text){
  ensure(item_.states[state].scripts, frame);
  item_.states[state].scripts[frame] = text;
}

CursorFrames::CursorFrames(std::vector<CursorState>& cursor) : cursor_(cursor){
  while (cursor_.size() < kStates){
    CursorState cs;
    //the last state is the loading icon (command 2)
    cs.command = cursor_.size() == kStates-1 ? 2 : 1;
    cursor_.push_back(cs);
  }
}

bool CursorFrames::frameExists(int state, int frame) const{
  return frame < (int)cursor_[state].frames.size();
}

std::vector<std::string> CursorFrames::frame(int state, int frame) const{
  const CursorState& cs = cursor_[state];
  if (frame >= (int)cs.frames.size() || cs.frames[frame].empty())
    return {};
  return {cs.frames[frame]};
}

void CursorFrames::setFramePart(int state, int frame, int part, const std::string& name){
  ensure(cursor_[state].frames, frame);
  cursor_[state].frames[frame] = name;
}

ExtendedFrame& ExtendedFrames::frameAt(int state, int frame){
  std::vector<ExtendedFrame>& frms = framesOf(state);
  ensure(frms, frame);
  return frms[frame];
}

bool ExtendedFrames::frameExists(int state, int frame) const{
  const std::vector<ExtendedFrame>& frms = framesOf(state);
  if (frame >= (int)frms.size())
    return false;
  if (emptyFramesExist_)
    return true;
  for (const std::string& name : frms[frame].names){
    if (!name.empty())
      return true;
  }
  return false;
}

std::vector<std::string> ExtendedFrames::frame(int state, int frame) const{
  const std::vector<ExtendedFrame>& frms = framesOf(state);
  if (frame >= (int)frms.size())
    return {};
  return frms[frame].names;
}

void ExtendedFrames::setFramePart(int state, int frame, int part, const std::string& name){
  ExtendedFrame& frm = frameAt(state, frame);
  ensure(frm.names, part);
  frm.offsets.resize(frm.names.size());
  frm.names[part] = name;
}

Vec2i ExtendedFrames::offset(int state, int frame, int part) const{
  const std::vector<ExtendedFrame>& frms = framesOf(state);
  if (frame >= (int)frms.size() || part >= (int)frms[frame].offsets.size())
    return Vec2i{0, 0};
  return frms[frame].offsets[part];
}

void ExtendedFrames::setOffset(int state, int frame, int part, Vec2i offset){
  ExtendedFrame& frm = frameAt(state, frame);
  ensure(frm.offsets, part);
  if (frm.names.size() < frm.offsets.size())
    frm.names.resize(frm.offsets.size());
  frm.offsets[part] = offset;
}

std::string ExtendedFrames::script(int state, int frame) const{
  const std::vector<ExtendedFrame>& frms = framesOf(state);
  return frame < (int)frms.size() ? frms[frame].script : "";
}

void ExtendedFrames::setScript(int state, int frame, const std::string& text){
  frameAt(state, frame).script = text;
}

ObjectFrames::ObjectFrames(Object& obj) : obj_(obj){
  if (obj_.states.size() < kStates)
    obj_.states.resize(kStates);
}

CharacterFrames::CharacterFrames(Character& chr) : chr_(chr){
  //the C# editor counts a character's frame only when it has an image
  emptyFramesExist_ = false;
  if (chr_.states.size() < kCharacterStates)
    chr_.states.resize(kCharacterStates);
  if (chr_.extraStateNames.size() < kCharacterStates-16)
    chr_.extraStateNames.resize(kCharacterStates-16);
}

bool frameEmpty(const StateFrames& data, int state, int frame){
  for (const std::string& name : data.frame(state, frame)){
    if (!name.empty())
      return false;
  }
  return true;
}

bool allFramesEmpty(const StateFrames& data){
  for (int s = 0; s < data.states(); ++s){
    for (int f = 0; f < data.frames(); ++f){
      if (!frameEmpty(data, s, f))
        return false;
    }
  }
  return true;
}

void copyFrame(StateFrames& data, int state, int from, int to){
  if (from == to)
    return;
  std::vector<std::string> parts = data.frame(state, from);
  for (int p = 0; p < (int)parts.size(); ++p){
    data.setFramePart(state, to, p, parts[p]);
    data.setOffset(state, to, p, data.offset(state, from, p));
  }
}

void moveFrame(StateFrames& data, int state, int from, int to){
  if (from == to)
    return;
  copyFrame(data, state, from, to);
  std::vector<std::string> parts = data.frame(state, from);
  for (int p = 0; p < (int)parts.size(); ++p)
    data.setFramePart(state, from, p, "");
}

int nextFrame(const StateFrames& data, int state, int frame){
  if (frame+1 >= data.frames() || !data.frameExists(state, frame+1))
    return 0;
  return frame+1;
}

void clearState(StateFrames& data, int state){
  for (int f = 0; f < data.frames(); ++f){
    //only the frames that are there, so that none are added
    std::vector<std::string> parts = data.frame(state, f);
    for (int p = 0; p < (int)parts.size(); ++p)
      data.setFramePart(state, f, p, "");
  }
}

CopiedState copyState(const StateFrames& data, int state){
  CopiedState copy;
  for (int f = 0; f < data.frames(); ++f){
    if (!data.frameExists(state, f))
      continue;
    copy.frames[f] = data.frame(state, f);
    for (int p = 0; p < data.parts(); ++p)
      copy.offsets[f].push_back(data.offset(state, f, p));
  }
  copy.size = data.size(state);
  copy.hotspot = data.hotspot(state);
  return copy;
}

void pasteState(StateFrames& data, int state, const CopiedState& copy){
  clearState(data, state);
  data.setSize(state, copy.size);
  data.setHotspot(state, copy.hotspot);
  for (const auto& entry : copy.frames){
    const std::vector<Vec2i>& offsets = copy.offsets.at(entry.first);
    for (int p = 0; p < (int)entry.second.size(); ++p){
      data.setFramePart(state, entry.first, p, entry.second[p]);
      data.setOffset(state, entry.first, p, p < (int)offsets.size() ? offsets[p] : Vec2i{0, 0});
    }
  }
}

std::string characterStateName(const Character& chr, int state){
  if (state < 16)
    return kCharacterStateNames[state];
  std::string name = state-16 < (int)chr.extraStateNames.size() ? chr.extraStateNames[state-16] : "";
  if (!name.empty())
    return name;
  name = "Extra " + std::to_string(state-15);
  if (state == 32)
    name += " (Stand left)";
  else if (state == 33)
    name += " (Walk left)";
  else if (state == 34)
    name += " (Stand, Talk left)";
  else if (state == 35)
    name += " (Walk, Talk left)";
  return name;
}

}
