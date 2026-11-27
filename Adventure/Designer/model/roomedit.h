#ifndef SD_ROOMEDIT_H
#define SD_ROOMEDIT_H

#include <optional>
#include <string>
#include <vector>

#include "project.h"

// The room editor's rules, as the C# StoryDesigner's room window (RoomDlg)
// has them: how objects and characters are placed, ordered and found, the
// depth map's scale, painting the walkmap and its scripts, the inventory
// field's sizes and the FX shapes' handles. Positions are the room's pixels.

namespace sd{

//! the room editor's views, keys 1 to 5
enum class RoomMode{
  Objects = 0,
  Walkmap,
  Deepmap,
  Inventory,
  SpecialFx,
};

//! the width of the game's window: the resolution, wider for 16:9 (the C# WindowXRes)
int windowWidth(const ProjectSettings& settings);

//! the zoom of a character at a position by the room's depth map (1 at the
//! lower line, smaller towards the upper one)
float roomScale(const Project& project, const Room& room, Vec2i pos);

//! the drawing order of an object: the back layer first, the front layer last
int objectDepth(const ObjectInstance& obj);
//! the drawing order of a character, by where it stands
int characterDepth(const Project& project, const CharacterInstance& chr);
//! the character's state shown in the room, from 0 (the left direction shows the right one mirrored)
int characterState(const CharacterInstance& chr);
//! where the character stands: its position plus its state's basepoint
Vec2i characterFoot(const Project& project, const CharacterInstance& chr);
void setCharacterFoot(const Project& project, CharacterInstance& chr, Vec2i foot);
//! turns the character (1 down, 2 up, 3 right, 4 left), keeping where it stands
void setLookDir(const Project& project, CharacterInstance& chr, int lookDir);

//! an object or a character of a room
struct RoomEntry{
  bool character = false;
  //! in the room's objects or its characters
  size_t index = 0;
  int depth = 0;
  bool operator==(const RoomEntry& e) const {return character == e.character && index == e.index;}
};
//! the objects and characters in the order they are drawn
std::vector<RoomEntry> drawOrder(const Project& project, const Room& room);
//! the object or character at a position: the one in front, or the one behind it
std::optional<RoomEntry> entryAt(const Project& project, const Room& room, Vec2i pos, bool second);
//! the rectangle that hits an entry: its position and size (unscaled, as the C# editor)
void entryBox(const Project& project, const Room& room, const RoomEntry& entry, Vec2i& pos, Vec2i& size);

//! places an object, named after it (and numbered if it's there already)
ObjectInstance& placeObject(Project& project, Room& room, const std::string& object, Vec2i pos);
//! places a character, named after it (and numbered if it's placed anywhere already)
CharacterInstance& placeCharacter(Project& project, const std::string& room, const std::string& character, Vec2i pos);
//! removes an object of the room and its script
void removeObjectInstance(Project& project, Room& room, size_t index);
//! removes a character of the room and its script
void removeCharacterInstance(Project& project, const std::string& room, size_t index);
//! "set to back" and "set to front": first or last of its kind (drawn first among equal depths); the new index
size_t moveToBack(Project& project, Room& room, const RoomEntry& entry);
size_t moveToFront(Project& project, Room& room, const RoomEntry& entry);
//! the depth of an object's depth handle dragged to y (in half fields)
int objectDepthAt(const Project& project, const Room& room, int y);

//! the walkmap's fields per walk grid field: 2 for a double walkmap
int walkmapScale(const Room& room);
//! the walkmap field at a position (not clamped)
Vec2i walkmapField(const Project& project, const Room& room, Vec2i pos);
bool inWalkmap(const Room& room, Vec2i field);
//! the pens of the walkmap
enum class WalkmapPen{
  Size1 = 0,
  Size2,
  Size3,
  ScriptPaster,
  ScriptEraser,
};
//! frees or blocks a field and its neighbours by the pen's size
void paintWalkmap(Room& room, Vec2i field, WalkmapPen pen, bool free);
//! the field's script, created if it has none
Script& walkmapScript(Project& project, Room& room, Vec2i field);
//! gives the field a script with the text
void pasteWalkmapScript(Project& project, Room& room, Vec2i field, const std::optional<std::string>& text);
//! removes the field's script
void eraseWalkmapScript(Project& project, Room& room, Vec2i field);

//! the depth map's line at y (in fields), for dragging its lines
int depthmapLineAt(const Project& project, const Room& room, int y);
//! moves the upper (0) or lower (1) line of the depth map, kept apart and in the room
void setDepthmapLine(const Project& project, Room& room, int line, int depth);

//! the inventory field's size in pixels
Vec2i inventoryPixels(const Room& room);
//! the inventory field's corner dragged to a position: the number of fields
void dragInventoryCorner(const Project& project, Room& room, Vec2i pos);
//! the inventory field's bottom (axis 1) or right (axis 0) edge dragged to a size in pixels: the fields' scale
void scaleInventory(Room& room, int axis, float pixels);

//! the y of a wall mirror's depth handle
int fxDepthHandleY(const Project& project, const FxShape& shape);
//! the depth of a wall mirror's handle dragged to y
int fxDepthAt(const Project& project, const Room& room, int y);
Vec2i fxCenter(const FxShape& shape);
//! moves an FX shape, as far as it stays in the room
void moveFxShape(const Room& room, FxShape& shape, Vec2i diff);
//! moves an FX shape's corner, kept in the room
void setFxCorner(const Room& room, FxShape& shape, int corner, Vec2i pos);

}

#endif
