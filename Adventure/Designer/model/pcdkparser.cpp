#include "pcdkparser.h"

#include <algorithm>

// A port of the C# StoryDesigner's PcdkParser.cs, which the script editor and
// the language list use; the syntax table is converted from it line by line.

namespace sd{

namespace{

bool isSpace(unsigned char c){
  //.NET's white space within Windows-1252: also the no-break space
  return c == ' ' || (c >= '\t' && c <= '\r') || c == 0xA0;
}

//! the text up to stopchar (trimmed) and stopchar's position, -1 if it's missing
int readUntil(char stopchar, const std::string& text, int fromIdx, std::string& result){
  for (int i = fromIdx; i < (int)text.size(); ++i){
    if (text[i] == stopchar){
      result = trim(text.substr(fromIdx, i-fromIdx));
      return i;
    }
  }
  result = fromIdx < (int)text.size() ? trim(text.substr(fromIdx)) : std::string();
  return -1;
}

//! the word before stopidx back to stopchar
int readBefore(char stopchar, const std::string& text, int stopidx, std::string& result){
  for (int i = stopidx-1; i >= 0; --i){
    if (text[i] == stopchar){
      result = trim(text.substr(i, stopidx-i));
      return i;
    }
  }
  result = trim(text.substr(0, stopidx));
  return 0;
}

}

std::string trim(const std::string& text){
  size_t start = 0;
  while (start < text.size() && isSpace((unsigned char)text[start]))
    ++start;
  size_t end = text.size();
  while (end > start && isSpace((unsigned char)text[end-1]))
    --end;
  return text.substr(start, end-start);
}

PcdkParser::PcdkParser(ScriptLang language) : language_(language){
  initSyntax();
}

void PcdkParser::addConditional(const std::string& name, const ArgDefs& args){
  if (language_ == ScriptLang::PCDK){
    addFunction("if_" + name, args, true);
    addFunction("ifnot_" + name, args, true);
  }
  else
    addFunction("is_" + name, args, false);
}

void PcdkParser::addFunction(const std::string& name, const ArgDefs& args, bool isKeyword){
  //a name defined again keeps its place, as in a .NET Dictionary
  auto iter = index_.find(name);
  if (iter != index_.end())
    functions_[iter->second].second = args;
  else{
    index_[name] = functions_.size();
    functions_.push_back(std::make_pair(name, args));
  }
  if (isKeyword)
    keywords_.push_back(name);
}

const PcdkParser::ArgDefs* PcdkParser::function(const std::string& name) const{
  auto iter = index_.find(name);
  if (iter == index_.end())
    return nullptr;
  return &functions_[iter->second].second;
}

bool PcdkParser::isKeyword(const std::string& name) const{
  return std::find(keywords_.begin(), keywords_.end(), name) != keywords_.end();
}

void PcdkParser::parseText(const std::string& text){
  int charidx = 0;
  size_t start = 0;
  while (true){
    size_t end = text.find('\n', start);
    std::string line = text.substr(start, end == std::string::npos ? std::string::npos : end-start);
    parseLine(charidx, line);
    charidx += (int)line.size();
    if (end == std::string::npos)
      break;
    start = end+1;
  }
}

void PcdkParser::parseLine(int charidx, const std::string& line){
  if (line.empty())
    return;
  parseFunction(charidx, line);
}

void PcdkParser::parseFunction(int startindex, const std::string& text){
  std::string result;
  int idx = -1;
  if (language_ == ScriptLang::Lua){
    idx = readUntil('.', text, 0, result);
    if (result != "on")
      idx = -1;
  }
  if (idx == -1)
    idx = readUntil('(', text, 0, result);
  if (idx != -1){
    if ((int)text.size() > idx+1 && text[idx+1] == '*'){
      if (onComment)
        onComment(startindex, (int)text.size());
    }
    else{
      Argument func;
      func.start = readBefore(' ', text, idx, result);
      func.stop = idx;
      func.text = result;
      func.definition = ArgDef("Command", ArgType::Function);
      int lastidx = idx+1;
      std::vector<Argument> arguments;
      const char argsep = language_ == ScriptLang::PCDK ? ';' : ',';
      while (true){
        int newidx = readUntil(argsep, text, lastidx, result);
        if (newidx != -1){
          addArgument(func, Argument{lastidx, newidx, result, ArgDef()}, arguments);
          lastidx = newidx+1;
        }
        else{
          newidx = readUntil(')', text, lastidx, result);
          if (newidx == -1 && onError)
            onError(startindex+lastidx, (int)result.size(), result, Error::ClosingBraceExpected);
          addArgument(func, Argument{lastidx, newidx, result, ArgDef()}, arguments);
          break;
        }
      }
      if (onFunction)
        onFunction(func, arguments, startindex);
    }
    return;
  }
  //a block's begin or end
  if (readUntil('{', text, 0, result) != -1)
    return;
  if (readUntil('}', text, 0, result) != -1)
    return;
  if (onError)
    onError(startindex, (int)text.size(), text, Error::LineNotRecognized);
}

void PcdkParser::addArgument(const Argument& func, Argument arg, std::vector<Argument>& arguments){
  if (arg.stop == -1)
    arg.stop = arg.start+(int)arg.text.size();
  const ArgDefs* defs = function(func.text);
  if (defs != nullptr && arguments.size() < defs->size())
    arg.definition = (*defs)[arguments.size()];
  arguments.push_back(arg);
}

void PcdkParser::initSyntax(){
  //lua specific key words
  if (language_ == ScriptLang::Lua)
  {
    ArgDefs largs(0);
    addFunction("end", largs, true);
    addFunction("if", largs, true);
    addFunction("then", largs, true);
    addFunction("else", largs, true);
    addFunction("elseif", largs, true);
    addFunction("for", largs, true);
    addFunction("function", largs, true);
  }

  //events
  ArgDefs args(1);
  args[0] = ArgDef("Event", ArgType::Event);
  args[0].additionalValues = { "mouse", "mouseout", "click", "release",
  "rightclick", "doubleclick", "enter", "exit", "loop1", "loop2", "link", "givelink",
  "cantall"};
  addFunction("on", args, true);

  //conditionals
  args = ArgDefs(2);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("State 0-10", ArgType::Integer);
  addConditional("obj", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("X (pixel)", ArgType::Integer);
  addConditional("xobj", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("Y (pixel)", ArgType::Integer);
  addConditional("yobj", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  addConditional("focus", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Room", ArgType::Room);
  addConditional("charin", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  addConditional("ischar", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Item", ArgType::Item);
  addConditional("hasitem", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Variable Name", ArgType::Variable);
  args[1] = ArgDef("Number Value", ArgType::Number);
  addConditional("num", args);
  args = ArgDefs(2);
  args[0] = ArgDef("String Name", ArgType::Variable);
  args[1] = ArgDef("Textline", ArgType::String);
  addConditional("string", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Bool Name", ArgType::PresetVariable);
  args[1] = ArgDef("true/false", ArgType::Boolean);
  addConditional("bool", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Key", ArgType::String);
  addConditional("keydown", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Key", ArgType::String);
  addConditional("keypressed", args);
  args = ArgDefs(1);
  args[0] = ArgDef("up OR down", ArgType::String);
  addConditional("mousewheel", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Gamecommand", ArgType::Command);
  addConditional("command", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Linkname", ArgType::String);
  addConditional("link", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Item", ArgType::Item);
  addConditional("givelink", args);
  args = ArgDefs(4);
  args[0] = ArgDef("Script", ArgType::Script);
  args[1] = ArgDef("Level 1-9", ArgType::Integer);
  args[2] = ArgDef("Row 1-9", ArgType::Integer);
  args[3] = ArgDef("Activate (true/false)", ArgType::Boolean);
  addConditional("textscene", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Room", ArgType::Room);
  addConditional("room", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Item", ArgType::Item);
  args[1] = ArgDef("State 0-10", ArgType::Integer);
  addConditional("item", args);

  //set commands
  args = ArgDefs(1);
  args[0] = ArgDef("Gamecommand", ArgType::Command);
  addFunction("command", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Mouseicons (1-8)", ArgType::Integer);
  addFunction("instmouse", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  args[0].additionalValues = { "none", "last" };
  addFunction("setfocus", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Bool Name", ArgType::PresetVariable);
  args[1] = ArgDef("true / false", ArgType::Boolean);
  addFunction("setbool", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Variable Name", ArgType::Variable);
  args[1] = ArgDef("Number Value", ArgType::Number);
  addFunction("setnum", args);
  args = ArgDefs(2);
  args[0] = ArgDef("String Name", ArgType::Variable);
  args[1] = ArgDef("Textline", ArgType::String);
  addFunction("setstring", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Variable Name", ArgType::Variable);
  args[1] = ArgDef("Range", ArgType::Integer);
  addFunction("randomnum", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Variable Name", ArgType::Variable);
  addFunction("savenum", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Variable Name", ArgType::Variable);
  addFunction("loadnum", args);
  args = ArgDefs(1);
  args[0] = ArgDef("String Name", ArgType::Variable);
  addFunction("savestring", args);
  args = ArgDefs(1);
  args[0] = ArgDef("String Name", ArgType::Variable);
  addFunction("loadstring", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Red Color Value (0-255)", ArgType::Integer);
  args[1] = ArgDef("Green Color Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("Blue Color Value (0-255)", ArgType::Integer);
  addFunction("offtextcolor", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Red Color Value (0-255)", ArgType::Integer);
  args[1] = ArgDef("Green Color Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("Blue Color Value (0-255)", ArgType::Integer);
  addFunction("infotextcolor", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Transparency (0-100)", ArgType::Integer);
  addFunction("settransparency", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Font (1-99)", ArgType::Integer);
  addFunction("setfont", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Screenchange", ArgType::String);
  const std::vector<std::string> screenchanges = {"direct", "fadeblack", "rectangle", "circle",
    "shutters", "clock", "blend", "blendslow"};
  args[0].additionalValues = screenchanges;
  addFunction("setscreenchange", args);
  args = ArgDefs(4);
  args[0] = ArgDef("Room", ArgType::Room);
  args[1] = ArgDef("X (walkmap)", ArgType::Integer);
  args[2] = ArgDef("Y (walkmap)", ArgType::Integer);
  args[3] = ArgDef("Free to Walk (true/false)", ArgType::Boolean);
  addFunction("setwalkmap", args);
  args = ArgDefs(6);
  args[0] = ArgDef("Room", ArgType::Room);
  args[1] = ArgDef("X1 (walkmap)", ArgType::Integer);
  args[2] = ArgDef("Y1 (walkmap)", ArgType::Integer);
  args[3] = ArgDef("X2 (walkmap)", ArgType::Integer);
  args[4] = ArgDef("Y2 (walkmap)", ArgType::Integer);
  args[5] = ArgDef("Free to Walk (true/false)", ArgType::Boolean);
  addFunction("set_rect_walkmap", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Variable name", ArgType::Variable);
  addFunction("sqrt", args);

  //sound and movie commands
  args = ArgDefs(2);
  args[0] = ArgDef("Soundeffect...", ArgType::Sound);
  args[1] = ArgDef("Volume (0-100)", ArgType::Integer, true);
  addFunction("playsound", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Musicfile...", ArgType::Music);
  args[1] = ArgDef("Pattern", ArgType::String, true);
  addFunction("playmusic", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Volume 0-100", ArgType::Integer);
  addFunction("musicvolume", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Volume 0-100", ArgType::Integer);
  addFunction("speechvolume", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Volume 0-100", ArgType::Integer);
  addFunction("soundvolume", args);
  args = ArgDefs(0);
  addFunction("stopmusic", args);
  args = ArgDefs(1);
  args[0] = ArgDef("1=Slow - 15=Fast OR Time in ms", ArgType::Integer);
  addFunction("fadespeed", args);
  args = ArgDefs(1);
  args[0] = ArgDef("EAX Preset", ArgType::String);
  args[0].additionalValues = {"paddedcell", "livingroom", "cave",
    "carpetedhallway", "alley", "mountains", "parkinglot", "drugged", "concerthall",
    "room", "stoneroom", "arena", "hallway", "forest", "quarry", "sewerpipe", "dizzy",
    "none", "bathroom", "auditorium", "hangar", "stonecorridor", "city", "plain",
    "underwater", "psychotic", "off"};
  addFunction("seteax", args);
  args = ArgDefs(6);
  args[0] = ArgDef("Video...", ArgType::Video);
  args[1] = ArgDef("pause script (true/false)", ArgType::Boolean);
  args[2] = ArgDef("X (pixel)", ArgType::Integer, true);
  args[3] = ArgDef("Y (pixel)", ArgType::Integer, true);
  args[4] = ArgDef("Width (pixel)", ArgType::Integer, true);
  args[5] = ArgDef("Height (pixel)", ArgType::Integer, true);
  addFunction("playvideo", args);
  addFunction("playavi", args);
  args = ArgDefs(0);
  addFunction("stopvideo", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Effectname", ArgType::DSPEffect);
  addFunction("setdsp", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Video...", ArgType::VideoSwf);
  args[1] = ArgDef("X (pixel)", ArgType::Integer, true);
  args[2] = ArgDef("Y (pixel)", ArgType::Integer, true);
  args[3] = ArgDef("Width (pixel)", ArgType::Integer, true);
  args[4] = ArgDef("Height (pixel)", ArgType::Integer, true);
  addFunction("playswf", args);
  args = ArgDefs(0);
  addFunction("stopswf", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Seconds", ArgType::Integer);
  addFunction("moviewait", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Soundeffect...", ArgType::Sound);
  args[1] = ArgDef("Volume (0-100)", ArgType::Integer, true);
  addFunction("loopsound", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Soundeffect...", ArgType::Sound);
  addFunction("loopstop", args);
  args = ArgDefs(7);
  args[0] = ArgDef("Filename Prefix", ArgType::String);
  args[1] = ArgDef("FPS 1-50", ArgType::Integer);
  args[2] = ArgDef("X (Screenpixel)", ArgType::Integer);
  args[3] = ArgDef("Y (Screenpixel)", ArgType::Integer);
  args[4] = ArgDef("Width (Pixel)", ArgType::Integer);
  args[5] = ArgDef("Height (Pixel)", ArgType::Integer);
  args[6] = ArgDef("wait (optional)", ArgType::String);
  args[6].additionalValues = { "wait" };
  addFunction("playanimation", args);

  //object commands
  args = ArgDefs(12);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("State 0-10", ArgType::Integer);
  for (int i = 0; i < 10; ++i)
  {
    args[i+2] = ArgDef("State 0-10", ArgType::Integer, true);
  }
  addFunction("setobj", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("State 0-10", ArgType::Integer);
  addFunction("instobj", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("X (pixel)", ArgType::Integer);
  args[2] = ArgDef("Y (pixel)", ArgType::Integer);
  args[3] = ArgDef("Speed 0=Immediately 9=Slow OR Time in ms", ArgType::Integer);
  args[4] = ArgDef("Optional: wait", ArgType::String, true);
  args[4].additionalValues = { "wait" };
  addFunction("moveobj", args);
  args = ArgDefs(21);
  args[0] = ArgDef("Groupname", ArgType::String);
  for (int i = 0; i < 10; ++i)
  {
    args[i + 1] = ArgDef("Object", ArgType::Object, true);
  }
  addFunction("group", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("Red Color Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("Green Color Value (0-255)", ArgType::Integer);
  args[3] = ArgDef("Blue Color Value (0-255)", ArgType::Integer);
  args[4] = ArgDef("fade (Optional)", ArgType::String, true);
  args[4].additionalValues = { "fade" };
  addFunction("setobjlight", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("Alpha Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("fade (Optional)", ArgType::String, true);
  args[2].additionalValues = { "fade" };
  addFunction("setobjalpha", args);

  //room commands
  args = ArgDefs(2);
  args[0] = ArgDef("Room", ArgType::Room);
  args[1] = ArgDef("Screenchange (Optional)", ArgType::String, true);
  args[1].additionalValues = screenchanges;
  addFunction("loadroom", args);
  args = ArgDefs(0);
  addFunction("unloadroom", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Room", ArgType::Room);
  args[1] = ArgDef("Time in ms (Optional)", ArgType::Integer, true);
  addFunction("subroom", args);
  args = ArgDefs(0);
  addFunction("return", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Room", ArgType::Room);
  args[1] = ArgDef("Red Color Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("Green Color Value (0-255)", ArgType::Integer);
  args[3] = ArgDef("Blue Color Value (0-255)", ArgType::Integer);
  args[4] = ArgDef("fade (Optional)", ArgType::String, true);
  args[4].additionalValues = { "fade" };
  addFunction("setlight", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Room", ArgType::Room);
  args[1] = ArgDef("X (walkmap)", ArgType::Integer);
  args[2] = ArgDef("Y (walkmap)", ArgType::Integer);
  args[3] = ArgDef("Don't scroll (true/false)", ArgType::Boolean);
  args[4] = ArgDef("Optional: up/down/left/right", ArgType::String, true);
  args[4].additionalValues = { "up", "down", "left", "right" };
  addFunction("setpos", args);
  args = ArgDefs(1);
  args[0] = ArgDef("1-20 (4 = Default) (+100 don't follow)", ArgType::Integer);
  addFunction("scrollspeed", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Seconds", ArgType::Integer);
  args[1] = ArgDef("Strength of Vibration", ArgType::Integer);
  addFunction("jiggle", args);

  //character commands
  args = ArgDefs(5);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("X (walkmap)", ArgType::Integer);
  args[2] = ArgDef("Y (walkmap)", ArgType::Integer);
  args[3] = ArgDef("Viewing Direction (1-4) OR 'dontwait'", ArgType::String, true);
  args[3].additionalValues = { "1", "2", "3", "4", "dontwait" };
  args[4] = ArgDef("Optional: dontwait", ArgType::String, true);
  args[4].additionalValues = { "dontwait" };
  addFunction("walkto", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Soundeffect...", ArgType::Sound);
  addFunction("setwalksound", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("up/down/right/left", ArgType::String);
  args[1].additionalValues = { "up", "down", "right", "left" };
  addFunction("stepto", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Character", ArgType::Character);
  args[2] = ArgDef("Optional: dontwait", ArgType::String, true);
  args[2].additionalValues = { "dontwait" };
  addFunction("follow", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Room", ArgType::Room);
  args[2] = ArgDef("X (walkmap)", ArgType::Integer);
  args[3] = ArgDef("Y (walkmap)", ArgType::Integer);
  args[4] = ArgDef("Viewing Direction (1-4)", ArgType::Integer, true);
  args[4].additionalValues = { "1", "2", "3", "4" };
  addFunction("beamto", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Viewing Direction (1-4) OR Character", ArgType::Character, true);
  args[1].additionalValues = { "1", "2", "3", "4" };
  addFunction("lookto", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Speech Text", ArgType::String);
  args[2] = ArgDef("Speech Soundfile OR 'dontwait')", ArgType::Sound, true);
  args[2].additionalValues = { "dontwait" };
  args[3] = ArgDef("Optional: DSP Effect OR 'dontwait')", ArgType::DSPEffect, true);
  args[3].additionalValues = { "dontwait" };
  args[4] = ArgDef("Optional: dontwait", ArgType::String, true);
  args[4].additionalValues = { "dontwait" };
  addFunction("speech", args);
  args = ArgDefs(5);
  args[0] = ArgDef("X (walkmap)", ArgType::Integer);
  args[1] = ArgDef("X (walkmap)", ArgType::Integer);
  args[2] = ArgDef("Speech Text", ArgType::String);
  args[3] = ArgDef("Speech Soundfile OR 'dontwait')", ArgType::Sound, true);
  args[3].additionalValues = { "dontwait" };
  args[4] = ArgDef("Optional: dontwait", ArgType::String, true);
  args[4].additionalValues = { "dontwait" };
  addFunction("offspeech", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  addFunction("pickup", args);
  args = ArgDefs(22);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Action (1-20)(Extraname)", ArgType::String);
  for (int i = 0; i < 20; ++i)
  {
    args[i + 2] = ArgDef("Action (1-20)(Extraname)", ArgType::String, true);
  }
  addFunction("setchar", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("X (walkmap)", ArgType::Integer);
  args[2] = ArgDef("Y (walkmap)", ArgType::Integer);
  args[3] = ArgDef("Viewing Direction (1-4) OR 'dontwait'", ArgType::String, true);
  args[3].additionalValues = { "1", "2", "3", "4", "dontwait" };
  args[4] = ArgDef("Optional: dontwait", ArgType::String, true);
  args[4].additionalValues = { "dontwait" };
  addFunction("runto", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Runspeed in per cent (50%-300%)", ArgType::Integer);
  addFunction("runspeed", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  addFunction("loadchar", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Character", ArgType::Character);
  addFunction("switchchar", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Alpha Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("fade (Optional)", ArgType::String, true);
  args[2].additionalValues = { "fade" };
  addFunction("setcharalpha", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Red Color Value (0-255)", ArgType::Integer);
  args[2] = ArgDef("Green Color Value (0-255)", ArgType::Integer);
  args[3] = ArgDef("Blue Color Value (0-255)", ArgType::Integer);
  args[4] = ArgDef("fade (Optional)", ArgType::String, true);
  args[4].additionalValues = { "fade" };
  addFunction("setcharlight", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Size in per cent", ArgType::Integer);
  args[2] = ArgDef("fade (Optional)", ArgType::String, true);
  args[2].additionalValues = { "fade" };
  addFunction("charzoom", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("true / false", ArgType::Boolean);
  addFunction("stopzooming", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Object", ArgType::Object);
  addFunction("linkchar", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  addFunction("unlinkchar", args);
  args = ArgDefs(2);
  args[0] = ArgDef("left/center/right or absolute/relative", ArgType::String);
  args[0].additionalValues = { "left", "center", "right", "absolute", "relative" };
  args[1] = ArgDef("left/center/right or absolute/relative", ArgType::String, true);
  args[1].additionalValues = { "left", "center", "right", "absolute", "relative" };
  addFunction("offalign", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Character", ArgType::Character);
  addFunction("charsimclick", args);

  //item commands
  args = ArgDefs(12);
  args[0] = ArgDef("Item", ArgType::Object);
  args[1] = ArgDef("State 0-10", ArgType::Integer);
  for (int i = 0; i < 10; ++i)
  {
    args[i + 2] = ArgDef("State 0-10", ArgType::Integer, true);
  }
  addFunction("setitem", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Character", ArgType::Character);
  args[0].additionalValues = { "off" };
  args[1] = ArgDef("Item", ArgType::Item);
  args[1].additionalValues = { "givelink" };
  args[2] = ArgDef("Inventory (1-5)", ArgType::Integer, true);
  addFunction("additem", args);
  addFunction("delitem", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Inventory (1-5)", ArgType::Integer);
  addFunction("showinventory", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Character", ArgType::Character);
  args[1] = ArgDef("Character", ArgType::Character);
  addFunction("exchange", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Amount of Steps", ArgType::Integer);
  addFunction("inv_up", args);
  addFunction("inv_down", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Linkname", ArgType::String);
  addFunction("link", args);
  args = ArgDefs(0);
  addFunction("givelink", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Item", ArgType::Item);
  addFunction("itemsimclick", args);

  //special effects
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("enablefxshape", args);
  args = ArgDefs(1);
  const std::vector<std::string> effects = {"mirror", "flip", "slowmotion", "scanlines",
    "fog", "noise", "colornoise", "monitor", "super8", "blur", "pixelate",
    "motionblur", "drugged", "bloom", "darkbloom", "whoosh", "art", "hell",
    "heat", "underwater", "classicdark", "whiteflash", "zoom", "lightning" };
  args[0] = ArgDef("Special FX", ArgType::String);
  std::vector<std::string> stopeffects = effects;
  stopeffects.push_back("all");
  args[0].additionalValues = stopeffects;
  addFunction("stopeffect", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Special FX", ArgType::String);
  args[0].additionalValues = effects;
  args[1] = ArgDef("Effect Value", ArgType::String);
  args[2] = ArgDef("fade (Optional)", ArgType::String);
  addFunction("starteffect", args);

  //text commands
  args = ArgDefs(8);
  args[0] = ArgDef("Textout Name", ArgType::String);
  args[1] = ArgDef("Textline", language_ == ScriptLang::Lua ? ArgType::LuaFunc : ArgType::String, true);
  args[2] = ArgDef("X (pixel)", ArgType::Integer, true);
  args[3] = ArgDef("Y (pixel)", ArgType::Integer, true);
  args[4] = ArgDef("Font (1-99)", ArgType::Integer, true);
  args[5] = ArgDef("Red Color Value (0-255)", ArgType::Integer, true);
  args[6] = ArgDef("Green Color Value (0-255)", ArgType::Integer, true);
  args[7] = ArgDef("Blue Color Value (0-255)", ArgType::Integer, true);
  addFunction("textout", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Textout Name", ArgType::String);
  addFunction("texthide", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Textout Name", ArgType::String);
  args[1] = ArgDef("Room (or any / taskbar / menu)", ArgType::Room);
  args[1].additionalValues = { "any", "taskbar", "menu" };
  addFunction("bindtext", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Textout Name", ArgType::String);
  args[1] = ArgDef("left / center / right", ArgType::String);
  args[1].additionalValues = { "left", "center", "right" };
  addFunction("textalign", args);
  args = ArgDefs(0);
  addFunction("showalltext", args);
  args = ArgDefs(0);
  addFunction("hidealltext", args);
  args = ArgDefs(5);
  args[0] = ArgDef("Textout Name", ArgType::String);
  args[1] = ArgDef("X (pixel)", ArgType::Integer);
  args[2] = ArgDef("Y (pixel)", ArgType::Integer);
  args[3] = ArgDef("Speed 0=Immediately 9=Slow OR time in ms", ArgType::Integer);
  args[4] = ArgDef("Optional: wait", ArgType::String, true);
  args[4].additionalValues = { "wait" };
  addFunction("movetext", args);
  args = ArgDefs(9);
  args[0] = ArgDef("String name", language_ == ScriptLang::Lua ? ArgType::LuaFunc : ArgType::String);
  args[1] = ArgDef("X (pixel)", ArgType::Integer);
  args[2] = ArgDef("Y (pixel)", ArgType::Integer);
  args[3] = ArgDef("Font (1-99)", ArgType::Integer, true);
  args[4] = ArgDef("Max length", ArgType::Integer, true);
  args[5] = ArgDef("Red Color Value (0-255)", ArgType::Integer, true);
  args[6] = ArgDef("Green Color Value (0-255)", ArgType::Integer, true);
  args[7] = ArgDef("Blue Color Value (0-255)", ArgType::Integer, true);
  args[8] = ArgDef("Initial content", ArgType::String, true);
  addFunction("entertext", args);

  //scene & game commands
  args = ArgDefs(2);
  args[0] = ArgDef("Script", ArgType::Script);
  args[1] = ArgDef("Optional: donthide", ArgType::String, true);
  args[1].additionalValues = { "donthide" };
  addFunction("cutscene", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Script", ArgType::Script);
  args[1] = ArgDef("loop count / infinitly / * / loop2", ArgType::String, true);
  args[1].additionalValues = { "infinitly", "*", "loop2" };
  addFunction(language_ == ScriptLang::Lua ? "startfunction" : "function", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Script", ArgType::Script);
  addFunction("stopfunction", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Optional: donthide", ArgType::String, true);
  args[0].additionalValues = { "donthide" };
  addFunction("minicut", args);
  args = ArgDefs(0);
  addFunction("minicutend", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Seconds", ArgType::Number);
  addFunction("wait", args);
  args = ArgDefs(0);
  addFunction("stopskip", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Textline", ArgType::String);
  args[1] = ArgDef("Visible (true/false)", ArgType::Boolean, true);
  addFunction("showinfo", args);
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("showmouse", args);
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("enablemouse", args);
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("taskbar", args);
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("enablemenu", args);
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("realtime", args);
  args = ArgDefs(0);
  addFunction("popupcoin", args);
  args = ArgDefs(0);
  addFunction("popupmenu", args);
  args = ArgDefs(1);
  args[0] = ArgDef("classic/intelligent/nothing", ArgType::String);
  args[0].additionalValues = { "classic", "intelligent", "nothing" };
  addFunction("rightclickstyle", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Language", ArgType::Language);
  addFunction("setlanguage", args);
  args = ArgDefs(4);
  args[0] = ArgDef("Script", ArgType::Script);
  args[1] = ArgDef("X (pixel)", ArgType::Integer, true);
  args[2] = ArgDef("Y (pixel)", ArgType::Integer, true);
  args[3] = ArgDef("Width in pixels", ArgType::Integer, true);
  addFunction("textscene", args);
  args = ArgDefs(0);
  addFunction("endscene", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Script", ArgType::Script);
  args[1] = ArgDef("Level 1-9", ArgType::Integer);
  args[1].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  args[2] = ArgDef("Row 1-9", ArgType::Integer);
  args[2].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  addFunction("activate", args);
  args = ArgDefs(0);
  addFunction("coinreturn", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Script", ArgType::Script);
  args[1] = ArgDef("Level 1-9", ArgType::Integer);
  args[1].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  args[2] = ArgDef("Row 1-9", ArgType::Integer);
  args[2].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  addFunction("deactivate", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Level 1-9", ArgType::Integer);
  args[0].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  addFunction("gotolevel", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Style (usetext / useitem)", ArgType::String);
  args[0].additionalValues = { "usetext", "useitem" };
  addFunction("textscenestyle", args);
  if (language_ == ScriptLang::Lua)
  {
    args = ArgDefs(2);
    args[0] = ArgDef("Seconds", ArgType::Number);
    args[1] = ArgDef("Function", ArgType::LuaFunc);
  }
  else
  {
    args = ArgDefs(1);
    args[0] = ArgDef("Seconds", ArgType::Number);
  }
  addFunction("timer", args);
  args = ArgDefs(0);
  addFunction("break", args);
  args = ArgDefs(language_ == ScriptLang::Lua ? 2 : 1);
  args[0] = ArgDef("Level 1-9", ArgType::Integer);
  args[0].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  if (language_ == ScriptLang::Lua)
  {
    args[1] = ArgDef("Function", ArgType::LuaFunc);
  }
  addFunction("level", args);
  args = ArgDefs(3);
  args[0] = ArgDef("Level 1-9", ArgType::Integer);
  args[0].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  args[1] = ArgDef("Textline", ArgType::String);
  args[2] = ArgDef("Activate (true/false)", ArgType::Boolean);
  addFunction("row", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Object", ArgType::Object);
  addFunction("simclick", args);

  //save & load menu
  args = ArgDefs(0);
  addFunction("restart", args);
  args = ArgDefs(0);
  addFunction("quit", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Gamesave slot 1-10", ArgType::Integer);
  args[0].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
  addFunction("savegame", args);
  args = ArgDefs(2);
  args[0] = ArgDef("Gamesave slot 1-10", ArgType::Integer);
  args[0].additionalValues = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
  args[1] = ArgDef("Script", ArgType::Script, true);
  addFunction("loadgame", args);
  args = ArgDefs(1);
  args[0] = ArgDef("true / false", ArgType::Boolean);
  addFunction("textenabled", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Textspeed (fast / normal / slow)", ArgType::String);
  args[0].additionalValues = { "fast", "normal", "slow" };
  addFunction("textspeed", args);

  //particles
  args = ArgDefs(6);
  args[0] = ArgDef("Object", ArgType::Object);
  args[1] = ArgDef("Speed", ArgType::Integer);
  args[2] = ArgDef("Amount (1-151)", ArgType::Integer);
  args[3] = ArgDef("Direction (0-360 degrees)", ArgType::Integer);
  args[4] = ArgDef("Rotation (0-360 degrees)", ArgType::Integer);
  args[5] = ArgDef("Variation in per cent", ArgType::Integer);
  addFunction("setparticles", args);
  args = ArgDefs(1);
  args[0] = ArgDef("View (1-4)", ArgType::String);
  args[0].additionalValues = { "front", "behindfront", "behindmiddle", "back" };
  addFunction("particleview", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Optional: fast", ArgType::String, true);
  args[0].additionalValues = { "fast" };
  addFunction("startparticles", args);
  args = ArgDefs(1);
  args[0] = ArgDef("Optional: fast", ArgType::String, true);
  args[0].additionalValues = { "fast" };
  addFunction("stopparticles", args);
}

}
