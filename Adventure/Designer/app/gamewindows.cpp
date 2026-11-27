// The Designer's windows for the game: Create game and Run game, the
// imports and the export, the fonts, and the media's players.

#include "designer.h"
#include "i18n.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>

#include <audio/audio.h>
#include <audio/audioplayer.h>
#include <platform/systemfonts.h>
#include <platform/userdirs.h>
#include <renderer/renderdevice.h>
#include <system/engine.h>
#include <ui/imgui_impl_cge_render.h>
#include <video/video.h>

#include <model/bitmapfont.h>
#include <model/gamefile.h>
#include <model/images.h>

#include "process.h"
#include "widgets.h"

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

std::string parentOf(const std::string& utf8){
  return fromPath(toPath(utf8).parent_path());
}

//! a texture for ImGui of an RGBA image
std::unique_ptr<CGE::Gfx::Texture> makeTexture(int width, int height, const void* rgba){
  CGE::Engine* engine = CGE::Engine::instance();
  CGE::Gfx::RenderDevice* device = engine ? engine->getRenderDevice() : nullptr;
  if (!device || width <= 0 || height <= 0)
    return nullptr;
  CGE::Gfx::TextureDesc desc;
  desc.width = width;
  desc.height = height;
  return device->createTexture(desc, rgba);
}

//! the file the engine plays for a video (VideoConverter.GetPlayableFile):
//! the video itself, or its conversion to Ogg Theora by videoconv next to
//! the StoryDesigner (Windows); empty and why if it can't be converted
std::string playableVideo(const std::string& file, std::string& error){
  fs::path video = toPath(toUtf8(file));
  std::string ext = fromPath(video.extension());
  for (char& c : ext)
    c = (char)tolower((unsigned char)c);
  if (ext == ".swf")
    return file;
#if defined(WIN32)
  fs::path converter = toPath(programFolder()) / "videoconv.exe";
#else
  fs::path converter = toPath(programFolder()) / "videoconv";
#endif
  std::error_code ec;
  //an installation without the converter packs the videos as they are
  if (!fs::is_regular_file(converter, ec))
    return file;
  std::string output;
  if (runProgram(fromPath(converter), {"--check", fromPath(video)}, output) == 0)
    return file;
  //next to the original, or in the user's data where that folder can't be written
  fs::path converted = video;
  converted.replace_extension(".ogv");
  {
    fs::path test = converted;
    test += ".tmp";
    std::ofstream probe(test, std::ios::binary);
    bool writable = (bool)probe;
    probe.close();
    fs::remove(test, ec);
    if (!writable){
      fs::path dir = toPath(CGE::UserDirs::getDataDir()) / "videos";
      fs::create_directories(dir, ec);
      converted = dir / converted.filename();
    }
  }
  if (fs::is_regular_file(converted, ec) && fs::file_size(converted, ec) > 0 && fs::last_write_time(converted, ec) >= fs::last_write_time(video, ec))
    return fromUtf8(fromPath(converted));
  if (runProgram(fromPath(converter), {fromPath(video), fromPath(converted)}, output) != 0){
    error = "The video " + fromPath(video.filename()) + " can't be converted:\n" + output;
    fs::remove(converted, ec);
    return std::string();
  }
  return fromUtf8(fromPath(converted));
}

}

//! the Fonts window's preview: the font's three lines, rendered again when the font changes
class FontPreview{
public:
  void draw(const FontInfo& info){
    const int width = 351, height = 181;
    if (!texture_ || !same(info, info_)){
      info_ = info;
      error_.clear();
      texture_.reset();
      Bitmap image;
      //the C# preview's lines (with A, O and U umlauts)
      std::vector<std::string> lines{"AaBbCcXxYyZz", "0123456789", "!?\xC4\xD6\xDC$%\xE4\xF6\xFC+"};
      if (renderFontPreview(info, lines, width, height, image, &error_))
        texture_ = makeTexture(image.width, image.height, image.pixels.data());
    }
    //on the C# preview's ControlDark
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x+width, pos.y+height), IM_COL32(160, 160, 160, 255));
    if (texture_)
      ImGui::Image(ImGui_ImplCGE_TextureID(texture_.get()), ImVec2((float)width, (float)height));
    else{
      ImGui::Dummy(ImVec2((float)width, (float)height));
      ImGui::TextUnformatted(error_.c_str());
    }
  }
  void release() {texture_.reset();}
