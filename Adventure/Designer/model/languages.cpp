#include "languages.h"

#include "pcdkparser.h"

// A port of AdvData.updateLanguageList and Language.addWord, beginRefresh and
// endRefresh of the C# StoryDesigner.

namespace sd{

namespace{

class Refresh{
public:
  Refresh(Project& project) : project_(project){
    origin_ = project_.languages.find("origin");
    if (origin_ == nullptr){
      Language origin;
      origin.name = "origin";
      project_.languages.add("origin", origin);
      origin_ = project_.languages.find("origin");
    }
    for (int i = 0; i < Language::NumSections; ++i)
      encountered_[i].assign(origin_->sections[i].size(), false);
  }

  //! a text (and for speech its voice) that a script or the settings use
  void addWord(Language::Section section, const std::string& text, const std::string* voice){
    const int sect = section;
    int sect2 = -1;
    if (section == Language::Speech)
      sect2 = Language::SpeechSounds;
    else if (section == Language::Offspeech)
      sect2 = Language::OffspeechSounds;
    std::vector<std::string>& words = origin_->sections[sect];
    for (size_t i = 0; i < words.size(); ++i){
      if (words[i] != text)
        continue;
      if (sect2 == -1){
        encountered_[sect][i] = true;
        return;
      }
      if (i < origin_->sections[sect2].size() && origin_->sections[sect2][i] == *voice){
        encountered_[sect][i] = true;
        encountered_[sect2][i] = true;
        return;
      }
    }
    words.push_back(text);
    encountered_[sect].push_back(true);
    if (sect2 != -1){
      origin_->sections[sect2].push_back(*voice);
      encountered_[sect2].push_back(true);
    }
  }

  void function(const PcdkParser::Argument& func, const std::vector<PcdkParser::Argument>& args){
    //the C# program fails on a call with too few arguments; this skips it
    const std::string& name = func.text;
    if (name == "speech" && args.size() > 1){
      std::string voice;
      if (args.size() > 2 && args[2].text != "dontwait")
        voice = args[2].text;
      addWord(Language::Speech, args[1].text, &voice);
    }
    else if (name == "offspeech" && args.size() > 2){
      std::string voice;
      if (args.size() > 3 && args[3].text != "dontwait")
        voice = args[3].text;
      addWord(Language::Offspeech, args[2].text, &voice);
    }
    else if (name == "showinfo" && args.size() > 0)
      addWord(Language::Showinfo, args[0].text, nullptr);
    else if (name == "textout" && args.size() > 1)
      addWord(Language::Textout, args[1].text, nullptr);
    else if (name == "setstring" && args.size() > 1)
      addWord(Language::Setstring, args[1].text, nullptr);
    else if (name == "row" && args.size() > 1)
      addWord(Language::Textscenes, args[1].text, nullptr);
  }

  //! removes what wasn't encountered from every language
  void end(){
    for (int i = 0; i < Language::NumSections; ++i){
      for (int j = (int)encountered_[i].size()-1; j >= 0; --j){
        if (encountered_[i][j])
          continue;
        for (auto& lang : project_.languages){
          std::vector<std::string>& words = lang.value.sections[i];
          //a translation can be shorter than origin
          if (j < (int)words.size())
            words.erase(words.begin()+j);
        }
      }
    }
  }

private:
  Project& project_;
  Language* origin_ = nullptr;
  std::vector<bool> encountered_[Language::NumSections];
};

}

void updateLanguageList(Project& project){
  Refresh refresh(project);
  PcdkParser parser(project.settings.scriptingLanguage);
  parser.onFunction = [&refresh](const PcdkParser::Argument& func, const std::vector<PcdkParser::Argument>& args, int){
    refresh.function(func, args);
  };
  for (int type = 0; type < Script::NumTypes; ++type){
    for (const auto& entry : project.scripts[type]){
      if (entry.value.text)
        parser.parseText(*entry.value.text);
    }
  }
  const ProjectSettings& settings = project.settings;
  refresh.addWord(Language::Commands, settings.walkText, nullptr);
  refresh.addWord(Language::Commands, settings.linkText, nullptr);
  refresh.addWord(Language::Commands, settings.giveLink, nullptr);
  for (const auto& cmd : settings.commands)
    refresh.addWord(Language::Commands, cmd.second, nullptr);
  refresh.end();
}

}
