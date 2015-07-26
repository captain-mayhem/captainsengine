#include "Data.h"
#include <sstream>

using namespace std;

Data::Data() : mDB("data.db"){
  createSchema();
}

void Data::createSchema(){
  mDB.execute("CREATE TABLE IF NOT EXISTS History (\n"
    "ID INTEGER PRIMARY KEY NOT NULL,\n"
    "Text TEXT NOT NULL"
    ");");
  mDB.execute("CREATE TABLE IF NOT EXISTS Words (\n"
    "ID INTEGER PRIMARY KEY NOT NULL,\n"
    "Name VARCHAR(100) NOT NULL,\n"
    "Type INTEGER NOT NULL"
    ");");
  mDB.execute("CREATE TABLE IF NOT EXISTS Nouns (\n"
    "ID INTEGER PRIMARY KEY NOT NULL REFERENCES Words(ID),\n"
    "Gender VARCHAR(1) NOT NULL"
    ");");
}

std::string Data::directQuery(const char* query){
  int res = mDB.execute(query);
  if (res < 0)
    return mDB.getError();
  ostringstream out;
  out << "Query returned " << res << " rows" << endl;
  for (int j = 0; j < mDB.getNumRows(); ++j){
    for (int i = 0; i < mDB.getNumCols(); ++i){
      out << mDB.getResultString(i, j) << "  ";
    }
    out << endl;
  }
  return out.str();
}

Word Data::lookupWord(const char* word){
  sprintf(mBuffer, "SELECT ID, Type FROM Words WHERE Name = '%s';", word);
  Word w;
  int ret = mDB.execute(mBuffer);
  if (ret <= 0)
    return w;
  w.id = mDB.getResultInt(0, 0);
  w.type = (Type)mDB.getResultInt(1, 0);
  return w;
}

Word Data::storeWord(const char* word, Type type){
  sprintf(mBuffer, "INSERT INTO Words (Name, Type)\n"
    "VALUES ('%s', %i);"
    , word, (int)type);
  int ret = mDB.execute(mBuffer);
  Word w;
  w.id = mDB.getLastID();
  w.type = type;
  return w;
}

void Data::updateWord(const Word& w){
  sprintf(mBuffer, "UPDATE Words\n"
    "SET Type = %i\n"
    "WHERE ID = %i;"
    , (int)w.type, w.id);
  mDB.execute(mBuffer);
}