private:
  static bool same(const FontInfo& a, const FontInfo& b){
    return a.name == b.name && a.bold == b.bold && a.italic == b.italic && a.size == b.size && a.outline == b.outline &&
      a.shadow == b.shadow && a.fill == b.fill && a.spacing == b.spacing;
  }
  FontInfo info_;
  std::unique_ptr<CGE::Gfx::Texture> texture_;
  std::string error_;
};

//! a sound's or music's player, or a video's, with its picture
class MediaPreview{
public:
  MediaPreview(ResourceID kind, const std::string& name) : kind_(kind), name_(name) {}
  ~MediaPreview(){
    //before the audio goes
    video_.reset();
    player_.reset();
  }
  bool is(ResourceID kind, const std::string& name) const {return kind == kind_ && toLower(name) == toLower(name_);}
  void load(const Project& project, const std::string& folder){
    std::vector<char> data;
    if (!readMediaFile(project, folder, kind_, name_, data)){
      error_ = "The file can't be found: not at its path, next to the project, or in the game's data.";
      return;
    }
    CGE::Audio* audio = CGE::Engine::instance() ? CGE::Engine::instance()->getAudio() : nullptr;
    if (kind_ == ResourceID::Video){
      video_ = CGE::Video::open(std::vector<unsigned char>(data.begin(), data.end()), audio, CGE::AudioGroup::Sound);
      if (!video_)
        error_ = "The video can't be played (the engine plays MPEG-1 and Ogg Theora).";
    }
    else if (audio){
      player_ = audio->createPlayer(std::vector<uint8_t>(data.begin(), data.end()), kind_ == ResourceID::Music ? CGE::AudioGroup::Music : CGE::AudioGroup::Sound);
      if (!player_)
        error_ = "The file can't be played (the engine plays WAV, MP3, FLAC and Ogg Vorbis).";
    }
    else
      error_ = "There is no sound output.";
  }
  void draw(){
    if (!error_.empty()){
      ImGui::TextDisabled("%s", error_.c_str());
      return;
    }
    const ImVec2 size(6*ImGui::GetFontSize(), 0);
    if (player_){
      if (ImGui::Button(player_->isPlaying() ? "Stop" : "Play", size)){
        if (player_->isPlaying())
          player_->stop();
        else{
          player_->seek(0);
          player_->play(loop_);
        }
      }
      ImGui::SameLine();
      ImGui::Checkbox(tr("Loop"), &loop_);
      ImGui::Text("%.1f of %.1f s", player_->getPosition(), player_->getLength());
    }
    else if (video_){
      if (ImGui::Button(video_->isPlaying() ? "Stop" : "Play", size)){
        if (video_->isPlaying())
          video_->stop();
        else
          video_->play();
      }
      ImGui::SameLine();
      ImGui::Text("%d x %d pixels, %.1f s", video_->getWidth(), video_->getHeight(), video_->getTime());
      if (video_->isPlaying())
        video_->update(ImGui::GetIO().DeltaTime);
      showPicture();
    }
  }
  void release() {texture_.reset();}
private:
  void showPicture(){
    if (!video_->getPixels())
      return;
    int w = video_->getWidth(), h = video_->getHeight();
    if (!texture_){
      texture_ = makeTexture(w, h, video_->getPixels());
      frame_ = video_->getFrameCount();
    }
    else if (frame_ != video_->getFrameCount()){
      frame_ = video_->getFrameCount();
      CGE::Gfx::TextureRegion region;
      region.width = w;
      region.height = h;
      CGE::Engine::instance()->getRenderDevice()->updateTexture(texture_.get(), region, video_->getPixels());
    }
    if (texture_)
      ImGui::Image(ImGui_ImplCGE_TextureID(texture_.get()), ImVec2((float)w, (float)h));
  }
  ResourceID kind_;
  std::string name_;
  std::string error_;
  std::unique_ptr<CGE::AudioPlayer> player_;
  std::unique_ptr<CGE::Video> video_;
  std::unique_ptr<CGE::Gfx::Texture> texture_;
  unsigned frame_ = 0;
  bool loop_ = false;
};

