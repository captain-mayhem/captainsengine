#include "widgets.h"
#include "i18n.h"

#include <algorithm>

namespace sd{

namespace{

int resizeCallback(ImGuiInputTextCallbackData* data){
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize){
    std::string* str = (std::string*)data->UserData;
    str->resize(data->BufTextLen);
    data->Buf = str->data();
  }
  return 0;
}

}

bool inputTextUtf8(const char* label, std::string& utf8, ImGuiInputTextFlags flags, const ImVec2& size){
  flags |= ImGuiInputTextFlags_CallbackResize;
  if (size.x != 0 || size.y != 0)
    return ImGui::InputTextMultiline(tr(label), utf8.data(), utf8.capacity()+1, size, flags, resizeCallback, &utf8);
  return ImGui::InputText(tr(label), utf8.data(), utf8.capacity()+1, flags, resizeCallback, &utf8);
}

bool inputText(const char* label, std::string& value, ImGuiInputTextFlags flags){
  std::string utf8 = toUtf8(value);
  if (!inputTextUtf8(label, utf8, flags))
    return false;
  std::string converted = fromUtf8(utf8);
  if (converted == value)
    return false;
  value = converted;
  return true;
}

bool nameCombo(const char* label, std::string& value, const std::vector<std::string>& names, bool allowNone){
  bool changed = false;
  std::string preview = value.empty() ? std::string("(none)") : toUtf8(value);
  if (ImGui::BeginCombo(tr(label), preview.c_str())){
    if (allowNone && ImGui::Selectable(tr("(none)"), value.empty())){
      changed = !value.empty();
      value.clear();
    }
    const std::string current = toLower(value);
    for (const std::string& name : names){
      bool selected = toLower(name) == current;
      if (ImGui::Selectable(toUtf8(name).c_str(), selected)){
        changed = changed || name != value;
        value = name;
      }
      if (selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

bool colorEdit(const char* label, uint32_t& color){
  float rgb[3] = {(color & 0xFF)/255.0f, ((color >> 8) & 0xFF)/255.0f, ((color >> 16) & 0xFF)/255.0f};
  if (!ImGui::ColorEdit3(tr(label), rgb, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
    return false;
  uint32_t value = 0;
  for (int i = 0; i < 3; ++i)
    value |= (uint32_t)(std::clamp(rgb[i], 0.0f, 1.0f)*255.0f + 0.5f) << (8*i);
  if (value == color)
    return false;
  color = value;
  return true;
}

bool checkbox(const char* label, bool& value){
  return ImGui::Checkbox(tr(label), &value);
}

bool inputInt(const char* label, int& value, int min, int max, int step){
  int v = value;
  if (!ImGui::InputInt(tr(label), &v, step, step*10))
    return false;
  v = std::clamp(v, min, max);
  if (v == value)
    return false;
  value = v;
  return true;
}

bool radioButtons(int& value, const std::vector<const char*>& labels, bool sameLine){
  bool changed = false;
  for (size_t i = 0; i < labels.size(); ++i){
    if (sameLine && i > 0)
      ImGui::SameLine();
    if (ImGui::RadioButton(tr(labels[i]), value == (int)i) && value != (int)i){
      value = (int)i;
      changed = true;
    }
  }
  return changed;
}

}
