#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <io/MemReader.h>
#include <platform/assets.h>

#include "testenv.h"

using CGE::Assets;

// main() sets the asset folder to tests/data.

TEST(Assets, RootIsTheTestDataFolder){
  EXPECT_EQ(Assets::getRoot(), TestEnv::dataDir());
  EXPECT_EQ(Assets::getFilePath("folder/a.txt"), TestEnv::dataDir() + "/folder/a.txt");
}

TEST(Assets, Exists){
  EXPECT_TRUE(Assets::exists("image.png"));
  EXPECT_TRUE(Assets::exists("folder/a.txt"));
  EXPECT_FALSE(Assets::exists("missing.txt"));
  EXPECT_FALSE(Assets::exists("folder/missing.txt"));
}

TEST(Assets, LoadText){
  std::string text;
  ASSERT_TRUE(Assets::loadText("folder/a.txt", text));
  EXPECT_EQ(text, "first\n");
  EXPECT_FALSE(Assets::loadText("missing.txt", text));
  EXPECT_TRUE(text.empty());
}

TEST(Assets, LoadBinary){
  std::vector<uint8> data;
  ASSERT_TRUE(Assets::load("image.png", data));
  ASSERT_EQ(data.size(), 78u);
  EXPECT_EQ(data[1], 'P');
  EXPECT_EQ(data[2], 'N');
  EXPECT_EQ(data[3], 'G');
  data.push_back(1);
  EXPECT_FALSE(Assets::load("missing.bin", data));
  EXPECT_TRUE(data.empty());
}

TEST(Assets, OpenGivesAReaderThatOwnsTheData){
  std::unique_ptr<CGE::MemReader> rdr = Assets::open("folder/b.txt");
  ASSERT_NE(rdr, nullptr);
  EXPECT_EQ(rdr->getSize(), 7u);
  EXPECT_EQ(rdr->readChar(), 's');
  EXPECT_EQ(rdr->readChar(), 'e');
  EXPECT_EQ(Assets::open("missing.txt"), nullptr);
}

TEST(Assets, AbsolutePathsArePlainFiles){
  std::string text;
  ASSERT_TRUE(Assets::loadText(TestEnv::dataFile("folder/a.txt"), text));
  EXPECT_EQ(text, "first\n");
  EXPECT_EQ(Assets::getFilePath(TestEnv::dataFile("x")), TestEnv::dataFile("x"));
}

TEST(Assets, IsAbsolute){
  EXPECT_TRUE(Assets::isAbsolute("/usr/share"));
  EXPECT_TRUE(Assets::isAbsolute("\\\\server\\share"));
  EXPECT_TRUE(Assets::isAbsolute("C:\\data"));
  EXPECT_TRUE(Assets::isAbsolute("c:/data"));
  EXPECT_FALSE(Assets::isAbsolute("data/file"));
  EXPECT_FALSE(Assets::isAbsolute("./file"));
  EXPECT_FALSE(Assets::isAbsolute(""));
}

TEST(Assets, ListFiles){
  std::vector<std::string> files = Assets::listFiles("folder");
  std::sort(files.begin(), files.end());
  EXPECT_EQ(files, (std::vector<std::string>{"a.txt", "b.txt"}));
  //only files, not folders
  std::vector<std::string> top = Assets::listFiles("");
  EXPECT_NE(std::find(top.begin(), top.end(), "image.png"), top.end());
  EXPECT_EQ(std::find(top.begin(), top.end(), "folder"), top.end());
  EXPECT_TRUE(Assets::listFiles("missing").empty());
}

TEST(Assets, SetRootIgnoresTrailingSeparators){
  const std::string old = Assets::getRoot();
  Assets::setRoot(TestEnv::dataDir() + "/folder/");
  EXPECT_EQ(Assets::getRoot(), TestEnv::dataDir() + "/folder");
  EXPECT_TRUE(Assets::exists("a.txt"));
  Assets::setRoot(old);
}
