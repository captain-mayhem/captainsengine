#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <platform/settings.h>
#include <platform/userdirs.h>

using CGE::Settings;

namespace{

//! a settings file in the test's user data folder, removed afterwards
class SettingsFile : public ::testing::Test{
protected:
  void SetUp() override{
    mFile = CGE::UserDirs::getDataDir() + "/" + ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".ini";
  }
  void TearDown() override{
    std::error_code err;
    std::filesystem::remove(std::filesystem::path(mFile), err);
  }
  std::string mFile;
};

}

TEST(Settings, EmptyStoreReturnsDefaults){
  Settings s;
  EXPECT_FALSE(s.has("missing"));
  EXPECT_EQ(s.getString("missing", "def"), "def");
  EXPECT_EQ(s.getInt("missing", 7), 7);
  EXPECT_DOUBLE_EQ(s.getFloat("missing", 1.5), 1.5);
  EXPECT_TRUE(s.getBool("missing", true));
  EXPECT_TRUE(s.getBinary("missing").empty());
  EXPECT_FALSE(s.isModified());
}

TEST(Settings, TypedValues){
  Settings s;
  s.setString("text", "hello");
  s.setInt("int", -42);
  s.setInt("big", 9007199254740993LL);  //not representable as a double
  s.setFloat("float", 0.1);
  s.setBool("flag", true);
  s.setBinary("bin", {0x00, 0xff, 0x10});
  EXPECT_EQ(s.getString("text"), "hello");
  EXPECT_EQ(s.getInt("int"), -42);
  EXPECT_EQ(s.getInt("big"), 9007199254740993LL);
  EXPECT_DOUBLE_EQ(s.getFloat("float"), 0.1);
  EXPECT_TRUE(s.getBool("flag"));
  EXPECT_EQ(s.getBinary("bin"), (std::vector<uint8_t>{0x00, 0xff, 0x10}));
}

TEST(Settings, WrongTypesGiveTheDefault){
  Settings s;
  s.setString("text", "not a number");
  s.setString("partial", "12abc");
  s.setBinary("bin", {1, 2});
  EXPECT_EQ(s.getInt("text", 3), 3);
  EXPECT_EQ(s.getInt("partial", 3), 3);
  EXPECT_DOUBLE_EQ(s.getFloat("text", 2.5), 2.5);
  EXPECT_EQ(s.getString("bin", "def"), "def");
  EXPECT_TRUE(s.getBinary("text").empty());
}

TEST(Settings, BoolSpellings){
  Settings s;
  for (const char* yes : {"true", "Yes", "ON", "1"}){
    s.setString("b", yes);
    EXPECT_TRUE(s.getBool("b", false)) << yes;
  }
  for (const char* no : {"false", "No", "off", "0"}){
    s.setString("b", no);
    EXPECT_FALSE(s.getBool("b", true)) << no;
  }
  s.setString("b", "maybe");
  EXPECT_TRUE(s.getBool("b", true));
  EXPECT_FALSE(s.getBool("b", false));
}


TEST(Settings, Groups){
  Settings s;
  s.setString("top", "1");
  s.setString("audio/music", "0.5");
  s.setString("profiles/Plain Text/font", "Georgia");
  s.setString("profiles/Plain Text/size", "12");
  s.setString("profiles/Dark/font", "Arial");
  s.setString("profiles/Dark/sub/deep", "x");
  EXPECT_EQ(s.getGroups(), (std::vector<std::string>{"audio", "profiles"}));
  EXPECT_EQ(s.getGroups("profiles"), (std::vector<std::string>{"Dark", "Plain Text"}));
  EXPECT_EQ(s.getGroups("profiles/Dark"), (std::vector<std::string>{"sub"}));
  EXPECT_TRUE(s.getGroups("audio").empty());
  EXPECT_EQ(s.getKeys(), (std::vector<std::string>{"top"}));
  EXPECT_EQ(s.getKeys("profiles/Plain Text"), (std::vector<std::string>{"font", "size"}));
  EXPECT_TRUE(s.getKeys("profiles").empty());
}

TEST(Settings, RemoveGroupRemovesEverythingBelow){
  Settings s;
  s.setString("profiles/Dark/font", "Arial");
  s.setString("profiles/Dark/sub/deep", "x");
  s.setString("profiles/Darker/font", "Times");  //same prefix, other group
  s.removeGroup("profiles/Dark");
  EXPECT_FALSE(s.has("profiles/Dark/font"));
  EXPECT_FALSE(s.has("profiles/Dark/sub/deep"));
  EXPECT_TRUE(s.has("profiles/Darker/font"));
  EXPECT_EQ(s.getGroups("profiles"), (std::vector<std::string>{"Darker"}));
}

