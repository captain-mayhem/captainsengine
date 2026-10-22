
#include "menu.h"
#include "graphics.h"
#include "editor.h"
#include "hqmExport.h"

#include <cfloat>
#include <cstdio>
#include <cstring>

#include <imgui.h>

#include "system/engine.h"
#include "system/file.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "renderer/texture.h"

using std::string;
using std::vector;

using CGE::Filesystem;
using CGE::Mesh;
using CGE::Model;
using CGE::Texture;

#ifdef WIN32
#define SEPARATOR "\\"
#endif
#ifdef UNIX
#define SEPARATOR "/"
#endif

namespace{

//! the dialogs; one is open at a time
enum class Dialog{
  None,
  File,
  AddModel,
  Properties,
  AssignTexture,
  Options,
};

//! what the file dialog was opened for
enum class FileAction{
  AddMesh,
  AddTexture,
  SaveScene,
  LoadScene,
};

const int NUM_ATTRIBS = 8;
const int NUM_STAGES = 8;
const char* PLANE_NAMES[] = {"XZ Plane", "XY Plane", "YZ Plane"};

//! the open dialog and what it shows
struct State{
  Dialog dialog = Dialog::None;
  //the dialog was opened in this frame: it is centred
  bool opened = false;
  //file dialog: the working directory, the path relative to it and its contents
  FileAction fileAction = FileAction::AddMesh;
  string cwd = Filesystem::getCwd();
  string path = ".";
  vector<string> dirs;
  vector<string> files;
  char filename[256] = "";
  //the selected list entry
  int selected = -1;
  //the texture stage
  int stage = 0;
  //the current model's attributes
  char attribs[NUM_ATTRIBS][64] = {};
  int plane = 0;
  float gridStep = 1;
};

State state;

void open(Dialog dialog){
  state.dialog = dialog;
  state.opened = true;
  state.selected = -1;
}

void close(){
  state.dialog = Dialog::None;
}

//! a dialog window of width x height font sizes, centred when it opens
/*! Always followed by ImGui::End(). */
bool beginDialog(const char* title, float width, float height){
  if (state.opened){
    const float size = ImGui::GetFontSize();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(width*size, height*size));
    state.opened = false;
  }
  bool keepOpen = true;
  bool visible = ImGui::Begin(title, &keepOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
  if (!keepOpen)
    close();
  return visible;
}

// --- file dialog ---

void listDirectory(){
  state.dirs = Filesystem::getDirectories(state.cwd+SEPARATOR+state.path);
  state.files = Filesystem::getFiles(state.cwd+SEPARATOR+state.path);
}

void openFile(FileAction action){
  state.fileAction = action;
  state.filename[0] = '\0';
  listDirectory();
  open(Dialog::File);
}

void changeDir(const string& dir){
  //collapse the path when .. is chosen and the situation is appropriate
  string& path = state.path;
  if (dir == ".." && path.length() > 1 && path[path.length()-1] != '.'){
    size_t n = path.find_last_of(SEPARATOR);
    if (n != path.npos)
      path.erase(n);
    else
      path += ".";
  }
  else
    path += SEPARATOR+dir;
  listDirectory();
}

void acceptFile(){
  string relFile = state.path+SEPARATOR+state.filename;
  string file = state.cwd+SEPARATOR+relFile;
  switch (state.fileAction){
    case FileAction::AddMesh:
      Graphic::instance()->addMesh(relFile);
      break;
    case FileAction::AddTexture:
      Graphic::instance()->addTexture(relFile);
      break;
    case FileAction::SaveScene:
      Graphic::instance()->getScene().save(file);
      Graphic::instance()->setFilename(file);
      break;
    case FileAction::LoadScene:
      Graphic::instance()->getScene().load(file);
      Graphic::instance()->setFilename(file);
      break;
  }
  close();
}

void fileDialog(){
  if (beginDialog("Choose a file", 50, 30)){
    ImGui::TextUnformatted(state.path.c_str());
    const ImGuiStyle& style = ImGui::GetStyle();
    const float half = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x)/2;
    const ImVec2 listSize(half, -2*ImGui::GetFrameHeightWithSpacing());
    string dir;
    if (ImGui::BeginListBox("##dirs", listSize)){
      for (const string& entry : state.dirs){
        if (ImGui::Selectable(entry.c_str()))
          dir = entry;
      }
      ImGui::EndListBox();
    }
    ImGui::SameLine();
    bool accept = false;
    if (ImGui::BeginListBox("##files", listSize)){
      for (const string& entry : state.files){
        if (ImGui::Selectable(entry.c_str(), entry == state.filename, ImGuiSelectableFlags_AllowDoubleClick)){
          snprintf(state.filename, sizeof(state.filename), "%s", entry.c_str());
          accept = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
        }
      }
      ImGui::EndListBox();
    }
    //after the lists, which it changes
    if (!dir.empty())
      changeDir(dir);
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##file", state.filename, sizeof(state.filename), ImGuiInputTextFlags_EnterReturnsTrue))
      accept = true;
    ImGui::BeginDisabled(state.filename[0] == '\0');
    if (ImGui::Button("OK", ImVec2(6*ImGui::GetFontSize(), 0)))
      accept = true;
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Abort", ImVec2(6*ImGui::GetFontSize(), 0)))
      close();
    if (accept && state.filename[0] != '\0')
      acceptFile();
  }
  ImGui::End();
}

// --- model dialogs ---

//! a list box of names that sets state.selected
void selectionList(const char* id, const vector<string>& names){
  if (ImGui::BeginListBox(id, ImVec2(-FLT_MIN, -ImGui::GetFrameHeightWithSpacing()*2))){
    for (int i = 0; i < (int)names.size(); i++){
      ImGui::PushID(i);
      if (ImGui::Selectable(names[i].c_str(), state.selected == i))
        state.selected = i;
      ImGui::PopID();
    }
    ImGui::EndListBox();
  }
}

