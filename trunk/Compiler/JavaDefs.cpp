
#include "JavaDefs.h"

using namespace Java;

ClassFile::ClassFile() : magic(0xCAFEBABE),
minor_version(0),
major_version(50),
constant_pool_count(1),
constant_pool(),
access_flags(0),
this_class(0),
super_class(0),
interfaces_count(0),
interfaces(),
fields_count(0),
fields(),
methods_count(0),
methods(),
attributes_count(0),
attributes()
{

}

ClassFile::~ClassFile(){
  for (Java::u2 i = 0; i < constant_pool_count-1; ++i){
    if (constant_pool[i]->tag == CONSTANT_Long || constant_pool[i]->tag == CONSTANT_Double)
      ++i; //don't delete the first entry, the same data is also at the second
    delete constant_pool[i];
  }
  for (Java::u2 i =0; i < fields_count; ++i){
    delete fields[i];
  }
  for (Java::u2 i =0; i < methods_count; ++i){
    delete methods[i];
  }
  for (Java::u2 i =0; i < attributes_count; ++i){
    delete attributes[i];
  }
}

field_info::field_info() : attributes_count(0), attributes() {

}

field_info::~field_info(){
  for (Java::u2 i =0; i < attributes_count; ++i){
    delete attributes[i];
  }
}

method_info::method_info() : attributes_count(0), attributes() {

}

method_info::~method_info(){
  for (Java::u2 i =0; i < attributes_count; ++i){
    delete attributes[i];
  }
}

Code_attribute::~Code_attribute(){
  delete [] code;
  for (Java::u2 i = 0; i < attributes_count; ++i){
    delete attributes[i];
  }
}
