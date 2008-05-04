//  ==================================================================
// |                         Hero Engine                              |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-03-25                                                       |
// |------------------------------------------------------------------|
// | $Id$:                                                    |
//  ==================================================================

namespace Gui{
  class GuiElement;
}
class AdventureCore;
class SQL;

class Application{
public:
  ~Application();
  static Application* instance() {return instance_;}
  static void init();
  static void rendererInit() {instance_->_rendererInit();}
  static void render() {instance_->_render();}
  static void mouseDown(int x, int y, int button) {instance_->_mouseDown(x,y,button);}
  void setupMainGui();
  AdventureCore* core() {return core_;}
private:
  Application();
  static Application* instance_;
  static void gotoButton(Gui::GuiElement* elem);
  void _rendererInit();
  void _render();
  void _mouseDown(int x, int y, int button);

  AdventureCore* core_;
  SQL* sql_;
};