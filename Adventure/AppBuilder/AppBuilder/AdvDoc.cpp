#include "AdvDoc.h"
#include <iomanip>
#include <sstream>
#include <system/utilities.h>
#include <system/engine.h>
#include <image/loader.h>
#include <io/BinFileReader.h>
#include "Sound.h"

#ifndef WIN32
int _stricmp(const char* str1, const char* str2);
#endif

const float FPS_MAX = 50.0f;
const int STATES_MAX = 10;
const int CHAR_STATES_MAX = 36;
const int FRAMES_MAX = 25;
const int FRAMES2_MAX = 30;
const int PARTS_MAX = 2;
const int FXSHAPES_MAX = 3;

#include <io/ZipReader.h>

AdvDocument::AdvDocument() : mUseCompressedData(false){
}

AdvDocument::~AdvDocument(){
}

bool AdvDocument::loadDocument(const std::string filename){
  mFilename = filename;
  CGE::ZipReader zrdr(mFilename);
  if (!zrdr.isWorking()){
    CGE::Engine::instance()->messageBox("Cannot find adventure file", "Error");
    return false;
  }
  CGE::MemReader rdr = zrdr.openEntry("game.001");
  if (!loadFile1(rdr)){
    CGE::Engine::instance()->messageBox("Failed loading project file", "Error");
    return false;
  }
  rdr = zrdr.openEntry("game.002");
  if(!loadFile2(rdr)){
    CGE::Engine::instance()->messageBox("Failed loading project file", "Error");
    return false;
  }
  rdr = zrdr.openEntry("game.003");
  if(!loadFile3(rdr)){
    CGE::Engine::instance()->messageBox("Failed loading project file", "Error");
    return false;
  }

  if (mFilename.substr(mFilename.size()-4) == ".dat")
    mUseCompressedData = true;
  
  mPath = mFilename;
  CGE::Utilities::replaceWith(mFilename, '\\', '/');
  int pos = mPath.find_last_of('/');
  mPath.erase(pos);
  mSettings.savedir = mPath+"/../saves";

  return true;
}

