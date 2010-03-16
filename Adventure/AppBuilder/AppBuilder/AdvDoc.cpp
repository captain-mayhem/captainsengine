#include "AdvDoc.h"
#include <iomanip>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/image.h>
#include <wx/filesys.h>
#include "GraphNodes.h"
#include "AdvMainTree.h"
#include "Sound.h"

const float FPS_MAX = 50.0f;
const int STATES_MAX = 10;
const int CHAR_STATES_MAX = 36;
const int FRAMES_MAX = 25;
const int FRAMES2_MAX = 30;
const int PARTS_MAX = 2;
const int FXSHAPES_MAX = 3;

IMPLEMENT_DYNAMIC_CLASS(AdvDocument, wxDocument);

AdvDocument::AdvDocument() : mStream(NULL){
  mView = NULL;
}

AdvDocument::~AdvDocument(){
  delete mStream;
}

wxOutputStream& AdvDocument::SaveObject(wxOutputStream& stream){
  return stream;
}

wxInputStream& AdvDocument::LoadObject(wxInputStream& stream){
  mView = (AdvMainTreeView*)GetFirstView();
  //wxDocument::LoadObject(stream);
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
  entry = zipstream.GetNextEntry();
  if(!loadFile3(zipstream)){
    delete entry;
    zipstream.CloseEntry();
    wxMessageBox("Failed loading project file", "Error");
    return stream;
  }
  delete entry;
  zipstream.CloseEntry();
  //if (GetFilename().EndsWith("dat")){
    wxFileName name(GetFilename());
    wxString path = name.GetPath();
    mStream = new wxFileSystem();
    mStream->ChangePathTo("file:"+path, true);
    //loadFonts();
    mSettings.savedir = path+"/../saves";
  //}
  //wxMessageBox("Project loaded successfully", "Info");
  return stream;
}