void addModelDialog(){
  if (beginDialog("Add model", 30, 30)){
    vector<string> names;
    for (Mesh* mesh : Graphic::instance()->getScene().getMeshes())
      names.push_back(mesh->getName());
    selectionList("##meshes", names);
    if (ImGui::Button("OK", ImVec2(6*ImGui::GetFontSize(), 0))){
      if (state.selected >= 0)
        Graphic::instance()->addModel(state.selected);
      close();
    }
  }
  ImGui::End();
}

void openProperties(){
  Model* mdl = Graphic::instance()->getCurrModel();
  for (int i = 0; i < NUM_ATTRIBS; i++){
    string text = mdl ? Editor::instance()->attribString(mdl->getAttrib(i)) : "-1";
    snprintf(state.attribs[i], sizeof(state.attribs[i]), "%s", text.c_str());
  }
  open(Dialog::Properties);
}

void evaluateAttribs(){
  Model* mdl = Graphic::instance()->getCurrModel();
  if (!mdl)
    return;
  for (int i = 0; i < NUM_ATTRIBS; i++)
    mdl->setAttrib(i, Editor::instance()->attribValue(state.attribs[i]));
}

void propertiesDialog(){
  if (beginDialog("Model properties", 25, 22)){
    if (!Graphic::instance()->getCurrModel())
      ImGui::TextUnformatted("No model selected");
    if (ImGui::Button("Assign texture"))
      open(Dialog::AssignTexture);
    for (int i = 0; i < NUM_ATTRIBS; i++){
      char label[32];
      snprintf(label, sizeof(label), "Attribute %d", i);
      ImGui::InputText(label, state.attribs[i], sizeof(state.attribs[i]));
    }
    if (ImGui::Button("Close", ImVec2(6*ImGui::GetFontSize(), 0))){
      evaluateAttribs();
      close();
    }
  }
  ImGui::End();
}

void assignTextureDialog(){
  if (beginDialog("Assign texture", 30, 30)){
    vector<Texture*> textures = Graphic::instance()->getScene().getTextures();
    vector<string> names;
    for (Texture* tex : textures)
      names.push_back(tex->getName());
    char stage[16];
    snprintf(stage, sizeof(stage), "Stage %d", state.stage);
    if (ImGui::BeginCombo("Texture stage", stage)){
      for (int i = 0; i < NUM_STAGES; i++){
        snprintf(stage, sizeof(stage), "Stage %d", i);
        if (ImGui::Selectable(stage, state.stage == i))
          state.stage = i;
      }
      ImGui::EndCombo();
    }
    selectionList("##textures", names);
    if (ImGui::Button("OK", ImVec2(6*ImGui::GetFontSize(), 0))){
      Model* mdl = Graphic::instance()->getCurrModel();
      if (mdl && state.selected >= 0 && state.selected < (int)textures.size())
        mdl->assignTexture(textures[state.selected], state.stage);
      close();
    }
  }
  ImGui::End();
}

// --- options ---

void openOptions(){
  state.plane = (int)Editor::instance()->getEditPlane();
  state.gridStep = Editor::instance()->getGridStep();
  open(Dialog::Options);
}

void optionsDialog(){
  if (beginDialog("Options", 22, 9)){
    ImGui::Combo("Edit plane", &state.plane, PLANE_NAMES, IM_ARRAYSIZE(PLANE_NAMES));
    ImGui::InputFloat("Grid step", &state.gridStep);
    if (ImGui::Button("OK", ImVec2(6*ImGui::GetFontSize(), 0))){
      Editor::instance()->setEditPlane((Editor::Plane)state.plane);
      Editor::instance()->setGridStep(state.gridStep);
      close();
    }
  }
  ImGui::End();
}

void hqmExport(){
  HQMExport exp;
  std::string file = Graphic::instance()->getFilename();
  unsigned idx = file.find_last_of('.');
  file.erase(idx+1);
  file += "hqm";
  exp.exportHQM(Graphic::instance()->getScene(), file);
}

//! the buttons, in the lower right corner
void toolWindow(){
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  const float margin = ImGui::GetFontSize();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - margin, viewport->WorkPos.y + viewport->WorkSize.y - margin),
    ImGuiCond_FirstUseEver, ImVec2(1, 1));
  if (ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)){
    const ImVec2 size(9*ImGui::GetFontSize(), 0);
    if (ImGui::Button("Export HQM", size))
      hqmExport();
    if (ImGui::Button("Load scene", size))
      openFile(FileAction::LoadScene);
    if (ImGui::Button("Save scene", size))
      openFile(FileAction::SaveScene);
    if (ImGui::Button("Properties", size))
      openProperties();
    if (ImGui::Button("Add model", size))
      open(Dialog::AddModel);
    if (ImGui::Button("Add mesh", size))
      openFile(FileAction::AddMesh);
    if (ImGui::Button("Add texture", size))
      openFile(FileAction::AddTexture);
    if (ImGui::Button("Options", size))
      openOptions();
  }
  ImGui::End();
}

}

void Menu::render(){
  toolWindow();
  switch (state.dialog){
    case Dialog::File:
      fileDialog();
      break;
    case Dialog::AddModel:
      addModelDialog();
      break;
    case Dialog::Properties:
      propertiesDialog();
      break;
    case Dialog::AssignTexture:
      assignTextureDialog();
      break;
    case Dialog::Options:
      optionsDialog();
      break;
    case Dialog::None:
      break;
  }
}
