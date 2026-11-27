#include "filedialog.h"
#include "i18n.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <filesystem>
#include <system_error>

#include <imgui.h>

namespace fs = std::filesystem;

namespace sd{

namespace{

fs::path toPath(const std::string& utf8){
  return fs::path(std::u8string(utf8.begin(), utf8.end()));
}

std::string fromPath(const fs::path& path){
  std::u8string str = path.u8string();
  return std::string(str.begin(), str.end());
}

}

void FileDialog::open(Mode mode, const std::string& title, const std::string& folder, const std::vector<std::string>& extensions, bool multiple){
  mode_ = mode;
  multiple_ = multiple && mode == Mode::Open;
  chosen_.clear();
  paths_.clear();
  title_ = title;
  std::error_code ec;
  folder_ = folder.empty() ? fromPath(fs::current_path(ec)) : folder;
  extensions_ = extensions;
  name_[0] = '\0';
  path_.clear();
  open_ = true;
  justOpened_ = true;
  list();
}

void FileDialog::setName(const std::string& name){
  snprintf(name_, sizeof(name_), "%s", name.c_str());
}

bool FileDialog::matches(const std::string& name) const{
  if (extensions_.empty())
    return true;
  std::string ext = fromPath(toPath(name).extension());
  std::transform(ext.begin(), ext.end(), ext.begin(), [](char c){return (char)tolower((unsigned char)c);});
  return std::find(extensions_.begin(), extensions_.end(), ext) != extensions_.end();
}

void FileDialog::list(){
  dirs_.clear();
  files_.clear();
  std::error_code ec;
  fs::path folder = toPath(folder_);
  if (folder.has_parent_path() && folder.parent_path() != folder)
    dirs_.push_back("..");
  for (fs::directory_iterator iter(folder, fs::directory_options::skip_permission_denied, ec), end; !ec && iter != end; iter.increment(ec)){
    std::string name = fromPath(iter->path().filename());
    if (iter->is_directory(ec))
      dirs_.push_back(name);
    else if (matches(name))
      files_.push_back(name);
  }
  std::sort(dirs_.begin()+(dirs_.empty() || dirs_[0] != ".." ? 0 : 1), dirs_.end());
  std::sort(files_.begin(), files_.end());
}

bool FileDialog::draw(){
  if (!open_)
    return false;
  if (justOpened_){
    const float size = ImGui::GetFontSize();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(50*size, 30*size));
    ImGui::SetNextWindowFocus();
    justOpened_ = false;
  }
  bool chosen = false;
  bool keepOpen = true;
  //the style's windows are see-through, which makes the lists hard to read over the views
  ImGui::SetNextWindowBgAlpha(1.0f);
  if (ImGui::Begin(title_.c_str(), &keepOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking)){
    ImGui::TextUnformatted(folder_.c_str());
    const ImGuiStyle& style = ImGui::GetStyle();
    const float half = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x)/2;
    const ImVec2 listSize(half, -2*ImGui::GetFrameHeightWithSpacing());
    std::string dir;
    if (ImGui::BeginListBox("##dirs", listSize)){
      for (const std::string& entry : dirs_){
        if (ImGui::Selectable((entry + "/").c_str()))
          dir = entry;
      }
      ImGui::EndListBox();
    }
    ImGui::SameLine();
    bool accept = false;
    if (ImGui::BeginListBox("##files", listSize)){
      for (const std::string& entry : files_){
        bool selected = entry == name_ || std::find(chosen_.begin(), chosen_.end(), entry) != chosen_.end();
        if (ImGui::Selectable(entry.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)){
          if (multiple_ && ImGui::GetIO().KeyCtrl){
            //Ctrl adds files to the chosen ones (the one chosen before too), or takes them off
            if (chosen_.empty() && name_[0] != '\0' && entry != name_)
              chosen_.push_back(name_);
            auto it = std::find(chosen_.begin(), chosen_.end(), entry);
            if (it != chosen_.end())
              chosen_.erase(it);
            else
              chosen_.push_back(entry);
            snprintf(name_, sizeof(name_), "%s", chosen_.empty() ? "" : chosen_.back().c_str());
          }
          else{
            chosen_.clear();
            snprintf(name_, sizeof(name_), "%s", entry.c_str());
            accept = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
          }
        }
      }
      ImGui::EndListBox();
    }
    //after the lists, which it changes
    if (!dir.empty()){
      chosen_.clear();
      fs::path folder = toPath(folder_);
      folder_ = fromPath(dir == ".." ? folder.parent_path() : folder / toPath(dir));
      list();
    }
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##file", name_, sizeof(name_), ImGuiInputTextFlags_EnterReturnsTrue))
      accept = true;
    ImGui::BeginDisabled(name_[0] == '\0');
    if (ImGui::Button(mode_ == Mode::Open ? "Open" : "Save", ImVec2(6*ImGui::GetFontSize(), 0)))
      accept = true;
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(tr("Cancel"), ImVec2(6*ImGui::GetFontSize(), 0)))
      keepOpen = false;
    if (accept && name_[0] != '\0'){
      std::string name = name_;
      //a name typed without an extension gets the first one
      if (mode_ == Mode::Save && !extensions_.empty() && !matches(name))
        name += extensions_[0];
      path_ = fromPath(toPath(folder_) / toPath(name));
      paths_.clear();
      if (multiple_ && !chosen_.empty()){
        for (const std::string& file : chosen_)
          paths_.push_back(fromPath(toPath(folder_) / toPath(file)));
      }
      else
        paths_.push_back(path_);
      chosen = true;
      keepOpen = false;
    }
  }
  ImGui::End();
  if (!keepOpen)
    open_ = false;
  return chosen;
}

}
