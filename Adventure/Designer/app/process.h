#ifndef SD_PROCESS_H
#define SD_PROCESS_H

#include <string>
#include <vector>

// Other programs the StoryDesigner starts: the game it made, and the video
// converter. Paths are UTF-8.

namespace sd{

//! starts a program in a folder without waiting for it; false and why if it can't be started
/*! On Linux the folder is also where the program's libraries are found
    (LD_LIBRARY_PATH), as the game's runtime has them next to it. */
bool startProgram(const std::string& program, const std::vector<std::string>& args, const std::string& folder, std::string* error = nullptr);
//! runs a program, without a window, until it ends; its exit code (-1 if it can't be started) and what it wrote
int runProgram(const std::string& program, const std::vector<std::string>& args, std::string& output);
//! the folder of the StoryDesigner's program
std::string programFolder();

}

#endif
