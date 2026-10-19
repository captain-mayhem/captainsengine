#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <platform/userdirs.h>
#include <system/file.h>

#include "testenv.h"

using CGE::Filesystem;

TEST(Filesystem, PathComponents){
  EXPECT_EQ(Filesystem::getPathComponent("a/b/c.txt"), "a/b");
  EXPECT_EQ(Filesystem::getPathComponent("a\\b\\c.txt"), "a\\b");
  EXPECT_EQ(Filesystem::getFileComponent("a/b/c.txt"), "c.txt");
  EXPECT_EQ(Filesystem::getFileComponent("a\\b\\c.txt"), "c.txt");
  EXPECT_EQ(Filesystem::getFileComponent("c.txt"), "c.txt");
  EXPECT_EQ(Filesystem::getExtension("a/b/c.tar.gz"), "gz");
  EXPECT_EQ(Filesystem::getExtension("noext"), "");
}

TEST(Filesystem, ExistsAndListing){
  EXPECT_TRUE(Filesystem::exists(TestEnv::dataFile("image.png")));
  EXPECT_TRUE(Filesystem::exists(TestEnv::dataFile("folder")));
  EXPECT_FALSE(Filesystem::exists(TestEnv::dataFile("missing")));
  std::vector<std::string> files = Filesystem::getFiles(TestEnv::dataFile("folder"));
  std::sort(files.begin(), files.end());
  EXPECT_EQ(files, (std::vector<std::string>{"a.txt", "b.txt"}));
  std::vector<std::string> dirs = Filesystem::getDirectories(TestEnv::dataDir());
  EXPECT_NE(std::find(dirs.begin(), dirs.end(), "folder"), dirs.end());
}

TEST(Filesystem, CreateDirsAndRemoveRecursively){
  const std::string base = CGE::UserDirs::getDataDir() + "/fs_test";
  const std::string deep = base + "/one/two/three";
  ASSERT_TRUE(Filesystem::createDirs(deep));
  EXPECT_TRUE(Filesystem::exists(deep));
  //an existing folder is fine
  EXPECT_TRUE(Filesystem::createDirs(deep));
  FILE* f = fopen((deep + "/file.txt").c_str(), "w");
  ASSERT_NE(f, nullptr);
  fclose(f);
  EXPECT_TRUE(Filesystem::removeDir(base, true));
  EXPECT_FALSE(Filesystem::exists(base));
}
