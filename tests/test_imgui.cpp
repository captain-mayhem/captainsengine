#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

//before imgui.h, which undefines X11's Status (a member of ImTextureData)
#include "testwindow.h"

#include <imgui.h>

#include <input/keyboard.h>
#include <input/touch.h>
#include <io/Tracing.h>
#include <io/TraceManager.h>
#include <renderer/nulldevice.h>
#include <renderer/renderdevice.h>
#include <ui/console.h>
#include <ui/imgui_impl_cge_platform.h>
#include <ui/imgui_impl_cge_render.h>

using namespace CGE;
using namespace CGE::Gfx;

// Dear ImGui in the engine: the platform backend (events, keys), the renderer
// backend on the Null device and, on a hidden window, on the OpenGL device
// (drawn into render targets and read back), and the console.

namespace{

//! an ImGui context with the engine's backends, without a window, on the Null device
class ImGuiTest : public ::testing::Test{
protected:
  void SetUp() override{
    ASSERT_TRUE(device_.init(NativeWindow(), DeviceDesc()));
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    //every event in the frame after it was added
    io.ConfigInputTrickleEventQueue = false;
    ASSERT_TRUE(ImGui_ImplCGE_Platform_Init(nullptr));
    ASSERT_TRUE(ImGui_ImplCGE_Render_Init(device_));
  }

  void TearDown() override{
    ImGui_ImplCGE_Render_Shutdown();
    ImGui_ImplCGE_Platform_Shutdown();
    ImGui::DestroyContext();
  }

  //! an ImGui frame of width x height pixels, with build() between NewFrame and Render
  template<typename Build>
  ImDrawData* frame(int width, int height, Build build){
    ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
    ImGui_ImplCGE_Render_NewFrame();
    ImGui_ImplCGE_Platform_NewFrame();
    ImGui::NewFrame();
    build();
    ImGui::Render();
    return ImGui::GetDrawData();
  }

  NullDevice device_;
};

}

TEST_F(ImGuiTest, EngineKeysAreImGuiKeys){
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_A), ImGuiKey_A);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_W), ImGuiKey_W);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_0), ImGuiKey_0);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_F2), ImGuiKey_F2);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_ESCAPE), ImGuiKey_Escape);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_RETURN), ImGuiKey_Enter);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_BACKSPACE), ImGuiKey_Backspace);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_DELETE), ImGuiKey_Delete);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_UP), ImGuiKey_UpArrow);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_PAGEDOWN), ImGuiKey_PageDown);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(KEY_CTRL), ImGuiKey_LeftCtrl);
  EXPECT_EQ(ImGui_ImplCGE_Platform_ToImGuiKey(0x3039), ImGuiKey_None);
}

TEST_F(ImGuiTest, EventsReachImGui){
  ImGui_ImplCGE_Platform_AddMousePos(10, 20);
  ImGui_ImplCGE_Platform_AddMouseButton(ImGuiMouseButton_Right, true);
  ImGui_ImplCGE_Platform_AddKey(KEY_A, true);
  ImGui_ImplCGE_Platform_AddKey(KEY_CTRL, true);
  ImGui_ImplCGE_Platform_AddMouseWheel(0, 1);
  frame(64, 48, [](){
    ImGuiIO& io = ImGui::GetIO();
    EXPECT_FLOAT_EQ(io.MousePos.x, 10);
    EXPECT_FLOAT_EQ(io.MousePos.y, 20);
    EXPECT_TRUE(io.MouseDown[ImGuiMouseButton_Right]);
    EXPECT_FALSE(io.MouseDown[ImGuiMouseButton_Left]);
    EXPECT_TRUE(ImGui::IsKeyDown(ImGuiKey_A));
    EXPECT_TRUE(io.KeyCtrl);
    EXPECT_FLOAT_EQ(io.MouseWheel, 1);
  });
  ImGui_ImplCGE_Platform_AddKey(KEY_A, false);
  ImGui_ImplCGE_Platform_AddKey(KEY_CTRL, false);
  ImGui_ImplCGE_Platform_AddMouseLeave();
  frame(64, 48, [](){
    EXPECT_FALSE(ImGui::IsKeyDown(ImGuiKey_A));
    EXPECT_FALSE(ImGui::GetIO().KeyCtrl);
    EXPECT_FALSE(ImGui::IsMousePosValid());
  });
}

