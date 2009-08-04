#include "AdvDoc.h"
#include <wx/zipstrm.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include "GraphNodes.h"
#include "AdvMainTree.h"

const int STATES_MAX = 10;
const int FRAMES_MAX = 25;
const int FRAMES2_MAX = 30;
const int PARTS_MAX = 2;

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
    delete entry;
    zipstream.CloseEntry();
    wxMessageBox("Failed loading project file", "Error");
    return stream;
  }
  delete entry;
  zipstream.CloseEntry();
  entry = zipstream.GetNextEntry();
  if(!loadFile2(zipstream)){
    delete entry;
    zipstream.CloseEntry();
    wxMessageBox("Failed loading project file", "Error");
    return stream;
  }
  delete entry;
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
    // ITEM
    if (type == "Item"){
      Item it;
      it.name = name;
      for (int state = 0; state < STATES_MAX; ++state){
        Frames frm;
        for (int frames = 0; frames < FRAMES_MAX; ++frames){
          str = txtstream.ReadLine();
          if (str != ";" && str.Length() > 0){
            frm.push_back(std::string(str.SubString(0,str.Length()-2).c_str()));
          }
        }
        str = txtstream.ReadLine();
        assert(str == "20");
        it.frames.push_back(frm);
      }
      mItems.push_back(it);
    }
    // OBJECT
    else if (type == "Object"){
      Object obj;
      obj.name = name;
      long val1, val2;
      str = txtstream.ReadLine();
      str.ToLong(&val1);
      str = txtstream.ReadLine();
      str.ToLong(&val2);
      obj.size = std::make_pair(val1, val2);
      str = txtstream.ReadLine();
      str.ToLong(&val1);
      obj.unknown = val1;
      assert(obj.unknown == 0);
      for (int state = 0; state < STATES_MAX; ++state){
        ExtendedFrames frms;
        for (int frames = 0; frames < FRAMES2_MAX; ++frames){
          ExtendedFrame frm;
          bool set = false;
          for (int parts = 0; parts < PARTS_MAX; ++parts){
            str = txtstream.ReadLine();
            if (str.Length() > 0){
              set = true;
              frm.names.push_back(std::string(str.c_str()));
            }
          }
          //read offsets
          str = txtstream.ReadLine();
          for (unsigned i = 0; i < frm.names.size(); ++i){
            int pos = str.Find(";");
            wxString num = str.SubString(0, pos-1);
            str = str.SubString(pos+1, str.Length());
            num.ToLong(&val1);
            pos = str.Find(";");
            num = str.SubString(0, pos-1);
            str = str.SubString(pos+1, str.Length());
            num.ToLong(&val2);
            frm.offsets.push_back(std::make_pair(val1,val2));
          }
          if (set)
            frms.push_back(frm);
        }
        str = txtstream.ReadLine();
        assert(str == "20");
        obj.frames.push_back(frms);
      }
      mObjects.push_back(obj);
    }
    // CHARACTER
    else if (type == "Character"){

    }
    else{
      wxMessageBox(type, "Unknown type found");
      return false;
    }
  }
  return true;
}

wxFileName AdvDocument::getFilename(ResourceID id, wxString name){
  return mImageNames[name];
}