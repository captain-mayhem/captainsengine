#ifndef CGE_JSON_H
#define CGE_JSON_H

#include <cge_export.h>

#include <string>
#include <utility>
#include <vector>

namespace CGE{

//! A parsed JSON document (RFC 8259), read-only
/*! For the engine's data files, e.g. texture atlases (TextureAtlas). Reading
    a member or an element that doesn't exist, or a value as the wrong type,
    gives a null value or the default, so files can be read without checking
    every step. Objects keep their members in the file's order. */
class CGE_API JsonValue{
public:
  enum Type{
    Null,
    Bool,
    Number,
    String,
    Array,
    Object,
  };

  //! a null value
  JsonValue();

  //! parses a whole document; false (and a message with the line) if it isn't valid JSON
  static bool parse(const std::string& text, JsonValue& value, std::string* error = nullptr);

  Type getType() const {return type_;}
  bool isNull() const {return type_ == Null;}
  bool isBool() const {return type_ == Bool;}
  bool isNumber() const {return type_ == Number;}
  bool isString() const {return type_ == String;}
  bool isArray() const {return type_ == Array;}
  bool isObject() const {return type_ == Object;}

  bool getBool(bool def = false) const {return type_ == Bool ? bool_ : def;}
  double getNumber(double def = 0) const {return type_ == Number ? number_ : def;}
  float getFloat(float def = 0) const {return type_ == Number ? (float)number_ : def;}
  //! the number rounded towards zero
  int getInt(int def = 0) const {return type_ == Number ? (int)number_ : def;}
  //! the string, UTF-8 encoded; empty for other types
  const std::string& getString() const {return string_;}

  //! the elements of an array or the members of an object
  size_t size() const;
  //! element i of an array; a null value if there is none
  const JsonValue& operator[](size_t i) const;
  //! the member of an object; a null value if there is none
  const JsonValue& operator[](const std::string& key) const;
  bool has(const std::string& key) const;
  //! the elements of an array; empty for other types
  const std::vector<JsonValue>& getElements() const {return elements_;}
  //! the members of an object in the file's order; empty for other types
  const std::vector<std::pair<std::string, JsonValue>>& getMembers() const {return members_;}

private:
  friend class JsonParser;
  Type type_;
  bool bool_;
  double number_;
  std::string string_;
  std::vector<JsonValue> elements_;
  std::vector<std::pair<std::string, JsonValue>> members_;
};

}

#endif
