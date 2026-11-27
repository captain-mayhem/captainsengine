#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <model/roomedit.h>

// The room editor's rules, ported from the C# StoryDesigner's RoomDlg.

using namespace sd;

namespace{

//! a project of 640x480 (walk grid 20) with a room of the window's size
Project makeProject(){
  Project project;
  project.settings.resolution = Vec2i{640, 480};
  Room room;
  room.name = "Hall";
  room.size = Vec2i{640, 480};
  room.depthmap = Vec2i{5, 10};
  room.zoom = 3;
  Vec2i wm = project.walkmapSize();
  room.walkmap.assign(wm.x, std::vector<WalkMapEntry>(wm.y));
  for (int i = 0; i < 3; ++i)
    room.fxShapes.push_back(FxShape::makeDefault(i));
  project.rooms.add(room.name, room);
  Object door;
  door.name = "Door";
  door.size = Vec2i{40, 80};
  door.states.resize(10);
  project.objects.add(door.name, door);
  Character chr;
  chr.name = "Herbert";
  chr.zoom = 100;
  chr.states.resize(36);
  for (CharacterState& state : chr.states){
    state.size = Vec2i{60, 100};
    state.basepoint = Vec2i{30, 99};
  }
  //standing to the right has its basepoint elsewhere
  chr.states[2].basepoint = Vec2i{20, 95};
  project.characters.add(chr.name, chr);
  return project;
}

}

TEST(RoomEdit, WindowWidthIsWiderFor16To9){
  ProjectSettings s;
  s.resolution = Vec2i{640, 480};
  EXPECT_EQ(windowWidth(s), 640);
  s.is16to9 = true;
  EXPECT_EQ(windowWidth(s), 840);
  s.resolution = Vec2i{1024, 768};
  EXPECT_EQ(windowWidth(s), 1344);
}

TEST(RoomEdit, ScaleFollowsTheDepthMap){
  Project project = makeProject();
  const Room& room = *project.rooms.find("Hall");
  //1 below the lower line (10 fields, y 200), the least above the upper one (y 100)
  EXPECT_FLOAT_EQ(roomScale(project, room, Vec2i{0, 300}), 1.0f);
  const float least = 1.0f - 100.0f/480*3*0.3f;
  EXPECT_FLOAT_EQ(roomScale(project, room, Vec2i{0, 50}), least);
  EXPECT_FLOAT_EQ(roomScale(project, room, Vec2i{0, 150}), 0.5f + 0.5f*least);
}

TEST(RoomEdit, ObjectsAreNumberedAndCharactersCountedEverywhere){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  ObjectInstance& first = placeObject(project, room, "door", Vec2i{100, 200});
  EXPECT_EQ(first.name, "Door");
  EXPECT_EQ(first.object, "Door");
  EXPECT_EQ(first.depth, 200*2/20 + 1);
  EXPECT_EQ(first.layer, 2);
  EXPECT_EQ(first.state, 1);
  EXPECT_EQ(placeObject(project, room, "Door", Vec2i{}).name, "Door1");
  //the C# editor counts the instances: after Door1 went, Door1 is free but Door2 isn't
  removeObjectInstance(project, room, 1);
  placeObject(project, room, "Door", Vec2i{});
  EXPECT_EQ(room.objects.back().name, "Door1");
  EXPECT_EQ(placeObject(project, room, "Door", Vec2i{}).name, "Door2");

  CharacterInstance& chr = placeCharacter(project, "Hall", "Herbert", Vec2i{10, 20});
  EXPECT_EQ(chr.name, "Herbert");
  EXPECT_EQ(chr.lookDir, 1);
  EXPECT_EQ(chr.room, "Hall");
  EXPECT_EQ(placeCharacter(project, "Kitchen", "Herbert", Vec2i{}).name, "Herbert1");
  EXPECT_EQ(project.roomCharacters("Kitchen").size(), 1u);
}

TEST(RoomEdit, RemovingAnInstanceRemovesItsScript){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  placeObject(project, room, "Door", Vec2i{});
  placeCharacter(project, "Hall", "Herbert", Vec2i{});
  Script obj;
  obj.type = Script::Object;
  obj.name = Script::objectScriptName("Door", "Hall");
  project.scripts[Script::Object].add(obj.name, obj);
  Script chr;
  chr.type = Script::Character;
  chr.name = "Herbert";
  project.scripts[Script::Character].add(chr.name, chr);
  removeObjectInstance(project, room, 0);
  removeCharacterInstance(project, "Hall", 0);
  EXPECT_TRUE(room.objects.empty());
  EXPECT_TRUE(project.roomCharacters("Hall").empty());
  EXPECT_TRUE(project.scripts[Script::Object].empty());
  EXPECT_TRUE(project.scripts[Script::Character].empty());
}

