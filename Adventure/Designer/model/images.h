#ifndef SD_IMAGES_H
#define SD_IMAGES_H

#include <memory>
#include <string>
#include <vector>

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

//! where a media file (image, sound, music, video) of the project is: the
//! file the project names, or one of that name in folder or in its media pool
//! folder's subfolder of folder; empty if none is there
std::string findMediaFile(const Project& project, const std::string& folder, ResourceID kind, const std::string& name);
//! the bytes of a media file, found as loadProjectImage finds an image
//! (also in the game's .dat of its kind: gfx, sfx, music or movie); false if none
bool readMediaFile(const Project& project, const std::string& folder, ResourceID kind, const std::string& name, std::vector<char>& data);

//! cuts the fully transparent border off a PNG file (UTF-8 path) and
//! writes it back, as the C# StoryDesigner's "Crop transparent image
//! parts" does when images are added; false and why if it can't
bool cropTransparentImage(const std::string& file, std::string* error = nullptr);
//! the file name of a path with / or \ (a project's paths are Windows')
std::string fileNameOf(const std::string& path);

}

#endif
