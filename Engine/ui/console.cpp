#include "console.h"

#include <cfloat>
#include <cstring>

#include <imgui.h>

#include "../system/engine.h"

using std::string;

using namespace CGE;

namespace{

//! the most output lines kept
const size_t MAX_LINES = 200;
//! the most entered lines kept for going back with up
const size_t MAX_HISTORY = 50;
//! the console's height in virtual units (Engine::getVirtualScreen)
const float CONSOLE_HEIGHT = 300;

}

namespace CGE{

//! the input line's ImGui callback
struct ConsoleCallbacks{
  //! up and down go through the entered lines
  static int history(ImGuiInputTextCallbackData* data){
    Console* cons = (Console*)data->UserData;
    if (data->EventFlag != ImGuiInputTextFlags_CallbackHistory || cons->history_.empty())
      return 0;
    const int count = (int)cons->history_.size();
    int pos = cons->historyPos_;
    if (data->EventKey == ImGuiKey_UpArrow)
      pos = pos < 0 ? count-1 : (pos > 0 ? pos-1 : 0);
    else if (data->EventKey == ImGuiKey_DownArrow && pos >= 0)
      pos = pos+1 < count ? pos+1 : -1;
    if (pos == cons->historyPos_)
      return 0;
    cons->historyPos_ = pos;
    data->DeleteChars(0, data->BufTextLen);
    if (pos >= 0)
      data->InsertChars(0, cons->history_[pos].c_str());
    return 0;
  }
};

}

Console::Console(){
  historyPos_ = -1;
  input_[0] = '\0';
  active_ = false;
  focusInput_ = false;
  scrollToEnd_ = false;
  new_ = false;
  password_ = false;
  process_ = nullptr;
  special_ = nullptr;
  mode_ = 0;
  additional_ = nullptr;
  //a nice starting message
  string start("HeroQuest v");
  start += VERSTRING;
  lines_.push_back(start);
  lines_.push_back("Console started...");
}

Console::~Console(){
}

void Console::toggle(){
  active_ = !active_;
  if (active_){
    focusInput_ = true;
    scrollToEnd_ = true;
  }
  else{
    input_[0] = '\0';
    historyPos_ = -1;
  }
}

void Console::addText(const string& text){
  size_t start = 0;
  while (start <= text.size()){
    size_t end = text.find('\n', start);
    if (end == string::npos)
      end = text.size();
    //a newline at the end doesn't start an empty line
    if (end == text.size() && start == end && start > 0)
      break;
    lines_.push_back(text.substr(start, end-start));
    start = end+1;
  }
  while (lines_.size() > MAX_LINES)
    lines_.pop_front();
  scrollToEnd_ = true;
}

void Console::update(){
  while (!entered_.empty()){
    string line = entered_.front();
    entered_.pop_front();
    if (!password_){
      lines_.push_back(line);
      while (lines_.size() > MAX_LINES)
        lines_.pop_front();
      if (!line.empty() && (history_.empty() || history_.back() != line))
        history_.push_back(line);
      while (history_.size() > MAX_HISTORY)
        history_.pop_front();
      scrollToEnd_ = true;
    }
    historyPos_ = -1;
    lastInput_ = line;
    new_ = true;
    //call the special or the main callback
    if (special_){
      void (*special)(const string&, int, void*) = special_;
      special_ = nullptr;
      special(line, mode_, additional_);
    }
    else if (process_)
      process_(line.c_str());
  }
}

void Console::render(){
  if (!active_ || !ImGui::GetCurrentContext())
    return;
  const ImGuiIO& io = ImGui::GetIO();
  float height = CONSOLE_HEIGHT*Engine::instance()->getVirtualScreen().getScale();
  if (height > io.DisplaySize.y)
    height = io.DisplaySize.y;
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, height));
  ImGui::SetNextWindowBgAlpha(0.7f);
  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
  if (ImGui::Begin("##CGE_Console", nullptr, flags)){
    //the output above the input line, scrolled to the end when something is added
    const float inputHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("##output", ImVec2(0, -inputHeight))){
      ImGui::PushTextWrapPos(0);
      for (const string& line : lines_)
        ImGui::TextUnformatted(line.c_str());
      ImGui::PopTextWrapPos();
      if (scrollToEnd_)
        ImGui::SetScrollHereY(1.0f);
      scrollToEnd_ = false;
    }
    ImGui::EndChild();

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (focusInput_){
      ImGui::SetKeyboardFocusHere();
      focusInput_ = false;
    }
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;
    if (password_)
      inputFlags |= ImGuiInputTextFlags_Password;
    if (ImGui::InputText("##input", input_, sizeof(input_), inputFlags, ConsoleCallbacks::history, this)){
      //passed on in update, outside the ImGui frame
      enter(input_);
      input_[0] = '\0';
      //Enter takes the focus away; the console keeps it
      focusInput_ = true;
    }
  }
  ImGui::End();
}