bool AdvDocument::loadFile1(wxInputStream& stream){
  wxTextInputStream txtstream(stream);
  wxString str = txtstream.ReadLine();
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  str = txtstream.ReadLine();
  mSettings.dir = txtstream.ReadLine();
  str = txtstream.ReadLine();
  if (str == "Resolution X : 640"){
    mSettings.resolution = Vec2i(640,480);
  }
  else
    assert(false);
  wxString font;
  do{
    font = txtstream.ReadLine();
  } while (font.substr(0, 11) != "GameFont : ");
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  str = txtstream.ReadLine();
  assert(str.SubString(0,13) == "Startskript : ");
  mSettings.startscript = str.SubString(14,str.Length());
  mSettings.mainscript = txtstream.ReadLine();
  mSettings.anywhere_room = txtstream.ReadLine();
  str = txtstream.ReadLine(); //Screenchange
  mSettings.screenchange = (ScreenChange)atoi(str.substr(15).c_str());
  //TODO
  for (int i = 0; i < 4; ++i){
    str = txtstream.ReadLine();
  }
  str = txtstream.ReadLine();
  mSettings.taskheight = atoi(str.substr(13).c_str());
  str = txtstream.ReadLine();
  mSettings.taskroom = str.substr(11).c_str();
  str = txtstream.ReadLine();
  mSettings.taskpopup = atoi(str.substr(12).c_str());
  str = txtstream.ReadLine();
  std::string taskshow = str.substr(11, 4).c_str();
  if (taskshow == "hide")
    mSettings.taskHideCompletely = true;
  else
    mSettings.taskHideCompletely = false;
  for (int i = 0; i < 4; ++i){
    str = txtstream.ReadLine();
  }
  str = txtstream.ReadLine();
  mSettings.targacolor = atoi(str.substr(13).c_str());
  str = txtstream.ReadLine();
  mSettings.bordercolor = atoi(str.substr(14).c_str());
  str = txtstream.ReadLine();
  mSettings.backgroundcolor = atoi(str.substr(18).c_str());
  str = txtstream.ReadLine();
  mSettings.textcolor = atoi(str.substr(12).c_str());
  str = txtstream.ReadLine(); //offtextcolor
  str = txtstream.ReadLine();
  mSettings.tsstyle = (TsStyle)(atoi(str.c_str())-1);
  str = txtstream.ReadLine();
  mSettings.tsborderstyle = atoi(str.c_str());
  str = txtstream.ReadLine();
  mSettings.tsbordercolor = atoi(str.c_str());
  str = txtstream.ReadLine();
  mSettings.tsareacolor = atoi(str.c_str());
  str = txtstream.ReadLine();
  mSettings.tsselectioncolor = atoi(str.c_str());
  str = txtstream.ReadLine();
  mSettings.tstextcolor = atoi(str.c_str());
  if (ver_major >= 3){
    for (int i = 0; i < 8; ++i){
      str = txtstream.ReadLine();
    }
  }
  else{
    //mSettings.tsselectioncolor = 15793151;
  }
  str = txtstream.ReadLine();
  mSettings.linktext = str.SubString(11, str.Length());
  str = txtstream.ReadLine();
  mSettings.givelink = str.SubString(11, str.Length());
  str = txtstream.ReadLine();
  mSettings.walktext = str.SubString(11, str.Length());
  while(!stream.Eof()){
    str = txtstream.ReadLine();
    if (str == "Booleans :"){
      str = txtstream.ReadLine();
      while (str != "Commands :"){
        wxString name = str.substr(0, str.size()-2);
        wxString val = str.substr(str.size()-1);
        mSettings.booleans[std::string(name)] = (val == "1");
        str = txtstream.ReadLine();
      }
    }
    if (str == "Commands :"){
      mSettings.pretty_commands.push_back(mSettings.walktext);
      mSettings.commands["walkto"] = (unsigned)mSettings.pretty_commands.size()-1;
      mSettings.pretty_commands.push_back("Loading");
      mSettings.commands["loading"] = (unsigned)mSettings.pretty_commands.size()-1;
      str = txtstream.ReadLine();
      while (str != "Mediapool :"){
        wxString pretty_name = txtstream.ReadLine();
        mSettings.pretty_commands.push_back((const char*)pretty_name.c_str());
        mSettings.commands[(const char*)str.c_str()] = (unsigned)mSettings.pretty_commands.size()-1;
        str = txtstream.ReadLine();
      }
    }
    if (str == "Mediapool :"){
      if (mView){
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
    }
    if (str == "Gamepool :"){
      if (mView){
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
    }
    //Images
    if (str == "Images :"){
      str = txtstream.ReadLine();
      while (str != "Sounds :"){
        wxString filename = txtstream.ReadLine();
        mImageNames[str] = wxFileName(filename, wxPATH_WIN);
        str = txtstream.ReadLine();
      }
    }
    //Sounds
    if (str == "Sounds :"){
      str = txtstream.ReadLine();
      while (str != "Music :"){
        wxString filename = txtstream.ReadLine();
        mSoundNames[str] = wxFileName(filename, wxPATH_WIN);
        str = txtstream.ReadLine();
      }
    }
    //Music
    if (str == "Music :"){
      str = txtstream.ReadLine();
      while (str != "Movie :"){
        wxString filename = txtstream.ReadLine();
        mMusicNames[str] = wxFileName(filename, wxPATH_WIN);
        str = txtstream.ReadLine();
      }
    }
    //Movie
    if (str == "Movie :"){
      str = txtstream.ReadLine();
      while (!str.empty()){
        wxString filename = txtstream.ReadLine();
        mMovieNames[str] = wxFileName(filename, wxPATH_WIN);
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
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
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
      int numStates = STATES_MAX;
      int delim = 2;
      if (ver_major == 2){
        numStates = 1;
        delim = 1;
      }
      for (int state = 0; state < numStates; ++state){
        ItemState is;
        for (int frames = 0; frames < FRAMES_MAX; ++frames){
          str = txtstream.ReadLine();
          if (str != ";" && str.Length() > 0){
            is.frames.push_back(std::string(str.SubString(0,str.Length()-delim).c_str()));
          }
        }
        str = txtstream.ReadLine();
        long val1;
        str.ToLong(&val1);
        is.fps = FPS_MAX/val1;
        it.states.push_back(is);
      }
      mItems[it.name] = it;
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
      for (int state = 0; state < STATES_MAX; ++state){
        ObjectState os;
        os.fps = readExtendedFrames(txtstream, os.frames);
        obj.states.push_back(os);
      }
      mObjects[obj.name] = obj;
    }
    // CHARACTER
    else if (type == "Character"){
      Character ch;
      ch.name = name;
      long val1, val2;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.textcolor = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.walkspeed = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.notzoom = (val1 != 0);
      if (ver_major >= 3)
        str = txtstream.ReadLine(); str.ToLong(&val1); ch.realleft = (val1 != 0);
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.memresistent = (val1 != 0);
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.ghost = (val1 != 0);
      ch.walksound = txtstream.ReadLine();
      if (ver_major >= 3){
        str = txtstream.ReadLine();
        unsigned pos = str.Find(";");
        while (pos < str.Length()){
          std::string state = std::string(str.SubString(0, pos-1));
          ch.extrastatenames.push_back(state);
          str = str.SubString(pos+1, str.Length());
          pos = str.Find(";");
        }
      }
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.fontid = val1;
      str = txtstream.ReadLine(); str.ToLong(&val1); ch.zoom = val1;
      //int states = ver_major >= 3 ? CHAR_STATES_MAX : CHAR_STATES_MAX-20;
      for (int state = 0; state < CHAR_STATES_MAX; ++state){
        CharacterState cs;
        str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
        cs.size = Vec2i(val1, val2);
        str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
        cs.basepoint = Vec2i(val1, val2);
        cs.fps = readExtendedFrames(txtstream, cs.frames);
        ch.states.push_back(cs);
      }
      mCharacters[ch.name] = ch;
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
      str = txtstream.ReadLine(); str.ToLong(&val1); rc.dir = (LookDir)(val1-1);
      str = txtstream.ReadLine(); str.ToLong(&val1); rc.unmovable = (val1 != 0);
      str = txtstream.ReadLine(); str.ToLong(&val1); rc.locked = (val1 != 0);
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
      room.scrolloffset = Vec2i(val1, val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); str = txtstream.ReadLine(); str.ToLong(&val2);
      room.depthmap = Vec2i(val1, val2);
      str = txtstream.ReadLine(); str.ToLong(&val1); room.zoom = val1;
      room.background = txtstream.ReadLine();
      room.parallaxbackground = txtstream.ReadLine();
      str = txtstream.ReadLine(); str.ToLong(&val1); room.doublewalkmap = (val1 != 0);
      if (ver_major >= 3){
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
        //TODO unknown
        txtstream.ReadLine();
      }
      //inventory
      str = txtstream.ReadLine();
      unsigned from = 0;
      unsigned to = (unsigned)str.find(';', from);
      room.invpos.x = atoi(str.substr(from, to-from).c_str());
      from = to+1; to = (unsigned)str.find(';', from);
      room.invpos.y = atoi(str.substr(from, to-from).c_str());
      from = to+1; to = (unsigned)str.find(';', from);
      room.invsize.x = atoi(str.substr(from, to-from).c_str());
      from = to+1; to = (unsigned)str.find(';', from);
      room.invsize.y = atoi(str.substr(from, to-from).c_str());
      from = to+1; to = (unsigned)str.find(';', from);
      wxString tmp = str.substr(from, to-from);
      size_t idx = tmp.find(',');
      if (idx != wxString::npos)
        tmp[idx] = '.';
      room.invscale.x = atof(tmp.c_str());
      from = to+1; to = (unsigned)str.find(';', from);
      tmp = str.substr(from, to-from);
      idx = tmp.find(',');
      if (idx != wxString::npos)
        tmp[idx] = '.';
      room.invscale.y = atof(tmp.c_str());
      //walkmap
      str = txtstream.ReadLine();
      int WALKMAP_X = 32;
      int walkGridSize = mSettings.resolution.x/WALKMAP_X;
      int WALKMAP_Y = mSettings.resolution.y/walkGridSize;
      WALKMAP_X *= 3;
      WALKMAP_Y *= 2;
      if (ver_major >= 3){
        //double walkmap
        WALKMAP_X *= 2;
        WALKMAP_Y *= 2;
      }
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
      mRooms[room.name] = room;
      mLastRoom = &mRooms[room.name];
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
      mLastRoom->objects.push_back(ro);
    }
    else{
      wxMessageBox(type, "Unknown type found");
      return false;
    }
  }
  return true;
}

bool AdvDocument::loadFile3(wxInputStream& stream){
  wxTextInputStream txtstream(stream);
  wxString str = txtstream.ReadLine();
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  txtstream.ReadLine();
  mLastScript = NULL;
  while(!stream.Eof()){
    str = txtstream.ReadLine();
    if (stream.Eof())
      return true;
    wxString rest;
    if (str.StartsWith("//", &rest)){
      int splitidx = rest.Find(" ");
      wxString type = rest.SubString(0, splitidx-1);
      wxString name = rest.SubString(splitidx+1, rest.Length());
      Script::Type scrType;
      if (type == "Cutscene")
        scrType = Script::CUTSCENE;
      else if (type == "Item")
        scrType = Script::ITEM;
      else if (type == "Character")
        scrType = Script::CHARACTER;
      else if (type == "Room")
        scrType = Script::ROOM;
      else if (type == "Object")
        scrType = Script::OBJECT;
      else if (type == "Walkmap")
        scrType = Script::WALKMAP;
      else{
        wxMessageBox("Unknown script type: "+type, "Error");
      }
      Script scr;
      scr.name = name;
      scr.type = scrType;
      if (scrType == Script::WALKMAP){
        std::string roomname;
        Vec2i pos;
        if (ver_major >= 3){
          roomname = std::string(name.substr(7).c_str());
          pos.x = atoi(name.substr(1,3).c_str());
          pos.y = atoi(name.substr(4,3).c_str());
        }
        else{
          roomname = std::string(name.substr(4).c_str());
          pos.x = atoi(name.substr(0,2).c_str());
          pos.y = atoi(name.substr(2,2).c_str());
        }
        mWMScripts[roomname].push_back(std::make_pair(pos,scr));
        mLastScript = &mWMScripts[roomname].back().second;
      }
      else{
        mScripts[std::make_pair(scr.type, scr.name)] = scr;
        mLastScript = &mScripts[std::make_pair(scr.type, scr.name)];
      }
    }
    else{
      mLastScript->text += str + "\n";
    }
  }
  return true;
}

//wxFileName AdvDocument::getFilename(ResourceID id, wxString name){
//  return mImageNames[name];
//}
wxImage AdvDocument::getImage(wxString name){
  wxFileName filename;
  std::map<wxString,wxFileName>::iterator iter = mImageNames.find(name);
  if (iter != mImageNames.end())
    filename = iter->second;
  else{
    for (iter = mImageNames.begin(); iter != mImageNames.end(); ++iter){
      if (stricmp(name.c_str(), iter->first.c_str()) == 0){
        filename = iter->second;
        break;
      }
    }
  }
  if (mStream){
    wxString path = "gfx.dat#zip:"+filename.GetFullName();
    wxFSFile* file = mStream->OpenFile(path, wxFS_READ | wxFS_SEEKABLE);
    wxBitmapType type = wxBITMAP_TYPE_ANY;
    if (filename.GetExt() == "pnj")
      type = wxBITMAP_TYPE_JPEG;
    wxImage image(*file->GetStream(), type);
    delete file;
    if (filename.GetExt() == "pnj"){
      path[path.length()-1] = 'a';
      wxFSFile* alphafile = mStream->OpenFile(path);
      wxImage alphaimage(*alphafile->GetStream(), wxBITMAP_TYPE_JPEG);
      delete alphafile;
      int size = alphaimage.GetWidth()*alphaimage.GetHeight();
      unsigned char* alphadata = (unsigned char*)malloc(size);
      for (int i = 0; i < size; ++i){
        alphadata[i] = alphaimage.GetData()[3*i];
      }
      image.SetAlpha(alphadata);
    }
    return image;
  }
  return wxImage(filename.GetFullPath());
}

bool AdvDocument::getSound(const std::string& name, DataBuffer& db){
  wxFileName filename = mSoundNames[name];
  db.name = filename.GetFullName();
  if (mStream){
    wxString path = "sfx.dat#zip:"+filename.GetFullName();
    wxFSFile* file = mStream->OpenFile(path, wxFS_READ | wxFS_SEEKABLE);
    if (file == NULL)
      return false;
    db.length = file->GetStream()->GetSize();
    db.data = new char[db.length];
    file->GetStream()->Read(db.data, db.length);
    delete file;
  }
  else{
    wxFileInputStream strm(filename.GetFullPath());
    db.length = strm.GetSize();
    db.data = new char[db.length];
    strm.Read(db.data, db.length);
  }
  return true;
}

bool AdvDocument::getMusic(const std::string& name, DataBuffer& db){
  wxFileName filename = mMusicNames[name];
  db.name = filename.GetFullName();
  if (mStream){
    wxString path = "music.dat#zip:"+filename.GetFullName();
    wxFSFile* file = mStream->OpenFile(path, wxFS_READ | wxFS_SEEKABLE);
    if (file == NULL)
      return false;
    db.length = file->GetStream()->GetSize();
    db.data = new char[db.length];
    file->GetStream()->Read(db.data, db.length);
    delete file;
  }
  else{
    wxFileInputStream strm(filename.GetFullPath());
    db.length = strm.GetSize();
    db.data = new char[db.length];
    strm.Read(db.data, db.length);
  }
  return true;
}

bool AdvDocument::getMovie(const std::string& name, DataBuffer& db){
  wxFileName filename = mMovieNames[name];
  db.name = filename.GetFullName();
  if (mStream){
    wxString path = "movie.dat#zip:"+filename.GetFullName();
    wxFSFile* file = mStream->OpenFile(path, wxFS_READ | wxFS_SEEKABLE);
    if (file == NULL)
      return false;
    db.length = file->GetStream()->GetSize();
    db.data = new char[db.length];
    file->GetStream()->Read(db.data, db.length);
    delete file;
  }
  else{
    wxFileInputStream strm(filename.GetFullPath());
    db.length = strm.GetSize();
    db.data = new char[db.length];
    strm.Read(db.data, db.length);
  }
  return true;
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

Room* AdvDocument::getRoom(std::string name){
  std::map<std::string,Room>::iterator iter = mRooms.find(name);
  if (iter == mRooms.end()){
    for (std::map<std::string,Room>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
      if (stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

Object* AdvDocument::getObject(std::string name){
  std::map<std::string,Object>::iterator iter = mObjects.find(name);
  if (iter == mObjects.end())
    return NULL;
  return &((*iter).second);
}

MouseCursor* AdvDocument::getCursor(){
  return &mCursor;
}

ProjectSettings* AdvDocument::getProjectSettings(){
  return &mSettings;
}

Character* AdvDocument::getCharacter(std::string name){
  std::map<std::string,Character>::iterator iter = mCharacters.find(name);
  if (iter == mCharacters.end())
    return NULL;
  return &((*iter).second);
}

Script* AdvDocument::getScript(Script::Type t, std::string name){
  std::map<std::pair<Script::Type,std::string>, Script>::iterator iter = mScripts.find(std::make_pair(t, name));
  if (iter == mScripts.end())
    return NULL;
  return &((*iter).second);
}

std::vector<std::pair<Vec2i,Script*> > AdvDocument::getWMScripts(std::string roomname){
  std::vector<std::pair<Vec2i,Script*> > result;
  std::map<std::string, std::vector<std::pair<Vec2i,Script> > >::iterator iter = mWMScripts.find(roomname);
  if (iter == mWMScripts.end())
    return result;
  for (unsigned i = 0; i < iter->second.size(); ++i){
    result.push_back(std::make_pair(iter->second[i].first, &iter->second[i].second));
  }
  return result;
}

FontData AdvDocument::getFont(int num){
  wxString number;
  if (num < 100)
    number << "0";
  if (num < 10)
    number << "0";
  number << num;
  wxString path;
  if (num == 0)
    path = "font.dat#zip:fontdata.sta";
  else{
    path << "fonts.dat#zip:font." << number << "#zip:fontdata." << number;
  }
  FontData fnt;
  wxFSFile* file = mStream->OpenFile(path);
  if (file == NULL)
    return fnt;
  wxTextInputStream in(*file->GetStream());
  long val;
  wxString str = in.ReadLine(); str.ToLong(&val); fnt.images.resize(2*val);
  str = in.ReadLine(); str.ToLong(&val); fnt.fontsize.x = val;
  str = in.ReadLine(); str.ToLong(&val); fnt.fontsize.y = val;
  str = in.ReadLine(); str.ToLong(&val); fnt.numChars.x = val;
  str = in.ReadLine(); str.ToLong(&val); fnt.numChars.y = val;
  fnt.charwidths.reserve(224);
  while(!file->GetStream()->Eof()){
    str = in.ReadLine(); str.ToLong(&val); fnt.charwidths.push_back(val);
  }
  delete file;
  for (unsigned i = 0; i < fnt.images.size()/2; ++i){
    path.Clear();
    if (num == 0)
      path << "font.dat#zip:fontsta.al" << (i+1);
    else
      path << "fonts.dat#zip:font." << number << "#zip:font" << num << ".al" << (i+1);
    wxFSFile* file = mStream->OpenFile(path);
    if (file == NULL)
      continue;
    fnt.images[2*i] = wxImage(*file->GetStream(), wxBITMAP_TYPE_BMP);
    delete file;
    path.Clear();
    if (num == 0)
      path << "font.dat#zip:fontsta.bm" << (i+1);
    else
      path << "fonts.dat#zip:font." << number << "#zip:font" << num << ".bm" << (i+1);
    file = mStream->OpenFile(path);
    fnt.images[2*i+1] = wxImage(*file->GetStream(), wxBITMAP_TYPE_BMP);
    delete file;
  }
  //mFonts.push_back(fnt);
  return fnt;
}

Item* AdvDocument::getItem(const std::string& name){
  std::map<std::string,Item>::iterator iter = mItems.find(name);
  if (iter == mItems.end()){
    for (std::map<std::string,Item>::iterator iter = mItems.begin(); iter != mItems.end(); ++iter){
      if (stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

