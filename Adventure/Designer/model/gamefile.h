#ifndef SD_GAMEFILE_H
#define SD_GAMEFILE_H

#include <functional>
#include <string>
#include <vector>

#include "project.h"

// What the C# StoryDesigner's AdvFileWriter and AdvFileReader make of a
// project besides its project file: the game (Create game), the export
// (.adz) and its unpacking, and the imports of a character (.adc) and a
// room (.adr).

namespace sd{

//! how a game is made
struct GameOptions{
  //! where engine1.dat (the Adventure and the engine's libraries),
  //! engine2.dat (the engine's fonts) and font.dat (the game's own font) are:
  //! the StoryDesigner's folder
  std::string runtimeFolder;
  //! the video the game plays for a video file (converted if the engine can't play it); empty and why if it fails
  std::function<std::string(const std::string& file, std::string& error)> playableVideo;
  //! called with the step being done, for a progress display
  std::function<void(const std::string& step)> progress;
};

//! Create game: the game's data in gamePath's folder's data/ (game.dat,
//! fonts.dat, font.dat, gfx.dat, sfx.dat, music.dat, movie.dat and the
//! loading image) and the runtime next to it, its program named as gamePath.
//! The media are found as the editor finds them (folder is the project's).
//! A protected project gets its password if it has none. warnings gets what
//! was left out; false and why if the game can't be written.
bool writeGame(Project& project, const std::string& folder, const std::string& gamePath, const GameOptions& options,
  std::vector<std::string>& warnings, std::string* error = nullptr);

//! the game's data for running the project as it is: the project file
//! (named after the project, its media at the paths found) with fonts.dat and
//! font.dat in dataFolder; the project file's path, empty on failure
std::string writeTestGame(Project& project, const std::string& folder, const std::string& dataFolder, const GameOptions& options,
  std::vector<std::string>& warnings, std::string* error = nullptr);

//! unpacks a runtime archive (engine1.dat, engine2.dat) into a folder; the
//! program (the entry ending in .exe, or called Adventure) gets the name program
//! unless it is empty
bool unpackRuntime(const std::string& archive, const std::string& folder, const std::string& program, std::string* error = nullptr);

//! the export (.adz): the project file and its media, in zips by the media
//! pool's folders, and the game icon
bool writeExport(Project& project, const std::string& folder, const std::string& adzPath, std::vector<std::string>& warnings, std::string* error = nullptr);
//! the folder an export is unpacked to by default: next to it, named as it
std::string defaultUnpackFolder(const std::string& adzPath);
//! unpacks an export into destFolder: its project file, and its media in
//! folders by the media pool's folders; the project file's path, empty on failure
std::string unpackAdz(const std::string& adzPath, const std::string& destFolder, std::string* error = nullptr);

//! imports a character export (.adc): the character, and its images into
//! folder/<the file's name>/; false and why if it can't be read
bool importCharacter(Project& project, const std::string& adcPath, const std::string& folder, std::string* error = nullptr);
//! imports a room export (.adr): the room with its objects and scripts, and its images
bool importRoom(Project& project, const std::string& adrPath, const std::string& folder, std::string* error = nullptr);

}

#endif
