#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <io/MemReader.h>
#include <io/ZipReader.h>

#include <model/bitmapfont.h>
#include <model/gamefile.h>
#include <model/projectfile.h>
#include <model/resources.h>

// The resources' rules (new, rename, duplicate, delete, folders), the bitmap
// fonts, and the game, export and import files, on the repository's test game.

using namespace sd;
namespace fs = std::filesystem;

namespace{

std::string utf8(const fs::path& p){
  std::u8string str = p.u8string();
  return std::string(str.begin(), str.end());
}

//! a fresh folder for a test's files
fs::path testFolder(const char* name){
  fs::path dir = fs::temp_directory_path()/"sd_gamefile_tests"/name;
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir);
  return dir;
}

std::vector<std::string> entries(const fs::path& zip){
  CGE::ZipReader rdr(utf8(zip));
  return rdr.getEntries();
}

bool has(const std::vector<std::string>& names, const std::string& name){
  for (const std::string& n : names){
    if (n == name)
      return true;
  }
  return false;
}

//! the test game unpacked from its export, read, with its media found
std::string unpackTestGame(const fs::path& dir, Project& project){
  std::string error;
  std::string adv = unpackAdz(SD_TEST_GAMES_DIR "/testadventure.adz", utf8(dir/"game"), &error);
  EXPECT_FALSE(adv.empty()) << error;
  EXPECT_TRUE(readProject(adv, project, &error)) << error;
  EXPECT_TRUE(resolveMedia(project, utf8(dir/"game")));
  return adv;
}

}

TEST(Resources, NewProjectHasThePoolsFolders){
  Project p;
  newPools(p);
  ASSERT_EQ(p.gamePool.size(), 5u);
  ASSERT_EQ(p.mediaPool.size(), 4u);
  EXPECT_EQ(p.gamePool[0].label, "1) CHARACTERS");
  EXPECT_EQ(p.gamePool[4].label, "5) ROOMS");
  EXPECT_EQ(p.mediaPool[3].label, "VIDEOS");
}

TEST(Resources, NewRenameDuplicateDelete){
  Project p;
  newPools(p);
  p.settings.resolution = {640, 480};
  std::string obj = newResource(p, ResourceID::Object);
  EXPECT_EQ(obj, "Object1");
  std::string room = newResource(p, ResourceID::Room);
  ASSERT_NE(p.rooms.find(room), nullptr);
  EXPECT_EQ(p.rooms.find(room)->fxShapes.size(), 3u);
  ObjectInstance inst;
  inst.name = "Door";
  inst.object = obj;
  p.rooms.find(room)->objects.push_back(inst);
  EXPECT_FALSE(pathInPool(p.gamePool, ResourceID::Object, obj).empty());

  //renaming the object renames where it is placed
  std::string error;
  EXPECT_TRUE(renameResource(p, ResourceID::Object, obj, "Gate", &error)) << error;
  EXPECT_EQ(p.rooms.find(room)->objects[0].object, "Gate");
  EXPECT_FALSE(pathInPool(p.gamePool, ResourceID::Object, "Gate").empty());
  //a name that is taken is refused
  std::string other = newResource(p, ResourceID::Object);
  EXPECT_FALSE(renameResource(p, ResourceID::Object, other, "gate", &error));

  //a duplicated room keeps its placed objects
  std::string copy = duplicateResource(p, ResourceID::Room, room);
  ASSERT_FALSE(copy.empty());
  ASSERT_NE(p.rooms.find(copy), nullptr);
  EXPECT_EQ(p.rooms.find(copy)->objects.size(), 1u);

  //deleting the object takes it out of the rooms and the pool
  deleteResource(p, ResourceID::Object, "Gate");
  EXPECT_EQ(p.objects.find("Gate"), nullptr);
  EXPECT_TRUE(p.rooms.find(room)->objects.empty());
  EXPECT_TRUE(pathInPool(p.gamePool, ResourceID::Object, "Gate").empty());
}

TEST(Resources, FoldersHoldTheirKindOnly){
  Project p;
  newPools(p);
  std::string item = newResource(p, ResourceID::Item);
  PoolPath folder = newFolder(p, true, PoolPath{2});
  ASSERT_EQ(folder.size(), 2u);
  PoolPath at = pathInPool(p.gamePool, ResourceID::Item, item);
  EXPECT_TRUE(movePoolNode(p, true, at, folder));
  at = pathInPool(p.gamePool, ResourceID::Item, item);
  EXPECT_EQ(at.size(), 3u);
  //not into the rooms' folder, nor a folder into itself
  EXPECT_FALSE(movePoolNode(p, true, at, PoolPath{4}));
  folder = PoolPath(at.begin(), at.end()-1);
  EXPECT_FALSE(movePoolNode(p, true, folder, folder));
  //the top folders can't be deleted; a folder goes with what it holds
  deletePoolNode(p, true, PoolPath{2});
  EXPECT_EQ(p.gamePool.size(), 5u);
  deletePoolNode(p, true, folder);
  EXPECT_EQ(p.items.find(item), nullptr);
}