TEST(Settings, RemoveValue){
  Settings s;
  s.setString("group/a", "1");
  s.setString("group/b", "2");
  s.remove("group/a");
  EXPECT_FALSE(s.has("group/a"));
  EXPECT_TRUE(s.has("group/b"));
  s.remove("group/b");
  //a group without values disappears
  EXPECT_TRUE(s.getGroups().empty());
}

TEST_F(SettingsFile, SaveAndLoadRoundTrip){
  const std::string tricky = "a=b \\ back\nnew\rret [x] ;not a comment";
  Settings s;
  s.setString("name", tricky);
  s.setString("empty", "");
  s.setInt("audio/volume", -42);
  s.setFloat("audio/music", 1.0 / 3.0);
  s.setBool("video/fullscreen", true);
  s.setBinary("profiles/Plain Text/swatches", {0x00, 0xff, 0x10, 0xab});
  s.setString("profiles/Plain Text/font", "Georgia");
  ASSERT_TRUE(s.save(mFile));
  EXPECT_FALSE(s.isModified());

  Settings t;
  ASSERT_TRUE(t.load(mFile));
  EXPECT_FALSE(t.isModified());
  EXPECT_EQ(t.getString("name"), tricky);
  EXPECT_TRUE(t.has("empty"));
  EXPECT_EQ(t.getString("empty", "def"), "");
  EXPECT_EQ(t.getInt("audio/volume"), -42);
  EXPECT_DOUBLE_EQ(t.getFloat("audio/music"), 1.0 / 3.0);
  EXPECT_TRUE(t.getBool("video/fullscreen"));
  EXPECT_EQ(t.getBinary("profiles/Plain Text/swatches"), (std::vector<uint8_t>{0x00, 0xff, 0x10, 0xab}));
  EXPECT_EQ(t.getString("profiles/Plain Text/font"), "Georgia");
  EXPECT_EQ(t.getGroups(), (std::vector<std::string>{"audio", "profiles", "video"}));
}

TEST_F(SettingsFile, ModifiedOnlyByRealChanges){
  Settings s;
  s.setString("a", "1");
  s.setBinary("b", {1});
  EXPECT_TRUE(s.isModified());
  ASSERT_TRUE(s.save(mFile));
  EXPECT_FALSE(s.isModified());
  //the same values again, and removing what isn't there
  s.setString("a", "1");
  s.setBinary("b", {1});
  s.remove("missing");
  s.removeGroup("missing");
  EXPECT_FALSE(s.isModified());
  s.setString("a", "2");
  EXPECT_TRUE(s.isModified());
}

TEST_F(SettingsFile, SaveReplacesTheFileAndLeavesNoTemporaryFile){
  Settings s;
  s.setString("a", "1");
  ASSERT_TRUE(s.save(mFile));
  s.setString("a", "2");
  ASSERT_TRUE(s.save(mFile));
  Settings t;
  ASSERT_TRUE(t.load(mFile));
  EXPECT_EQ(t.getString("a"), "2");
  EXPECT_FALSE(std::filesystem::exists(std::filesystem::path(mFile + ".tmp")));
}

TEST_F(SettingsFile, LoadReadsHandWrittenFiles){
  //comments, blank lines, CRLF line ends, lines without '='
  FILE* f = fopen(mFile.c_str(), "wb");
  ASSERT_NE(f, nullptr);
  fputs("; comment\r\n# comment\r\n\r\ntop=1\r\n[group]\r\nnoequals\r\nkey= spaced value \r\n", f);
  fclose(f);
  Settings s;
  ASSERT_TRUE(s.load(mFile));
  EXPECT_EQ(s.getString("top"), "1");
  EXPECT_EQ(s.getString("group/key"), " spaced value ");
  EXPECT_EQ(s.getKeys("group"), (std::vector<std::string>{"key"}));
}

TEST_F(SettingsFile, LoadingAMissingFileFailsAndEmptiesTheStore){
  Settings s;
  s.setString("a", "1");
  EXPECT_FALSE(s.load(mFile + ".missing"));
  EXPECT_FALSE(s.has("a"));
}

TEST(Settings, UserStoreIsSavedInTheConfigFolder){
  Settings::user().setString("test/value", "x");
  ASSERT_TRUE(Settings::saveUser());
  EXPECT_FALSE(Settings::user().isModified());
  Settings t;
  ASSERT_TRUE(t.load(CGE::UserDirs::getConfigDir() + "/settings.ini"));
  EXPECT_EQ(t.getString("test/value"), "x");
}
