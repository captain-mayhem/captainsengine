#include "json.h"

#include <locale>
#include <sstream>

namespace CGE{

//! a recursive descent parser for JsonValue::parse
class JsonParser{
public:
  explicit JsonParser(const std::string& text) : text_(text), pos_(0), line_(1){}

  bool parseDocument(JsonValue& value, std::string* error){
    skipSpace();
    bool ok = parseValue(value, 0);
    if (ok){
      skipSpace();
      if (pos_ < text_.size())
        ok = fail("text after the value");
    }
    if (!ok && error)
      *error = "line " + std::to_string(line_) + ": " + message_;
    return ok;
  }

private:
  //! nesting deeper than this is refused, so the stack can't overflow
  static const int MAX_DEPTH = 256;

  bool fail(const char* message){
    if (message_.empty())
      message_ = message;
    return false;
  }

  bool atEnd() const {return pos_ >= text_.size();}
  char peek() const {return atEnd() ? '\0' : text_[pos_];}

  void skipSpace(){
    while (!atEnd()){
      char c = text_[pos_];
      if (c == '\n')
        ++line_;
      else if (c != ' ' && c != '\t' && c != '\r')
        return;
      ++pos_;
    }
  }

  bool expectWord(const char* word){
    for (const char* c = word; *c; ++c){
      if (peek() != *c)
        return fail("unknown value");
      ++pos_;
    }
    return true;
  }

  bool parseValue(JsonValue& value, int depth){
    if (depth > MAX_DEPTH)
      return fail("nested too deeply");
    value = JsonValue();
    switch (peek()){
      case '{':
        return parseObject(value, depth);
      case '[':
        return parseArray(value, depth);
      case '"':
        value.type_ = JsonValue::String;
        return parseString(value.string_);
      case 't':
        value.type_ = JsonValue::Bool;
        value.bool_ = true;
        return expectWord("true");
      case 'f':
        value.type_ = JsonValue::Bool;
        value.bool_ = false;
        return expectWord("false");
      case 'n':
        return expectWord("null");
      case '\0':
        if (atEnd())
          return fail("unexpected end");
        return fail("unexpected character");
      default:
        return parseNumber(value);
    }
  }

  bool parseObject(JsonValue& value, int depth){
    value.type_ = JsonValue::Object;
    ++pos_;
    skipSpace();
    if (peek() == '}'){
      ++pos_;
      return true;
    }
    while (true){
      skipSpace();
      if (peek() != '"')
        return fail("expected a member name");
      std::string key;
      if (!parseString(key))
        return false;
      skipSpace();
      if (peek() != ':')
        return fail("expected ':'");
      ++pos_;
      skipSpace();
      value.members_.emplace_back(key, JsonValue());
      if (!parseValue(value.members_.back().second, depth+1))
        return false;
      skipSpace();
      char c = peek();
      ++pos_;
      if (c == '}')
        return true;
      if (c != ',')
        return fail("expected ',' or '}'");
    }
  }

  bool parseArray(JsonValue& value, int depth){
    value.type_ = JsonValue::Array;
    ++pos_;
    skipSpace();
    if (peek() == ']'){
      ++pos_;
      return true;
    }
    while (true){
      skipSpace();
      value.elements_.emplace_back();
      if (!parseValue(value.elements_.back(), depth+1))
        return false;
      skipSpace();
      char c = peek();
      ++pos_;
      if (c == ']')
        return true;
      if (c != ',')
        return fail("expected ',' or ']'");
    }
  }

  bool parseHex4(unsigned& code){
    code = 0;
    for (int i = 0; i < 4; ++i){
      char c = peek();
      ++pos_;
      code <<= 4;
      if (c >= '0' && c <= '9')
        code |= c-'0';
      else if (c >= 'a' && c <= 'f')
        code |= c-'a'+10;
      else if (c >= 'A' && c <= 'F')
        code |= c-'A'+10;
      else
        return fail("bad \\u escape");
    }
    return true;
  }

