#include "settings.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <locale>
#include <map>
#include <sstream>
#include <system_error>

#include "../io/Tracing.h"
#include "userdirs.h"

using namespace CGE;

TR_CHANNEL(CGE_Settings);

namespace{

struct Value{
  bool binary = false;
  std::string text;
  std::vector<uint8_t> data;
};

typedef std::map<std::string, Value> Group;

//! splits "a/b/name" into the group "a/b" and the name
void splitKey(const std::string& key, std::string& group, std::string& name){
  size_t pos = key.find_last_of('/');
  if (pos == std::string::npos){
    group.clear();
    name = key;
  }
  else{
    group = key.substr(0, pos);
    name = key.substr(pos + 1);
  }
}

std::string escape(const std::string& s){
  std::string out;
  out.reserve(s.size());
  for (char ch : s){
    if (ch == '\\')
      out += "\\\\";
    else if (ch == '\n')
      out += "\\n";
    else if (ch == '\r')
      out += "\\r";
    else
      out += ch;
  }
  return out;
}

std::string unescape(const std::string& s){
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); ++i){
    if (s[i] == '\\' && i + 1 < s.size()){
      char ch = s[++i];
      out += ch == 'n' ? '\n' : ch == 'r' ? '\r' : ch;
    }
    else
      out += s[i];
  }
  return out;
}

std::string toHex(const std::vector<uint8_t>& data){
  static const char digits[] = "0123456789abcdef";
  std::string out;
  out.reserve(data.size() * 2);
  for (uint8_t byte : data){
    out += digits[byte >> 4];
    out += digits[byte & 0xf];
  }
  return out;
}

int hexDigit(char ch){
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 10;
  return -1;
}

std::vector<uint8_t> fromHex(const std::string& hex){
  std::vector<uint8_t> out;
  out.reserve(hex.size() / 2);
  for (size_t i = 0; i + 1 < hex.size(); i += 2){
    int hi = hexDigit(hex[i]);
    int lo = hexDigit(hex[i + 1]);
    if (hi < 0 || lo < 0)
      break;
    out.push_back((uint8_t)((hi << 4) | lo));
  }
  return out;
}

//! number conversions independent of the C locale
template<typename T>
bool parseNumber(const std::string& text, T& value){
  std::istringstream in(text);
  in.imbue(std::locale::classic());
  T tmp;
  in >> tmp;
  if (in.fail())
    return false;
  in >> std::ws;
  if (!in.eof())
    return false;
  value = tmp;
  return true;
}

template<typename T>
std::string formatNumber(T value, int precision = 0){
  std::ostringstream out;
  out.imbue(std::locale::classic());
  if (precision > 0)
    out.precision(precision);
  out << value;
  return out.str();
}

//! ASCII only, independent of the C locale
std::string lower(std::string s){
  for (char& ch : s){
    if (ch >= 'A' && ch <= 'Z')
      ch = (char)(ch - 'A' + 'a');
  }
  return s;
}

std::unique_ptr<Settings> userSettings;
std::string userSettingsFile;

}

struct Settings::Impl{
  //! group path -> values; the top level is ""
  std::map<std::string, Group> groups;
  bool modified = false;

  const Value* find(const std::string& key) const{
    std::string group, name;
    splitKey(key, group, name);
    auto git = groups.find(group);
    if (git == groups.end())
      return nullptr;
    auto vit = git->second.find(name);
    return vit == git->second.end() ? nullptr : &vit->second;
  }

  Value& create(const std::string& key){
    std::string group, name;
    splitKey(key, group, name);
    modified = true;
    return groups[group][name];
  }
};

Settings::Settings() : mImpl(std::make_unique<Impl>()){
}

Settings::~Settings(){
}

bool Settings::load(const std::string& filename){
  clear();
  mImpl->modified = false;
  FILE* f = fopen(filename.c_str(), "rb");
  if (!f)
    return false;
  std::string content;
  char buffer[4096];
  size_t read;
  while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0)
    content.append(buffer, read);
  fclose(f);

  std::string group;
  size_t pos = 0;
  while (pos < content.size()){
    size_t end = content.find('\n', pos);
    if (end == std::string::npos)
      end = content.size();
    std::string line = content.substr(pos, end - pos);
    pos = end + 1;
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.empty() || line[0] == ';' || line[0] == '#')
      continue;
    if (line[0] == '[' && line.back() == ']'){
      group = line.substr(1, line.size() - 2);
      continue;
    }
    size_t eq = line.find('=');
    if (eq == std::string::npos)
      continue;
    std::string name = line.substr(0, eq);
    std::string text = line.substr(eq + 1);
    static const std::string binSuffix = ":bin";
    Value value;
    if (name.size() > binSuffix.size() && name.compare(name.size() - binSuffix.size(), binSuffix.size(), binSuffix) == 0){
      name.erase(name.size() - binSuffix.size());
      value.binary = true;
      value.data = fromHex(text);
    }
    else
      value.text = unescape(text);
    mImpl->groups[group][name] = std::move(value);
  }
  mImpl->modified = false;
  return true;
}

