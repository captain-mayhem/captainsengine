#include "scripteditor.h"

#include <algorithm>
#include <cfloat>

namespace sd{

namespace{

bool isWordEnd(char c){
  //as the C# editor: a name ends at white space and at these
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ';' || c == '(' || c == ')' || c == '.';
}

void addMatch(std::vector<std::string>& matches, const std::string& name, const std::string& typed){
  if (toLower(name).compare(0, typed.size(), toLower(typed)) == 0)
    matches.push_back(name);
}

std::string extensionOf(const std::string& file){
  size_t dot = file.rfind('.');
  return dot == std::string::npos ? std::string() : toLower(file.substr(dot));
}

}

int ScriptEditor::callback(ImGuiInputTextCallbackData* data){
  ScriptEditor* self = (ScriptEditor*)data->UserData;
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize){
    self->utf8_.resize(data->BufTextLen);
    data->Buf = self->utf8_.data();
    return 0;
  }
  if (data->EventFlag != ImGuiInputTextFlags_CallbackAlways)
    return 0;
  if (self->setCursor_ >= 0){
    data->CursorPos = std::min(self->setCursor_, data->BufTextLen);
    data->SelectionStart = data->SelectionEnd = data->CursorPos;
    self->setCursor_ = -1;
  }
  //a new line starts as far in as the one before, as in the C# editor
  if (self->enterPressed_ && data->CursorPos > 0 && data->Buf[data->CursorPos-1] == '\n'){
    int prevEnd = data->CursorPos-1;
    int prevStart = prevEnd;
    while (prevStart > 0 && data->Buf[prevStart-1] != '\n')
      --prevStart;
    int indent = prevStart;
    while (indent < prevEnd && (data->Buf[indent] == ' ' || data->Buf[indent] == '\t'))
      ++indent;
    if (indent > prevStart){
      std::string spaces(data->Buf+prevStart, data->Buf+indent);
      data->InsertChars(data->CursorPos, spaces.c_str());
    }
  }
  self->enterPressed_ = false;
  self->cursor_ = data->CursorPos;
  return 0;
}

bool ScriptEditor::draw(Script& script, const Project& project){
  bool changed = false;
  const std::string& text = script.text ? *script.text : std::string();
  if (!loaded_ || text != modelText_){
    //new, or changed elsewhere
    utf8_ = toUtf8(text);
    modelText_ = text;
    loaded_ = true;
    dirty_ = true;
  }
  if (project.settings.scriptingLanguage != language_){
    language_ = project.settings.scriptingLanguage;
    dirty_ = true;
  }

  const float fontSize = ImGui::GetFontSize();
  const float helpWidth = 14*fontSize;
  const float errorHeight = ImGui::GetTextLineHeightWithSpacing()*(errors_.empty() ? 1 : std::min((int)errors_.size(), 4)) + ImGui::GetStyle().ItemSpacing.y*2;
  const ImVec2 avail = ImGui::GetContentRegionAvail();
  const float height = std::max(avail.y - errorHeight - ImGui::GetFrameHeightWithSpacing(), 6*fontSize);

  if (focus_){
    ImGui::SetKeyboardFocusHere();
    focus_ = false;
  }
  enterPressed_ = ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);
  ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackAlways;
  if (ImGui::InputTextMultiline("##text", utf8_.data(), utf8_.capacity()+1, ImVec2(std::max(avail.x - helpWidth, 10*fontSize), height), flags, callback, this))
    dirty_ = true;
  if (dirty_){
    //the model keeps two spaces for a tab, as the C# editor saves it
    std::string newText;
    for (char c : fromUtf8(utf8_)){
      if (c == '\t')
        newText += "  ";
      else
        newText += c;
    }
    if (newText != modelText_){
      script.text = newText;
      modelText_ = newText;
      changed = true;
    }
    errors_ = checkScript(modelText_, language_);
    dirty_ = false;
    lastHelpCursor_ = -1;
  }
  if (cursor_ != lastHelpCursor_){
    helpAt(project);
    lastHelpCursor_ = cursor_;
  }

  //the help for the cursor's place
  ImGui::SameLine();
  ImGui::BeginGroup();
  ImGui::TextDisabled("%s", help_.info.empty() ? " " : toUtf8(help_.info).c_str());
  if (ImGui::BeginListBox("##matches", ImVec2(helpWidth - ImGui::GetStyle().ItemSpacing.x, height - ImGui::GetTextLineHeightWithSpacing()))){
    for (const std::string& match : help_.matches){
      if (ImGui::Selectable(toUtf8(match).c_str())){
        std::string utf8 = toUtf8(match);
        utf8_.replace(help_.wordStart, help_.wordEnd - help_.wordStart, utf8);
        setCursor_ = help_.wordStart + (int)utf8.size();
        cursor_ = setCursor_;
        focus_ = true;
        dirty_ = true;
      }
    }
    ImGui::EndListBox();
  }
  ImGui::EndGroup();

  //where the cursor is, and what the game can't read
  int line = 1, column = 1;
  for (int i = 0; i < cursor_ && i < (int)utf8_.size(); ++i){
    if (utf8_[i] == '\n'){
      ++line;
      column = 1;
    }
    else if (((unsigned char)utf8_[i] & 0xC0) != 0x80)
      ++column;
  }
  ImGui::TextDisabled("Line %d, column %d", line, column);
  ImGui::SameLine();
  if (errors_.empty())
    ImGui::TextDisabled("- the game reads this script");
  else
    ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "- %d error%s", (int)errors_.size(), errors_.size() == 1 ? "" : "s");
  if (!errors_.empty() && ImGui::BeginChild("##errors", ImVec2(-FLT_MIN, errorHeight))){
    for (size_t i = 0; i < errors_.size(); ++i){
      const ScriptError& err = errors_[i];
      ImGui::PushID((int)i);
      std::string label = "Line " + std::to_string(err.line) + ": " + toUtf8(err.message);
      if (ImGui::Selectable(label.c_str())){
        //to the error's line
        int pos = 0;
        for (int l = 1; l < err.line && pos < (int)utf8_.size(); ++pos){
          if (utf8_[pos] == '\n')
            ++l;
        }
        setCursor_ = pos;
        cursor_ = pos;
        focus_ = true;
      }
      ImGui::PopID();
    }
  }
  if (!errors_.empty())
    ImGui::EndChild();
  return changed;
}

