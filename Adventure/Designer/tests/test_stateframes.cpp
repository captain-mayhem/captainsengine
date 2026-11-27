#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include <model/images.h>
#include <model/projectfile.h>
#include <model/stateframes.h>

// The state and frame editor's model (the C# IStateFrameData of items,
// objects, characters and the cursor) and the images of its previews.

using namespace sd;

TEST(StateFrames, ItemFramesAreFilledUpToThePartSet){
  Item item;
  ItemFrames data(item);
  EXPECT_EQ(data.states(), 10);
  data.setFramePart(0, 2, 0, "apple");
  //the frames before are there, without images, as in the C# editor
  EXPECT_TRUE(data.frameExists(0, 1));
  EXPECT_FALSE(data.frameExists(0, 3));
  EXPECT_TRUE(data.frame(0, 1).empty());
  EXPECT_EQ(data.frame(0, 2), std::vector<std::string>{"apple"});
  EXPECT_TRUE(frameEmpty(data, 0, 0));
  EXPECT_FALSE(frameEmpty(data, 0, 2));
  data.setScript(0, 4, "playSound(bite)");
  EXPECT_EQ(data.script(0, 4), "playSound(bite)");
  EXPECT_EQ(data.script(0, 3), "");
  //the animation goes through the frames that are there, then starts again
  EXPECT_EQ(nextFrame(data, 0, 1), 2);
  EXPECT_EQ(nextFrame(data, 0, 2), 0);
}

TEST(StateFrames, ObjectPartsKeepTheirOffsets){
  Object obj;
  obj.size = Vec2i{100, 100};
  ObjectFrames data(obj);
  EXPECT_TRUE(allFramesEmpty(data));
  data.setFramePart(1, 0, 1, "door_open");
  data.setOffset(1, 0, 1, Vec2i{10, 20});
  EXPECT_EQ(data.frame(1, 0), (std::vector<std::string>{"", "door_open"}));
  EXPECT_EQ(data.offset(1, 0, 1), (Vec2i{10, 20}));
  EXPECT_EQ(data.offset(1, 0, 0), (Vec2i{0, 0}));
  EXPECT_FALSE(allFramesEmpty(data));
  //one size for all of an object's states
  data.setSize(3, Vec2i{40, 50});
  EXPECT_EQ(data.size(0), (Vec2i{40, 50}));
  //an object's frame without images is still there
  data.setFramePart(1, 0, 1, "");
  EXPECT_TRUE(data.frameExists(1, 0));
  EXPECT_TRUE(frameEmpty(data, 1, 0));
}

TEST(StateFrames, FramesAreCopiedAndMoved){
  Object obj;
  ObjectFrames data(obj);
  data.setFramePart(0, 0, 0, "a");
  data.setFramePart(0, 0, 1, "b");
  data.setOffset(0, 0, 1, Vec2i{5, 6});
  copyFrame(data, 0, 0, 3);
  EXPECT_EQ(data.frame(0, 3), (std::vector<std::string>{"a", "b"}));
  EXPECT_EQ(data.offset(0, 3, 1), (Vec2i{5, 6}));
  moveFrame(data, 0, 3, 5);
  EXPECT_TRUE(frameEmpty(data, 0, 3));
  EXPECT_EQ(data.frame(0, 5), (std::vector<std::string>{"a", "b"}));
  EXPECT_EQ(data.offset(0, 5, 1), (Vec2i{5, 6}));
}

TEST(StateFrames, CharacterStatesAreCopiedPastedAndCleared){
  Character chr;
  CharacterFrames data(chr);
  EXPECT_EQ(data.states(), 36);
  data.setFramePart(3, 0, 0, "walk1");
  data.setFramePart(3, 1, 0, "walk2");
  data.setOffset(3, 1, 0, Vec2i{2, 3});
  data.setSize(3, Vec2i{80, 150});
  data.setHotspot(3, Vec2i{40, 149});
  //a character's frame is there only with an image
  EXPECT_TRUE(data.frameExists(3, 1));
  EXPECT_FALSE(data.frameExists(3, 2));
  CopiedState copy = copyState(data, 3);
  pasteState(data, 20, copy);
  EXPECT_EQ(data.frame(20, 1), std::vector<std::string>{"walk2"});
  EXPECT_EQ(data.offset(20, 1, 0), (Vec2i{2, 3}));
  EXPECT_EQ(data.size(20), (Vec2i{80, 150}));
  EXPECT_EQ(data.hotspot(20), (Vec2i{40, 149}));
  clearState(data, 3);
  EXPECT_FALSE(data.frameExists(3, 0));
  EXPECT_TRUE(frameEmpty(data, 3, 1));
  //the size stays
  EXPECT_EQ(data.size(3), (Vec2i{80, 150}));
}

TEST(StateFrames, CharacterStatesHaveTheCSharpNames){
  Character chr;
  CharacterFrames data(chr);
  EXPECT_EQ(characterStateName(chr, 0), "Stand front");
  EXPECT_EQ(characterStateName(chr, 15), "Pickup right");
  EXPECT_EQ(characterStateName(chr, 16), "Extra 1");
  EXPECT_EQ(characterStateName(chr, 33), "Extra 18 (Walk left)");
  chr.extraStateNames[1] = "Dance";
  EXPECT_EQ(characterStateName(chr, 17), "Dance");
}

TEST(StateFrames, CursorStatesStartAsTheCSharpOnes){
  std::vector<CursorState> cursor;
  CursorFrames data(cursor);
  ASSERT_EQ(cursor.size(), 10u);
  EXPECT_EQ(cursor[0].command, 1);
  EXPECT_EQ(cursor[9].command, 2);
  data.setHotspot(2, Vec2i{4, 5});
  EXPECT_EQ(cursor[2].highlight, (Vec2i{4, 5}));
}

