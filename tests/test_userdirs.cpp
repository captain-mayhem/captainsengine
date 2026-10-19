#include <cstdio>
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>

#include <platform/assets.h>
#include <platform/userdirs.h>
#include <system/file.h>

// main() sets a per-process application name and, on Linux, points the XDG
// folders to a temporary folder.

using CGE::UserDirs;

TEST(UserDirs, FoldersAreCreatedAbsoluteAndNamedAfterTheApp){
  const std::string data = UserDirs::getDataDir();
  const std::string config = UserDirs::getConfigDir();
  for (const std::string& dir : {data, config}){
    EXPECT_TRUE(CGE::Assets::isAbsolute(dir)) << dir;
    EXPECT_TRUE(CGE::Filesystem::exists(dir)) << dir;
    EXPECT_EQ(CGE::Filesystem::getFileComponent(dir), UserDirs::getAppName()) << dir;
    EXPECT_NE(dir.back(), '/');
  }
}

TEST(UserDirs, FilesCanBeWrittenAndReadBack){
  const std::string file = UserDirs::getDataDir() + "/userdirs_test.txt";
  FILE* f = fopen(file.c_str(), "w");
  ASSERT_NE(f, nullptr);
  fputs("saved", f);
  fclose(f);
  UserDirs::commit();
  std::string text;
  EXPECT_TRUE(CGE::Assets::loadText(file, text));
  EXPECT_EQ(text, "saved");
  remove(file.c_str());
}

#if defined(_WIN32)
TEST(UserDirs, WindowsUsesRoamingAppData){
  const char* appdata = getenv("APPDATA");
  ASSERT_NE(appdata, nullptr);
  EXPECT_EQ(UserDirs::getDataDir(), std::string(appdata) + "/" + UserDirs::getAppName());
  EXPECT_EQ(UserDirs::getConfigDir(), UserDirs::getDataDir());
}
#elif defined(LINUX)
TEST(UserDirs, LinuxFollowsXdg){
  EXPECT_EQ(UserDirs::getDataDir(), std::string(getenv("XDG_DATA_HOME")) + "/" + UserDirs::getAppName());
  EXPECT_EQ(UserDirs::getConfigDir(), std::string(getenv("XDG_CONFIG_HOME")) + "/" + UserDirs::getAppName());
}
#endif