void ScriptEditor::helpAt(const Project& project){
  help_ = Help();
  const int cursor = std::min(cursor_, (int)utf8_.size());
  //the word around the cursor, which a match replaces
  int start = cursor;
  while (start > 0 && !isWordEnd(utf8_[start-1]))
    --start;
  int end = cursor;
  while (end < (int)utf8_.size() && !isWordEnd(utf8_[end]))
    ++end;
  help_.wordStart = start;
  help_.wordEnd = end;

  //the cursor's line, in the project's encoding
  int lineStart = cursor;
  while (lineStart > 0 && utf8_[lineStart-1] != '\n')
    --lineStart;
  size_t lineEnd = utf8_.find('\n', cursor);
  std::string line = fromUtf8(utf8_.substr(lineStart, lineEnd == std::string::npos ? std::string::npos : lineEnd - lineStart));
  const int pos = (int)fromUtf8(utf8_.substr(lineStart, cursor - lineStart)).size();
  if (trim(line).empty())
    return;

  PcdkParser parser(language_);
  bool found = false;
  parser.onFunction = [&](const PcdkParser::Argument& func, const std::vector<PcdkParser::Argument>& args, int){
    if (found)
      return;
    if (func.start <= pos && pos <= func.stop){
      found = true;
      help_.info = func.definition.name;
      fillMatches(parser, func.definition, trim(line.substr(func.start, pos - func.start)), project);
      return;
    }
    for (const PcdkParser::Argument& arg : args){
      if (arg.start <= pos && pos <= arg.stop){
        found = true;
        help_.info = arg.definition.name;
        fillMatches(parser, arg.definition, trim(line.substr(arg.start, std::max(pos - arg.start, 0))), project);
        return;
      }
    }
  };
  parser.onError = [&](int charpos, int length, const std::string& text, PcdkParser::Error error){
    if (found || error != PcdkParser::Error::LineNotRecognized)
      return;
    //a command being typed: its name is the last word
    found = true;
    size_t space = text.rfind(' ');
    int wordStart = space == std::string::npos ? charpos : charpos + (int)space + 1;
    help_.info = "Command";
    fillMatches(parser, PcdkParser::ArgDef("Command", PcdkParser::ArgType::Function), trim(line.substr(wordStart, std::max(pos - wordStart, 0))), project);
  };
  parser.parseLine(0, line);
}

void ScriptEditor::fillMatches(const PcdkParser& parser, const PcdkParser::ArgDef& def, const std::string& typed, const Project& project){
  std::vector<std::string>& m = help_.matches;
  for (const std::string& value : def.additionalValues)
    addMatch(m, value, typed);
  using T = PcdkParser::ArgType;
  switch (def.type){
    case T::Boolean:
      addMatch(m, "true", typed);
      addMatch(m, "false", typed);
      break;
    case T::Character:
      addMatch(m, "self", typed);
      for (const auto& room : project.characterInstances){
        for (const CharacterInstance& chr : room.value)
          addMatch(m, chr.name, typed);
      }
      break;
    case T::Command:
      for (const auto& cmd : project.settings.commands)
        addMatch(m, cmd.first, typed);
      break;
    case T::Event:
      for (const auto& cmd : project.settings.commands){
        addMatch(m, cmd.first, typed);
        addMatch(m, "cant" + cmd.first, typed);
      }
      break;
    case T::Function:
      for (const auto& func : parser.functions())
        addMatch(m, func.first, typed);
      break;
    case T::Item:
      for (const auto& item : project.items)
        addMatch(m, item.value.name, typed);
      break;
    case T::Music:
      for (const auto& music : project.music)
        addMatch(m, music.key, typed);
      break;
    case T::Object:
      for (const auto& room : project.rooms){
        for (const ObjectInstance& obj : room.value.objects)
          addMatch(m, obj.name, typed);
      }
      break;
    case T::PresetVariable:
      for (const auto& b : project.settings.booleans)
        addMatch(m, b.first, typed);
      break;
    case T::Room:
      for (const auto& room : project.rooms)
        addMatch(m, room.value.name, typed);
      break;
    case T::Script:
      for (const auto& scr : project.scripts[Script::Cutscene])
        addMatch(m, scr.value.name, typed);
      break;
    case T::Sound:
      for (const auto& sound : project.sounds)
        addMatch(m, sound.key, typed);
      break;
    case T::Video:
    case T::VideoSwf:
      for (const auto& video : project.videos){
        if ((extensionOf(video.value) == ".swf") == (def.type == T::VideoSwf))
          addMatch(m, video.key, typed);
      }
      break;
    case T::Language:
      for (const auto& lang : project.languages)
        addMatch(m, lang.key, typed);
      break;
    case T::DSPEffect:
      for (const DSPEffect& effect : project.settings.dspEffects)
        addMatch(m, effect.name, typed);
      addMatch(m, "off", typed);
      break;
    default:
      break;
  }
}

}
