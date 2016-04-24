#include <extern/libxml/tinyxml.h>
#include "AdvDoc.h"
#include <iomanip>
#include <sstream>
#include <system/utilities.h>
#include <system/engine.h>
#include <image/loader.h>
#include <io/BinFileReader.h>
#include "Sound.h"
#include "Engine.h"
#include <system/allocation.h>
#include <system/file.h>
#include "Console.h"

const float FPS_MAX = 50.0f;
const int STATES_MAX = 10;
const int CHAR_STATES_MAX = 36;
const int FRAMES_MAX = 25;
const int FRAMES2_MAX = 30;
const int PARTS_MAX = 2;
const int FXSHAPES_MAX = 3;

#include <io/ZipReader.h>
#include <io/Tracing.h>

using namespace adv;

TR_CHANNEL(ADV_DATA);

AdvDocument::AdvDocument() : mUseCompressedData(false), mSSCB(NULL), mFCB(NULL){
}

AdvDocument::~AdvDocument(){
}

bool AdvDocument::loadDocument(const std::string filename){
  TR_USE(ADV_DATA);
  TR_DEBUG("loading filename %s", filename.c_str());
  mFilename = filename;
  CGE::ZipReader zrdr(mFilename);
  if (!zrdr.isWorking()){
    CGE::Engine::instance()->messageBox("Cannot find adventure (game.dat)", "Error");
    return false;
  }
  string entry = "game";
  if (filename.substr(filename.size() - 4) == ".adv"){
    size_t idx = filename.find_last_of("/");
    entry = filename.substr(idx + 1, filename.size() - (idx + 1) - 4);
  }
  int ver_major = 0, ver_minor = 0;
  CGE::MemReader rdr = zrdr.openEntry(entry + ".001");
  if (!loadFile1(rdr, ver_major, ver_minor)){
    CGE::Engine::instance()->messageBox("Failed loading project file", "Error");
    return false;
  }

  {
    TR_USE(ADV_Console);
    TR_INFO("Game.dat opened... File version: %i.%i.", ver_major, ver_minor);
  }

  if (mFilename.substr(mFilename.size() - 4) == ".dat")
    mUseCompressedData = true;

  CGE::Utilities::replaceWith(mFilename, '\\', '/');
  mPath = mFilename;
  size_t pos = mPath.find_last_of('/');
  mPath.erase(pos);

  //spash screen
  if (mSSCB){
    CGE::ImageLoader loader;
    CGE::Image* splash = NULL;
    if (!mSettings.splashscreen.empty()){
      string splashfile = mImageNames[mSettings.splashscreen];
      if (mUseCompressedData){
        size_t namepos = splashfile.find_last_of('/');
        splashfile = mPath + "/" + splashfile.substr(namepos + 1);
      }
      if (splashfile.size() > 0 && splashfile[splashfile.size() - 1] == 'j')
        splashfile[splashfile.size() - 1] = 'g';
      splash = loader.load(splashfile.c_str());
      if (!splash){
        std::string ext = CGE::Filesystem::getExtension(splashfile);
        splash = loader.load((mPath + "/loading." + ext).c_str());
      }
    }
    if (!splash){
      splash = loader.load((mPath + "/loading.gif").c_str());//try hard-coded path for compatibility
    }
    if (splash)
      mSSCB(splash->getWidth(), splash->getHeight(), splash->getNumChannels(), splash->getData());
    delete splash;
  }

  if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
    mFontsPNG = !CGE::Filesystem::exists(mPath + "/font.dat");
  }
  else
    mFontsPNG = false;

  rdr = zrdr.openEntry(entry + ".002");
  if (!loadFile2(rdr, ver_major, ver_minor)){
    CGE::Engine::instance()->messageBox("Failed loading project file", "Error");
    return false;
  }
  rdr = zrdr.openEntry(entry + ".003");
  if (!loadFile3(rdr)){
    CGE::Engine::instance()->messageBox("Failed loading project file", "Error");
    return false;
  }
  rdr = zrdr.openEntry(entry + ".004");
  loadFile4(rdr);
  rdr = zrdr.openEntry(entry + ".005");
  if (!loadFile5(rdr)){
    mZipPwd = "";
  }
  rdr = zrdr.openEntry(entry + ".010");
  if (rdr.isWorking())
    loadFile10(rdr);
  else{
    mSettings.script_lang = PCDK_SCRIPT;
  }

  {
    TR_USE(ADV_Console);
    TR_INFO("All game data read:");
    TR_INFO("%4i rooms", mRooms.size());
    TR_INFO("%4i characters", mCharacters.size());
    TR_INFO("%4i items", mItems.size());
    TR_INFO("%4i objects", mObjects.size());
    int scriptcount = 0;
    for (std::map<std::pair<Script::Type, std::string>, Script>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
      if (iter->first.first == Script::CUTSCENE)
        ++scriptcount;
    }
    TR_INFO("%4i scripts", scriptcount);
  }

  if (mSettings.savedir.empty())
    mSettings.savedir = mPath + "/../saves";
  else{
    string dirname = mSettings.dir;
    CGE::Utilities::replaceWith(dirname, '\\', '/');
    size_t idx = dirname.find_last_of('/');
    //dirname = dirname.erase(idx);
    //idx = dirname.find_last_of('/');
    dirname.erase(0, idx);
    mSettings.savedir = CGE::Filesystem::combinePath(mSettings.savedir, dirname);
  }

  CGE::Filesystem::createDir(mSettings.savedir);

  return true;
}

