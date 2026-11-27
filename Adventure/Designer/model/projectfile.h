#ifndef SD_PROJECTFILE_H
#define SD_PROJECTFILE_H

#include <string>
#include <utility>
#include <vector>

#include "project.h"

// Reading and writing project files: an .adv project or a game's game.dat, a
// zip archive of the text entries <name>.001 (settings, pools, media),
// .002 (cursor, items, objects, characters, rooms), .003 (scripts),
// .004 (languages), .005 (the data files' password, if protected) and .010
// (advanced settings). This is the C# StoryDesigner's reader and writer,
// ported line by line: a project saved here is what it saved.

namespace sd{

//! reads a project file; error gets why it failed
bool readProject(const std::string& filename, Project& project, std::string* error = nullptr);
//! reads a project file from memory; filename names the project
bool readProject(const void* data, size_t size, const std::string& filename, Project& project, std::string* error = nullptr);

//! reads the resources of a character's or room's export (.adc's char.dat,
//! .adr's room.dat: the .002 entry's list without its header, in the
//! oldest version) into a project; createdObjects gets the objects made for
//! placed objects the project hasn't got
bool importObjects(const std::string& text, Project& project, std::vector<std::string>* createdObjects, std::string* error = nullptr);
//! reads the scripts of a room's export (script.dat: the .003 entry without its header)
bool importScripts(const std::string& text, Project& project, std::string* error = nullptr);

//! the entries of a project file, by their extensions (".001", ...)
/*! dat: written as a game's game.dat (PNG images named as JPEGs if the
    project converts them). A protected project without a password gets one. */
std::vector<std::pair<std::string, std::string>> writeProjectEntries(Project& project, bool dat);
//! writes a project file (.adv or game.dat; the entries are named after the file)
bool writeProject(const std::string& filename, Project& project, std::string* error = nullptr);

}

#endif
