#ifndef SD_WIDGETS_H
#define SD_WIDGETS_H

#include <string>
#include <vector>

#include <imgui.h>

#include <model/project.h>

namespace sd{

// ImGui widgets for the project's values. The project's texts are
// Windows-1252 and ImGui's UTF-8; these convert. Each returns true when the
// value changed.

//! a text field for a project text
bool inputText(const char* label, std::string& value, ImGuiInputTextFlags flags = 0);
//! a text field for UTF-8 text
bool inputTextUtf8(const char* label, std::string& utf8, ImGuiInputTextFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
//! a combo box of names, with "(none)" for an empty value when allowNone
bool nameCombo(const char* label, std::string& value, const std::vector<std::string>& names, bool allowNone = true);
//! a colour as the files keep it: Windows' COLORREF, red in the low byte
bool colorEdit(const char* label, uint32_t& color);
bool checkbox(const char* label, bool& value);
//! an integer in [min, max]
bool inputInt(const char* label, int& value, int min, int max, int step = 1);
//! radio buttons for an int, one per label, counted from 0
bool radioButtons(int& value, const std::vector<const char*>& labels, bool sameLine = false);

//! the names of a list, in its order
template <typename T>
std::vector<std::string> namesOf(const NamedList<T>& list, std::string (*nameOf)(const T&)){
  std::vector<std::string> names;
  for (const auto& entry : list)
    names.push_back(nameOf(entry.value));
  return names;
}

}

#endif