bool Settings::save(const std::string& filename){
  TR_USE(CGE_Settings);
  std::string content;
  for (const auto& group : mImpl->groups){
    if (group.second.empty())
      continue;
    if (!group.first.empty())
      content += "[" + group.first + "]\n";
    for (const auto& value : group.second){
      if (value.second.binary)
        content += value.first + ":bin=" + toHex(value.second.data) + "\n";
      else
        content += value.first + "=" + escape(value.second.text) + "\n";
    }
  }

  std::string tmpname = filename + ".tmp";
  FILE* f = fopen(tmpname.c_str(), "wb");
  if (!f){
    TR_ERROR("cannot write %s", tmpname.c_str());
    return false;
  }
  bool ok = fwrite(content.data(), 1, content.size(), f) == content.size();
  ok = fclose(f) == 0 && ok;
  std::error_code err;
  if (ok)
    std::filesystem::rename(std::filesystem::path(tmpname), std::filesystem::path(filename), err);
  if (!ok || err){
    TR_ERROR("cannot write %s", filename.c_str());
    std::filesystem::remove(std::filesystem::path(tmpname), err);
    return false;
  }
  mImpl->modified = false;
  return true;
}

void Settings::clear(){
  if (!mImpl->groups.empty())
    mImpl->modified = true;
  mImpl->groups.clear();
}

bool Settings::has(const std::string& key) const{
  return mImpl->find(key) != nullptr;
}

std::string Settings::getString(const std::string& key, const std::string& def) const{
  const Value* value = mImpl->find(key);
  if (!value || value->binary)
    return def;
  return value->text;
}

int64_t Settings::getInt(const std::string& key, int64_t def) const{
  const Value* value = mImpl->find(key);
  int64_t ret = def;
  if (value && !value->binary && !parseNumber(value->text, ret))
    ret = def;
  return ret;
}

double Settings::getFloat(const std::string& key, double def) const{
  const Value* value = mImpl->find(key);
  double ret = def;
  if (value && !value->binary && !parseNumber(value->text, ret))
    ret = def;
  return ret;
}

bool Settings::getBool(const std::string& key, bool def) const{
  const Value* value = mImpl->find(key);
  if (!value || value->binary)
    return def;
  std::string text = lower(value->text);
  if (text == "true" || text == "yes" || text == "on" || text == "1")
    return true;
  if (text == "false" || text == "no" || text == "off" || text == "0")
    return false;
  return def;
}

std::vector<uint8_t> Settings::getBinary(const std::string& key) const{
  const Value* value = mImpl->find(key);
  if (!value || !value->binary)
    return std::vector<uint8_t>();
  return value->data;
}

void Settings::setString(const std::string& key, const std::string& text){
  const Value* old = mImpl->find(key);
  if (old && !old->binary && old->text == text)
    return;
  Value& value = mImpl->create(key);
  value.binary = false;
  value.text = text;
  value.data.clear();
}

void Settings::setInt(const std::string& key, int64_t value){
  setString(key, formatNumber(value));
}

void Settings::setFloat(const std::string& key, double value){
  //17 significant digits read back as the same double
  setString(key, formatNumber(value, 17));
}

void Settings::setBool(const std::string& key, bool value){
  setString(key, value ? "true" : "false");
}

void Settings::setBinary(const std::string& key, const std::vector<uint8_t>& data){
  const Value* old = mImpl->find(key);
  if (old && old->binary && old->data == data)
    return;
  Value& value = mImpl->create(key);
  value.binary = true;
  value.text.clear();
  value.data = data;
}

void Settings::remove(const std::string& key){
  std::string group, name;
  splitKey(key, group, name);
  auto git = mImpl->groups.find(group);
  if (git == mImpl->groups.end())
    return;
  if (git->second.erase(name) > 0)
    mImpl->modified = true;
  if (git->second.empty())
    mImpl->groups.erase(git);
}

void Settings::removeGroup(const std::string& group){
  if (group.empty()){
    clear();
    return;
  }
  std::string prefix = group + "/";
  for (auto it = mImpl->groups.begin(); it != mImpl->groups.end();){
    if (it->first == group || it->first.compare(0, prefix.size(), prefix) == 0){
      it = mImpl->groups.erase(it);
      mImpl->modified = true;
    }
    else
      ++it;
  }
}

std::vector<std::string> Settings::getGroups(const std::string& group) const{
  std::vector<std::string> result;
  std::string prefix = group.empty() ? "" : group + "/";
  for (const auto& entry : mImpl->groups){
    const std::string& path = entry.first;
    if (path.size() <= prefix.size() || path.compare(0, prefix.size(), prefix) != 0)
      continue;
    //only the next part; deeper groups make their ancestors visible too
    std::string child = path.substr(prefix.size());
    child = child.substr(0, child.find('/'));
    if (result.empty() || result.back() != child)
      result.push_back(child);
  }
  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

std::vector<std::string> Settings::getKeys(const std::string& group) const{
  std::vector<std::string> result;
  auto git = mImpl->groups.find(group);
  if (git == mImpl->groups.end())
    return result;
  for (const auto& value : git->second)
    result.push_back(value.first);
  return result;
}

bool Settings::isModified() const{
  return mImpl->modified;
}

Settings& Settings::user(){
  TR_USE(CGE_Settings);
  if (!userSettings){
    userSettings = std::make_unique<Settings>();
    userSettingsFile = UserDirs::getConfigDir() + "/settings.ini";
    if (userSettings->load(userSettingsFile))
      TR_INFO("settings loaded from %s", userSettingsFile.c_str());
  }
  return *userSettings;
}

bool Settings::saveUser(){
  if (!userSettings || !userSettings->isModified())
    return true;
  bool ok = userSettings->save(userSettingsFile);
  UserDirs::commit();
  return ok;
}