TEST(StateFrames, EditedFramesAreWrittenAndReadBack){
  Project project;
  std::string error;
  ASSERT_TRUE(readProject(std::string(SD_TEST_DATA_DIR) + "/testadventure_cs.adv", project, &error)) << error;
  ASSERT_FALSE(project.objects.size() == 0);
  Object& obj = project.objects.begin()->value;
  std::string objName = obj.name;
  ObjectFrames objects(obj);
  objects.setFramePart(2, 4, 0, "first");
  objects.setFramePart(2, 4, 1, "second");
  objects.setOffset(2, 4, 1, Vec2i{7, 8});
  objects.setScript(2, 4, "a;b");
  objects.setFpsDivider(2, 7);
  ASSERT_FALSE(project.characters.size() == 0);
  Character& chr = project.characters.begin()->value;
  std::string chrName = chr.name;
  CharacterFrames characters(chr);
  characters.setFramePart(30, 2, 0, "jump");
  characters.setHotspot(30, Vec2i{11, 12});
  chr.extraStateNames[14] = "Jump";
  std::string path = (std::filesystem::temp_directory_path() / "sd_test_frames.adv").string();
  ASSERT_TRUE(writeProject(path, project, &error)) << error;
  Project again;
  bool read = readProject(path, again, &error);
  std::error_code ec;
  std::filesystem::remove(path, ec);
  ASSERT_TRUE(read) << error;
  Object* obj2 = again.objects.find(objName);
  ASSERT_NE(obj2, nullptr);
  ObjectFrames objects2(*obj2);
  EXPECT_EQ(objects2.frame(2, 4), (std::vector<std::string>{"first", "second"}));
  EXPECT_EQ(objects2.offset(2, 4, 1), (Vec2i{7, 8}));
  EXPECT_EQ(objects2.script(2, 4), "a;b");
  EXPECT_EQ(objects2.fpsDivider(2), 7);
  Character* chr2 = again.characters.find(chrName);
  ASSERT_NE(chr2, nullptr);
  CharacterFrames characters2(*chr2);
  EXPECT_EQ(characters2.frame(30, 2), std::vector<std::string>{"jump"});
  EXPECT_EQ(characters2.hotspot(30), (Vec2i{11, 12}));
  EXPECT_EQ(characterStateName(*chr2, 30), "Jump");
}

namespace{

//! a project whose images are on another computer's disk, as in the test games
Project imageProject(){
  Project project;
  project.images.add("wisp", "C:\\Users\\someone\\gfx\\wisp.png");
  project.images.add("box", "D:\\game\\gfx\\box.pnj");
  project.images.add("missing", "C:\\gfx\\missing.png");
  project.images.add("deep", "C:\\gfx\\deep.png");
  PoolNode images{(int)ResourceID::Folder, "IMAGES", {}};
  PoolNode folder{(int)ResourceID::Folder, "CHARACTER", {}};
  folder.children.push_back(PoolNode{(int)ResourceID::Image, "deep", {}});
  images.children.push_back(folder);
  project.mediaPool.push_back(images);
  return project;
}

const std::string kImages = std::string(SD_TEST_DATA_DIR) + "/images";

}

TEST(ProjectImages, AreFoundNextToTheProject){
  Project project = imageProject();
  std::unique_ptr<CGE::Image> img = loadProjectImage(project, kImages, "wisp");
  ASSERT_TRUE(img);
  EXPECT_EQ(img->getWidth(), 4u);
  EXPECT_EQ(img->getHeight(), 2u);
  ASSERT_EQ(img->getNumChannels(), 4u);
  //the top left pixel is red, the bottom right one transparent
  EXPECT_EQ(img->getPixelChannel(0, 0, 0), 255);
  EXPECT_EQ(img->getPixelChannel(0, 0, 2), 0);
  EXPECT_EQ(img->getPixelChannel(0, 0, 3), 255);
  EXPECT_EQ(img->getPixelChannel(3, 1, 3), 0);
}

TEST(ProjectImages, AreFoundInTheFolderOfTheirMediaPoolFolder){
  Project project = imageProject();
  std::unique_ptr<CGE::Image> img = loadProjectImage(project, kImages, "deep");
  ASSERT_TRUE(img);
  EXPECT_EQ(img->getWidth(), 3u);
  EXPECT_EQ(img->getHeight(), 5u);
}

TEST(ProjectImages, PnjTakeTheirAlphaFromThePnaInGfxDat){
  Project project = imageProject();
  std::unique_ptr<CGE::Image> img = loadProjectImage(project, kImages, "box");
  ASSERT_TRUE(img);
  EXPECT_EQ(img->getWidth(), 16u);
  EXPECT_EQ(img->getHeight(), 8u);
  ASSERT_EQ(img->getNumChannels(), 4u);
  EXPECT_GT(img->getPixelChannel(2, 4, 1), 150);
  EXPECT_GT(img->getPixelChannel(2, 4, 3), 200);
  EXPECT_LT(img->getPixelChannel(13, 4, 3), 50);
}

TEST(ProjectImages, MissingOnesAreNull){
  Project project = imageProject();
  EXPECT_FALSE(loadProjectImage(project, kImages, "missing"));
  EXPECT_FALSE(loadProjectImage(project, kImages, "unknown"));
  EXPECT_FALSE(loadProjectImage(project, "", "wisp"));
  EXPECT_EQ(fileNameOf("C:\\a\\b.png"), "b.png");
  EXPECT_EQ(fileNameOf("a/b/c.png"), "c.png");
}
