#include "AdvDoc.h"
#include <wx/zipstrm.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include "GraphNodes.h"
#include "AdvMainTree.h"

const float FPS_MAX = 50.0f;
const int STATES_MAX = 10;
const int CHAR_STATES_MAX = 36;
const int FRAMES_MAX = 25;
const int FRAMES2_MAX = 30;
const int PARTS_MAX = 2;
const int FXSHAPES_MAX = 3;
const int WALKMAP_X = 32*2*2;
const int WALKMAP_Y = 24*2*2;

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
    str.ToLong(&val1);
    cs.fps = FPS_MAX/val1;
    long val2;
    str = txtstream.ReadLine();
    str.ToLong(&val1);
    str = txtstream.ReadLine();
    str.ToLong(&val2);
    cs.highlight = Vec2i(val1,val2);
    mCursor.push_back(cs);
  }
  while(!stream.Eof()){
    str = txtstream.ReadLine();
    if (stream.Eof())
      return true;
    wxString rest;
    if (!str.StartsWith("//", &rest) && !str.StartsWith(";;", &rest))
      return false;
    int splitidx = rest.Find(" ");
    wxString type = rest.SubString(0, splitidx-1);
    wxString name = rest.SubString(splitidx+1, rest.Length());
    // ITEM
    if (type == "Item"){
      Item it;
      it.name = name;
      for (int state = 0; state < STATES_MAX; ++state){
        ItemState is;
        for (int frames = 0; frames < FRAMES_MAX; ++frames){
          str = txtstream.ReadLine();
          if (str != ";" && str.Length() > 0){
            is.frames.push_back(std::string(str.SubString(0,str.Length()-2).c_str()));
          }
        }
        str = txtstream.ReadLine();
        long val1;
        str.ToLong(&val1);
        is.fps = FPS_MAX/val1;
        it.states.push_back(is);
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
      obj.size = Vec2i(val1, val2);
      str = txtstream.ReadLine();
      str.ToLong(&val1);
      obj.lighten = (val1 != 0);
      //assert(obj.lighten == false);
      for (int state = 0; state < STATES_MAX; ++state){
        ObjectState os;
        os.fps = readExtendedFrames(txtstream, os.frames);
        obj.states.push_back(os);
      }
      mObjects.push_back(obj);
    }
    // CHARACTER
    else if (type == "Character"){
      Character ch;
      ch.name = name;
      long val1, val2;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.textcolor = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.walkspeed = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.notzoom = (val1 != 0);
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.realleft = (val1 != 0);
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.memresistent = (val1 != 0);
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.ghost = (val1 != 0);
      ch.walksound = txtstream.ReadLine();
      str = txtstream.ReadLine();
      unsigned pos = str.Find(";");
      while (pos < str.Length()){
        std::string state = str.SubString(0, pos-1);
        ch.extrastatenames.push_back(state);
        str = str.SubString(pos+1, str.Length());
        pos = str.Find(";");
      }
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.fontid = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.zoom = val1;
      for (int state = 0; state < CHAR_STATES_MAX; ++state){
        CharacterState cs;
        str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
        cs.size = Vec2i(val1, val2);
        str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
        cs.basepoint = Vec2i(val1, val2);
        cs.fps = readExtendedFrames(txtstream, cs.frames);
        ch.states.push_back(cs);
      }
      mCharacters.push_back(ch);
    }
    // RCHARACTER
    else if (type == "Rcharacter"){
      Rcharacter rc;
      rc.name = name;
      rc.character = txtstream.ReadLine();
      rc.room = txtstream.ReadLine();
      long val1, val2;
      str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
      rc.position = Vec2i(val1,val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); rc.dir = (LookDir)val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); rc.unknown = val1;
      assert(rc.unknown == -1);
      str = txtstream.ReadLine(); str.ToLong(&val1); rc.unmovable = (val1 != 0);
      mRoomCharacters.push_back(rc);
    }
    // ROOM
    else if (type == "Room"){
      Room room;
      room.name = name;
      long val1, val2;
      str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
      room.size = Vec2i(val1,val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
      room.parallaxpoint = Vec2i(val1, val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
      room.depthmap = Vec2i(val1, val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); room.zoom = val1;
      room.background = txtstream.ReadLine();
      room.parallaxbackground = txtstream.ReadLine();
      str = txtstream.ReadLine(); str.ToLong(&val1); room.doublewalkmap = (val1 != 0);
      for (int i = 0; i < FXSHAPES_MAX; ++i){
        //TODO
        txtstream.ReadLine();
        txtstream.ReadLine();
        txtstream.ReadLine();
        txtstream.ReadLine();
        txtstream.ReadLine();
        txtstream.ReadLine();
        txtstream.ReadLine();
        txtstream.ReadLine();
      }
      //TODO inventory
      txtstream.ReadLine();
      txtstream.ReadLine();
      str = txtstream.ReadLine();
      //int gridsize = 640/32;
      //int xsize = room.size.x/gridsize;
      //int ysize = room.size.y/gridsize;
      room.walkmap.resize(WALKMAP_X);
      for(int i = 0; i < WALKMAP_X; ++i){
        room.walkmap[i].resize(WALKMAP_Y);
      }
      for (int i = 0; i < WALKMAP_X*WALKMAP_Y; ++i){
        wxChar ch = str[2*i];
        wxChar ch2 = str[2*i+1];
        bool walkable = true;
        bool script = false;
        if (ch == '1')
          walkable = false;
        if (ch2 == '1')
          script = true;
        int x = i/WALKMAP_Y;
        int y = i%WALKMAP_Y;
        room.walkmap[x][y].walkable = walkable;
        room.walkmap[x][y].script = script;
      }
      mRooms.push_back(room);
    }
    else if (type == "Roomobject"){
      Roomobject ro;
      ro.name = name;
      ro.object = txtstream.ReadLine();
      long val1, val2;
      str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
      ro.position = Vec2i(val1,val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); ro.state = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ro.layer = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ro.wm_depth = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ro.locked = (val1 != 0);
      mRooms.back().objects.push_back(ro);
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

float AdvDocument::readExtendedFrames(wxTextInputStream& txtstream, ExtendedFrames& frms){
  wxString str;
  long val1, val2;
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
      frm.offsets.push_back(Vec2i(val1,val2));
    }
    if (set)
      frms.push_back(frm);
  }
  str = txtstream.ReadLine();
  str.ToLong(&val1);
  float fps = FPS_MAX/val1;
  return fps;
}