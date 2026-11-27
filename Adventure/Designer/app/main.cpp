// The StoryDesigner: the Adventure's editor, on the engine's ImGui with
// docked windows. "Designer <project.adv>" opens a project at the start.

#include <imgui.h>

#include <renderer/renderdevice.h>
#include <system/application.h>
#include <system/engine.h>
#include <window/window.h>

#include "designer.h"

class DesignerApp : public CGE::IApplication{
public:
  void init(int argc, char** argv) override{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //the layout is made at the start (Designer::layout), not kept
    io.IniFilename = nullptr;
    if (argc > 1)
      designer_.openProject(argv[1]);
    updateTitle();
  }

  void render() override{
    CGE::Engine* engine = CGE::Engine::instance();
    CGE::Gfx::RenderDevice* device = engine->getRenderDevice();
    CGE::Gfx::PassDesc pass;
    pass.clearColor = {0.12f, 0.12f, 0.13f, 1.0f};
    if (device->beginPass(pass))
      device->endPass();
    designer_.gui();
    updateTitle();
    if (designer_.quitRequested())
      engine->requestShutdown();
  }

  //! Escape belongs to the dialogs, the main menu quits
  void onKeyDown(int key) override {}

private:
  void updateTitle(){
    std::string title = "StoryDesigner - " + sd::toUtf8(designer_.project().settings.projectName);
    if (title != title_){
      title_ = title;
      if (CGE::AppWindow* win = CGE::Engine::instance()->getWindow())
        win->setTitle(title_);
    }
  }

  sd::Designer designer_;
  std::string title_;
};

CGE_MAIN(DesignerApp)
