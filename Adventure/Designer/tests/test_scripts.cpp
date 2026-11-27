#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <io/ZipReader.h>

#include <model/languages.h>
#include <model/pcdkparser.h>
#include <model/projectfile.h>
#include <model/scriptcheck.h>

// The script tools: the C# StoryDesigner's line parser and its language list
// (data/<game>_lang_cs.adv is what the C# program wrote after updating the
// list of data/<game>_cs.adv), and the check with the game's parsers.

using namespace sd;

namespace{

std::string readAll(CGE::MemReader rdr){
  std::string ret(rdr.getSize(), '\0');
  if (!ret.empty())
    rdr.readBytes((unsigned char*)&ret[0], (unsigned)ret.size());
  return ret;
}

void expectLanguagesAsByCSharp(const std::string& game){
  Project project;
  std::string error;
  ASSERT_TRUE(readProject(std::string(SD_TEST_DATA_DIR) + "/" + game + "_cs.adv", project, &error)) << error;
  updateLanguageList(project);
  std::vector<std::pair<std::string, std::string>> written = writeProjectEntries(project, false);
  CGE::ZipReader zip(std::string(SD_TEST_DATA_DIR) + "/" + game + "_lang_cs.adv");
  std::vector<std::string> names = zip.getEntries();
  ASSERT_EQ(written.size(), names.size());
  for (size_t i = 0; i < written.size(); ++i)
    EXPECT_TRUE(written[i].second == readAll(zip.openEntry(names[i]))) << names[i] << " differs";
}

}

TEST(Scripts, LanguageListIsUpdatedAsByTheCSharpStoryDesigner){
  expectLanguagesAsByCSharp("testadventure");
}

TEST(Scripts, LuaLanguageListIsUpdatedAsByTheCSharpStoryDesigner){
  expectLanguagesAsByCSharp("airfight");
}

TEST(Scripts, UnusedTextsLeaveEveryLanguage){
  Project project;
  Language german;
  german.name = "german";
  project.languages.add("origin", Language{"origin", {}});
  project.languages.find("origin")->sections[Language::Showinfo] = {"Door", "Old lamp", "Safe"};
  german.sections[Language::Showinfo] = {"T\xFCr", "Alte Lampe", "Tresor"};
  project.languages.add("german", german);
  Script scr;
  scr.name = "Intro";
  scr.text = std::string("showinfo(Safe)\nshowinfo(Door)\nshowinfo(Window)");
  project.scripts[Script::Cutscene].add("intro", scr);
  updateLanguageList(project);
  EXPECT_EQ(project.languages.find("origin")->sections[Language::Showinfo], (std::vector<std::string>{"Door", "Safe", "Window"}));
  EXPECT_EQ(project.languages.find("german")->sections[Language::Showinfo], (std::vector<std::string>{"T\xFCr", "Tresor"}));
}

TEST(Scripts, ParserKnowsTheCommandsArguments){
  PcdkParser pcdk(ScriptLang::PCDK);
  const PcdkParser::ArgDefs* speech = pcdk.function("speech");
  ASSERT_NE(speech, nullptr);
  ASSERT_GE(speech->size(), 2u);
  EXPECT_EQ((*speech)[0].type, PcdkParser::ArgType::Character);
  EXPECT_TRUE(pcdk.isKeyword("if_bool"));
  EXPECT_EQ(pcdk.function("is_bool"), nullptr);
  PcdkParser lua(ScriptLang::Lua);
  EXPECT_NE(lua.function("is_bool"), nullptr);
  EXPECT_TRUE(lua.isKeyword("then"));

  std::vector<std::string> seen;
  pcdk.onFunction = [&seen](const PcdkParser::Argument& func, const std::vector<PcdkParser::Argument>& args, int){
    seen.push_back(func.text);
    for (const PcdkParser::Argument& arg : args)
      seen.push_back(arg.text + ":" + arg.definition.name);
  };
  pcdk.parseText("on(click){\n  setbool( found ; true )\n}");
  EXPECT_EQ(seen, (std::vector<std::string>{"on", "click:Event", "setbool", "found:Bool Name", "true:true / false"}));
}

TEST(Scripts, TestGamesScriptsAreReadByTheGame){
  for (const char* game : {"testadventure", "airfight"}){
    Project project;
    std::string error;
    ASSERT_TRUE(readProject(std::string(SD_TEST_DATA_DIR) + "/" + game + "_cs.adv", project, &error)) << error;
    int count = 0;
    for (int type = 0; type < Script::NumTypes; ++type){
      for (const auto& entry : project.scripts[type]){
        if (!entry.value.text)
          continue;
        ++count;
        std::vector<ScriptError> errors = checkScript(*entry.value.text, project.settings.scriptingLanguage);
        EXPECT_TRUE(errors.empty()) << game << ": " << entry.value.name << " line " << errors[0].line << ": " << errors[0].message;
      }
    }
    EXPECT_GT(count, 0) << game;
  }
}

TEST(Scripts, ErrorsAreFoundInTheirLines){
  std::vector<ScriptError> errors = checkScript("on (click){\n  speech(self; Hello)\n  setbool(found; true\n}\n", ScriptLang::PCDK);
  ASSERT_FALSE(errors.empty());
  EXPECT_EQ(errors[0].line, 4) << errors[0].message;
  EXPECT_FALSE(errors[0].message.empty());
  EXPECT_TRUE(checkScript("on (click){\n  speech(self; Hello)\n}\n", ScriptLang::PCDK).empty());

  errors = checkScript("on.click = function()\n  x = = 1\nend\n", ScriptLang::Lua);
  ASSERT_EQ(errors.size(), 1u);
  EXPECT_EQ(errors[0].line, 2);
  EXPECT_TRUE(checkScript("x = 1\n", ScriptLang::Lua).empty());
}