bool AdvDocument::loadFile1(CGE::MemReader& txtstream){
  std::string str = txtstream.readLine();
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  str = txtstream.readLine();
  mSettings.dir = txtstream.readLine();
  str = txtstream.readLine();
  if (str == "Resolution X : 640"){
    mSettings.resolution = Vec2i(640,480);
  }
  else
    DebugBreak();
  std::string font;
  do{
    font = txtstream.readLine();
  } while (font.substr(0, 11) != "GameFont : ");
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  mSettings.tsbackground = txtstream.readLine();
  str = txtstream.readLine();
  if(str.substr(0,14) != "Startskript : ")
    DebugBreak();
  mSettings.startscript = str.substr(14);
  mSettings.mainscript = txtstream.readLine();
  mSettings.anywhere_room = txtstream.readLine();
  str = txtstream.readLine(); //Screenchange
  mSettings.screenchange = (ScreenChange)atoi(str.substr(15).c_str());
  //TODO
  for (int i = 0; i < 4; ++i){
    str = txtstream.readLine();
  }
  str = txtstream.readLine();
  mSettings.taskheight = atoi(str.substr(13).c_str());
  str = txtstream.readLine();
  mSettings.taskroom = str.substr(11).c_str();
  str = txtstream.readLine();
  mSettings.taskpopup = atoi(str.substr(12).c_str());
  str = txtstream.readLine();
  std::string taskshow = str.substr(11, 4).c_str();
  if (taskshow == "hide")
    mSettings.taskHideCompletely = true;
  else
    mSettings.taskHideCompletely = false;
  for (int i = 0; i < 4; ++i){
    str = txtstream.readLine();
  }
  str = txtstream.readLine();
  mSettings.targacolor = atoi(str.substr(13).c_str());
  str = txtstream.readLine();
  mSettings.bordercolor = atoi(str.substr(14).c_str());
  str = txtstream.readLine();
  mSettings.backgroundcolor = atoi(str.substr(18).c_str());
  str = txtstream.readLine();
  mSettings.textcolor = atoi(str.substr(12).c_str());
  str = txtstream.readLine(); //offtextcolor
  str = txtstream.readLine();
  mSettings.tsstyle = (TsStyle)(atoi(str.c_str())-1);
  str = txtstream.readLine();
  mSettings.tsborderstyle = atoi(str.c_str());
  str = txtstream.readLine();
  mSettings.tsbordercolor = atoi(str.c_str());
  str = txtstream.readLine();
  mSettings.tsareacolor = atoi(str.c_str());
  str = txtstream.readLine();
  mSettings.tsselectioncolor = atoi(str.c_str());
  str = txtstream.readLine();
  mSettings.tstextcolor = atoi(str.c_str());
  if (ver_major >= 3 || (ver_major >=2 && ver_minor >= 7)){
    for (int i = 0; i < 8; ++i){
      str = txtstream.readLine();
    }
  }
  else{
    //mSettings.tsselectioncolor = 15793151;
  }
  str = txtstream.readLine();
  mSettings.linktext = str.substr(11);
  str = txtstream.readLine();
  mSettings.givelink = str.substr(11);
  str = txtstream.readLine();
  mSettings.walktext = str.substr(11);
  while(txtstream.isWorking()){
    str = txtstream.readLine();
    if (str == "Booleans :"){
      str = txtstream.readLine();
      while (str != "Commands :"){
        std::string name;
        std::string val = str.substr(str.size()-1);
        if (val == "1")
          name = str.substr(0, str.size()-2);
        else
          name = str.substr(0, str.size()-1);
        mSettings.booleans[std::string(name)] = (val == "1");
        str = txtstream.readLine();
      }
    }
    if (str == "Commands :"){
      mSettings.pretty_commands.push_back(mSettings.walktext);
      mSettings.commands["walkto"] = (unsigned)mSettings.pretty_commands.size()-1;
      mSettings.pretty_commands.push_back("Loading");
      mSettings.commands["loading"] = (unsigned)mSettings.pretty_commands.size()-1;
      str = txtstream.readLine();
      while (str != "Mediapool :"){
        std::string pretty_name = txtstream.readLine();
        mSettings.pretty_commands.push_back((const char*)pretty_name.c_str());
        mSettings.commands[(const char*)str.c_str()] = (unsigned)mSettings.pretty_commands.size()-1;
        str = txtstream.readLine();
      }
    }
    if (str == "Mediapool :"){
    }
    if (str == "Gamepool :"){
    }
    //Images
    if (str == "Images :"){
      str = txtstream.readLine();
      while (str != "Sounds :"){
        std::string filename = txtstream.readLine();
        CGE::Utilities::replaceWith(filename, '\\', '/');
        mImageNames[str] = filename;
        str = txtstream.readLine();
      }
    }
    //Sounds
    if (str == "Sounds :"){
      str = txtstream.readLine();
      while (str != "Music :"){
        std::string filename = txtstream.readLine();
        CGE::Utilities::replaceWith(filename, '\\', '/');
        mSoundNames[str] = filename;
        str = txtstream.readLine();
      }
    }
    //Music
    if (str == "Music :"){
      str = txtstream.readLine();
      while (str != "Movie :"){
        std::string filename = txtstream.readLine();
        CGE::Utilities::replaceWith(filename, '\\', '/');
        mMusicNames[str] = filename;
        str = txtstream.readLine();
      }
    }
    //Movie
    if (str == "Movie :"){
      str = txtstream.readLine();
      while (!str.empty()){
        std::string filename = txtstream.readLine();
        CGE::Utilities::replaceWith(filename, '\\', '/');
        mMovieNames[str] = filename;
        str = txtstream.readLine();
      }
    }
  }
  return true;
}

