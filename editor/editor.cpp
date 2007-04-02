
#include "editor.h"

Editor* Editor::edi_ = NULL;

Editor::Editor(){
}

Editor::~Editor(){
}

void Editor::init(){
  edi_ = new Editor();
}
