#ifndef SD_LANGUAGES_H
#define SD_LANGUAGES_H

#include "project.h"

namespace sd{

//! the language "origin" again from the scripts and the commands, as the C#
//! StoryDesigner does before showing its language tools: new texts are added,
//! texts no script uses any more leave origin and the translations
void updateLanguageList(Project& project);

}

#endif