TEST_F(ImGuiTest, CharactersGoIntoTheActiveTextField){
  char text[16] = "";
  int frames = 0;
  auto build = [&](){
    //with a size, so it isn't hidden in its first frame to measure itself
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(200, 100));
    ImGui::Begin("text");
    if (frames++ == 0)
      ImGui::SetKeyboardFocusHere();
    ImGui::InputText("##text", text, sizeof(text));
    ImGui::End();
  };
  //the focus request activates the field in the next frame, ImGui reports it in the one after
  frame(200, 100, build);
  frame(200, 100, build);
  frame(200, 100, build);
  ASSERT_TRUE(ImGui::GetIO().WantTextInput);
  //Unicode code points: an a umlaut, a euro sign and one beyond the BMP (a gem stone)
  ImGui_ImplCGE_Platform_AddChar('h');
  ImGui_ImplCGE_Platform_AddChar(0xE4);
  ImGui_ImplCGE_Platform_AddChar(0x20AC);
  ImGui_ImplCGE_Platform_AddChar(0x1F48E);
  //control characters aren't text
  ImGui_ImplCGE_Platform_AddChar('\r');
  frame(200, 100, build);
  EXPECT_STREQ(text, "h\xC3\xA4\xE2\x82\xAC\xF0\x9F\x92\x8E");
  //the engine keeps the keyboard from the application then
  EXPECT_TRUE(ImGui::GetIO().WantCaptureKeyboard);
}

TEST_F(ImGuiTest, ThePrimaryFingerIsImGuisMouse){
  Input::Touch::init();
  Input::Touch* touch = Input::Touch::instance();
  touch->down(5, 10, 20);
  touch->down(6, 40, 40);
  frame(64, 48, [](){
    ImGuiIO& io = ImGui::GetIO();
    EXPECT_EQ(io.MouseSource, ImGuiMouseSource_TouchScreen);
    EXPECT_FLOAT_EQ(io.MousePos.x, 10);
    EXPECT_FLOAT_EQ(io.MousePos.y, 20);
    EXPECT_TRUE(io.MouseDown[ImGuiMouseButton_Left]);
  });
  //the second finger isn't the mouse
  touch->move(6, 30, 30);
  touch->move(5, 12, 22);
  frame(64, 48, [](){
    EXPECT_FLOAT_EQ(ImGui::GetIO().MousePos.x, 12);
    EXPECT_FLOAT_EQ(ImGui::GetIO().MousePos.y, 22);
  });
  //lifted, the finger doesn't keep hovering
  touch->up(5, 12, 22);
  frame(64, 48, [](){
    EXPECT_FALSE(ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]);
    EXPECT_FALSE(ImGui::IsMousePosValid());
  });
  //a mouse afterwards is a mouse again
  ImGui_ImplCGE_Platform_AddMousePos(3, 4);
  frame(64, 48, [](){
    EXPECT_EQ(ImGui::GetIO().MouseSource, ImGuiMouseSource_Mouse);
  });
  Input::Touch::release();
}

TEST_F(ImGuiTest, RendererCreatesTheFontTextureAndDraws){
  NullDevice& device = device_;
  ASSERT_TRUE(device.beginFrame());
  ImDrawData* data = frame(64, 48, [](){
    ImGui::SetNextWindowSize(ImVec2(64, 48));
    ImGui::Begin("window");
    ImGui::Text("hello");
    ImGui::End();
  });
  ASSERT_GT(data->TotalVtxCount, 0);
  //outside a pass only the textures are handled
  ImGui_ImplCGE_Render_RenderDrawData(data);
  ImTextureData* atlas = ImGui::GetIO().Fonts->TexData;
  EXPECT_EQ(atlas->Status, ImTextureStatus_OK);
  EXPECT_NE(atlas->GetTexID(), ImTextureID_Invalid);
  EXPECT_EQ(device.getDrawCount(), 0);
  ASSERT_TRUE(device.beginPass(PassDesc()));
  ImGui_ImplCGE_Render_RenderDrawData(data);
  device.endPass();
  device.endFrame();
  EXPECT_GT(device.getDrawCount(), 0);
  //the textures go with the backend (and the next one is asked to create them again)
  ImGui_ImplCGE_Render_Shutdown();
  EXPECT_EQ(atlas->TexID, ImTextureID_Invalid);
  EXPECT_EQ(atlas->BackendUserData, nullptr);
  EXPECT_EQ(atlas->Status, ImTextureStatus_WantCreate);
}

// --- the console ---

namespace{

std::vector<std::string> processed;
std::vector<std::string> special;

void processLine(const char* line){
  processed.push_back(line);
}

void specialLine(const std::string& line, int mode, void* data){
  special.push_back(line + " " + std::to_string(mode));
}

}

