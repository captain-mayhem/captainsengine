#ifndef CGE_SETTINGS_H
#define CGE_SETTINGS_H

#include <cge_export.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace CGE{

//! A persistent key/value store kept in an INI-style text file
/*! Keys are paths whose last part is the value's name and whose other parts
    name nested groups, separated by '/': "volume", "audio/music",
    "profiles/Plain Text/font". Groups can be listed (getGroups) and removed
    with everything below them (removeGroup).

    The file has one section per group with values, plus "name=value" lines:

        volume=80
        [audio]
        music=0.5
        [profiles/Plain Text]
        font=Georgia
        swatches:bin=ff0000ff00ff

    Values are stored as text. "\", line breaks and carriage returns in
    values are escaped with a backslash; binary values are hex-encoded and
    marked with ":bin". Names must not contain '=', '/' or line breaks.

    Settings::user() is the application's own store; see there. */
class CGE_API Settings{
public:
  Settings();
  ~Settings();
  Settings(const Settings&) = delete;
  Settings& operator=(const Settings&) = delete;

  //! replaces the contents with the file's; false (and empty) if it can't be read
  bool load(const std::string& filename);
  //! writes the contents to a file (through a temporary file, so an
  //! interrupted save leaves the old file intact)
  bool save(const std::string& filename);
  //! removes all values
  void clear();

  //! is there a value with this key
  bool has(const std::string& key) const;
  std::string getString(const std::string& key, const std::string& def = "") const;
  int64_t getInt(const std::string& key, int64_t def = 0) const;
  double getFloat(const std::string& key, double def = 0) const;
  //! true, yes, on and 1 are true; false, no, off and 0 are false
  bool getBool(const std::string& key, bool def = false) const;
  //! empty if there is no binary value with this key
  std::vector<uint8_t> getBinary(const std::string& key) const;

  void setString(const std::string& key, const std::string& value);
  void setInt(const std::string& key, int64_t value);
  void setFloat(const std::string& key, double value);
  void setBool(const std::string& key, bool value);
  void setBinary(const std::string& key, const std::vector<uint8_t>& value);

  //! removes one value
  void remove(const std::string& key);
  //! removes a group with all its values and the groups below it
  void removeGroup(const std::string& group);
  //! the names of the groups directly below a group ("" is the top level), sorted
  std::vector<std::string> getGroups(const std::string& group = "") const;
  //! the names of the values directly in a group, sorted
  std::vector<std::string> getKeys(const std::string& group = "") const;

  //! changed since the last load or save
  bool isModified() const;

  //! the application's settings, loaded on first use from settings.ini in UserDirs::getConfigDir()
  /*! The engine saves them (saveUser) when the application goes into the
      background and when it shuts down, if they were changed. */
  static Settings& user();
  //! saves the application's settings now if they were changed, and commits them (UserDirs::commit)
  static bool saveUser();
private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}

#endif
