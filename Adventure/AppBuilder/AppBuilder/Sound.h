#include <string>

#include <wx/mediactrl.h>

class SoundPlayer : wxMediaCtrl{
public:
  SoundPlayer(const std::string file);
  ~SoundPlayer();
  static void setParent(wxWindow* wnd) {mParent = wnd;}
  void play() {Play();}
protected:
  static wxWindow* mParent;
  static int mIDCount;
};