TEST(Console, SplitsTextAtNewlines){
  Console cons;
  size_t start = cons.getLines().size();
  cons.addText("one\ntwo\n");
  cons << std::string("three");
  ASSERT_EQ(cons.getLines().size(), start+3);
  EXPECT_EQ(cons.getLines()[start], "one");
  EXPECT_EQ(cons.getLines()[start+1], "two");
  EXPECT_EQ(cons.getLines()[start+2], "three");
}

TEST(Console, EnteredLinesGoToTheCallbacks){
  processed.clear();
  special.clear();
  Console cons;
  cons.setCbFunc(processLine);
  cons.setSpecialFunc(specialLine, 7, nullptr);
  cons.enter("first");
  cons.enter("second");
  EXPECT_TRUE(processed.empty());
  cons.update();
  //the special callback takes one line, the main callback the others
  ASSERT_EQ(special.size(), 1u);
  EXPECT_EQ(special[0], "first 7");
  ASSERT_EQ(processed.size(), 1u);
  EXPECT_EQ(processed[0], "second");
  EXPECT_TRUE(cons.hasNewContent());
  EXPECT_EQ(cons.getText(), "second");
  EXPECT_FALSE(cons.hasNewContent());
  EXPECT_EQ(cons.getLines().back(), "second");
}

TEST(Console, PasswordsArentShown){
  Console cons;
  cons.setPasswordInput(true);
  size_t lines = cons.getLines().size();
  cons.enter("secret");
  cons.update();
  EXPECT_EQ(cons.getLines().size(), lines);
  EXPECT_EQ(cons.getText(), "secret");
}

#if defined(CGE_TEST_WINDOW)

namespace{

using TestEnv::TestWindow;

struct Pixel{
  uint8_t r, g, b, a;
};

//! collects the errors and warnings that are traced
class ProblemCollector : public CGE::TraceOutputter{
public:
  bool init() override {return true;}
  void trace(unsigned channel, int level, const char* function, const char* message) override{
    if (level <= TRACE_WARNING)
      problems.push_back(message);
  }
  std::vector<std::string> problems;
};

const int TARGET_SIZE = 32;

class ImGuiGL : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("OpenGL");
    ASSERT_NE(device_, nullptr);
    DeviceDesc desc;
    desc.debug = true;
    if (!window_.create(*device_, desc))
      GTEST_SKIP() << "no window";
    if (!device_->init(window_.native(), desc))
      GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
    RenderTargetDesc rt;
    rt.width = rt.height = TARGET_SIZE;
    rt.depthFormat = Format::Undefined;
    target_ = device_->createRenderTarget(rt);
    ASSERT_NE(target_, nullptr);
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ASSERT_TRUE(ImGui_ImplCGE_Platform_Init(nullptr));
    ASSERT_TRUE(ImGui_ImplCGE_Render_Init(*device_));
    CGE::TraceManager::instance()->setTraceOutputter(&collector_);
    collector_.problems.clear();
    collecting_ = true;
  }

  void TearDown() override{
    if (ImGui::GetCurrentContext()){
      ImGui_ImplCGE_Render_Shutdown();
      ImGui_ImplCGE_Platform_Shutdown();
      ImGui::DestroyContext();
    }
    target_.reset();
    if (collecting_){
      CGE::TraceManager::instance()->removeTraceOutputter(&collector_);
      for (const std::string& problem : collector_.problems)
        ADD_FAILURE() << "traced: " << problem;
    }
  }

  //! draws what draw() adds to the foreground draw list into the black target
  /*! Returns the pixels, rows from the bottom up. */
  template<typename Draw>
  std::vector<Pixel> render(Draw draw){
    ImGui::GetIO().DisplaySize = ImVec2((float)TARGET_SIZE, (float)TARGET_SIZE);
    ImGui_ImplCGE_Render_NewFrame();
    ImGui_ImplCGE_Platform_NewFrame();
    ImGui::NewFrame();
    draw(ImGui::GetForegroundDrawList());
    ImGui::Render();
    std::vector<Pixel> pixels(TARGET_SIZE*TARGET_SIZE);
    EXPECT_TRUE(device_->beginFrame());
    PassDesc pass;
    pass.target = target_.get();
    pass.clearColor = {0, 0, 0, 1};
    EXPECT_TRUE(device_->beginPass(pass));
    ImGui_ImplCGE_Render_RenderDrawData(ImGui::GetDrawData());
    device_->endPass();
    EXPECT_TRUE(device_->readPixels(target_.get(), 0, {0, 0, TARGET_SIZE, TARGET_SIZE}, pixels.data()));
    device_->endFrame();
    return pixels;
  }

  //! the pixel at (x, y) in ImGui's coordinates (y down)
  static const Pixel& at(const std::vector<Pixel>& pixels, int x, int y){
    return pixels[(TARGET_SIZE-1-y)*TARGET_SIZE + x];
  }

  //declared first, so the device goes before the window
  TestWindow window_;
  std::unique_ptr<RenderDevice> device_;
  std::unique_ptr<Gfx::RenderTarget> target_;
  ProblemCollector collector_;
  bool collecting_ = false;
};

