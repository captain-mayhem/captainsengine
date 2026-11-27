#ifndef SD_IMAGES_H
#define SD_IMAGES_H

#include <memory>
#include <string>

#include <image/image.h>

#include "project.h"

namespace sd{

//! reads an image of the project for the editor's previews, as RGBA
/*! Tries the file the project names, then a file of the same name in
    folder, then in folder's subfolder named as the image's media pool folder
    (as an unpacked .adz has them), then the entry of that name in folder's
    gfx.dat (the game's images, with the project's password); a .pnj (a JPEG) takes its alpha from
    the .pna of the same name, as in the game. Null if it isn't found or can't
    be read. */
std::unique_ptr<CGE::Image> loadProjectImage(const Project& project, const std::string& folder, const std::string& name);

//! the file name of a path with / or \ (a project's paths are Windows')
std::string fileNameOf(const std::string& path);

}

#endif
