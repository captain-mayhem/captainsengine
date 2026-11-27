// The project's windows: its settings, the commands and bools, and the
// languages; the C# StoryDesigner's Settings, CommandsAndBools and
// LanguageDlg dialogs, changing the project directly.

#include "designer.h"
#include "i18n.h"

#include <algorithm>
#include <cfloat>

#include <model/languages.h>

#include "widgets.h"

namespace sd{

namespace{

std::string roomName(const Room& room) {return room.name;}
std::string scriptName(const Script& scr) {return scr.name;}

//! the C# dialog's words for a fading time in milliseconds
const char* fadingText(int ms){
  if (ms == 0)
    return "<none>";
  if (ms > 1600)
    return "really slow";
  if (ms > 1200)
    return "slow";
  if (ms > 800)
    return "average";
  if (ms > 400)
    return "fast";
  return "very fast";
}

bool fading(const char* label, int& value){
  ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
  bool changed = inputInt(label, value, 0, 2000, 100);
  ImGui::SameLine();
  ImGui::TextDisabled("%s", tr(fadingText(value)));
  return changed;
}

//! the image names, sorted for the combo boxes
std::vector<std::string> imageNames(const Project& project){
  std::vector<std::string> names;
  for (const auto& image : project.images)
    names.push_back(image.key);
  std::sort(names.begin(), names.end());
  return names;
}

}

void Designer::settingsWindow(){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(windowTitle(kSettings), &showSettings_)){
    ImGui::End();
    return;
  }
  ProjectSettings& s = project_.settings;
  const std::vector<std::string> rooms = namesOf<Room>(project_.rooms, roomName);
  const std::vector<std::string> scripts = namesOf<Script>(project_.scripts[Script::Cutscene], scriptName);
  const std::vector<std::string> images = imageNames(project_);
  const float fieldWidth = 16*ImGui::GetFontSize();
  bool c = false;
  if (ImGui::BeginTabBar("pages")){
    if (ImGui::BeginTabItem(tr("General"))){
      ImGui::SetNextItemWidth(fieldWidth);
      c |= inputText("Project name", s.projectName);
      ImGui::TextUnformatted(tr("Aspect ratio:"));
      ImGui::SameLine();
      int wide = s.is16to9 ? 0 : 1;
      if (radioButtons(wide, {"16:9", "4:3"}, true)){
        s.is16to9 = wide == 0;
        c = true;
      }
      //the resolution is kept as 4:3; 16:9 widens it
      static const Vec2i kResolutions[] = {{320, 200}, {320, 240}, {640, 480}, {800, 600}, {1024, 768}, {1440, 1080}};
      static const char* const kNames43[] = {"320x200", "320x240", "640x480", "800x600", "1024x768", "1440x1080"};
      static const char* const kNames169[] = {"420x200", "420x240", "840x480", "1050x600", "1344x768", "1890x1080"};
      int res = -1;
      for (int i = 0; i < 6; ++i){
        if (s.resolution.x == kResolutions[i].x && s.resolution.y == kResolutions[i].y)
          res = i;
      }
      const char* const* resNames = s.is16to9 ? kNames169 : kNames43;
      ImGui::SetNextItemWidth(fieldWidth);
      if (ImGui::BeginCombo(tr("Game resolution"), res >= 0 ? resNames[res] : "(other)")){
        for (int i = 0; i < 6; ++i){
          if (ImGui::Selectable(resNames[i], i == res) && i != res){
            s.resolution = kResolutions[i];
            c = true;
          }
        }
        ImGui::EndCombo();
      }
      c |= checkbox("No antialiasing", s.notAntialiased);
      c |= checkbox("Mute music when speech", s.muteMusicWhenSpeech);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= inputText("Symbol (.ico file)", s.gameIcon);
      c |= checkbox("Text on / off", s.textOnOff);
      c |= checkbox("Draw dragged icon items", s.drawDraggedItemIcons);
      c |= checkbox("Group items", s.groupItems);
      c |= checkbox("Action text", s.actionText);
      ImGui::SameLine();
      ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
      c |= inputInt("Action text height", s.actionTextHeight, -99, 999);
      c |= checkbox("Silent delete", s.silentDelete);
      c |= checkbox("Info line", s.infoLine);
      c |= checkbox("Protect game files", s.protectGameFile);
      //the editor's own settings, kept for all projects
      checkbox("Create backups", prefs_.createBackups);
      checkbox("Fit images", prefs_.fitImages);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("\"Loading\" image", s.loadingImage, images);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("\"Settings\" picture", s.settingsPicture, images);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(tr("Script and taskbar"))){
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Start cutscene", s.startScript, scripts);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Main script", s.mainScript, scripts);
      ImGui::SeparatorText(tr("Screen change"));
      int sc = (int)s.screenChange;
      if (radioButtons(sc, {"Direct", "Fade out to black", "Rectangle", "Circle", "Shutters", "Clock", "Blend", "Blend slow"})){
        s.screenChange = (ScreenChange)sc;
        c = true;
      }
      ImGui::SeparatorText(tr("Taskbar"));
      c |= checkbox("Taskbar", s.showTaskbar);
      ImGui::BeginDisabled(!s.showTaskbar);
      ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
      c |= inputInt("Height", s.taskHeight, 0, 2000);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Room with the taskbar", s.taskRoom, rooms);
      c |= checkbox("When down, hide completely", s.taskHideCompletely);
      ImGui::TextUnformatted(tr("Appears from:"));
      ImGui::SameLine();
      int top = s.taskbarFromTop ? 0 : 1;
      if (radioButtons(top, {"Top", "Bottom"}, true)){
        s.taskbarFromTop = top == 0;
        c = true;
      }
      ImGui::TextUnformatted(tr("Taskbar style:"));
      c |= radioButtons(s.taskPopup, {"Always there", "Direct popup", "Scrolling popup"}, true);
      ImGui::EndDisabled();
      ImGui::SeparatorText(tr("Anywhere"));
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Room for \"anywhere\" objects", s.anywhereRoom, rooms);
      ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
      c |= inputInt("Transparency", s.anywhereTransparency, 0, 100);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(tr("Menus and colors"))){
      ImGui::SeparatorText(tr("Menu colors"));
      c |= colorEdit("Border", s.borderColor);
      c |= colorEdit("Background", s.backgroundColor);
      c |= colorEdit("Text", s.textColor);
      ImGui::Separator();
      c |= checkbox("Use custom menu", s.customMenu);
      ImGui::BeginDisabled(!s.customMenu);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Room with custom menu", s.customMenuRoom, rooms);
      ImGui::EndDisabled();
      c |= fading("Fading##menu", s.menuFading);
      ImGui::Separator();
      c |= colorEdit("Offspeech color", s.offspeechColor);
      c |= colorEdit("Infotext color", s.infotextColor);
      c |= colorEdit("Targa transparent color", s.targaColor);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(tr("Textscene"))){
      ImGui::SeparatorText(tr("Style"));
      c |= radioButtons(s.tsStyle, {"Solid", "50% transparent", "Additive", "None"}, true);
      ImGui::SeparatorText(tr("Border"));
      ImGui::PushID("border");
      c |= radioButtons(s.tsBorderStyle, {"Rectangles", "Lines", "Box", "None"}, true);
      ImGui::PopID();
      c |= fading("Fading##textscene", s.textSceneFading);
      ImGui::SeparatorText(tr("Colors"));
      c |= colorEdit("Area color", s.tsAreaColor);
      c |= colorEdit("Border color", s.tsBorderColor);
      c |= colorEdit("Text color", s.tsTextColor);
      c |= colorEdit("Selection color", s.tsSelectionColor);
      ImGui::SeparatorText(tr("Appearance"));
      int symbols = s.tsUseSymbols ? 1 : 0;
      if (radioButtons(symbols, {"Text", "Symbol"}, true)){
        s.tsUseSymbols = symbols == 1;
        c = true;
      }
      c |= checkbox("Use background image", s.tsUseBgImage);
      ImGui::BeginDisabled(!s.tsUseBgImage);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Background image", s.tsBackground, images);
      ImGui::EndDisabled();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(tr("Coin interface"))){
      c |= checkbox("Activate coin interface", s.coinActivated);
      ImGui::BeginDisabled(!s.coinActivated);
      ImGui::SetNextItemWidth(fieldWidth);
      c |= nameCombo("Room that contains the interface", s.coinRoom, rooms);
      c |= checkbox("Auto-popup with right mouse click", s.coinAutoPopup);
      c |= fading("Fading##coin", s.coinFading);
      //the centre on a 190 x 140 field, as in the C# dialog; drag the cross
      ImGui::TextUnformatted(tr("Interface center (drag the cross):"));
      const ImVec2 size(190, 140);
      ImVec2 origin = ImGui::GetCursorScreenPos();
      ImGui::InvisibleButton("##center", size);
      ImDrawList* draw = ImGui::GetWindowDrawList();
      draw->AddRectFilled(origin, ImVec2(origin.x+size.x, origin.y+size.y), ImGui::GetColorU32(ImGuiCol_FrameBg));
      if (ImGui::IsItemActive() && s.coinActivated){
        ImVec2 mouse = ImGui::GetIO().MousePos;
        Vec2i center{(int)std::clamp(mouse.x - origin.x, 0.0f, size.x), (int)std::clamp(mouse.y - origin.y, 0.0f, size.y)};
        if (center.x != s.coinCenter.x || center.y != s.coinCenter.y){
          s.coinCenter = center;
          c = true;
        }
      }
      const ImVec2 p(origin.x + s.coinCenter.x, origin.y + s.coinCenter.y);
      const ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
      draw->AddLine(ImVec2(p.x-8, p.y), ImVec2(p.x+9, p.y), col);
      draw->AddLine(ImVec2(p.x, p.y-8), ImVec2(p.x, p.y+9), col);
      draw->AddCircle(p, 5, col);
      ImGui::SameLine();
      ImGui::TextDisabled("%d, %d", s.coinCenter.x, s.coinCenter.y);
      ImGui::EndDisabled();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(tr("Advanced"))){
      c |= checkbox("Png to Jpeg conversion", s.pngToJpeg);
      checkbox("Crop transparent image parts", prefs_.cropTransparent);
      int lang = (int)s.scriptingLanguage;
      ImGui::SetNextItemWidth(fieldWidth);
      if (ImGui::Combo(tr("Scripting language"), &lang, "PCDK\0Lua\0")){
        if (lang != (int)s.scriptingLanguage){
          s.scriptingLanguage = (ScriptLang)lang;
          c = true;
        }
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  if (c)
    changed();
  ImGui::End();
}

void Designer::commandsWindow(){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(windowTitle(kCommands), &showCommands_)){
    ImGui::End();
    return;
  }
  ProjectSettings& s = project_.settings;
  bool c = false;
  const float fontSize = ImGui::GetFontSize();
  ImGui::SeparatorText(tr("Commands"));
  int remove = -1;
  if (ImGui::BeginTable("commands", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)){
    ImGui::TableSetupColumn("Command");
    ImGui::TableSetupColumn("Text");
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 2*fontSize);
    ImGui::TableHeadersRow();
    for (size_t i = 0; i < s.commands.size(); ++i){
      ImGui::PushID((int)i);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-FLT_MIN);
      c |= inputText("##command", s.commands[i].first);
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-FLT_MIN);
      c |= inputText("##text", s.commands[i].second);
      ImGui::TableNextColumn();
      if (ImGui::SmallButton(tr("-")))
        remove = (int)i;
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
  if (remove >= 0){
    s.commands.erase(s.commands.begin()+remove);
    c = true;
  }
  if (ImGui::Button(tr("Add command"))){
    s.commands.push_back(std::make_pair(std::string("command") + std::to_string(s.commands.size()+1), std::string("Command")));
    c = true;
  }

  ImGui::SeparatorText(tr("Bools"));
  ImGui::TextDisabled("Checked: true at the start");
  remove = -1;
  for (size_t i = 0; i < s.booleans.size(); ++i){
    ImGui::PushID((int)(1000+i));
    c |= checkbox("##value", s.booleans[i].second);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(16*fontSize);
    c |= inputText("##name", s.booleans[i].first);
    ImGui::SameLine();
    if (ImGui::SmallButton(tr("-")))
      remove = (int)i;
    ImGui::PopID();
  }
  if (remove >= 0){
    s.booleans.erase(s.booleans.begin()+remove);
    c = true;
  }
  if (ImGui::Button(tr("Add bool"))){
    std::string name = "bool" + std::to_string(s.booleans.size()+1);
    s.booleans.push_back(std::make_pair(name, false));
    c = true;
  }

  ImGui::SeparatorText(tr("Texts"));
  ImGui::SetNextItemWidth(16*fontSize);
  c |= inputText("Link text", s.linkText);
  ImGui::SetNextItemWidth(16*fontSize);
  c |= inputText("Give link", s.giveLink);
  ImGui::SetNextItemWidth(16*fontSize);
  c |= inputText("Walk text", s.walkText);
  if (c)
    changed();
  ImGui::End();
}

