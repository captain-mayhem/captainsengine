#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <io/ZipReader.h>

#include <model/projectfile.h>

// The project files against the C# StoryDesigner: data/<game>_cs.adv is what
// it wrote after reading the test game (its reader and writer, run with Mono,
// with Windows' line ends). Reading the game and writing it here must give
// the same entries, byte for byte.

using namespace sd;

namespace{

std::string readAll(CGE::MemReader rdr){
  std::string ret(rdr.getSize(), '\0');
  if (!ret.empty())
    rdr.readBytes((unsigned char*)&ret[0], (unsigned)ret.size());
  return ret;
}

//! the entries of a zip archive by name
std::vector<std::pair<std::string, std::string>> zipEntries(CGE::ZipReader& zip){
  std::vector<std::pair<std::string, std::string>> entries;
  for (const std::string& name : zip.getEntries())
    entries.push_back(std::make_pair(name, readAll(zip.openEntry(name))));
  return entries;
}

//! the .adv project inside a game's .adz archive
std::string advFromAdz(const std::string& adz, std::string& advName){
  CGE::ZipReader zip(adz);
  for (const std::string& name : zip.getEntries()){
    if (name.size() > 4 && name.substr(name.size()-4) == ".adv"){
      advName = name;
      return readAll(zip.openEntry(name));
    }
  }
  return std::string();
}

//! the first line where two texts differ, for the failure message
std::string firstDifference(const std::string& a, const std::string& b){
  size_t i = 0;
  while (i < a.size() && i < b.size() && a[i] == b[i])
    ++i;
  size_t start = a.rfind('\n', i == 0 ? 0 : i-1);
  start = start == std::string::npos ? 0 : start+1;
  return "at byte " + std::to_string(i) + ": \"" + a.substr(start, i-start+20) + "\" vs \"" + b.substr(start, i-start+20) + "\"";
}

//! reads the game, writes it and compares with what the C# StoryDesigner wrote
void expectWrittenAsByCSharp(const std::string& adz, const std::string& golden){
  std::string advName;
  std::string adv = advFromAdz(std::string(SD_TEST_GAMES_DIR) + "/" + adz, advName);
  ASSERT_FALSE(adv.empty());
  Project project;
  std::string error;
  ASSERT_TRUE(readProject(adv.data(), adv.size(), advName, project, &error)) << error;
  std::vector<std::pair<std::string, std::string>> written = writeProjectEntries(project, false);

  CGE::ZipReader zip(std::string(SD_TEST_DATA_DIR) + "/" + golden);
  std::vector<std::pair<std::string, std::string>> expected = zipEntries(zip);
  ASSERT_EQ(written.size(), expected.size());
  for (size_t i = 0; i < written.size(); ++i){
    //the C# writer named the entries after its output file
    const std::string& name = expected[i].first;
    EXPECT_EQ(name.substr(name.size()-4), written[i].first);
    EXPECT_TRUE(written[i].second == expected[i].second) << name << " differs " << firstDifference(written[i].second, expected[i].second);
  }
}

}

TEST(ProjectFile, TestAdventureIsWrittenAsByTheCSharpStoryDesigner){
  expectWrittenAsByCSharp("testadventure.adz", "testadventure_cs.adv");
}

TEST(ProjectFile, AirfightIsWrittenAsByTheCSharpStoryDesigner){
  expectWrittenAsByCSharp("airfight.adz", "airfight_cs.adv");
}

TEST(ProjectFile, ReadsWhatItWrote){
  Project project;
  std::string error;
  std::string golden = std::string(SD_TEST_DATA_DIR) + "/testadventure_cs.adv";
  ASSERT_TRUE(readProject(golden, project, &error)) << error;
  EXPECT_EQ(project.settings.projectName, "testadventure_cs");
  EXPECT_EQ(project.settings.resolution.x, 640);
  EXPECT_GT(project.rooms.size(), 0u);
  EXPECT_GT(project.mediaPool.size(), 0u);
  EXPECT_GT(project.gamePool.size(), 0u);
  //what was read from its own output is written the same again
  std::vector<std::pair<std::string, std::string>> written = writeProjectEntries(project, false);
  CGE::ZipReader zip(golden);
  std::vector<std::pair<std::string, std::string>> expected = zipEntries(zip);
  ASSERT_EQ(written.size(), expected.size());
  for (size_t i = 0; i < written.size(); ++i)
    EXPECT_TRUE(written[i].second == expected[i].second) << expected[i].first << " differs " << firstDifference(written[i].second, expected[i].second);
}

TEST(ProjectFile, CaseIsIgnoredInNames){
  EXPECT_EQ(toLower("Room \xC4\x8A Z"), "room \xE4\x9A z");
  NamedList<int> list;
  EXPECT_TRUE(list.add("Kitchen", 1));
  EXPECT_FALSE(list.add("KITCHEN", 2));
  ASSERT_NE(list.find("kitchen"), nullptr);
  EXPECT_EQ(*list.find("kitchen"), 1);
  EXPECT_TRUE(list.add("Hall", 3));
  EXPECT_TRUE(list.remove("kitchen"));
  ASSERT_NE(list.find("hall"), nullptr);
  EXPECT_EQ(*list.find("hall"), 3);
  EXPECT_EQ(list.begin()->key, "hall");
}

TEST(ProjectFile, MissingFileIsAnError){
  Project project;
  std::string error;
  EXPECT_FALSE(readProject(std::string(SD_TEST_DATA_DIR) + "/missing.adv", project, &error));
  EXPECT_FALSE(error.empty());
}

TEST(ProjectFile, WritesAnArchiveNamedAfterTheFile){
  Project project;
  std::string error;
  ASSERT_TRUE(readProject(std::string(SD_TEST_DATA_DIR) + "/airfight_cs.adv", project, &error)) << error;
  std::string path = (std::filesystem::temp_directory_path() / "sd_test_written.adv").string();
  ASSERT_TRUE(writeProject(path, project, &error)) << error;
  std::vector<std::pair<std::string, std::string>> entries;
  {
    //closed before the file is removed, which Windows refuses for an open file
    CGE::ZipReader zip(path);
    entries = zipEntries(zip);
  }
  std::vector<std::pair<std::string, std::string>> expected = writeProjectEntries(project, false);
  Project again;
  bool read = readProject(path, again, &error);
  std::error_code ec;
  std::filesystem::remove(path, ec);
  EXPECT_FALSE(ec) << ec.message();
  ASSERT_EQ(entries.size(), expected.size());
  for (size_t i = 0; i < entries.size(); ++i){
    EXPECT_EQ(entries[i].first, "sd_test_written" + expected[i].first);
    EXPECT_GT(entries[i].second.size(), 0u);
    EXPECT_TRUE(entries[i].second == expected[i].second);
  }
  ASSERT_TRUE(read) << error;
  EXPECT_EQ(again.rooms.size(), project.rooms.size());
}

TEST(ProjectFile, TextsAreShownInUtf8){
  //a umlaut, the euro sign (0x80) and a curly quote (0x93)
  EXPECT_EQ(toUtf8("K\xE4se \x80 \x93"), "K\xC3\xA4se \xE2\x82\xAC \xE2\x80\x9C");
  EXPECT_EQ(fromUtf8("K\xC3\xA4se \xE2\x82\xAC \xE2\x80\x9C"), "K\xE4se \x80 \x93");
  //a gem stone isn't in Windows-1252
  EXPECT_EQ(fromUtf8("\xF0\x9F\x92\x8E!"), "?!");
}