TEST(Resources, MediaAreNamedAfterTheirFiles){
  Project p;
  newPools(p);
  std::string error;
  EXPECT_TRUE(addMedia(p, ResourceID::Image, "/somewhere/Tree.png", PoolPath(), &error)) << error;
  ASSERT_NE(p.images.find("Tree"), nullptr);
  EXPECT_EQ(*p.images.find("Tree"), "/somewhere/Tree.png");
  EXPECT_FALSE(addMedia(p, ResourceID::Image, "/elsewhere/tree.jpg", PoolPath(), &error));
  EXPECT_FALSE(pathInPool(p.mediaPool, ResourceID::Image, "Tree").empty());
}

TEST(BitmapFont, RendersEveryCharacter){
  FontInfo info;
  info.name = "Arial";
  info.size = 12;
  info.outline = 1;
  BitmapFont font;
  std::string error;
  ASSERT_TRUE(renderBitmapFont(info, font, &error)) << error;
  EXPECT_EQ(font.widths.size(), (size_t)kFontChars);
  ASSERT_FALSE(font.images.empty());
  EXPECT_EQ(font.images.size(), font.alphaImages.size());
  EXPECT_GT(font.cellSize.x, 0);
  EXPECT_GT(font.cellSize.y, 0);
  //'W' is wider than 'i'
  EXPECT_GT(font.widths['W'-kFirstFontChar], font.widths['i'-kFirstFontChar]);

  std::string bmp = writeBmp(font.images[0]);
  ASSERT_GT(bmp.size(), 54u);
  EXPECT_EQ(bmp.substr(0, 2), "BM");

  std::string archive = fontArchive(font, 3);
  CGE::ZipReader zip(archive.data(), archive.size());
  std::vector<std::string> names = zip.getEntries();
  EXPECT_TRUE(has(names, "fontdata.003"));
  EXPECT_TRUE(has(names, "font3.bm1"));
  EXPECT_TRUE(has(names, "font3.al1"));
}

TEST(GameFile, CreatesTheTestGame){
  fs::path dir = testFolder("create");
  Project p;
  unpackTestGame(dir, p);
  GameOptions options;
  options.runtimeFolder = utf8(dir/"noruntime");
  std::vector<std::string> warnings;
  std::string error;
  ASSERT_TRUE(writeGame(p, utf8(dir/"game"), utf8(dir/"out"/"Testgame"), options, warnings, &error)) << error;
  fs::path data = dir/"out"/"data";
  ASSERT_TRUE(fs::exists(data/"game.dat"));
  //the game's project file reads back
  Project back;
  EXPECT_TRUE(readProject(utf8(data/"game.dat"), back, &error)) << error;
  EXPECT_EQ(back.rooms.size(), p.rooms.size());
  //every image is in gfx.dat
  std::vector<std::string> gfx = entries(data/"gfx.dat");
  EXPECT_EQ(gfx.size(), p.images.size());
  //a font archive per font
  std::vector<std::string> fonts = entries(data/"fonts.dat");
  EXPECT_EQ(fonts.size(), p.settings.fonts.size());
  EXPECT_TRUE(has(fonts, "font.001"));
}

TEST(GameFile, ExportsAndUnpacksAgain){
  fs::path dir = testFolder("export");
  Project p;
  unpackTestGame(dir, p);
  std::vector<std::string> warnings;
  std::string error;
  fs::path adz = dir/"again.adz";
  ASSERT_TRUE(writeExport(p, utf8(dir/"game"), utf8(adz), warnings, &error)) << error;
  EXPECT_TRUE(warnings.empty());
  std::vector<std::string> names = entries(adz);
  EXPECT_TRUE(has(names, "gfx.dat"));
  Project back;
  std::string adv = unpackAdz(utf8(adz), utf8(dir/"again"), &error);
  ASSERT_FALSE(adv.empty()) << error;
  ASSERT_TRUE(readProject(adv, back, &error)) << error;
  EXPECT_TRUE(resolveMedia(back, utf8(dir/"again")));
  EXPECT_EQ(back.images.size(), p.images.size());
  EXPECT_EQ(back.rooms.size(), p.rooms.size());
  //the project is the same, but for where the media are
  for (auto& e : back.images)
    e.value = fs::path(std::u8string(e.value.begin(), e.value.end())).filename().string();
  for (auto& e : p.images)
    e.value = fs::path(std::u8string(e.value.begin(), e.value.end())).filename().string();
  EXPECT_EQ(writeProjectEntries(back, false), writeProjectEntries(p, false));
}
