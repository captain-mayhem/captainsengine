#ifndef SD_STATEFRAMES_H
#define SD_STATEFRAMES_H

#include <map>
#include <string>
#include <vector>

#include "project.h"

namespace sd{

//! the states and frames of an item, object, character or the mouse cursor,
//! as the state and frame editor sees them (the C# IStateFrameData)
/*! An empty image name is no image. Setting a part beyond the frames fills the
    frames between with empty ones, as the C# editor does. */
class StateFrames{
public:
  virtual ~StateFrames() = default;
  virtual int states() const = 0;
  //! the frames a state can have (25 or 30)
  virtual int frames() const = 0;
  //! the image parts of a frame (1 or 2)
  virtual int parts() const = 0;
  //! the frame is there (a state's animation stops at the first that isn't)
  virtual bool frameExists(int state, int frame) const = 0;
  //! the images of the frame's parts, empty ones included; empty if there are none
  virtual std::vector<std::string> frame(int state, int frame) const = 0;
  virtual void setFramePart(int state, int frame, int part, const std::string& name) = 0;
  virtual int fpsDivider(int state) const = 0;
  virtual void setFpsDivider(int state, int divider) = 0;
  //! the size of the picture
  virtual Vec2i size(int state) const = 0;
  virtual void setSize(int state, Vec2i size) {}
  //! the basepoint or the cursor's highlight
  virtual Vec2i hotspot(int state) const {return Vec2i{0, 0};}
  virtual void setHotspot(int state, Vec2i spot) {}
  virtual Vec2i offset(int state, int frame, int part) const {return Vec2i{0, 0};}
  virtual void setOffset(int state, int frame, int part, Vec2i offset) {}
  virtual std::string script(int state, int frame) const {return "";}
  virtual void setScript(int state, int frame, const std::string& text) {}
};

class ItemFrames : public StateFrames{
public:
  explicit ItemFrames(Item& item);
  int states() const override {return (int)item_.states.size();}
  int frames() const override {return 25;}
  int parts() const override {return 1;}
  bool frameExists(int state, int frame) const override;
  std::vector<std::string> frame(int state, int frame) const override;
  void setFramePart(int state, int frame, int part, const std::string& name) override;
  int fpsDivider(int state) const override {return item_.states[state].fpsDivider;}
  void setFpsDivider(int state, int divider) override {item_.states[state].fpsDivider = divider;}
  Vec2i size(int state) const override {return Vec2i{50, 50};}
  std::string script(int state, int frame) const override;
  void setScript(int state, int frame, const std::string& text) override;
private:
  Item& item_;
};

class CursorFrames : public StateFrames{
public:
  explicit CursorFrames(std::vector<CursorState>& cursor);
  int states() const override {return (int)cursor_.size();}
  int frames() const override {return 25;}
  int parts() const override {return 1;}
  bool frameExists(int state, int frame) const override;
  std::vector<std::string> frame(int state, int frame) const override;
  void setFramePart(int state, int frame, int part, const std::string& name) override;
  int fpsDivider(int state) const override {return cursor_[state].fpsDivider;}
  void setFpsDivider(int state, int divider) override {cursor_[state].fpsDivider = divider;}
  Vec2i size(int state) const override {return Vec2i{32, 32};}
  Vec2i hotspot(int state) const override {return cursor_[state].highlight;}
  void setHotspot(int state, Vec2i spot) override {cursor_[state].highlight = spot;}
private:
  std::vector<CursorState>& cursor_;
};

//! the frames of objects and characters: up to two parts with offsets, and a script
class ExtendedFrames : public StateFrames{
public:
  int frames() const override {return 30;}
  int parts() const override {return 2;}
  bool frameExists(int state, int frame) const override;
  std::vector<std::string> frame(int state, int frame) const override;
  void setFramePart(int state, int frame, int part, const std::string& name) override;
  Vec2i offset(int state, int frame, int part) const override;
  void setOffset(int state, int frame, int part, Vec2i offset) override;
  std::string script(int state, int frame) const override;
  void setScript(int state, int frame, const std::string& text) override;
protected:
  virtual std::vector<ExtendedFrame>& framesOf(int state) = 0;
  virtual const std::vector<ExtendedFrame>& framesOf(int state) const = 0;
  //! frameExists counts frames without images (objects) or not (characters)
  bool emptyFramesExist_ = true;
private:
  ExtendedFrame& frameAt(int state, int frame);
};

class ObjectFrames : public ExtendedFrames{
public:
  explicit ObjectFrames(Object& obj);
  int states() const override {return (int)obj_.states.size();}
  int fpsDivider(int state) const override {return obj_.states[state].fpsDivider;}
  void setFpsDivider(int state, int divider) override {obj_.states[state].fpsDivider = divider;}
  //! one size for all the object's states
  Vec2i size(int state) const override {return obj_.size;}
  void setSize(int state, Vec2i size) override {obj_.size = size;}
protected:
  std::vector<ExtendedFrame>& framesOf(int state) override {return obj_.states[state].frames;}
  const std::vector<ExtendedFrame>& framesOf(int state) const override {return obj_.states[state].frames;}
private:
  Object& obj_;
};

class CharacterFrames : public ExtendedFrames{
public:
  explicit CharacterFrames(Character& chr);
  int states() const override {return (int)chr_.states.size();}
  int fpsDivider(int state) const override {return chr_.states[state].fpsDivider;}
  void setFpsDivider(int state, int divider) override {chr_.states[state].fpsDivider = divider;}
  Vec2i size(int state) const override {return chr_.states[state].size;}
  void setSize(int state, Vec2i size) override {chr_.states[state].size = size;}
  //! the basepoint
  Vec2i hotspot(int state) const override {return chr_.states[state].basepoint;}
  void setHotspot(int state, Vec2i spot) override {chr_.states[state].basepoint = spot;}
protected:
  std::vector<ExtendedFrame>& framesOf(int state) override {return chr_.states[state].frames;}
  const std::vector<ExtendedFrame>& framesOf(int state) const override {return chr_.states[state].frames;}
private:
  Character& chr_;
};

//! the frame has no images
bool frameEmpty(const StateFrames& data, int state, int frame);
//! no frame of any state has an image
bool allFramesEmpty(const StateFrames& data);
//! copies the frame's parts and their offsets onto another frame
void copyFrame(StateFrames& data, int state, int from, int to);
//! moves them, leaving the frame empty
void moveFrame(StateFrames& data, int state, int from, int to);
//! the next frame of the animation, back to the first after the last
int nextFrame(const StateFrames& data, int state, int frame);

//! a state copied to be pasted: its frames, offsets, size and hotspot
struct CopiedState{
  std::map<int, std::vector<std::string>> frames;
  std::map<int, std::vector<Vec2i>> offsets;
  Vec2i size;
  Vec2i hotspot;
};
//! removes every image of the state
void clearState(StateFrames& data, int state);
CopiedState copyState(const StateFrames& data, int state);
//! replaces the state by a copy
void pasteState(StateFrames& data, int state, const CopiedState& copy);

//! the name of a character's state, the extra states 17 to 36 (16 to 35
//! counted from 0) by their names or "Extra n"
std::string characterStateName(const Character& chr, int state);

}

#endif
