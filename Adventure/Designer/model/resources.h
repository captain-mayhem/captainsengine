#ifndef SD_RESOURCES_H
#define SD_RESOURCES_H

#include <string>
#include <vector>

#include "project.h"

// The game pool's and media pool's resources: making, renaming, duplicating
// and deleting them, adding media files, and the pools' folders, by the C#
// StoryDesigner's rules (its Form1 and AdvData).

namespace sd{

//! a node of a pool by the child indices from the pool's top
using PoolPath = std::vector<size_t>;

//! the pools of a new project: the game pool's five folders, the media pool's four
void newPools(Project& project);

//! the pool a kind of resource is in
bool inGamePool(ResourceID kind);
//! the top folder of a kind of resource in its pool (characters 0, scripts 1, ...; images 0, music 1, ...)
int topFolder(ResourceID kind);
//! the kind of resource a pool's node belongs to: its top folder's
ResourceID kindAt(const std::vector<PoolNode>& pool, bool game, const PoolPath& path);

PoolNode* poolNode(std::vector<PoolNode>& pool, const PoolPath& path);
PoolNode* findInPool(std::vector<PoolNode>& pool, ResourceID kind, const std::string& name);
//! the path of a resource's node, empty if it isn't in the pool
PoolPath pathInPool(const std::vector<PoolNode>& pool, ResourceID kind, const std::string& name);
//! the nodes sorted by their labels, as the C# tree view sorts them
void sortPool(std::vector<PoolNode>& nodes);
//! removes a resource's node; true if it was there
bool removeFromPool(std::vector<PoolNode>& pool, ResourceID kind, const std::string& name);

//! a name not taken: base followed by 1, 2, ... (the C# findFreeName)
template<typename T>
std::string freeName(const NamedList<T>& list, const std::string& base){
  for (int count = 1;; ++count){
    std::string test = base + std::to_string(count);
    if (list.find(test) == nullptr)
      return test;
  }
}

//! the folder a new resource goes into: folder if it is one of the
//! resource's folders, else the top folder of the resource's kind
PoolPath targetFolder(Project& project, ResourceID kind, const PoolPath& folder);
//! adds a resource's node into a folder (as targetFolder chooses) and sorts it
void addToPool(Project& project, ResourceID kind, const std::string& name, const PoolPath& folder);

//! a new item, object, character, room or cutscene with the C# defaults; its name
std::string newResource(Project& project, ResourceID kind, const PoolPath& folder = PoolPath());
//! a copy of a game resource under a free name (with its scripts, as the C# copies them); its name, empty if it can't
std::string duplicateResource(Project& project, ResourceID kind, const std::string& name, const PoolPath& folder = PoolPath());
//! renames a game resource and what refers to it by name (scripts, placed objects and characters); false and why if it can't
bool renameResource(Project& project, ResourceID kind, const std::string& from, const std::string& to, std::string* error = nullptr);
//! deletes a resource and what belongs to it (its scripts, its placed objects and characters)
void deleteResource(Project& project, ResourceID kind, const std::string& name);
//! deletes a pool's node: a resource, or a folder with all it holds
void deletePoolNode(Project& project, bool game, const PoolPath& path);

//! a new folder in the folder of path (or in path, if it is a folder); its path
PoolPath newFolder(Project& project, bool game, const PoolPath& path);
//! renames a folder; false if a folder next to it has the name
bool renameFolder(Project& project, bool game, const PoolPath& path, const std::string& to);
//! moves a node into a folder of the same kind (not into itself); false if it can't
bool movePoolNode(Project& project, bool game, const PoolPath& from, const PoolPath& toFolder);

//! the media of a kind (images, sounds, music, videos), null for other kinds
NamedList<std::string>* mediaFiles(Project& project, ResourceID kind);
const NamedList<std::string>* mediaFiles(const Project& project, ResourceID kind);
//! adds a media file under its name without extension; false and why if the name is taken
bool addMedia(Project& project, ResourceID kind, const std::string& file, const PoolPath& folder = PoolPath(), std::string* error = nullptr);

//! the media files that aren't where the project says, found in folder
//! and its subfolders by their file names (the C# resolveMedia); false if
//! some are still missing
bool resolveMedia(Project& project, const std::string& folder);

}

#endif
