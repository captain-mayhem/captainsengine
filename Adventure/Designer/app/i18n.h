#ifndef SD_I18N_H
#define SD_I18N_H

#include <string>

// The StoryDesigner's own texts in the language chosen in its Language menu:
// English, or German where the C# StoryDesigner has them in German (its main
// menu, the pools, the project settings and the messages). The English text
// is the key of its translation.

namespace sd{

enum class UiLanguage{
  English,
  German,
};

void setUiLanguage(UiLanguage language);
UiLanguage uiLanguage();
//! the language of the system's first language that has one, else English
UiLanguage systemUiLanguage();
//! a text in the language; an ImGui id after "##" is kept as it is
const char* tr(const char* text);
//! a window's title in the language, with the English title as its ImGui id
//! in every language (so the window keeps its place in the layout)
const char* windowTitle(const char* title);

}

#endif