void Designer::languagesWindow(){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(windowTitle(kLanguages), &showLanguages_)){
    ImGui::End();
    return;
  }
  bool c = false;
  const float fontSize = ImGui::GetFontSize();
  Language* origin = project_.languages.find("origin");
  if (origin == nullptr){
    ImGui::TextDisabled("The project has no texts yet.");
    ImGui::End();
    return;
  }
  if (project_.languages.find(language_) == nullptr || toLower(language_) == "origin")
    language_.clear();
  ImGui::SetNextItemWidth(12*fontSize);
  if (ImGui::BeginCombo(tr("Translation"), language_.empty() ? "(none)" : toUtf8(language_).c_str())){
    if (ImGui::Selectable(tr("(none)"), language_.empty()))
      language_.clear();
    for (const auto& lang : project_.languages){
      if (lang.key == "origin")
        continue;
      if (ImGui::Selectable(toUtf8(lang.value.name).c_str(), toLower(language_) == lang.key))
        language_ = lang.value.name;
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine();
  if (ImGui::Button(tr("Add language..."))){
    nameFor_ = NameFor::Language;
    nameBuf_[0] = '\0';
  }
  ImGui::SameLine();
  ImGui::BeginDisabled(language_.empty());
  if (ImGui::Button(tr("Delete language"))){
    project_.languages.remove(language_);
    language_.clear();
    c = true;
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  if (ImGui::Button(tr("Gather texts"))){
    updateLanguageList(project_);
    c = true;
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Takes the texts from the scripts and commands again");
  //the pointers can move when a language was added or deleted
  origin = project_.languages.find("origin");
  Language* translation = language_.empty() ? nullptr : project_.languages.find(language_);

  struct Page{
    const char* title;
    Language::Section section;
    int sounds;
  };
  static const Page kPages[] = {
    {"Speech", Language::Speech, Language::SpeechSounds},
    {"Offspeech", Language::Offspeech, Language::OffspeechSounds},
    {"Showinfo", Language::Showinfo, -1},
    {"Textout", Language::Textout, -1},
    {"Setstring", Language::Setstring, -1},
    {"Row (textscene)", Language::Textscenes, -1},
    {"Commands", Language::Commands, -1},
  };
  if (ImGui::BeginTabBar("sections")){
    for (const Page& page : kPages){
      if (!ImGui::BeginTabItem(page.title))
        continue;
      const std::vector<std::string>& texts = origin->sections[page.section];
      int columns = 1 + (page.sounds >= 0 ? 1 : 0) + (translation ? 1 + (page.sounds >= 0 ? 1 : 0) : 0);
      if (ImGui::BeginTable("texts", columns, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY)){
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Text", 0, 3.0f);
        if (page.sounds >= 0)
          ImGui::TableSetupColumn("Voice", 0, 1.0f);
        if (translation){
          ImGui::TableSetupColumn(toUtf8(translation->name).c_str(), 0, 3.0f);
          if (page.sounds >= 0)
            ImGui::TableSetupColumn("Its voice", 0, 1.0f);
        }
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < texts.size(); ++i){
          ImGui::PushID((int)i);
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::TextWrapped("%s", toUtf8(texts[i]).c_str());
          if (page.sounds >= 0){
            ImGui::TableNextColumn();
            const std::vector<std::string>& voices = origin->sections[page.sounds];
            ImGui::TextUnformatted(i < voices.size() ? toUtf8(voices[i]).c_str() : "");
          }
          if (translation){
            //a translation shorter than origin grows when a text is given
            auto edit = [&](const char* id, std::vector<std::string>& words){
              std::string word = i < words.size() ? words[i] : std::string();
              ImGui::SetNextItemWidth(-FLT_MIN);
              if (inputText(id, word)){
                if (words.size() <= i)
                  words.resize(i+1);
                words[i] = word;
                c = true;
              }
            };
            ImGui::TableNextColumn();
            edit("##text", translation->sections[page.section]);
            if (page.sounds >= 0){
              ImGui::TableNextColumn();
              edit("##voice", translation->sections[page.sounds]);
            }
          }
          ImGui::PopID();
        }
        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  if (c)
    changed();
  ImGui::End();
}

}
