
#include "menu.h"
#include "graphics.h"
#include "editor.h"
#include "tdmExport.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>

#include <imgui.h>

#include "system/engine.h"
#include "system/file.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "mesh/modelfile.h"
#include "renderer/material.h"
#include "renderer/scenenode.h"

using std::string;
using std::vector;

using CGE::Filesystem;
using CGE::Material;
using CGE::Mesh;
using CGE::Model;
using CGE::ModelFile;
using CGE::SceneNode;

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
  AddModelFile,
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
  //only glTF files for a glTF model
  if (state.fileAction == FileAction::AddModelFile){
    vector<string> files;
    for (const string& file : state.files){
      if (Graphic::isModelFile(file))
        files.push_back(file);
    }
    state.files.swap(files);
  }
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
      //glTF files are models of their own
      if (Graphic::isModelFile(relFile))
        Graphic::instance()->addModelFile(relFile);
      else
        Graphic::instance()->addMesh(relFile);
      break;
    case FileAction::AddModelFile:
      Graphic::instance()->addModelFile(relFile);
      break;
    case FileAction::AddTexture:
      Graphic::instance()->addTexture(relFile);
      break;
    case FileAction::SaveScene:
      Graphic::instance()->getScene().save(file);
      Graphic::instance()->setFilename(file);
      if (!Graphic::instance()->getModelFiles().empty())
        Graphic::instance()->setMessage("The scene file keeps the meshes' models, not the glTF models");
      break;
    case FileAction::LoadScene:
      Graphic::instance()->loadScene(file);
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
    //the meshes, then the glTF models
    vector<string> names;
    for (Mesh* mesh : Graphic::instance()->getScene().getMeshes())
      names.push_back(mesh->getName());
    size_t meshes = names.size();
    for (const std::unique_ptr<ModelFile>& model : Graphic::instance()->getModelFiles())
      names.push_back(model->getRoot()->getName()+" (glTF)");
    selectionList("##meshes", names);
    if (ImGui::Button("OK", ImVec2(6*ImGui::GetFontSize(), 0))){
      if (state.selected >= 0 && state.selected < (int)meshes)
        Graphic::instance()->addModel(state.selected);
      else if (state.selected >= (int)meshes)
        Graphic::instance()->addModelFileInstance(state.selected-(int)meshes);
      close();
    }
  }
  ImGui::End();
}

// --- the selected model ---

//! the node and its children as a tree, with the names of their meshes
void nodeTree(SceneNode* node){
  string label = node->getName().empty() ? "(no name)" : node->getName();
  if (node->getType() == SceneNode::MESH && ((Model*)node)->getMesh())
    label += "  [mesh "+((Model*)node)->getMesh()->getName()+"]";
  ImGui::PushID(node);
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
  if (node->getChildren().empty())
    flags |= ImGuiTreeNodeFlags_Leaf;
  if (ImGui::TreeNodeEx(label.c_str(), flags)){
    for (SceneNode* child : node->getChildren())
      nodeTree(child);
    ImGui::TreePop();
  }
  ImGui::PopID();
}

//! the materials of the meshes at and below node
void collectMaterials(SceneNode* node, vector<Material*>& materials){
  if (node->getType() == SceneNode::MESH && ((Model*)node)->getMesh()){
    for (Material* mat : ((Model*)node)->getMesh()->getMaterials()){
      if (mat && std::find(materials.begin(), materials.end(), mat) == materials.end())
        materials.push_back(mat);
    }
  }
  for (SceneNode* child : node->getChildren())
    collectMaterials(child, materials);
}

bool colorEdit(const char* label, CGE::Color& color, bool alpha){
  float c[4] = {color.r, color.g, color.b, color.a};
  bool changed = alpha ? ImGui::ColorEdit4(label, c) : ImGui::ColorEdit3(label, c);
  if (changed)
    color = CGE::Color(c[0], c[1], c[2], alpha ? c[3] : color.a);
  return changed;
}