TEST(RoomEdit, DrawOrderAndHits){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  ObjectInstance& back = placeObject(project, room, "Door", Vec2i{100, 100});
  back.layer = 0;
  ObjectInstance& middle = placeObject(project, room, "Door", Vec2i{110, 100});
  middle.layer = 1;
  middle.depth = 25;
  placeObject(project, room, "Door", Vec2i{120, 100});
  //the character stands at y 80+99 = 179: depth 17
  placeCharacter(project, "Hall", "Herbert", Vec2i{100, 80});
  std::vector<RoomEntry> order = drawOrder(project, room);
  ASSERT_EQ(order.size(), 4u);
  EXPECT_EQ(order[0], (RoomEntry{false, 0}));
  EXPECT_EQ(order[1], (RoomEntry{true, 0}));
  EXPECT_EQ(order[1].depth, 17);
  EXPECT_EQ(order[2], (RoomEntry{false, 1}));
  EXPECT_EQ(order[3], (RoomEntry{false, 2}));
  //at 130,150 the front door, behind it the middle one
  EXPECT_EQ(*entryAt(project, room, Vec2i{130, 150}, false), (RoomEntry{false, 2}));
  EXPECT_EQ(*entryAt(project, room, Vec2i{130, 150}, true), (RoomEntry{false, 1}));
  //at 105,90 only the character
  EXPECT_EQ(*entryAt(project, room, Vec2i{105, 90}, false), (RoomEntry{true, 0}));
  EXPECT_FALSE(entryAt(project, room, Vec2i{105, 90}, true).has_value());
  EXPECT_FALSE(entryAt(project, room, Vec2i{500, 400}, false).has_value());
  //set to back: the first of the objects, drawn first of its depth
  EXPECT_EQ(moveToBack(project, room, RoomEntry{false, 2}), 0u);
  EXPECT_EQ(room.objects[0].position.x, 120);
  EXPECT_EQ(moveToFront(project, room, RoomEntry{false, 0}), 2u);
  EXPECT_EQ(room.objects[2].position.x, 120);
}

TEST(RoomEdit, TurningKeepsWhereTheCharacterStands){
  Project project = makeProject();
  CharacterInstance& chr = placeCharacter(project, "Hall", "Herbert", Vec2i{100, 80});
  EXPECT_EQ(characterFoot(project, chr), (Vec2i{130, 179}));
  setLookDir(project, chr, 3);
  EXPECT_EQ(characterState(chr), 2);
  EXPECT_EQ(chr.position, (Vec2i{110, 84}));
  EXPECT_EQ(characterFoot(project, chr), (Vec2i{130, 179}));
  //left shows the right state mirrored
  setLookDir(project, chr, 4);
  EXPECT_EQ(characterState(chr), 2);
  EXPECT_EQ(chr.position, (Vec2i{110, 84}));
  setCharacterFoot(project, chr, Vec2i{200, 200});
  EXPECT_EQ(chr.position, (Vec2i{180, 105}));
}

TEST(RoomEdit, WalkmapPensAndScripts){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  EXPECT_EQ(walkmapField(project, room, Vec2i{45, 61}), (Vec2i{2, 3}));
  room.doubleWalkmap = true;
  EXPECT_EQ(walkmapField(project, room, Vec2i{45, 61}), (Vec2i{4, 6}));
  room.doubleWalkmap = false;
  paintWalkmap(room, Vec2i{5, 5}, WalkmapPen::Size1, true);
  EXPECT_TRUE(room.walkmap[5][5].isFree);
  EXPECT_FALSE(room.walkmap[6][5].isFree);
  paintWalkmap(room, Vec2i{10, 10}, WalkmapPen::Size2, true);
  int count = 0;
  for (const auto& column : room.walkmap)
    for (const WalkMapEntry& e : column)
      count += e.isFree ? 1 : 0;
  EXPECT_EQ(count, 1 + 5);
  paintWalkmap(room, Vec2i{20, 20}, WalkmapPen::Size3, true);
  EXPECT_TRUE(room.walkmap[22][20].isFree);
  EXPECT_TRUE(room.walkmap[21][21].isFree);
  EXPECT_FALSE(room.walkmap[22][22].isFree);
  //at the edge the fields outside are left out
  paintWalkmap(room, Vec2i{0, 0}, WalkmapPen::Size3, true);
  EXPECT_TRUE(room.walkmap[2][0].isFree);
  paintWalkmap(room, Vec2i{0, 0}, WalkmapPen::Size1, false);
  EXPECT_FALSE(room.walkmap[0][0].isFree);

  pasteWalkmapScript(project, room, Vec2i{3, 4}, std::string("loadRoom(Kitchen)"));
  EXPECT_TRUE(room.walkmap[3][4].hasScript);
  const Script* scr = project.scripts[Script::Walkmap].find("#004005Hall");
  ASSERT_NE(scr, nullptr);
  EXPECT_EQ(scr->type, Script::Walkmap);
  EXPECT_EQ(*scr->text, "loadRoom(Kitchen)");
  //the field's script is found again, not made twice
  EXPECT_EQ(&walkmapScript(project, room, Vec2i{3, 4}), scr);
  EXPECT_EQ(project.scripts[Script::Walkmap].size(), 1u);
  eraseWalkmapScript(project, room, Vec2i{3, 4});
  EXPECT_FALSE(room.walkmap[3][4].hasScript);
  EXPECT_TRUE(project.scripts[Script::Walkmap].empty());
}

