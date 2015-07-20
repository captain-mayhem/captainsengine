#ifndef ITEM_HH
#define ITEM_HH

#include <string>
#include <fstream>
#include <iostream>
#include "math/vector.h"
#include "system/engine.h"

#include "common.h"

using std::ifstream;
using std::ofstream;
using std::string;

//using CGE::Vector2D;

class Item{
  public:
    enum Type{
      Armory,
      NoItem,
      Potion,
      Spell,
      Unusable,
      Artifact,
    };
    Item();
    Item(const Item& i);
    Item(string name, short id, Type typ, string additional="", string description="");
    ~Item() {}
    Item operator=(Item it);
    inline void setName(string name) {name_ = name;}
    inline string getName() {return name_;}
    inline void setId(short id) {id_ = id;}
    inline short getId() {return id_;}
    inline void setAdditional(string text) {additional_ = text;}
    inline string getAdditional() {return additional_;}
    inline void setDescription(string description) {description_ = description;}
    inline string getDescription() {return description_;}
    inline void setAdditional(int value) {additional_ = toStr(value);}
    inline int getAdditionalInt() {return toInt(additional_);}
    inline void setType(Type type) {type_ = type;}
    inline Type getType(){return type_;}
    inline void increase() {count_++;}
    inline bool decrease() {return (count_-- > 0);}
    inline void reset() {count_ = 0;}
    inline short getNumber() {return count_;}
    inline bool isValid() {return (id_ != -1 && count_ > 0);}
    bool use(::CGE::Vector2D source, ::CGE::Vector2D target);
    //! writes item data
    void write(ofstream& out)const;
    //! reads item data
    void read(ifstream& in);
    string toString();
    void fromString(string it);
  private:
    string name_;
    string additional_;
    string description_;
    Type type_;
    short id_;
    short count_;
};

#endif
