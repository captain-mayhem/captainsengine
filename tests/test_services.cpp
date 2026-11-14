#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <platform/services.h>

using CGE::Platform;

// The tests run without a window; what needs one (the clipboard, visible
// cursors) is checked in the applications.

TEST(Platform, TickClockAdvancesInMilliseconds){
  uint64_t t0 = Platform::getTickMs();
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  uint64_t t1 = Platform::getTickMs();
  EXPECT_GE(t1, t0 + 25);
  EXPECT_LT(t1, t0 + 2000);
}

TEST(Platform, CursorIsRememberedWithoutAWindow){
  Platform::setCursor(Platform::CURSOR_WAIT);
  EXPECT_EQ(Platform::getCursor(), Platform::CURSOR_WAIT);
  Platform::setCursor(Platform::CURSOR_ARROW);
  EXPECT_EQ(Platform::getCursor(), Platform::CURSOR_ARROW);
}

TEST(Platform, ClipboardNeedsAWindow){
  EXPECT_FALSE(Platform::setClipboardText("text"));
}

TEST(Platform, LanguagesAreTags){
  //whatever this machine prefers: a language, maybe a region, no POSIX parts
  for (const std::string& tag : Platform::getLanguages()){
    EXPECT_FALSE(tag.empty());
    EXPECT_EQ(tag.find_first_of("_.@ "), std::string::npos) << tag;
    EXPECT_NE(tag, "C");
  }
}

#if defined(LINUX)
namespace{
//! sets the locale's variables for a test and restores them afterwards
class LocaleEnv{
public:
  LocaleEnv(){
    for (int i = 0; i < 4; ++i){
      const char* value = getenv(kVars[i]);
      had_[i] = value != nullptr;
      saved_[i] = value ? value : "";
      unsetenv(kVars[i]);
    }
  }
  ~LocaleEnv(){
    for (int i = 0; i < 4; ++i){
      if (had_[i])
        setenv(kVars[i], saved_[i].c_str(), 1);
      else
        unsetenv(kVars[i]);
    }
  }
  static constexpr const char* kVars[4] = {"LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"};
private:
  bool had_[4];
  std::string saved_[4];
};
}

TEST(Platform, LanguagesFromTheEnvironment){
  LocaleEnv env;
  EXPECT_TRUE(Platform::getLanguages().empty());
  setenv("LANG", "de_DE.UTF-8", 1);
  EXPECT_EQ(Platform::getLanguages(), (std::vector<std::string>{"de-DE"}));
  //LC_MESSAGES before LANG, LC_ALL before both
  setenv("LC_MESSAGES", "fr_FR@euro", 1);
  EXPECT_EQ(Platform::getLanguages(), (std::vector<std::string>{"fr-FR"}));
  setenv("LC_ALL", "en_GB.UTF-8", 1);
  EXPECT_EQ(Platform::getLanguages(), (std::vector<std::string>{"en-GB"}));
  //LANGUAGE's list first, without repeating the locale's
  setenv("LANGUAGE", "de_AT:de::en_GB", 1);
  EXPECT_EQ(Platform::getLanguages(), (std::vector<std::string>{"de-AT", "de", "en-GB"}));
  //nothing for the C locale, also with a list
  setenv("LC_ALL", "C.UTF-8", 1);
  EXPECT_TRUE(Platform::getLanguages().empty());
  setenv("LC_ALL", "POSIX", 1);
  EXPECT_TRUE(Platform::getLanguages().empty());
}
#endif

TEST(Platform, ImportedNames){
  const std::filesystem::path dir = std::filesystem::temp_directory_path() / "cge_test_imported_names";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  const std::string folder = dir.string();
  // a file's own name, made safe
  EXPECT_EQ(Platform::importedName(folder, "woods.zip", false), "woods.zip");
  EXPECT_EQ(Platform::importedName(folder, "a/b\\c\td.zip", false), "a_b_c_d.zip");
  EXPECT_EQ(Platform::importedName(folder, "", false), "upload");
  EXPECT_EQ(Platform::importedName(folder, "..", false), "upload");
  // a taken name: another one, or the same one to replace it
  std::ofstream(dir / "woods.zip") << "x";
  std::ofstream(dir / "woods (2).zip") << "x";
  std::ofstream(dir / "notes") << "x";
  std::ofstream(dir / ".hidden") << "x";
  EXPECT_EQ(Platform::importedName(folder, "woods.zip", false), "woods (3).zip");
  EXPECT_EQ(Platform::importedName(folder, "woods.zip", true), "woods.zip");
  EXPECT_EQ(Platform::importedName(folder, "notes", false), "notes (2)");
  EXPECT_EQ(Platform::importedName(folder, ".hidden", false), ".hidden (2)");
  std::filesystem::remove_all(dir);
}

TEST(Platform, NoTransferUntilOneStarts){
  std::string name = "x";
  EXPECT_EQ(Platform::pollTransfer(&name), Platform::TRANSFER_NONE);
  EXPECT_EQ(name, "");
#if defined(LINUX)
  // nothing to choose with on Linux
  EXPECT_FALSE(Platform::canTransferFiles());
  EXPECT_FALSE(Platform::importFile("/tmp", ".zip", false));
  EXPECT_FALSE(Platform::exportFile("/tmp/x.zip"));
  EXPECT_EQ(Platform::pollTransfer(), Platform::TRANSFER_NONE);
#elif defined(WIN32)
  EXPECT_TRUE(Platform::canTransferFiles());
  // a file that isn't there isn't exported (and no dialog opens)
  EXPECT_FALSE(Platform::exportFile("C:/no/such/file.zip"));
  EXPECT_EQ(Platform::pollTransfer(), Platform::TRANSFER_NONE);
#endif
}
