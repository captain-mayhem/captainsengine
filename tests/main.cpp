// Test runner of the engine's unit tests. Every test process gets its own user
// folders (UserDirs), so that parallel runs (ctest -j) don't share files, and
// removes them at the end.

#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include <platform/assets.h>
#include <platform/userdirs.h>

#include "testenv.h"

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

int main(int argc, char** argv){
  ::testing::InitGoogleTest(&argc, argv);

  CGE::Assets::setRoot(TestEnv::dataDir());

  const std::string appName = "cge_tests_" + std::to_string(getpid());
  CGE::UserDirs::setAppName(appName);
#if !defined(_WIN32)
  //keep the user's real XDG folders untouched (Windows has no such override)
  const std::filesystem::path xdgRoot = std::filesystem::temp_directory_path() / appName;
  setenv("XDG_DATA_HOME", (xdgRoot / "data").string().c_str(), 1);
  setenv("XDG_CONFIG_HOME", (xdgRoot / "config").string().c_str(), 1);
#endif

  int result = RUN_ALL_TESTS();

  std::error_code err;
  std::filesystem::remove_all(std::filesystem::path(CGE::UserDirs::getDataDir()), err);
  std::filesystem::remove_all(std::filesystem::path(CGE::UserDirs::getConfigDir()), err);
#if !defined(_WIN32)
  std::filesystem::remove_all(xdgRoot, err);
#endif
  return result;
}
