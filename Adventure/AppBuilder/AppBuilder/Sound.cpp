#include "Sound.h"

wxWindow* SoundPlayer::mParent = NULL;
int SoundPlayer::mIDCount = 0;

SoundPlayer::SoundPlayer(const std::string file) :
wxMediaCtrl(mParent, ++mIDCount, file, wxDefaultPosition, wxDefaultSize){
  ShowPlayerControls(wxMEDIACTRLPLAYERCONTROLS_NONE);
}

SoundPlayer::~SoundPlayer(){

}