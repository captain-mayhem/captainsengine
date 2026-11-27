// The StoryDesigner's About window and its remote control (the C#
// RemoteControl plugin): a running game is driven over the network, by the
// mouse on a field that stands for its window and by scripts sent to it.

#include "designer.h"
#include "i18n.h"

#include <chrono>
#include <cstdio>
#include <thread>

#include <system/socket.h>

#include "widgets.h"

namespace sd{

RemoteControl::RemoteControl() = default;
RemoteControl::~RemoteControl() = default;

bool RemoteControl::connect(const std::string& host, int port, std::string& error){
  disconnect();
  socket_ = std::make_unique<CGE::Socket>();
  if (!socket_->create() || !socket_->connect(host, port)){
    socket_.reset();
    error = "Could not connect to CGE";
    return false;
  }
  //the greeting with the game's size, sent when the game accepts; waited
  //for a few seconds only, since a busy game never answers
  socket_->set_non_blocking(true);
  std::string hello;
  for (int wait = 0; wait < 60 && hello.find('\n') == std::string::npos; ++wait){
    std::string part;
    int got = socket_->poll(part);
    if (got < 0)
      break;
    if (got == 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    hello += part;
  }
  if (sscanf(hello.c_str(), "cge %d %d", &size_.x, &size_.y) != 2 || size_.x <= 0 || size_.y <= 0){
    socket_.reset();
    error = "Could not connect to CGE";
    return false;
  }
  return true;
}

void RemoteControl::disconnect(){
  socket_.reset();
}

void RemoteControl::send(const std::string& line){
  if (socket_ && !socket_->send(line + "\r\n"))
    socket_.reset();
}

void Designer::aboutWindow(){
  const char* const title = windowTitle("About StoryDesigner");
  ImGui::SetNextWindowSize(ImVec2(28*ImGui::GetFontSize(), 0), ImGuiCond_Appearing);
  if (ImGui::Begin(title, &showAbout_, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize)){
    ImGui::TextUnformatted("StoryDesigner");
    ImGui::Text(tr("Version %s"), SD_VERSION);
    ImGui::TextUnformatted(tr("A tool to create Point&Click-Adventures"));
    ImGui::Separator();
    ImGui::TextUnformatted("Copyright \xC2\xA9 Marco Staginski");
    ImGui::TextUnformatted(tr("Part of Captain's Game Engine, under the LGPL 2.1."));
  }
  ImGui::End();
}

void Designer::remoteWindow(){
  if (!remote_)
    remote_ = std::make_unique<RemoteControl>();
  static char host[128] = "127.0.0.1";
  static int port = 28406;
  static std::string script;
  ImGui::SetNextWindowSize(ImVec2(40*ImGui::GetFontSize(), 30*ImGui::GetFontSize()), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(windowTitle("Remote control"), &showRemote_, ImGuiWindowFlags_NoDocking)){
    ImGui::End();
    return;
  }
  if (!remote_->connected()){
    ImGui::SetNextItemWidth(12*ImGui::GetFontSize());
    ImGui::InputText(tr("IP address / hostname"), host, sizeof(host));
    ImGui::SetNextItemWidth(8*ImGui::GetFontSize());
    ImGui::InputInt(tr("Port"), &port);
    if (ImGui::Button(tr("Connect"))){
      std::string error;
      if (!remote_->connect(host, port, error))
        error_ = tr(error.c_str());
    }
    ImGui::TextDisabled("%s", tr("Start the game first (Run game)."));
  }
  else{
    if (ImGui::Button(tr("Disconnect")))
      remote_->disconnect();
    ImGui::SameLine();
    ImGui::TextDisabled("%s:%d", host, port);
  }
  if (remote_->connected() && ImGui::BeginTabBar("remote")){
    if (ImGui::BeginTabItem(tr("Remote control"))){
      //the game's window, scaled into the room left; the mouse there is the game's
      const Vec2i game = remote_->size();
      ImVec2 avail = ImGui::GetContentRegionAvail();
      float scale = std::min(avail.x/game.x, avail.y/game.y);
      if (scale > 0){
        ImVec2 size(game.x*scale, game.y*scale);
        ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##game", size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        ImGui::GetWindowDrawList()->AddRectFilled(origin, ImVec2(origin.x+size.x, origin.y+size.y), ImGui::GetColorU32(ImGuiCol_FrameBg));
        if (ImGui::IsItemHovered()){
          ImVec2 mouse = ImGui::GetIO().MousePos;
          int x = (int)((mouse.x - origin.x)/scale);
          int y = (int)((mouse.y - origin.y)/scale);
          char line[64];
          if (ImGui::GetIO().MouseDelta.x != 0 || ImGui::GetIO().MouseDelta.y != 0){
            snprintf(line, sizeof(line), "mps %d %d", x, y);
            remote_->send(line);
          }
          if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            snprintf(line, sizeof(line), "mcl %d %d", x, y);
            remote_->send(line);
          }
          else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
            snprintf(line, sizeof(line), "mcr %d %d", x, y);
            remote_->send(line);
          }
        }
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(tr("Script window"))){
      //a script the game runs at once, in the game's own text (Windows-1252)
      if (ImGui::Button(tr("send"))){
        remote_->send("scr");
        std::string text = fromUtf8(script);
        size_t start = 0;
        while (start <= text.size()){
          size_t end = text.find('\n', start);
          remote_->send(text.substr(start, end == std::string::npos ? std::string::npos : end-start));
          if (end == std::string::npos)
            break;
          start = end+1;
        }
        remote_->send("***");
      }
      inputTextUtf8("##script", script, ImGuiInputTextFlags_AllowTabInput, ImVec2(-1, -1));
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

}
