#ifndef SD_FILEDIALOG_H
#define SD_FILEDIALOG_H

#include <string>
#include <vector>

namespace sd{

//! an ImGui window to choose a file to open or a name to save as
class FileDialog{
public:
  enum class Mode{
    Open,
    Save,
  };
  //! opens the dialog in folder, for files with one of the extensions (".adv")
  /*! multiple: several files can be chosen to open, with Ctrl (paths()). */
  void open(Mode mode, const std::string& title, const std::string& folder, const std::vector<std::string>& extensions, bool multiple = false);
  bool isOpen() const {return open_;}
  //! the file name the dialog starts with (UTF-8)
  void setName(const std::string& name);
  //! draws the dialog; true when a file was chosen (path())
  bool draw();
  //! the chosen file (UTF-8)
  const std::string& path() const {return path_;}
  //! the chosen files, for a dialog that chooses several
  const std::vector<std::string>& paths() const {return paths_;}
  //! the folder shown (UTF-8)
  const std::string& folder() const {return folder_;}
private:
  void list();
  bool matches(const std::string& name) const;

  bool open_ = false;
  bool justOpened_ = false;
  Mode mode_ = Mode::Open;
  std::string title_;
  std::string folder_;
  std::vector<std::string> extensions_;
  std::vector<std::string> dirs_;
  std::vector<std::string> files_;
  char name_[512] = "";
  std::string path_;
  bool multiple_ = false;
  //! the files chosen with Ctrl
  std::vector<std::string> chosen_;
  std::vector<std::string> paths_;
};

}

#endif