//! a material's settings, changed where they are shown
void materialEditor(Material& mat){
  const char* shading = mat.isMetallicRoughness() ? "metallic-roughness" : "Phong";
  ImGui::Text("Shading: %s", shading);
  CGE::Color base = mat.getDiffuse();
  base.a = mat.getOpacity();
  if (colorEdit(mat.isMetallicRoughness() ? "Base colour" : "Diffuse", base, true)){
    mat.setDiffuse(CGE::Color(base.r, base.g, base.b, 1));
    mat.setOpacity(base.a);
  }
  if (mat.isMetallicRoughness()){
    float metallic = mat.getMetallic();
    if (ImGui::SliderFloat("Metallic", &metallic, 0, 1))
      mat.setMetallic(metallic);
    float roughness = mat.getRoughness();
    if (ImGui::SliderFloat("Roughness", &roughness, 0, 1))
      mat.setRoughness(roughness);
    CGE::Color emissive = mat.getEmissive();
    if (colorEdit("Emissive", emissive, false))
      mat.setEmissive(emissive);
    float strength = mat.getEmissiveStrength();
    if (ImGui::DragFloat("Emissive strength", &strength, 0.05f, 0, 100))
      mat.setEmissiveStrength(strength);
    if (mat.getNormalTexture()){
      float scale = mat.getNormalScale();
      if (ImGui::SliderFloat("Normal scale", &scale, 0, 2))
        mat.setNormalScale(scale);
    }
    if (mat.getOcclusionTexture()){
      float occlusion = mat.getOcclusionStrength();
      if (ImGui::SliderFloat("Occlusion", &occlusion, 0, 1))
        mat.setOcclusionStrength(occlusion);
    }
    string textures;
    if (mat.getDiffuseTexture())
      textures += " base colour,";
    if (mat.getMetallicRoughnessTexture())
      textures += " metallic-roughness,";
    if (mat.getNormalTexture())
      textures += " normal,";
    if (mat.getOcclusionTexture())
      textures += " occlusion,";
    if (mat.getEmissiveTexture())
      textures += " emissive,";
    if (textures.empty())
      textures = " none";
    else
      textures.pop_back();
    ImGui::TextWrapped("Textures:%s", textures.c_str());
  }
  else{
    CGE::Color specular = mat.getSpecular();
    if (colorEdit("Specular", specular, false))
      mat.setSpecular(specular);
    float power = mat.getPower();
    if (ImGui::SliderFloat("Shininess", &power, 1, 128))
      mat.setPower(power);
  }
  const char* modes[] = {"default", "opaque", "mask", "blend"};
  ImGui::Text("Alpha: %s%s", modes[(int)mat.getAlphaMode()], mat.isDoubleSided() ? ", double-sided" : "");
}

//! the selected node: its hierarchy, its materials and its glTF file's warnings
void modelWindow(){
  SceneNode* node = Graphic::instance()->getCurrNode();
  if (!node)
    return;
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  const float size = ImGui::GetFontSize();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x+size, viewport->WorkPos.y+size), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(28*size, 30*size), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Model", nullptr, ImGuiWindowFlags_NoSavedSettings)){
    ModelFile* file = Graphic::instance()->getModelFileOf(node);
    ImGui::Text("%s%s", node->getName().empty() ? "(no name)" : node->getName().c_str(), file ? " (glTF)" : "");
    if (ImGui::CollapsingHeader("Nodes", ImGuiTreeNodeFlags_DefaultOpen))
      nodeTree(node);
    vector<Material*> materials;
    collectMaterials(node, materials);
    if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)){
      if (file)
        ImGui::TextDisabled("Changes apply to every copy of the model");
      for (Material* mat : materials){
        ImGui::PushID(mat);
        if (ImGui::TreeNode(mat->getName().empty() ? "(no name)" : mat->getName().c_str())){
          materialEditor(*mat);
          ImGui::TreePop();
        }
        ImGui::PopID();
      }
      if (materials.empty())
        ImGui::TextDisabled("none");
    }
    if (file && !file->getWarnings().empty() && ImGui::CollapsingHeader("Warnings")){
      for (const string& warning : file->getWarnings())
        ImGui::BulletText("%s", warning.c_str());
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
    const CGE::Scene& scene = Graphic::instance()->getScene();
    vector<string> names;
    for (const string& file : scene.getDeviceTextureNames())
      names.push_back(file.substr(file.find_last_of('/')+1));
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
      if (mdl && state.selected >= 0 && state.selected < (int)names.size())
        mdl->assignDeviceTexture(scene.getDeviceTextureAt(state.selected), state.stage);
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

void tdmExport(){
  TDMExport exp;
  std::string file = Graphic::instance()->getFilename();
  unsigned idx = file.find_last_of('.');
  file.erase(idx+1);
  file += "tdm";
  exp.exportTDM(Graphic::instance()->getScene(), file);
}

//! the buttons, in the lower right corner
void toolWindow(){
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  const float margin = ImGui::GetFontSize();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - margin, viewport->WorkPos.y + viewport->WorkSize.y - margin),
    ImGuiCond_FirstUseEver, ImVec2(1, 1));
  if (ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)){
    const ImVec2 size(9*ImGui::GetFontSize(), 0);
    if (ImGui::Button("Export TDM", size))
      tdmExport();
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
    if (ImGui::Button("Add glTF model", size))
      openFile(FileAction::AddModelFile);
    if (ImGui::Button("Add texture", size))
      openFile(FileAction::AddTexture);
    if (ImGui::Button("Options", size))
      openOptions();
    if (ImGui::Button("Frame (F)", size))
      Graphic::instance()->frame(Graphic::instance()->getCurrNode());
    bool lights = Graphic::instance()->hasDefaultLights();
    if (ImGui::Checkbox("Default lights", &lights))
      Graphic::instance()->setDefaultLights(lights);
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Light from the camera while the scene has no lights");
    const string& message = Graphic::instance()->getMessage();
    if (!message.empty()){
      ImGui::PushTextWrapPos(ImGui::GetCursorPosX()+size.x);
      ImGui::TextUnformatted(message.c_str());
      ImGui::PopTextWrapPos();
      if (ImGui::SmallButton("OK"))
        Graphic::instance()->setMessage("");
    }
  }
  ImGui::End();
}

}

void Menu::render(){
  toolWindow();
  modelWindow();
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
