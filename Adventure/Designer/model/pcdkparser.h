#ifndef SD_PCDKPARSER_H
#define SD_PCDKPARSER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "project.h"

namespace sd{

//! the C# StoryDesigner's line parser for scripts: the script commands with
//! their arguments' kinds, for completion, highlighting and the language list.
//! It doesn't check a script; checkScript does that with the game's parser.
class PcdkParser{
public:
  enum class Error{
    LineNotRecognized,
    ClosingBraceExpected,
  };
  enum class ArgType{
    Unknown,
    Function,
    Command,
    Event,
    String,
    Number,
    Integer,
    Boolean,
    Variable,
    PresetVariable,
    Object,
    Character,
    Item,
    Script,
    Room,
    Sound,
    Music,
    Video,
    VideoSwf,
    Language,
    LuaFunc,
    DSPEffect,
  };
  struct ArgDef{
    ArgDef() = default;
    ArgDef(const std::string& name, ArgType type, bool optional = false) : name(name), type(type), optional(optional) {}
    std::string name;
    ArgType type = ArgType::Unknown;
    bool optional = false;
    std::vector<std::string> additionalValues;
  };
  using ArgDefs = std::vector<ArgDef>;
  //! a command's name or an argument, by its bytes in the parsed line
  struct Argument{
    int start = 0;
    int stop = 0;
    std::string text;
    ArgDef definition;
  };

  explicit PcdkParser(ScriptLang language);

  //! a comment from charpos on, length bytes long
  std::function<void(int charpos, int length)> onComment;
  //! a command with its arguments; their positions count from the line's start, startidx
  std::function<void(const Argument& func, const std::vector<Argument>& args, int startidx)> onFunction;
  std::function<void(int charpos, int length, const std::string& text, Error error)> onError;

  //! parses the lines of a script ("\n" between them); like the C# parser,
  //! a line's start doesn't count the line ends before it
  void parseText(const std::string& text);
  //! parses one line that starts at charidx in its script
  void parseLine(int charidx, const std::string& line);

  bool isKeyword(const std::string& name) const;
  //! the commands in the order they were defined
  const std::vector<std::pair<std::string, ArgDefs>>& functions() const {return functions_;}
  //! a command's arguments, nullptr if it's unknown
  const ArgDefs* function(const std::string& name) const;

private:
  void initSyntax();
  void addConditional(const std::string& name, const ArgDefs& args);
  void addFunction(const std::string& name, const ArgDefs& args, bool isKeyword = false);
  void parseFunction(int startindex, const std::string& text);
  void addArgument(const Argument& func, Argument arg, std::vector<Argument>& arguments);

  ScriptLang language_;
  std::vector<std::pair<std::string, ArgDefs>> functions_;
  std::unordered_map<std::string, size_t> index_;
  std::vector<std::string> keywords_;
};

//! a text without the white space around it, as .NET's Trim (Windows-1252)
std::string trim(const std::string& text);

}

#endif