TEST(RoomEdit, DepthMapLinesStayApart){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  EXPECT_EQ(depthmapLineAt(project, room, 95), 5);
  EXPECT_EQ(depthmapLineAt(project, room, 89), 4);
  setDepthmapLine(project, room, 0, 12);
  EXPECT_EQ(room.depthmap.x, 9);
  setDepthmapLine(project, room, 0, -3);
  EXPECT_EQ(room.depthmap.x, 1);
  setDepthmapLine(project, room, 1, 0);
  EXPECT_EQ(room.depthmap.y, 2);
  //the room is 24 fields high
  setDepthmapLine(project, room, 1, 40);
  EXPECT_EQ(room.depthmap.y, 23);
}

TEST(RoomEdit, ObjectDepthHandle){
  Project project = makeProject();
  const Room& room = *project.rooms.find("Hall");
  //half fields of 10 pixels, the handle 5 above its line
  EXPECT_EQ(objectDepthAt(project, room, 95), 10);
  EXPECT_EQ(objectDepthAt(project, room, -50), 1);
  EXPECT_EQ(objectDepthAt(project, room, 1000), 480/10 - 2);
}

TEST(RoomEdit, InventoryField){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  room.hasInventory = true;
  room.invPos = Vec2i{100, 50};
  room.invScale = Vec2f{1.0f, 1.0f};
  room.invSpacing = 10;
  dragInventoryCorner(project, room, Vec2i{100+30+3*60, 50+5+2*60});
  EXPECT_EQ(room.invSize, (Vec2i{3, 2}));
  EXPECT_EQ(inventoryPixels(room), (Vec2i{3*60+30, 2*60+5}));
  //at most as many fields as fit the screen, at least one
  dragInventoryCorner(project, room, Vec2i{5000, 0});
  EXPECT_EQ(room.invSize, (Vec2i{(640-30)/60, 1}));
  room.invSize = Vec2i{3, 2};
  scaleInventory(room, 1, (2*60+5)/2.0f);
  EXPECT_FLOAT_EQ(room.invScale.y, 0.5f);
  scaleInventory(room, 0, 10000);
  EXPECT_FLOAT_EQ(room.invScale.x, 1.5f);
  EXPECT_EQ(inventoryPixels(room), (Vec2i{3*(75+10)+30, 2*(25+10)+5}));
}

TEST(RoomEdit, FxShapesStayInTheRoom){
  Project project = makeProject();
  Room& room = *project.rooms.find("Hall");
  FxShape& shape = room.fxShapes[0];
  //the default: 105,100 to 205,200
  EXPECT_EQ(fxCenter(shape), (Vec2i{155, 150}));
  moveFxShape(room, shape, Vec2i{-200, 10});
  EXPECT_EQ(shape.positions[0], (Vec2i{0, 110}));
  EXPECT_EQ(shape.positions[2], (Vec2i{100, 210}));
  moveFxShape(room, shape, Vec2i{1000, 1000});
  EXPECT_EQ(shape.positions[2], (Vec2i{640, 480}));
  EXPECT_EQ(shape.positions[0], (Vec2i{540, 380}));
  setFxCorner(room, shape, 1, Vec2i{700, -5});
  EXPECT_EQ(shape.positions[1], (Vec2i{640, 0}));
  shape.depth = 15;
  EXPECT_EQ(fxDepthHandleY(project, shape), 300);
  EXPECT_EQ(fxDepthAt(project, room, 295), 15);
  EXPECT_EQ(fxDepthAt(project, room, 10000), 24);
}
