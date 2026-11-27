#ifndef SD_SCRIPTEDITOR_H
#define SD_SCRIPTEDITOR_H

#include <string>
#include <vector>

#include <imgui.h>

#include <model/pcdkparser.h>
#include <model/project.h>
#include <model/scriptcheck.h>

namespace sd{

//! the editor of one script: the text, the game parser's errors below it, and
//! the C# editor's help for the command or argument at the cursor (what it is,
//! and the names that fit, which a click puts in)
class ScriptEditor{
public:
  //! draws the editor; true when it changed the script
  bool draw(Script& script, const Project& project);
private:
  struct Help{
    std::string info;
    std::vector<std::string> matches;
    //! the word the cursor is in, in the UTF-8 text
    int wordStart = 0;
    int wordEnd = 0;
  };

  static int callback(ImGuiInputTextCallbackData* data);
  void update(const Project& project);
  void helpAt(const Project& project);
  void fillMatches(const PcdkParser& parser, const PcdkParser::ArgDef& def, const std::string& typed, const Project& project);

  //! the text being edited (UTF-8, tabs kept); the model gets it with spaces
  std::string utf8_;
  //! what the model had when it was last read or written
  std::string modelText_;
  bool loaded_ = false;
  std::vector<ScriptError> errors_;
  bool dirty_ = true;
  int cursor_ = 0;
  int lastHelpCursor_ = -1;
  //! applied in the next frame, when the text field has the focus again
  int setCursor_ = -1;
  bool focus_ = false;
  bool enterPressed_ = false;
  ScriptLang language_ = ScriptLang::PCDK;
  Help help_;
};

}

#endif