void Designer::showMedia(ResourceID kind, const std::string& name){
  //one player at a time, as the C# media player windows play one file
  if (!media_ || !media_->is(kind, name)){
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && media_){
      ImGui::TextDisabled("Click here to play it.");
      if (!ImGui::IsWindowHovered() || !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;
    }
    media_ = std::make_unique<MediaPreview>(kind, name);
    media_->load(project_, projectFolder());
  }
  media_->draw();
}

void Designer::fontsWindow(){
  ImGui::SetNextWindowDockID(mainDock_, ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(windowTitle(kFonts), &showFonts_)){
    ImGui::End();
    return;
  }
  if (fontFamilies_.empty())
    fontFamilies_ = CGE::SystemFonts::getFamilies();
  if (!fontPreview_)
    fontPreview_ = std::make_unique<FontPreview>();
  std::vector<FontInfo>& fonts = project_.settings.fonts;
  if (font_ >= (int)fonts.size())
    font_ = (int)fonts.size()-1;
  //the list, numbered as the game's fonts, and its buttons
  ImGui::BeginGroup();
  if (ImGui::BeginListBox("##fonts", ImVec2(16*ImGui::GetFontSize(), 12*ImGui::GetFontSize()))){
    for (int i = 0; i < (int)fonts.size(); ++i){
      std::string label = std::to_string(i+1) + " - " + toUtf8(fonts[i].name) + " (" + std::to_string(fonts[i].size) + ")";
      if (ImGui::Selectable((label + "##" + std::to_string(i)).c_str(), i == font_))
        font_ = i;
    }
    ImGui::EndListBox();
  }
  const ImVec2 button(4*ImGui::GetFontSize(), 0);
  if (ImGui::Button(tr("Add"), button)){
    FontInfo info;
    info.name = "Arial";
    info.size = 12;
    fonts.push_back(info);
    font_ = (int)fonts.size()-1;
    changed();
  }
  ImGui::SameLine();
  ImGui::BeginDisabled(font_ < 0);
  if (ImGui::Button(tr("Remove"), button)){
    fonts.erase(fonts.begin()+font_);
    if (font_ >= (int)fonts.size())
      --font_;
    changed();
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::BeginDisabled(font_ <= 0);
  if (ImGui::Button(tr("up"), button)){
    std::swap(fonts[font_], fonts[font_-1]);
    --font_;
    changed();
  }
  ImGui::EndDisabled();
  ImGui::SameLine();
  ImGui::BeginDisabled(font_ < 0 || font_ >= (int)fonts.size()-1);
  if (ImGui::Button(tr("down"), button)){
    std::swap(fonts[font_], fonts[font_+1]);
    ++font_;
    changed();
  }
  ImGui::EndDisabled();
  ImGui::EndGroup();
  if (font_ < 0){
    ImGui::TextDisabled("The project has no fonts of its own: add one.");
    ImGui::End();
    return;
  }
  ImGui::SameLine();
  ImGui::BeginGroup();
  FontInfo& info = fonts[font_];
  bool changedFont = false;
  ImGui::SetNextItemWidth(16*ImGui::GetFontSize());
  std::string family = toUtf8(info.name);
  if (ImGui::BeginCombo(tr("Font"), family.c_str())){
    for (const std::string& f : fontFamilies_){
      if (ImGui::Selectable(f.c_str(), f == family)){
        info.name = fromUtf8(f);
        changedFont = true;
      }
    }
    ImGui::EndCombo();
  }
  static const char* const charsets[] = {"Western", "East european", "Russian", "Greek", "Turkish", "Arabic", "Hebrew",
    "Vietnamese", "Thai", "Baltic", "Japanese ShiftJIS", "Dos OEM", "Symbol"};
  ImGui::SetNextItemWidth(16*ImGui::GetFontSize());
  if (ImGui::Combo(tr("Charset"), &info.charset, charsets, IM_ARRAYSIZE(charsets)))
    changedFont = true;
  changedFont |= checkbox("Bold", info.bold);
  ImGui::SameLine();
  changedFont |= checkbox("Italic", info.italic);
  ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
  changedFont |= inputInt("Size", info.size, 6, 40);
  ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
  changedFont |= inputInt("Letter spacing", info.spacing, -5, 5);
  ImGui::SeparatorText(tr("Outline"));
  changedFont |= radioButtons(info.outline, {"None", "Shadow A", "Shadow B", "Shadow A+B"}, true);
  ImGui::SeparatorText(tr("Shadow"));
  changedFont |= radioButtons(info.shadow, {"None", "25 %", "50 %", "75 %"}, true);
  ImGui::SeparatorText(tr("Fill"));
  changedFont |= radioButtons(info.fill, {"Normal", "Falloff 1", "Falloff 2", "Scanlines"}, true);
  ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
  changedFont |= inputInt("Fading", info.fading, 0, 2000, 100);
  ImGui::SameLine();
  const char* speed = "<none>";
  if (info.fading > 1600)
    speed = "really slow";
  else if (info.fading > 1200)
    speed = "slow";
  else if (info.fading > 800)
    speed = "average";
  else if (info.fading > 400)
    speed = "fast";
  else if (info.fading > 0)
    speed = "very fast";
  ImGui::TextDisabled("%s", speed);
  ImGui::EndGroup();
  if (changedFont)
    changed();
  ImGui::SeparatorText(tr("Preview"));
  fontPreview_->draw(info);
  ImGui::End();
}

std::string Designer::runtimeFolder() const{
  return fromUtf8(programFolder());
}

void Designer::fileChosen(){
  const std::string path = fileDialog_.path();
  switch (fileFor_){
    case FileFor::OpenProject:
      openProject(path);
      break;
    case FileFor::SaveProject:
      if (save(path) && pending_ != Pending::None)
        runPending();
      break;
    case FileFor::AddMedia:{
      mediaFolder(mediaKind_) = fileDialog_.folder();
      std::string errors;
      for (const std::string& file : fileDialog_.paths()){
        std::string cp1252 = fromUtf8(file);
        if (mediaKind_ == ResourceID::Video){
          //the engine plays MPEG-1 and Theora; others are converted now
          std::string error;
          std::string playable = playableVideo(cp1252, error);
          if (playable.empty()){
            errors += error + "\n";
            continue;
          }
          //named after the original file
          std::string error2;
          NamedList<std::string>* files = mediaFiles(project_, mediaKind_);
          if (addMedia(project_, mediaKind_, cp1252, mediaFolder_, &error2)){
            std::string name = fileNameOf(cp1252);
            name = name.substr(0, name.find_last_of('.'));
            files->set(name, playable);
            changed();
          }
          else
            errors += error2 + "\n";
          continue;
        }
        std::string error;
        //the C# editor crops before adding; only PNG files, which it writes back as PNG
        if (mediaKind_ == ResourceID::Image && prefs_.cropTransparent && toLower(file).ends_with(".png") && !cropTransparentImage(file, &error)){
          errors += error + "\n";
          continue;
        }
        if (addMedia(project_, mediaKind_, cp1252, mediaFolder_, &error))
          changed();
        else
          errors += error + "\n";
      }
      if (!errors.empty())
        error_ = errors;
      break;
    }
    case FileFor::ImportAdz:
      importFile(path);
      break;
    case FileFor::ExportAdz:{
      std::vector<std::string> warnings;
      std::string error;
      if (!writeExport(project_, projectFolder(), fromUtf8(path), warnings, &error))
        error_ = error;
      else if (!warnings.empty()){
        message_ = "The project is exported to " + path + ", without:";
        for (const std::string& w : warnings)
          message_ += "\n" + w;
      }
      break;
    }
    case FileFor::ImportCharacter:
    case FileFor::ImportRoom:{
      //the images go next to the project (the C# one: next to its file)
      std::string folder = path_.empty() ? fromUtf8(parentOf(path)) : fromUtf8(parentOf(path_));
      std::string error;
      bool ok = fileFor_ == FileFor::ImportCharacter ? importCharacter(project_, fromUtf8(path), folder, &error)
        : importRoom(project_, fromUtf8(path), folder, &error);
      if (!ok)
        error_ = error;
      else{
        changed();
        images_.clear();
        message_ = tr("Done.");
      }
      break;
    }
    case FileFor::CreateGame:
      startCreateGame(path);
      break;
  }
}

void Designer::importFile(const std::string& path){
  //unpacked next to the export, into a folder of its name (the C# unpackAdz)
  std::string error;
  std::string dest = toUtf8(defaultUnpackFolder(fromUtf8(path)));
  std::string adv = unpackAdz(fromUtf8(path), fromUtf8(dest), &error);
  if (adv.empty()){
    error_ = error;
    return;
  }
  if (openProject(toUtf8(adv)))
    message_ = "The project is unpacked to " + dest + ".";
}

void Designer::startCreateGame(const std::string& chosen){
  std::string path = chosen;
#if defined(WIN32)
  if (fromPath(toPath(path).extension()).empty())
    path += ".exe";
#endif
  task_ = std::make_unique<Task>();
  Task* task = task_.get();
  task->run = false;
  task->project = project_;
  task->folder = parentOf(path);
  task->program = path;
  gamePath_ = path;
  std::string projectFolderCp = projectFolder();
  std::string runtime = runtimeFolder();
  task->thread = std::thread([task, path, projectFolderCp, runtime]{
    GameOptions options;
    options.runtimeFolder = runtime;
    options.playableVideo = playableVideo;
    options.progress = [task](const std::string& step){
      std::lock_guard<std::mutex> lock(task->mutex);
      task->step = step;
    };
    task->ok = writeGame(task->project, projectFolderCp, fromUtf8(path), options, task->warnings, &task->error);
    task->done = true;
  });
}

void Designer::startRunGame(){
  //the game's data in a folder of its own, the runtime in another, made once
  fs::path base = fs::temp_directory_path() / "StoryDesigner";
  std::string name = toUtf8(project_.settings.projectName);
  task_ = std::make_unique<Task>();
  Task* task = task_.get();
  task->run = true;
  task->project = project_;
  fs::path runtimeDir = base / "runtime";
  fs::path dataDir = base / "games" / toPath(name.empty() ? "game" : name);
#if defined(WIN32)
  task->program = fromPath(runtimeDir / "Adventure.exe");
#else
  task->program = fromPath(runtimeDir / "Adventure");
#endif
  task->folder = fromPath(runtimeDir);
  std::string projectFolderCp = projectFolder();
  std::string runtime = runtimeFolder();
  task->thread = std::thread([task, projectFolderCp, runtime, runtimeDir, dataDir]{
    GameOptions options;
    options.runtimeFolder = runtime;
    options.progress = [task](const std::string& step){
      std::lock_guard<std::mutex> lock(task->mutex);
      task->step = step;
    };
    options.progress("runtime");
    std::error_code ec;
    fs::path engine = toPath(toUtf8(runtime)) / "engine1.dat";
    //the runtime again when the StoryDesigner's is newer
    fs::path stamp = runtimeDir / "engine1.dat";
    bool fresh = fs::is_regular_file(stamp, ec) && fs::last_write_time(stamp, ec) >= fs::last_write_time(engine, ec);
    if (!fresh){
      fs::create_directories(runtimeDir, ec);
      std::string error;
      if (!unpackRuntime(fromUtf8(fromPath(engine)), fromUtf8(fromPath(runtimeDir)), std::string(), &error) ||
          !unpackRuntime(fromUtf8(fromPath(toPath(toUtf8(runtime)) / "engine2.dat")), fromUtf8(fromPath(runtimeDir)), std::string(), &error)){
        task->error = error;
        task->done = true;
        return;
      }
      fs::copy_file(engine, stamp, fs::copy_options::overwrite_existing, ec);
    }
    //the game started from here listens for the remote control (Plugins)
    fs::path ini = runtimeDir / "engine.ini";
    if (!fs::exists(ini, ec))
      std::ofstream(ini) << "debugPorts = true\n";
    options.progress("the project");
    std::string file = writeTestGame(task->project, projectFolderCp, fromUtf8(fromPath(dataDir)), options, task->warnings, &task->error);
    if (!file.empty()){
      task->args.push_back(toUtf8(file));
      task->ok = true;
    }
    task->done = true;
  });
}

void Designer::startGame(const std::string& program, const std::vector<std::string>& args, const std::string& folder){
  std::string error;
  if (!startProgram(program, args, folder, &error))
    error_ = error;
}

void Designer::taskPopup(){
  const char* const working = "Working";
  if (task_){
    if (!ImGui::IsPopupOpen(working))
      ImGui::OpenPopup(working);
    if (ImGui::BeginPopupModal(working, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)){
      std::string step;
      {
        std::lock_guard<std::mutex> lock(task_->mutex);
        step = task_->step;
      }
      ImGui::TextUnformatted(tr(task_->run ? "Preparing the game..." : "Creating the game..."));
      ImGui::TextDisabled("%s", step.c_str());
      if (task_->done)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
    if (!task_->done)
      return;
    task_->thread.join();
    //the password a protected project got
    project_.zipPassword = task_->project.zipPassword;
    std::string warnings;
    for (const std::string& w : task_->warnings)
      warnings += "\n" + w;
    if (!task_->ok)
      error_ = task_->error + warnings;
    else if (task_->run){
      startGame(task_->program, task_->args, task_->folder);
      if (!warnings.empty())
        message_ = "The game runs without:" + warnings;
    }
    else{
      //the C# StoryDesigner keeps the game's folder in the project
      project_.settings.directory = fromUtf8(task_->folder);
      char text[1024];
      snprintf(text, sizeof(text), tr("The game was created under %s."), task_->program.c_str());
      message_ = text + (warnings.empty() ? std::string() : tr("\nLeft out:") + warnings);
      askStart_ = true;
    }
    task_.reset();
  }
  const char* const done = tr("Information");
  if (!message_.empty() && !ImGui::IsPopupOpen(done) && error_.empty())
    ImGui::OpenPopup(done);
  if (ImGui::BeginPopupModal(done, nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
    ImGui::TextUnformatted(message_.c_str());
    const ImVec2 size(6*ImGui::GetFontSize(), 0);
    if (askStart_){
      ImGui::TextUnformatted(tr("Start it now?"));
      if (ImGui::Button(tr("Yes"), size)){
        startGame(gamePath_, {}, parentOf(gamePath_));
        message_.clear();
        askStart_ = false;
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(tr("No"), size) || ImGui::IsKeyPressed(ImGuiKey_Escape)){
        message_.clear();
        askStart_ = false;
        ImGui::CloseCurrentPopup();
      }
    }
    else if (ImGui::Button(tr("OK"), size) || ImGui::IsKeyPressed(ImGuiKey_Escape)){
      message_.clear();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void Designer::releaseTextures(){
  images_.clear();
  if (fontPreview_)
    fontPreview_->release();
  if (media_)
    media_->release();
}

//the previews' types are complete here
Designer::Designer(){
  loadPreferences();
  newPoolsInLanguage();
}

Designer::~Designer(){
  if (task_ && task_->thread.joinable())
    task_->thread.join();
}

void Designer::stopMedia(){
  media_.reset();
}

}
