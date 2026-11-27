#ifndef SD_STATEFRAMEEDITOR_H
#define SD_STATEFRAMEEDITOR_H

#include <cstdint>
#include <string>
#include <vector>

#include <imgui.h>

#include <model/stateframes.h>

#include "imagecache.h"

namespace sd{

//! the ImGui drag and drop payload of an image from the media pool: its name
inline const char* const kImagePayload = "SD_IMAGE";

//! how a state and frame editor looks and what it offers (the C#
//! StateFrameImage's properties)
struct StateFrameStyle{
  //! the states' names; buttons for up to ten, a list for more
  std::vector<std::string> labels;
  //! the images are stretched over the picture (items, cursors)
  bool scaleToBox = false;
  //! the picture's size can be changed
  bool sizeFields = true;
  //! the hotspot (basepoint, highlight) is shown and dragged
  bool hotspot = false;
  //! the picture's pixels per hotspot unit (3 for the cursor)
  float hotspotScale = 1.0f;
  //! the first image dropped onto an empty object sets its size
  bool adaptFirstDrop = false;
  //! frames have scripts (double click)
  bool scripts = true;
  bool animation = true;
  bool imageNames = true;
  //! the picture's background (0xAARRGGBB as ImGui's colours: ImU32)
  ImU32 background = IM_COL32(58, 110, 165, 255);
};

//! edits the states and frames of an item, object, character or the cursor:
//! the frames are filled with images dragged from the media pool, the parts
//! dragged into place, and the animation played
class StateFrameEditor{
public:
  //! draws the editor; true when the data changed
  bool draw(StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder);
  int state() const {return state_;}
  int frame() const {return frame_;}
  void setState(int state);
  //! draws the current frame of the state into a rectangle (for small previews)
  void drawFrame(ImDrawList* list, StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder, ImVec2 pos, ImVec2 size);
private:
  bool stateButtons(StateFrames& data, const StateFrameStyle& style);
  bool frameStrip(StateFrames& data, const StateFrameStyle& style);
  bool picture(StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder);
  bool sizeAndSpeed(StateFrames& data, const StateFrameStyle& style);
  bool popups(StateFrames& data, const StateFrameStyle& style);
  bool dropImage(StateFrames& data, const StateFrameStyle& style, ImageCache& images, const Project& project, const std::string& folder, const std::string& name, Vec2i pos);
  void removeSelectedPart(StateFrames& data);

  int state_ = 0;
  int frame_ = 0;
  int selectedPart_ = -1;
  //! what the mouse drags in the picture: 0 nothing, the part+1, or kDragHotspot
  int dragging_ = 0;
  Vec2i dragOffset_;
  bool playing_ = false;
  double nextTick_ = 0;
  //! a frame dropped onto another, until copy or move is chosen
  int fromFrame_ = -1;
  int toFrame_ = -1;
  bool openCopyMove_ = false;
  bool openRemove_ = false;
  bool openScript_ = false;
  std::string scriptText_;
  //! the size fields' values while they are typed
  int width_ = 0;
  int height_ = 0;
  int sizeState_ = -1;
};

}

#endif
