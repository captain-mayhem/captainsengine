#ifndef DATA_H
#define DATA_H

#include "SQL.h"

enum Type{
  UNKNOWN,
  NOUN,
  VERB,
};

struct Word{
  Word() : id(-1), type(UNKNOWN) {}
  int id;
  Type type;
};

class Data{
public:
  Data();
  Word lookupWord(const char* word);
  Word storeWord(const char* word, Type type);
  void updateWord(const Word& w);
  std::string directQuery(const char* query);
protected:
  void createSchema();
  SQL mDB;
  char mBuffer[1024];
};

#endif