  static void appendUtf8(std::string& out, unsigned code){
    if (code < 0x80)
      out += (char)code;
    else if (code < 0x800){
      out += (char)(0xC0 | (code >> 6));
      out += (char)(0x80 | (code & 0x3F));
    }
    else if (code < 0x10000){
      out += (char)(0xE0 | (code >> 12));
      out += (char)(0x80 | ((code >> 6) & 0x3F));
      out += (char)(0x80 | (code & 0x3F));
    }
    else{
      out += (char)(0xF0 | (code >> 18));
      out += (char)(0x80 | ((code >> 12) & 0x3F));
      out += (char)(0x80 | ((code >> 6) & 0x3F));
      out += (char)(0x80 | (code & 0x3F));
    }
  }

  bool parseString(std::string& out){
    ++pos_;
    while (true){
      if (atEnd())
        return fail("unterminated string");
      char c = text_[pos_++];
      if (c == '"')
        return true;
      if ((unsigned char)c < 0x20)
        return fail("control character in a string");
      if (c != '\\'){
        out += c;
        continue;
      }
      c = peek();
      ++pos_;
      switch (c){
        case '"': out += '"'; break;
        case '\\': out += '\\'; break;
        case '/': out += '/'; break;
        case 'b': out += '\b'; break;
        case 'f': out += '\f'; break;
        case 'n': out += '\n'; break;
        case 'r': out += '\r'; break;
        case 't': out += '\t'; break;
        case 'u':{
          unsigned code;
          if (!parseHex4(code))
            return false;
          //a surrogate pair is one character
          if (code >= 0xD800 && code < 0xDC00 && peek() == '\\' && pos_+1 < text_.size() && text_[pos_+1] == 'u'){
            size_t back = pos_;
            pos_ += 2;
            unsigned low;
            if (!parseHex4(low))
              return false;
            if (low >= 0xDC00 && low < 0xE000)
              code = 0x10000 + ((code-0xD800) << 10) + (low-0xDC00);
            else
              pos_ = back;
          }
          appendUtf8(out, code);
          break;
        }
        default:
          return fail("bad escape in a string");
      }
    }
  }

  static bool isDigit(char c) {return c >= '0' && c <= '9';}

  bool parseNumber(JsonValue& value){
    size_t start = pos_;
    if (peek() == '-')
      ++pos_;
    if (peek() == '0')
      ++pos_;
    else if (isDigit(peek())){
      while (isDigit(peek()))
        ++pos_;
    }
    else
      return fail("unexpected character");
    if (peek() == '.'){
      ++pos_;
      if (!isDigit(peek()))
        return fail("bad number");
      while (isDigit(peek()))
        ++pos_;
    }
    if (peek() == 'e' || peek() == 'E'){
      ++pos_;
      if (peek() == '+' || peek() == '-')
        ++pos_;
      if (!isDigit(peek()))
        return fail("bad number");
      while (isDigit(peek()))
        ++pos_;
    }
    //independent of the C locale's decimal separator
    std::istringstream in(text_.substr(start, pos_-start));
    in.imbue(std::locale::classic());
    double number = 0;
    in >> number;
    value.type_ = JsonValue::Number;
    value.number_ = number;
    return true;
  }

  const std::string& text_;
  size_t pos_;
  int line_;
  std::string message_;
};

namespace{

const JsonValue& nullValue(){
  static const JsonValue null;
  return null;
}

}

JsonValue::JsonValue() : type_(Null), bool_(false), number_(0){
}

bool JsonValue::parse(const std::string& text, JsonValue& value, std::string* error){
  JsonParser parser(text);
  if (parser.parseDocument(value, error))
    return true;
  value = JsonValue();
  return false;
}

size_t JsonValue::size() const{
  if (type_ == Array)
    return elements_.size();
  if (type_ == Object)
    return members_.size();
  return 0;
}

const JsonValue& JsonValue::operator[](size_t i) const{
  if (i < elements_.size())
    return elements_[i];
  return nullValue();
}

const JsonValue& JsonValue::operator[](const std::string& key) const{
  for (const auto& member : members_){
    if (member.first == key)
      return member.second;
  }
  return nullValue();
}

bool JsonValue::has(const std::string& key) const{
  for (const auto& member : members_){
    if (member.first == key)
      return true;
  }
  return false;
}

}