bool AdvDocument::loadFile2(CGE::MemReader& txtstream){
  std::string str = txtstream.readLine();
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  txtstream.readLine();
  for (int state = 0; state < STATES_MAX; ++state){
    CursorState cs;
    for (int frames = 0; frames < FRAMES_MAX; ++frames){
      str = txtstream.readLine();
      if (!str.empty()){
        cs.frames.push_back(std::string(str.c_str()));
      }
    }
    str = txtstream.readLine();
    int val1;
    val1 = atoi(str.c_str());
    cs.command = val1;
    str = txtstream.readLine();
    val1 = atoi(str.c_str());
    cs.fps = FPS_MAX/val1;
    long val2;
    str = txtstream.readLine();
    val1 = atoi(str.c_str());
    str = txtstream.readLine();
    val2 = atoi(str.c_str());
    cs.highlight = Vec2i(val1,val2);
    mCursor.push_back(cs);
  }
  while(txtstream.isWorking()){
    str = txtstream.readLine();
    if (!txtstream.isWorking())
      return true;
    std::string rest = str.substr(2);
    if (!(str[0] == '/' && str[1] == '/') && !(str[0] == ';' && str[1] == ';'))
      return false;
    int splitidx = rest.find(" ");
    std::string type = rest.substr(0, splitidx);
    std::string name = rest.substr(splitidx+1);
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
          str = txtstream.readLine();
          if (str != ";" && str.length() > 0){
            is.frames.push_back(std::string(str.substr(0,str.length()+1-delim).c_str()));
          }
        }
        str = txtstream.readLine();
        long val1;
        val1 = atoi(str.c_str());
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
      str = txtstream.readLine();
      val1 = atoi(str.c_str());
      str = txtstream.readLine();
      val2 = atoi(str.c_str());
      obj.size = Vec2i(val1, val2);
      str = txtstream.readLine();
      val1 = atoi(str.c_str());
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
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.textcolor = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.walkspeed = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.notzoom = (val1 != 0);
      if (ver_major >= 3)
        str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.realleft = (val1 != 0);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.memresistent = (val1 != 0);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.ghost = (val1 != 0);
      ch.walksound = txtstream.readLine();
      if (ver_major >= 3){
        str = txtstream.readLine();
        unsigned pos = str.find(";");
        while (pos < str.length()){
          std::string state = std::string(str.substr(0, pos));
          ch.extrastatenames.push_back(state);
          str = str.substr(pos+1);
          pos = str.find(";");
        }
      }
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.fontid = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.zoom = val1;
      //int states = ver_major >= 3 ? CHAR_STATES_MAX : CHAR_STATES_MAX-20;
      for (int state = 0; state < CHAR_STATES_MAX; ++state){
        CharacterState cs;
        str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
        cs.size = Vec2i(val1, val2);
        str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
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
      rc.character = txtstream.readLine();
      rc.room = txtstream.readLine();
      long val1, val2;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      rc.position = Vec2i(val1,val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); rc.dir = (LookDir)(val1-1);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); rc.unmovable = (val1 != 0);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); rc.locked = (val1 != 0);
      mRoomCharacters.push_back(rc);
    }
    // ROOM
    else if (type == "Room"){
      Room room;
      room.name = name;
      long val1, val2;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      room.size = Vec2i(val1,val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      room.scrolloffset = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      room.depthmap = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); room.zoom = val1;
      room.background = txtstream.readLine();
      room.parallaxbackground = txtstream.readLine();
      str = txtstream.readLine(); val1 = atoi(str.c_str()); room.doublewalkmap = (val1 != 0);
      if (ver_major >= 3){
        for (int i = 0; i < FXSHAPES_MAX; ++i){
          //TODO
          txtstream.readLine();
          txtstream.readLine();
          txtstream.readLine();
          txtstream.readLine();
          txtstream.readLine();
          txtstream.readLine();
          txtstream.readLine();
          txtstream.readLine();
        }
        //TODO unknown
        txtstream.readLine();
      }
      //inventory
      str = txtstream.readLine();
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
      std::string tmp = str.substr(from, to-from);
      size_t idx = tmp.find(',');
      if (idx != std::string::npos)
        tmp[idx] = '.';
      room.invscale.x = (float)atof(tmp.c_str());
      from = to+1; to = (unsigned)str.find(';', from);
      tmp = str.substr(from, to-from);
      idx = tmp.find(',');
      if (idx != std::string::npos)
        tmp[idx] = '.';
      room.invscale.y = (float)atof(tmp.c_str());
      //walkmap
      str = txtstream.readLine();
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
        char ch = str[2*i];
        char ch2 = str[2*i+1];
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
      ro.object = txtstream.readLine();
      long val1, val2;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      ro.position = Vec2i(val1,val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.state = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.layer = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.wm_depth = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.locked = (val1 != 0);
      mLastRoom->objects.push_back(ro);
    }
    else{
      CGE::Engine::instance()->messageBox(type, "Unknown type found");
      return false;
    }
  }
  return true;
}

