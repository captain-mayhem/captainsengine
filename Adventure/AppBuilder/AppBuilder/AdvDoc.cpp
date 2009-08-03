#include "AdvDoc.h"
#include <wx/zipstrm.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include "GraphNodes.h"
#include "AdvMainTree.h"

const int STATES_MAX = 10;
const int FRAMES_MAX = 25;

IMPLEMENT_DYNAMIC_CLASS(AdvDocument, wxDocument);

AdvDocument::AdvDocument(){
  mView = NULL;
}

AdvDocument::~AdvDocument(){

}

wxOutputStream& AdvDocument::SaveObject(wxOutputStream& stream){
  return stream;
}

wxInputStream& AdvDocument::LoadObject(wxInputStream& stream){
  mView = (AdvMainTreeView*)GetFirstView();
  wxDocument::LoadObject(stream);
  wxZipInputStream zipstream(stream);
  wxZipEntry* entry = zipstream.GetNextEntry();
  if (!loadFile1(zipstream)){
    wxMessageBox("Failed loading project file", "Error");
    return stream;
  }
  delete entry;
  entry = zipstream.GetNextEntry();
  if(!loadFile2(zipstream)){
    wxMessageBox("Failed loading project file", "Error");
    return stream;
  }
  zipstream.CloseEntry();
  wxMessageBox("Project loaded successfully", "Info");
  return stream;
}

bool AdvDocument::loadFile1(wxInputStream& stream){
  wxTextInputStream txtstream(stream);
  wxString str = txtstream.ReadLine();
  if (str != "3.2 Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  wxString unknown = txtstream.ReadLine();
  wxString workdir = txtstream.ReadLine();
  wxString resolution = txtstream.ReadLine();
  wxString font = txtstream.ReadLine();
  while(!stream.Eof()){
    wxString str = txtstream.ReadLine();
    if (str == "Mediapool :"){
      wxTreeCtrl* mediapool = mView->getMediapool();
      wxTreeItemId currNode = mediapool->AddRoot("Mediapool");
      int level = -1;
      str = txtstream.ReadLine();
      while (str != "Gamepool :"){
        if (!str.empty())
          level = insertTreeElement(mediapool, str, &currNode, level);
        str = txtstream.ReadLine();
      }
    }
    if (str == "Gamepool :"){
      //Gamepool
      wxTreeCtrl* gamepool = mView->getGamepool();
      wxTreeItemId currNode = gamepool->AddRoot("Gamepool");
      int level = -1;
      str = txtstream.ReadLine();
      while (str != "Images :"){
        if (!str.empty())
          level = insertTreeElement(gamepool, str, &currNode, level);
        str = txtstream.ReadLine();
      }
    }
    //Images
    if (str == "Images :"){
      str = txtstream.ReadLine();
      while (str != "Sounds :"){
        wxString filename = txtstream.ReadLine();
        mImageNames[str] = wxFileName(filename);
        str = txtstream.ReadLine();
      }
    }
  }
  return true;
}

int AdvDocument::insertTreeElement(wxTreeCtrl* tree, const wxString& name, wxTreeItemId* current, int curr_level){
  int level = 0;
  for (unsigned i = 0; i < name.size(); ++i){
    if (name[i] == '"')
      ++level;
    else
      break;
  }
  wxString idstring = name.substr(level,2);
  ResourceID id = (ResourceID)atoi(idstring.c_str());
  wxString label = name.substr(level+2);
  int leveldiff = level - curr_level;
  while(leveldiff <= 0){
    *current = tree->GetItemParent(*current);
    ++leveldiff;
  }
  *current = tree->AppendItem(*current, label, -1, -1, new AdvTreeItemData(id));
  return level;
}

bool AdvDocument::loadFile2(wxInputStream& stream){
  wxTextInputStream txtstream(stream);
  wxString str = txtstream.ReadLine();
  if (str != "3.2 Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  txtstream.ReadLine();
  for (int state = 0; state < STATES_MAX; ++state){
    CursorState cs;
    for (int frames = 0; frames < FRAMES_MAX; ++frames){
      str = txtstream.ReadLine();
      if (!str.empty()){
        cs.frames.push_back(std::string(str.c_str()));
      }
    }
    str = txtstream.ReadLine();
    long val1;
    str.ToLong(&val1);
    cs.command = val1;
    str = txtstream.ReadLine();
    assert(str == "20");
    long val2;
    str = txtstream.ReadLine();
    str.ToLong(&val1);
    str = txtstream.ReadLine();
    str.ToLong(&val2);
    cs.highlight = std::make_pair(val1,val2);
    mCursor.push_back(cs);
  }
  while(!stream.Eof()){
    str = txtstream.ReadLine();
    wxString rest;
    if (!str.StartsWith("//", &rest))
      return false;
    int splitidx = rest.Find(" ");
    wxString type = rest.SubString(0, splitidx-1);
    wxString name = rest.SubString(splitidx+1, rest.Length());
    break;
  }
  return true;
}

wxFileName AdvDocument::getFilename(ResourceID id, wxString name){
  return mImageNames[name];
}