bool AdvDocument::loadFile1(CGE::MemReader& txtstream, int& ver_major, int& ver_minor){
  TR_USE(ADV_DATA);
  std::string str = txtstream.readLine();
  ver_major = atoi(str.substr(0, 1).c_str());
  ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  str = txtstream.readLine();
  if (ver_major > 1)
    mSettings.dir = txtstream.readLine();
  else
    mSettings.dir = "";
  str = txtstream.readLine();
  if (str == "Resolution X : 640"){
    mSettings.resolution = Vec2i(640, 480);
  }
  else if (str == "Resolution X : 800"){
    mSettings.resolution = Vec2i(800, 600);
  }
  else if (str == "Resolution X : 320"){
    mSettings.resolution = Vec2i(320, 240);
  }
  else
    TR_BREAK("Unknown resolution");
  mSettings.wm_resolution = mSettings.resolution;
  std::string font = txtstream.readLine();
  while (font.substr(0, 11) != "GameFont : "){
    size_t pos = font.find(';', 0);//name
    pos = font.find(';', pos + 1);//bold
    pos = font.find(';', pos + 1);//italic
    pos = font.find(';', pos + 1);//size
    pos = font.find(';', pos + 1);//outline
    if (ver_major > 1)
      pos = font.find(';', pos + 1);//charset
    int fading = 0;
    if (ver_major > 2 || (ver_major == 2 && ver_minor > 0)){
      pos = font.find(';', pos + 1);//shadow
      pos = font.find(';', pos + 1);//fill
      //fading
      std::string tmp = font.substr(pos + 1);
      fading = atoi(tmp.c_str());
    }
    mSettings.font_fading.push_back(fading);
    font = txtstream.readLine();
  };
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  str = txtstream.readLine();
  if (ver_major > 1){
    mSettings.splashscreen = txtstream.readLine();
    if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
      size_t idx = mSettings.splashscreen.find(';');
      if (idx != mSettings.splashscreen.npos){
        mSettings.splashscreen.erase(idx);
      }
    }
    str = txtstream.readLine();
    mSettings.tsbackground = txtstream.readLine();
  }
  else{
    mSettings.tsbackground = "";
  }
  str = txtstream.readLine();
  if (str.substr(0, 14) != "Startskript : ")
    TR_BREAK("Invalid content %s", str.c_str());
  mSettings.startscript = str.substr(14);
  mSettings.mainscript = txtstream.readLine();
  mSettings.anywhere_room = txtstream.readLine();
  str = txtstream.readLine(); //Screenchange
  mSettings.screenchange = (ScreenChange)atoi(str.substr(15).c_str());
  str = txtstream.readLine().substr(9); //flags
  mSettings.draw_dragged_items = str[1] == '1';
  mSettings.show_actiontext = str[2] == '1';
  mSettings.show_taskbar = str[3] == '1';
  if (ver_major >= 3){
    mSettings.group_items = str[6] == '1';
  }
  else{
    mSettings.group_items = false;
  }
  bool is16to9 = false;
  if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
    is16to9 = str[8] == '1';
  }
  if (is16to9){
    if (mSettings.resolution.x == 320)
      mSettings.resolution.x = 420;
    else if (mSettings.resolution.x == 640)
      mSettings.resolution.x = 840;
    else if (mSettings.resolution.x == 800)
      mSettings.resolution.x = 1050;
    else if (mSettings.resolution.x == 1024)
      mSettings.resolution.x = 1344;
    else if (mSettings.resolution.x == 1440)
      mSettings.resolution.x = 1890;
  }
  if (ver_major > 2 || (ver_major == 2 && ver_minor > 0)){
    str = txtstream.readLine(); //action text height
  }
  str = txtstream.readLine();
  mSettings.has_menuroom = str == "-1";
  mSettings.menuroom = txtstream.readLine();
  str = txtstream.readLine();
  mSettings.taskheight = atoi(str.substr(13).c_str());
  str = txtstream.readLine();
  mSettings.taskroom = str.substr(11).c_str();
  str = txtstream.readLine();
  mSettings.taskpopup = atoi(str.substr(12).c_str());
  str = txtstream.readLine();
  if (ver_major > 1){
    std::string taskshow = str.substr(11, 4).c_str();
    if (taskshow == "hide")
      mSettings.taskHideCompletely = true;
    else
      mSettings.taskHideCompletely = false;
    mSettings.right_click = atoi(str.substr(16).c_str());
  }
  else{
    mSettings.taskHideCompletely = false;
    mSettings.right_click = 0;
  }
  if (ver_major > 2 || (ver_major == 2 && ver_minor > 0)){
    str = txtstream.readLine();
    str = txtstream.readLine();
    mSettings.menu_fading = atoi(str.c_str());
    str = txtstream.readLine();
    mSettings.textscene_fading = atoi(str.c_str());
    str = txtstream.readLine();
    mSettings.anywhere_transparency = atoi(str.c_str());
  }
  else{
    mSettings.anywhere_transparency = 0;
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
  if (ver_major > 1){
    size_t pos = str.find(';');
    std::string color = str.substr(15, pos - 15);
    mSettings.offspeechcolor = atoi(color.c_str());
    color = str.substr(pos + 1);
    mSettings.infotextcolor = atoi(color.c_str());
  }
  else{
    std::string color = str.substr(15);
    mSettings.offspeechcolor = atoi(color.c_str());
    mSettings.infotextcolor = 0xffffff;
  }
  str = txtstream.readLine();
  mSettings.tsstyle = (TsStyle)(atoi(str.c_str()) - 1);
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
  if (ver_major >= 3 || (ver_major >= 2 && ver_minor >= 7)){
    str = txtstream.readLine();
    str = txtstream.readLine();
    str = txtstream.readLine();
    mSettings.coinActivated = str == "-1";
    str = txtstream.readLine();
    mSettings.coinAutoPopup = str == "-1";
    mSettings.coinRoom = txtstream.readLine();
    str = txtstream.readLine();
    mSettings.coinFading = atoi(str.c_str());
    str = txtstream.readLine();
    Vec2f div(130.0f, 180.0f);
    if (ver_major >= 3 || (ver_major == 3) && ver_minor >= 5)
      div = Vec2f(140.0f, 190.0f);
    mSettings.coinCenter.y = (int)(atoi(str.c_str()) / div.x*mSettings.resolution.y);
    str = txtstream.readLine();
    mSettings.coinCenter.x = (int)(atoi(str.c_str()) / div.y*mSettings.resolution.x);
  }
  else{
    mSettings.coinActivated = false;
  }
  if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
    //dsp effects
    for (int i = 0; i < 25; ++i){
      str = txtstream.readLine();
      size_t pos = str.find(';', 0);//dsp
      pos = str.find(';', pos + 1);//id
      size_t newpos = str.find(';', pos + 1);//name
      std::string name = str.substr(pos+1, newpos - pos-1);
      std::string estr = str.substr(newpos + 1);
      int effectnum = atoi(estr.c_str());
      for (int i = 0; i < effectnum; ++i){
        str = txtstream.readLine();
      }
      if (effectnum > 0){
        DSPEffect effect;
        effect.type = (DSPEffect::Type)effectnum;
        std::string val;
        size_t pos = 0;
        for (int i = 0; i < 6; ++i){
          size_t newpos = str.find(';', pos);
          val = str.substr(pos, newpos - pos);
          effect.params[i] = atoi(val.c_str());
          pos = newpos + 1;
        }
        mSettings.dspeffects[name] = effect;
      }
      for (int i = effectnum; i < 6; ++i){
        str = txtstream.readLine();
      }
    }
  }
  str = txtstream.readLine();
  mSettings.linktext = str.substr(11);
  str = txtstream.readLine();
  mSettings.givelink = str.substr(11);
  str = txtstream.readLine();
  mSettings.walktext = str.substr(11);
  while (txtstream.isWorking()){
    str = txtstream.readLine();
    if (str == "Booleans :"){
      str = txtstream.readLine();
      while (str != "Commands :"){
        String name;
        std::string val = str.substr(str.size() - 1);
        if (val == "1")
          name = str.substr(0, str.size() - 2).c_str();
        else
          name = str.substr(0, str.size() - 1).c_str();
        mSettings.booleans[name] = (val == "1");
        str = txtstream.readLine();
      }
    }
    if (str == "Commands :"){
      mSettings.pretty_commands.push_back(mSettings.walktext);
      mSettings.commands["walkto"] = (unsigned)mSettings.pretty_commands.size() - 1;
      mSettings.pretty_commands.push_back("Loading");
      mSettings.commands["loading"] = (unsigned)mSettings.pretty_commands.size() - 1;
      str = txtstream.readLine();
      while (str != "Mediapool :"){
        std::string pretty_name = txtstream.readLine();
        mSettings.pretty_commands.push_back((const char*)pretty_name.c_str());
        mSettings.commands[(const char*)str.c_str()] = (unsigned)mSettings.pretty_commands.size() - 1;
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
        mImageNames[str] = toLower(filename);
        str = txtstream.readLine();
      }
    }
    //Sounds
    if (str == "Sounds :"){
      str = txtstream.readLine();
      while (str != "Music :"){
        std::string filename = txtstream.readLine();
        CGE::Utilities::replaceWith(filename, '\\', '/');
        mSoundNames[str] = toLower(filename);
        str = txtstream.readLine();
      }
    }
    //Music
    if (str == "Music :"){
      str = txtstream.readLine();
      while (str != "Movie :"){
        std::string filename = txtstream.readLine();
        CGE::Utilities::replaceWith(filename, '\\', '/');
        CGE::Utilities::replaceWith(str, " - ", "-"); //a hack to transform the names in a way that the script execution is transforming them
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

bool AdvDocument::loadFile2(CGE::MemReader& txtstream, int ver_major, int ver_minor){
  std::string str = txtstream.readLine();
  int local_ver_major = atoi(str.substr(0, 1).c_str());
  int local_ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  txtstream.readLine();
  mCursor.reserve(STATES_MAX);
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
    cs.fps = FPS_MAX / val1;
    long val2;
    str = txtstream.readLine();
    val1 = atoi(str.c_str());
    str = txtstream.readLine();
    val2 = atoi(str.c_str());
    cs.highlight = Vec2i(val1, val2);
    if (ver_major > 3 || (ver_major == 3 && ver_minor >= 6)){
      str = txtstream.readLine();
      cs.itemoffset.x = atoi(str.c_str());
      str = txtstream.readLine();
      cs.itemoffset.y = atoi(str.c_str());
    }
    else{
      cs.itemoffset = Vec2i(16, 16);
    }
    mCursor.push_back(cs);
  }
  while (txtstream.isWorking()){
    str = txtstream.readLine();
    if (!txtstream.isWorking())
      return true;
    std::string rest = str.substr(2);
    if (!(str[0] == '/' && str[1] == '/') && !(str[0] == ';' && str[1] == ';'))
      return false;
    size_t splitidx = rest.find(" ");
    std::string type = rest.substr(0, splitidx);
    std::string name = rest.substr(splitidx + 1);
    // ITEM
    if (type == "Item"){
      Item it;
      it.name = name;
      int numStates = STATES_MAX;
      int delim = 2;
      if (ver_major < 3 || (ver_major == 3 && ver_minor == 0)){
        numStates = 1;
        delim = 1;
      }
      it.states.reserve(numStates);
      for (int state = 0; state < numStates; ++state){
        ItemState is;
        is.frames.reserve(FRAMES_MAX);
        for (int frames = 0; frames < FRAMES_MAX; ++frames){
          str = txtstream.readLine();
          size_t pos = str.find(";");
          Frame frm;
          frm.name = str.substr(0, pos);
          if (pos != std::string::npos)
            frm.script = animationScript(str.substr(pos + 1));
          else
            frm.script = "";
          is.frames.push_back(frm);
        }
        str = txtstream.readLine();
        long val1;
        val1 = atoi(str.c_str());
        is.fps = FPS_MAX / val1;
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
      obj.states.reserve(STATES_MAX);
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
      if (ver_major > 3 || (ver_major == 3 && ver_minor > 0)){
        str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.realleft = (val1 != 0);
      }
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.memresistent = (val1 != 0);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.ghost = (val1 != 0);
      ch.walksound = txtstream.readLine();
      if (ver_major >= 3){
        str = txtstream.readLine();
        size_t pos = str.find(";");
        while (pos < str.length()){
          std::string state = std::string(str.substr(0, pos));
          ch.extrastatenames.push_back(state);
          str = str.substr(pos + 1);
          pos = str.find(";");
        }
      }
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.fontid = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ch.zoom = val1;
      //int states = ver_major >= 3 ? CHAR_STATES_MAX : CHAR_STATES_MAX-20;
      ch.states.reserve(CHAR_STATES_MAX);
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
      rc.position = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); rc.dir = (LookDir)(val1 - 1);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); rc.unmovable = (val1 == 0);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); rc.locked = (val1 != 0);
      mRoomCharacters.push_back(rc);
    }
    // ROOM
    else if (type == "Room"){
      Room room;
      room.name = name;
      long val1, val2;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      room.size = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      room.scrolloffset = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); str = txtstream.readLine(); val2 = atoi(str.c_str());
      room.depthmap = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); room.zoom = val1;
      room.background = txtstream.readLine();
      room.parallaxbackground = txtstream.readLine();
      if (ver_major >= 3){
        str = txtstream.readLine(); val1 = atoi(str.c_str()); room.doublewalkmap = (val1 != 0);
      }
      else{
        room.doublewalkmap = false;
      }
      if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
        str = txtstream.readLine();//lighting color
        room.lighting.r = atoi(str.c_str());
        size_t split = str.find(';');
        str.erase(0, split+1);
        room.lighting.g = atoi(str.c_str());
        split = str.find(';');
        str.erase(0, split + 1);
        room.lighting.b = atoi(str.c_str());
      }
      if (ver_major >= 3 || (ver_major == 2 && ver_minor >= 8)){
        room.fxshapes.reserve(FXSHAPES_MAX);
        for (int i = 0; i < FXSHAPES_MAX; ++i){
          FXShape shape;
          str = txtstream.readLine();
          size_t split = str.find(";");
          shape.active = str.substr(0, split) != "0";
          shape.dependingOnRoomPosition = str.substr(split + 1) != "0";
          str = txtstream.readLine();
          shape.effect = (FXShape::FxEffect)atoi(str.c_str());
          shape.room = txtstream.readLine();
          str = txtstream.readLine();
          shape.depth = atoi(str.c_str());
          str = txtstream.readLine();
          shape.mirrorOffset.x = atoi(str.c_str());
          str = txtstream.readLine();
          shape.mirrorOffset.y = atoi(str.c_str());
          str = txtstream.readLine();
          shape.strength = atoi(str.c_str());
          str = txtstream.readLine();
          split = -1;
          for (int i = 0; i < 8; ++i){
            size_t idx = str.find(";", split + 1);
            std::string tmp = str.substr(split + 1, idx - split - 1);
            if (i % 2 == 0)
              shape.points[i / 2].x = atoi(tmp.c_str());
            else
              shape.points[i / 2].y = atoi(tmp.c_str());
            split = idx;
          }
          room.fxshapes.push_back(shape);
        }
      }
      //inventory
      //has inventory TODO use
      txtstream.readLine();
      str = txtstream.readLine();
      unsigned from = 0;
      unsigned to = (unsigned)str.find(';', from);
      room.invpos.x = atoi(str.substr(from, to - from).c_str());
      from = to + 1; to = (unsigned)str.find(';', from);
      room.invpos.y = atoi(str.substr(from, to - from).c_str());
      from = to + 1; to = (unsigned)str.find(';', from);
      room.invsize.x = atoi(str.substr(from, to - from).c_str());
      from = to + 1; to = (unsigned)str.find(';', from);
      room.invsize.y = atoi(str.substr(from, to - from).c_str());
      from = to + 1; to = (unsigned)str.find(';', from);
      std::string tmp = str.substr(from, to - from);
      size_t idx = tmp.find(',');
      if (idx != std::string::npos)
        tmp[idx] = '.';
      room.invscale.x = (float)atof(tmp.c_str());
      from = to + 1; to = (unsigned)str.find(';', from);
      tmp = str.substr(from, to - from);
      idx = tmp.find(',');
      if (idx != std::string::npos)
        tmp[idx] = '.';
      room.invscale.y = (float)atof(tmp.c_str());
      if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
        from = to + 1; to = (unsigned)str.find(';', from);
        room.invspacing = atoi(str.substr(from, to - from).c_str());
      }
      else
        room.invspacing = 10;
      //walkmap
      str = txtstream.readLine();
      int WALKMAP_X = 32;
      int walkGridSize = mSettings.wm_resolution.x / WALKMAP_X;
      int WALKMAP_Y = mSettings.wm_resolution.y / walkGridSize;
      WALKMAP_X *= 3;
      WALKMAP_Y *= 2;
      if (ver_major >= 3){
        //double walkmap
        WALKMAP_X *= 2;
        WALKMAP_Y *= 2;
      }
      room.walkmap.resize(WALKMAP_X);
      for (int i = 0; i < WALKMAP_X; ++i){
        room.walkmap[i].resize(WALKMAP_Y);
      }
      for (int i = 0; i < WALKMAP_X*WALKMAP_Y; ++i){
        char ch = str[2 * i];
        char ch2 = str[2 * i + 1];
        bool walkable = true;
        bool script = false;
        if (ch == '1')
          walkable = false;
        if (ch2 == '1')
          script = true;
        int x = i / WALKMAP_Y;
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
      ro.position = Vec2i(val1, val2);
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.state = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.layer = val1;
      str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.wm_depth = val1*2;
      if (ver_major > 3 || (ver_major == 3 && ver_minor >= 5)){
        //double walk map
        str = txtstream.readLine(); val1 = atoi(str.c_str()); ro.wm_depth = val1;
      }
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
  while (txtstream.isWorking()){
    str = txtstream.readLine();
    //if (!txtstream.isWorking())
    //  return true;
    if (str.size() >= 2 && str[0] == '/' && str[1] == '/'){
      std::string rest = str.substr(2);
      size_t splitidx = rest.find(" ");
      std::string type = rest.substr(0, splitidx);
      std::string name = rest.substr(splitidx + 1);
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
        CGE::Engine::instance()->messageBox("Unknown script type: " + type, "Error");
      }
      Script scr;
      scr.name = name;
      scr.type = scrType;
      if (scrType == Script::WALKMAP){
        std::string roomname;
        Vec2i pos;
        if (ver_major >= 3){
          roomname = std::string(name.substr(7).c_str());
          pos.x = atoi(name.substr(1, 3).c_str());
          pos.y = atoi(name.substr(4, 3).c_str());
        }
        else{
          roomname = std::string(name.substr(4).c_str());
          pos.x = atoi(name.substr(0, 2).c_str());
          pos.y = atoi(name.substr(2, 2).c_str());
        }
        mWMScripts[roomname].push_back(std::make_pair(pos, scr));
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

bool AdvDocument::loadFile4(CGE::MemReader& txtstream){
  TR_USE(ADV_DATA);
  std::string language;
  Language::Section section;
  while (txtstream.isWorking()){
    std::string str = txtstream.readLine();
    if (str.substr(0, 3) == "*/*"){
      size_t idx = str.find(';');
      language = str.substr(3, idx - 3);
      std::string sectionstr = str.substr(idx + 1);
      if (sectionstr == "speech"){
        section = Language::SPEECH;
      }
      else if (sectionstr == "voicespeech"){
        section = Language::SPEECH_SOUNDS;
      }
      else if (sectionstr == "offspeech"){
        section = Language::OFFSPEECH;
      }
      else if (sectionstr == "voiceoffspeech"){
        section = Language::OFFSPEECH_SOUNDS;
      }
      else if (sectionstr == "showinfo"){
        section = Language::SHOWINFO;
      }
      else if (sectionstr == "textout"){
        section = Language::TEXTOUT;
      }
      else if (sectionstr == "setstring"){
        section = Language::SETSTRING;
      }
      else if (sectionstr == "row"){
        section = Language::TEXTSCENES;
      }
      else if (sectionstr == "kommandos"){
        section = Language::COMMANDS;
      }
      else{
        TR_BREAK("Unknown section %s", sectionstr.c_str());
      }
      continue;
    }
    if (str.size() >= 5 && str.substr(0, 5) == "*NEU*")
      str.erase(0, 5);
    mLanguages[language].sections[section].push_back(str);
  }
  return true;
}

bool AdvDocument::loadFile5(CGE::MemReader& txtstream){
  if (!txtstream.isWorking())
    return false;
  std::string str = txtstream.readLine();
  std::string pwd;
  for (int i = 0; i < 20; ++i){
    int val = atoi(str.substr(i * 3, 3).c_str());
    char curr;
    if (i >= 15)
      curr = val / 3;
    else if (i >= 10)
      curr = val / 6;
    else if (i >= 5)
      curr = val / 4;
    else
      curr = val / 5;
    pwd += curr;
  }
  mZipPwd = pwd;
  return true;
}

CGE::Image* AdvDocument::getImage(const std::string& name){
  TR_USE(ADV_DATA);
  mMuty.lock();
  std::string idxname = toLower(name);
  std::string filename;
  std::map<std::string, std::string>::iterator iter = mImageNames.find(idxname);
  if (iter != mImageNames.end())
    filename = iter->second;
  else{
    TR_ERROR("Image %s not found", name.c_str());
    mMuty.unlock();
    return NULL;
  }
  if (mUseCompressedData){
    size_t namepos = filename.find_last_of('/');
    static CGE::ZipReader zrdr(mPath + "/gfx.dat");
    std::string imagename = filename.substr(namepos + 1);
    CGE::MemReader rdr = zrdr.openEntry(imagename, mZipPwd);
    if (!rdr.isWorking()){
      mMuty.unlock();
      return NULL;
    }
    size_t extpos = filename.find_last_of('.');
    CGE::Image* img = NULL;
    if (filename.substr(extpos + 1) == "pnj"){
      CGE::Image* rgbimage = CGE::ImageLoader::load(rdr.getData(), rdr.getSize(), CGE::ImageLoader::JPG);
      filename[filename.length() - 1] = 'a';
      rdr = zrdr.openEntry(filename.substr(namepos + 1), mZipPwd);
      CGE::Image* alphaimage = CGE::ImageLoader::load(rdr.getData(), rdr.getSize(), CGE::ImageLoader::JPG);
      img = new CGE::Image(rgbimage->getNumChannels(), rgbimage->getWidth(), rgbimage->getHeight(), rgbimage->getData(), alphaimage->getNumChannels(), alphaimage->getData());
      delete rgbimage;
      delete alphaimage;
    }
    else{
      CGE::ImageLoader::Type imgtype = CGE::ImageLoader::determineType(imagename);
      img = CGE::ImageLoader::load(rdr.getData(), rdr.getSize(), imgtype);
    }
    mMuty.unlock();
    return img;
  }
  CGE::Image* img = CGE::ImageLoader::load(filename.c_str());
  mMuty.unlock();
  return img;
}

bool AdvDocument::getSound(const std::string& name, DataBuffer& db){
  TR_USE(ADV_DATA);
  mMuty.lock();
  std::map<std::string, std::string>::iterator iter = mSoundNames.find(toLower(name));
  if (iter == mSoundNames.end()){
    TR_WARN("Sound %s not found", name.c_str());
    mMuty.unlock();
    return false;
  }
  std::string filename = iter->second;
  size_t pos = filename.find_last_of('/');
  db.name = filename.substr(pos + 1);
  if (mUseCompressedData){
    static CGE::ZipReader zrdr(mPath + "/sfx.dat");
    CGE::MemReader rdr = zrdr.openEntry(filename.substr(pos + 1), mZipPwd);
    if (!rdr.isWorking()){
      mMuty.unlock();
      return false;
    }
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
  mMuty.unlock();
  return true;
}

bool AdvDocument::getMusic(const std::string& name, DataBuffer& db){
  mMuty.lock();
  std::string filename = mMusicNames[name];
  size_t pos = filename.find_last_of('/');
  db.name = filename.substr(pos + 1);
  if (mUseCompressedData){
    static CGE::ZipReader zrdr(mPath + "/music.dat");
    CGE::MemReader rdr = zrdr.openEntry(filename.substr(pos + 1), mZipPwd);
    if (!rdr.isWorking()){
      mMuty.unlock();
      return false;
    }
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
  mMuty.unlock();
  return true;
}

bool AdvDocument::getMovie(const std::string& name, DataBuffer& db){
  mMuty.lock();
  std::string filename = mMovieNames[name];
  size_t pos = filename.find_last_of('/');
  db.name = filename.substr(pos + 1);
  if (mUseCompressedData){
    static CGE::ZipReader zrdr(mPath + "/movie.dat");
    CGE::MemReader rdr = zrdr.openEntry(filename.substr(pos + 1), mZipPwd);
    if (!rdr.isWorking()){
      mMuty.unlock();
      return false;
    }
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
  mMuty.unlock();
  return true;
}

float AdvDocument::readExtendedFrames(CGE::MemReader& txtstream, ExtendedFrames& frms){
  std::string str;
  long val1, val2;
  int realFrames = 0;
  frms.reserve(FRAMES2_MAX);
  for (int frames = 0; frames < FRAMES2_MAX; ++frames){
    ExtendedFrame frm;
    bool set[PARTS_MAX];
    for (int parts = 0; parts < PARTS_MAX; ++parts){
      str = txtstream.readLine();
      if (str.length() > 0){
        set[parts] = true;
        frm.names.push_back(std::string(str.c_str()));
      }
      else
        set[parts] = false;
    }
    //read offsets
    str = txtstream.readLine();
    size_t pos = 0;
    for (unsigned i = 0; i < PARTS_MAX; ++i){
      pos = str.find(";");
      std::string num = str.substr(0, pos);
      str = str.substr(pos + 1);
      val1 = atoi(num.c_str());
      pos = str.find(";");
      num = str.substr(0, pos);
      str = str.substr(pos + 1);
      val2 = atoi(num.c_str());
      if (set[i])
        frm.offsets.push_back(Vec2i(val1, val2));
    }
    if (set[0] || set[1])
      realFrames = frames + 1;
    frm.script = animationScript(str);
    frms.push_back(frm);
  }
  frms.resize(realFrames);
  str = txtstream.readLine();
  val1 = atoi(str.c_str());
  float fps = FPS_MAX / val1;
  return fps;
}

Room* AdvDocument::getRoom(std::string name){
  std::map<std::string, Room>::iterator iter = mRooms.find(name);
  if (iter == mRooms.end()){
    for (std::map<std::string, Room>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

Object* AdvDocument::getObject(std::string name){
  std::map<std::string, Object>::iterator iter = mObjects.find(name);
  if (iter == mObjects.end()){
    for (std::map<std::string, Object>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
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
  std::map<std::string, Character>::iterator iter = mCharacters.find(name);
  if (iter == mCharacters.end())
    return NULL;
  return &((*iter).second);
}

Script* AdvDocument::getScript(Script::Type t, std::string name){
  std::map<std::pair<Script::Type, std::string>, Script>::iterator iter = mScripts.find(std::make_pair(t, name));
  if (iter == mScripts.end()){
    for (std::map<std::pair<Script::Type, std::string>, Script>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.second.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

std::vector<std::pair<Vec2i, Script*> > AdvDocument::getWMScripts(std::string roomname){
  std::vector<std::pair<Vec2i, Script*> > result;
  std::map<std::string, std::vector<std::pair<Vec2i, Script> > >::iterator iter = mWMScripts.find(roomname);
  if (iter == mWMScripts.end())
    return result;
  for (unsigned i = 0; i < iter->second.size(); ++i){
    result.push_back(std::make_pair(iter->second[i].first, &iter->second[i].second));
  }
  return result;
}

bool AdvDocument::parseFontData(const std::string& name, FontData& data){
  TiXmlDocument doc;
  if (!doc.LoadFile(name.c_str()))
    return false;
  TiXmlNode* font = doc.FirstChild();
  TiXmlElement* elem = font->ToElement();
  data.fontsize.x = atoi(elem->Attribute("width"));
  data.fontsize.y = atoi(elem->Attribute("height"));
  for (TiXmlNode* child = font->FirstChild(); child != NULL; child = child->NextSibling()){
    TiXmlElement* elem = child->ToElement();
    const char* value = elem->Attribute("ascii");
    if (!value)
      value = elem->Attribute("ucode");
    unsigned charidx = atoi(value) - 0x20;
    if (data.glyphmap.size() <= charidx)
      data.glyphmap.resize(charidx + 1);
    FontData::Glyph gl;
    gl.x = atoi(elem->Attribute("x"));
    gl.y = atoi(elem->Attribute("y"));
    gl.yoffset = atoi(elem->Attribute("top"));
    gl.xoffset = atoi(elem->Attribute("leading"));
    gl.w = atoi(elem->Attribute("width"));
    gl.h = atoi(elem->Attribute("height"));
    gl.advwidth = gl.w+gl.xoffset+atoi(elem->Attribute("trailing"));
    data.glyphmap[charidx] = gl;
  }
  if (data.glyphmap.empty())
    return false;
  if (data.glyphmap[0].advwidth == 0)
    data.glyphmap[0].advwidth = data.fontsize.x/3;
  return true;
}

FontData AdvDocument::getFont(int num){
  std::ostringstream number;
  //if (num < 100)
  number << "0";
  //if (num < 10)
  number << "0";
  number << num;
  CGE::ZipReader* firstzrdr = NULL;
  CGE::MemReader firstrdr;
  CGE::ZipReader* zrdr = NULL;
  CGE::MemReader in;
  if (num == 0){
    if (hasUnifiedFonts()){
      FontData ret;
      ret.images.resize(1);
      ret.images[0] = CGE::ImageLoader::load((mPath + "/font.png").c_str());
      ret.numChars.x = 1000000;//just one texture, so always return first
      ret.numChars.y = 1000000;
      parseFontData(mPath+"/font.xml", ret);
      return ret;
    }
    zrdr = new CGE::ZipReader(mPath + "/font.dat");
    in = zrdr->openEntry("fontdata.sta");
  }
  else{
    firstzrdr = new CGE::ZipReader(mPath + "/fonts.dat");
    firstrdr = firstzrdr->openEntry("font." + number.str());
    zrdr = new CGE::ZipReader(firstrdr.getData(), firstrdr.getSize());
    in = zrdr->openEntry("fontdata." + number.str());
  }
  FontData fnt;
  if (!in.isWorking()){
    delete firstzrdr;
    delete zrdr;
    return fnt;
  }
  long val;
  std::string str = in.readLine(); val = atoi(str.c_str()); fnt.images.resize(mFontsPNG ? val : 2 * val);
  str = in.readLine(); val = atoi(str.c_str()); fnt.fontsize.x = val;
  str = in.readLine(); val = atoi(str.c_str()); fnt.fontsize.y = val;
  str = in.readLine(); val = atoi(str.c_str()); fnt.numChars.x = val;
  str = in.readLine(); val = atoi(str.c_str()); fnt.numChars.y = val;
  fnt.glyphmap.reserve(224);
  unsigned charnum = 0;
  while (in.isWorking()){
    FontData::Glyph glyph;
    unsigned posInTex = charnum % (fnt.numChars.x*fnt.numChars.y);
    unsigned rownum = posInTex / fnt.numChars.x;
    unsigned colnum = charnum % fnt.numChars.x;
    glyph.x = colnum*fnt.fontsize.x;
    glyph.y = rownum*fnt.fontsize.y;
    glyph.w = fnt.fontsize.x;
    glyph.h = fnt.fontsize.y;
    glyph.xoffset = 0;
    glyph.yoffset = 0;
    ++charnum;
    str = in.readLine(); val = atoi(str.c_str());
    glyph.advwidth = val;
    fnt.glyphmap.push_back(glyph);
  }
  for (unsigned i = 0; i < (mFontsPNG ? fnt.images.size() : fnt.images.size() / 2); ++i){
    number.str("");
    number.clear();
    if (num == 0){
      number << "fontsta.al" << (i + 1);
      in = zrdr->openEntry(number.str());
    }
    else{
      number << "font" << num << (mFontsPNG ? ".pn" : ".al") << (i + 1);
      in = zrdr->openEntry(number.str());
    }
    if (!in.isWorking())
      continue;
    fnt.images[2 * i] = CGE::ImageLoader::load(in.getData(), in.getSize(), mFontsPNG ? CGE::ImageLoader::PNG : CGE::ImageLoader::BMP);
    if (!mFontsPNG){
      number.str("");
      number.clear();
      if (num == 0){
        number << "fontsta.bm" << (i + 1);
        in = zrdr->openEntry(number.str());
      }
      else{
        number << "font" << num << ".bm" << (i + 1);
        in = zrdr->openEntry(number.str());
      }
      fnt.images[2 * i + 1] = CGE::ImageLoader::load(in.getData(), in.getSize(), CGE::ImageLoader::BMP);
    }
  }
  delete firstzrdr;
  delete zrdr;
  return fnt;
}

Item* AdvDocument::getItem(const std::string& name){
  std::map<std::string, Item>::iterator iter = mItems.find(name);
  if (iter == mItems.end()){
    for (std::map<std::string, Item>::iterator iter = mItems.begin(); iter != mItems.end(); ++iter){
      if (_stricmp(name.c_str(), iter->first.c_str()) == 0)
        return &iter->second;
    }
    return NULL;
  }
  return &((*iter).second);
}

Language::Section AdvDocument::getLanguageSection(const String& fname, int argnum){
  String funcname = fname.toLower();
  if (funcname == "speech"){
    if (argnum == 1)
      return Language::SPEECH;
    else if (argnum == 2)
      return Language::SPEECH_SOUNDS;
  }
  else if (funcname == "offspeech"){
    if (argnum == 2)
      return Language::OFFSPEECH;
    else if (argnum == 3)
      return Language::OFFSPEECH_SOUNDS;
  }
  else if (funcname == "showinfo")
    return Language::SHOWINFO;
  else if (funcname == "textout")
    return Language::TEXTOUT;
  else if (funcname == "setstring")
    return Language::SETSTRING;
  else if (funcname == "row")
    return Language::TEXTSCENES;
  else{
    TR_USE(ADV_DATA);
    TR_BREAK("Unknown function %s", funcname.c_str());
  }
  return Language::NUM_SECTIONS;
}

std::string AdvDocument::getLanguageString(const std::string& language, Language::Section section, int strindex){
  return mLanguages[language].sections[section][strindex];
}

int AdvDocument::getLanguageIndex(const std::string& language, Language::Section section, const std::string& str){
  TR_USE(ADV_DATA);
  std::vector<std::string>& sect = mLanguages[language].sections[section];
  for (unsigned i = 0; i < sect.size(); ++i){
    if (sect[i] == str)
      return i;
  }
  TR_INFO("Language string %s not found", str.c_str());
  return 0;
}

bool AdvDocument::loadFile10(CGE::MemReader& txtstream){
  std::string str = txtstream.readLine();
  int ver_major = atoi(str.substr(0, 1).c_str());
  int ver_minor = atoi(str.substr(2, 1).c_str());
  if (str.substr(4) != "Point&Click Project File. DO NOT MODIFY!!"){
    return false;
  }
  txtstream.readLine();
  str = txtstream.readLine();
  //mSettings.noPngToJpeg = str == "-1";
  if (ver_major < 3 || (ver_major == 3 && ver_minor < 5))
    return true;
  str = txtstream.readLine();
  if (str == "LUA")
    mSettings.script_lang = LUA_SCRIPT;
  else
    mSettings.script_lang = PCDK_SCRIPT;
  return true;
}

std::string AdvDocument::animationScript(const std::string& input){
  std::string ret;
  for (unsigned i = 0; i < input.size(); ++i){
    if (input[i] == '\xaf')
      ret.append(1, ';');
    else
      ret.append(1, input[i]);
  }
  return ret;
}

Roomobject* AdvDocument::findRoomObject(const std::string& name, Room*& containingRoom){
  for (std::map<std::string, Room>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::vector<Roomobject>::iterator objiter = iter->second.objects.begin(); objiter != iter->second.objects.end(); ++objiter){
      if (_stricmp(objiter->name.c_str(), name.c_str()) == 0){
        containingRoom = &iter->second;
        return &*objiter;
      }
    }
  }
  return NULL;
}