::testing::AssertionResult isColor(const Pixel& p, int r, int g, int b){
  if (abs(p.r-r) <= 2 && abs(p.g-g) <= 2 && abs(p.b-b) <= 2)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "pixel (" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b
    << ") instead of (" << r << ", " << g << ", " << b << ")";
}

}

TEST_F(ImGuiGL, RectanglesAreUprightInTheirColour){
  //the upper half red, a blended blue square in the lower left
  std::vector<Pixel> pixels = render([](ImDrawList* list){
    list->AddRectFilled(ImVec2(0, 0), ImVec2(TARGET_SIZE, TARGET_SIZE/2), IM_COL32(255, 0, 0, 255));
    list->AddRectFilled(ImVec2(0, TARGET_SIZE/2), ImVec2(8, TARGET_SIZE), IM_COL32(0, 0, 255, 128));
  });
  EXPECT_TRUE(isColor(at(pixels, 4, 4), 255, 0, 0));
  EXPECT_TRUE(isColor(at(pixels, 28, 12), 255, 0, 0));
  EXPECT_TRUE(isColor(at(pixels, 20, 24), 0, 0, 0));
  EXPECT_TRUE(isColor(at(pixels, 4, 24), 0, 0, 128));
}

TEST_F(ImGuiGL, DrawsAreClippedToTheirClipRect){
  std::vector<Pixel> pixels = render([](ImDrawList* list){
    list->PushClipRect(ImVec2(8, 0), ImVec2(16, 8));
    list->AddRectFilled(ImVec2(0, 0), ImVec2(TARGET_SIZE, TARGET_SIZE), IM_COL32(0, 255, 0, 255));
    list->PopClipRect();
  });
  EXPECT_TRUE(isColor(at(pixels, 12, 4), 0, 255, 0));
  EXPECT_TRUE(isColor(at(pixels, 4, 4), 0, 0, 0));
  EXPECT_TRUE(isColor(at(pixels, 20, 4), 0, 0, 0));
  EXPECT_TRUE(isColor(at(pixels, 12, 12), 0, 0, 0));
}

TEST_F(ImGuiGL, ImagesShowDeviceTextures){
  //2x1: red left, green right
  const uint8_t texels[] = {255, 0, 0, 255, 0, 255, 0, 255};
  TextureDesc desc;
  desc.width = 2;
  desc.height = 1;
  std::unique_ptr<Texture> texture = device_->createTexture(desc, texels);
  ASSERT_NE(texture, nullptr);
  std::vector<Pixel> pixels = render([&](ImDrawList* list){
    list->AddImage(ImGui_ImplCGE_TextureID(texture.get()), ImVec2(0, 0), ImVec2(TARGET_SIZE, TARGET_SIZE));
  });
  //sampled linearly: pure colours near the edges
  EXPECT_TRUE(isColor(at(pixels, 1, 16), 255, 0, 0));
  EXPECT_TRUE(isColor(at(pixels, 30, 16), 0, 255, 0));
}

TEST_F(ImGuiGL, WindowsWithTextAreDrawn){
  std::vector<Pixel> pixels = render([](ImDrawList*){
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(TARGET_SIZE, TARGET_SIZE));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 255, 255));
    ImGui::Begin("window", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::Text("WW");
    ImGui::End();
    ImGui::PopStyleColor();
  });
  //the font atlas exists on the device now
  EXPECT_EQ(ImGui::GetIO().Fonts->TexData->Status, ImTextureStatus_OK);
  EXPECT_TRUE(isColor(at(pixels, TARGET_SIZE-3, TARGET_SIZE-3), 0, 0, 255));
  //some of the text's pixels are light
  int light = 0;
  for (const Pixel& p : pixels){
    if (p.r > 128 && p.g > 128)
      light++;
  }
  EXPECT_GT(light, 5);
}


#endif