bool AdvDocument::loadFile3(CGE::MemReader& txtstream){
  std::string str = txtstream.readLine();
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  txtstream.readLine();
  mLastScript = NULL;
  while(txtstream.isWorking()){
    str = txtstream.readLine();
    //if (!txtstream.isWorking())
    //  return true;
    if (str.size() >= 2 && str[0] == '/' && str[1] == '/'){
      std::string rest = str.substr(2);
      int splitidx = rest.find(" ");
      std::string type = rest.substr(0, splitidx);
      std::string name = rest.substr(splitidx+1);
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
        CGE::Engine::instance()->messageBox("Unknown script type: "+type, "Error");
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

CGE::Image* AdvDocument::getImage(const std::string& name){
  std::string filename;
  std::map<std::string,std::string>::iterator iter = mImageNames.find(name);
  if (iter != mImageNames.end())
    filename = iter->second;
  else{
    for (iter = mImageNames.begin(); iter != mImageNames.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.c_str()) == 0){
        filename = iter->second;
        break;
      }
    }
  }
  if (mUseCompressedData){
    int namepos = filename.find_last_of('/');
    static CGE::ZipReader zrdr(mPath+"/gfx.dat");
    std::string imagename = filename.substr(namepos+1);
    CGE::MemReader rdr = zrdr.openEntry(imagename);
    if (!rdr.isWorking())
      return NULL;
    int extpos = filename.find_last_of('.');
    CGE::Image* img = NULL;
    if (filename.substr(extpos+1) == "pnj"){
      CGE::Image* rgbimage = CGE::ImageLoader::load(rdr.getData(), rdr.getSize(), CGE::ImageLoader::JPG);
      filename[filename.length()-1] = 'a';
      rdr = zrdr.openEntry(filename.substr(namepos+1));
      CGE::Image* alphaimage = CGE::ImageLoader::load(rdr.getData(), rdr.getSize(), CGE::ImageLoader::JPG);
      img = new CGE::Image(rgbimage->getNumChannels(), rgbimage->getWidth(), rgbimage->getHeight(), rgbimage->getData(), alphaimage->getNumChannels(), alphaimage->getData());
      delete rgbimage;
      delete alphaimage;
    }
    else{
      CGE::ImageLoader::Type imgtype = CGE::ImageLoader::determineType(imagename);
      img = CGE::ImageLoader::load(rdr.getData(), rdr.getSize(), imgtype);
    }
    return img;
  }
  CGE::Image* img = CGE::ImageLoader::load(filename.c_str());
  return img;
}

bool AdvDocument::getSound(const std::string& name, DataBuffer& db){
  std::string filename = mSoundNames[name];
  int pos = filename.find_last_of('/');
  db.name = filename.substr(pos+1);
  if (mUseCompressedData){
    static CGE::ZipReader zrdr(mPath+"/sfx.dat");
    CGE::MemReader rdr = zrdr.openEntry(filename.substr(pos+1));
    if (!rdr.isWorking())
      return false;
    db.length = rdr.getSize();
    db.data = new char[db.length];
    memcpy(db.data, rdr.getData(), db.length);
  }
  else{
    CGE::BinFileReader strm(filename);
    db.length = strm.getSize();
    db.data = new char[db.length];
    strm.readBytes((unsigned char*)db.data, db.length);
  }
  return true;
}

bool AdvDocument::getMusic(const std::string& name, DataBuffer& db){
  std::string filename = mMusicNames[name];
  int pos = filename.find_last_of('/');
  db.name = filename.substr(pos+1);
  if (mUseCompressedData){
    static CGE::ZipReader zrdr(mPath+"/music.dat");
    CGE::MemReader rdr = zrdr.openEntry(filename.substr(pos+1));
    if (!rdr.isWorking())
      return false;
    db.length = rdr.getSize();
    db.data = new char[db.length];
    memcpy(db.data, rdr.getData(), db.length);
  }
  else{
    CGE::BinFileReader strm(filename);
    db.length = strm.getSize();
    db.data = new char[db.length];
    strm.readBytes((unsigned char*)db.data, db.length);
  }
  return true;
}

bool AdvDocument::getMovie(const std::string& name, DataBuffer& db){
  std::string filename = mMovieNames[name];
  int pos = filename.find_last_of('/');
  db.name = filename.substr(pos+1);
  if (mUseCompressedData){
    static CGE::ZipReader zrdr(mPath+"/movie.dat");
    CGE::MemReader rdr = zrdr.openEntry(filename.substr(pos+1));
    if (!rdr.isWorking())
      return false;
    db.length = rdr.getSize();
    db.data = new char[db.length];
    memcpy(db.data, rdr.getData(), db.length);
  }
  else{
    CGE::BinFileReader strm(filename);
    db.length = strm.getSize();
    db.data = new char[db.length];
    strm.readBytes((unsigned char*)db.data, db.length);
  }
  return true;
}

float AdvDocument::readExtendedFrames(CGE::MemReader& txtstream, ExtendedFrames& frms){
  std::string str;
  long val1, val2;
  for (int frames = 0; frames < FRAMES2_MAX; ++frames){
    ExtendedFrame frm;
    bool set = false;
    for (int parts = 0; parts < PARTS_MAX; ++parts){
      str = txtstream.readLine();
      if (str.length() > 0){
        set = true;
        frm.names.push_back(std::string(str.c_str()));
      }
    }
    //read offsets
    str = txtstream.readLine();
    for (unsigned i = 0; i < frm.names.size(); ++i){
      int pos = str.find(";");
      std::string num = str.substr(0, pos);
      str = str.substr(pos+1);
      val1 = atoi(num.c_str());
      pos = str.find(";");
      num = str.substr(0, pos);
      str = str.substr(pos+1);
      val2 = atoi(num.c_str());
      frm.offsets.push_back(Vec2i(val1,val2));
    }
    if (set)
      frms.push_back(frm);
  }
  str = txtstream.readLine();
  val1 = atoi(str.c_str());
  float fps = FPS_MAX/val1;
  return fps;
}

Room* AdvDocument::getRoom(std::string name){
  std::map<std::string,Room>::iterator iter = mRooms.find(name);
  if (iter == mRooms.end()){
    for (std::map<std::string,Room>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

Object* AdvDocument::getObject(std::string name){
  std::map<std::string,Object>::iterator iter = mObjects.find(name);
  if (iter == mObjects.end()){
    for (std::map<std::string,Object>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
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
  if (iter == mScripts.end()){
    for (std::map<std::pair<Script::Type,std::string>,Script>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.second.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
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
  std::ostringstream number;
  if (num < 100)
  number << "0";
  if (num < 10)
  number << "0";
  number << num;
  CGE::ZipReader* firstzrdr = NULL;
  CGE::MemReader firstrdr;
  CGE::ZipReader* zrdr = NULL;
  CGE::MemReader in;
  if (num == 0){
  zrdr = new CGE::ZipReader(mPath+"/font.dat");
  in = zrdr->openEntry("fontdata.sta");
  }
  else{
  firstzrdr = new CGE::ZipReader(mPath+"/fonts.dat");
  firstrdr = firstzrdr->openEntry("font."+number.str());
  zrdr = new CGE::ZipReader(firstrdr.getData(), firstrdr.getSize());
  in = zrdr->openEntry("fontdata."+number.str());
  }
  FontData fnt;
  if (!in.isWorking()){
  delete firstzrdr;
  delete zrdr;
  return fnt;
  }
  long val;
  std::string str = in.readLine(); val = atoi(str.c_str()); fnt.images.resize(2*val);
  str = in.readLine(); val = atoi(str.c_str()); fnt.fontsize.x = val;
  str = in.readLine(); val = atoi(str.c_str()); fnt.fontsize.y = val;
  str = in.readLine(); val = atoi(str.c_str()); fnt.numChars.x = val;
  str = in.readLine(); val = atoi(str.c_str()); fnt.numChars.y = val;
  fnt.charwidths.reserve(224);
  while(in.isWorking()){
  str = in.readLine(); val = atoi(str.c_str()); fnt.charwidths.push_back(val);
  }
  for (unsigned i = 0; i < fnt.images.size()/2; ++i){
  number.str("");
  number.clear();
  if (num == 0){
  number << "fontsta.al" << (i+1);
  in = zrdr->openEntry(number.str());
  }
  else{
  number << "font" << num << ".al" << (i+1);
  in = zrdr->openEntry(number.str());
  }
  if (!in.isWorking())
  continue;
  fnt.images[2*i] = CGE::ImageLoader::load(in.getData(), in.getSize(), CGE::ImageLoader::BMP);
  number.str("");
  number.clear();
  if (num == 0){
  number << "fontsta.bm" << (i+1);
  in = zrdr->openEntry(number.str());
  }
  else{
  number << "font" << num << ".bm" << (i+1);
  in = zrdr->openEntry(number.str());
  }
  fnt.images[2*i+1] = CGE::ImageLoader::load(in.getData(), in.getSize(), CGE::ImageLoader::BMP);
  }
  delete firstzrdr;
  delete zrdr;
  return fnt;
}

Item* AdvDocument::getItem(const std::string& name){
  std::map<std::string,Item>::iterator iter = mItems.find(name);
  if (iter == mItems.end()){
    for (std::map<std::string,Item>::iterator iter = mItems.begin(); iter != mItems.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

Room* AdvDocument::getRoom(Object* obj){
  for (std::map<std::string,Room>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::vector<Roomobject>::iterator objiter = iter->second.objects.begin(); objiter != iter->second.objects.end(); ++objiter){
      if (objiter->name == obj->name)
        return &iter->second;
    }
  }
  DebugBreak();
  return NULL;
}

