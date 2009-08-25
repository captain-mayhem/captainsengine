#ifndef JAVADEFS_H
#define JAVADEFS_H

#include <vector>

namespace Java{

typedef unsigned int u4;
typedef unsigned short u2;
typedef unsigned char u1;

struct cp_info;
struct field_info;
struct method_info;
struct attribute_info;

struct ClassFile{
  ClassFile();
  ~ClassFile();
  u4 magic;
  u2 minor_version;
  u2 major_version;
  u2 constant_pool_count;
  std::vector<cp_info*> constant_pool;
  u2 access_flags;
  u2 this_class;
  u2 super_class;
  u2 interfaces_count;
  std::vector<u2> interfaces;
  u2 fields_count;
  std::vector<field_info*> fields;
  u2 methods_count;
  std::vector<method_info*> methods;
  u2 attributes_count;
  std::vector<attribute_info*> attributes;
};

#define ACC_PUBLIC 0x0001;
#define ACC_PRIVATE 0x0002;
#define ACC_PROTECTED 0x0004;
#define ACC_STATIC 0x0008;
#define ACC_FINAL 0x0010;
#define ACC_SUPER 0x0020;
#define ACC_SYNCHRONIZED 0x0020;
#define ACC_VOLATILE 0x0040;
#define ACC_TRANSIENT 0x0080;
#define ACC_NATIVE 0x0100;
#define ACC_INTERFACE 0x0200;
#define ACC_ABSTRACT 0x0400;
#define ACC_STRICT 0x800;

struct cp_info{
  cp_info();
  virtual ~cp_info(){}
  cp_info(u1 tag) {this->tag = tag;}
  u1 tag;
};

#define CONSTANT_Class 7
#define CONSTANT_Fieldref 9
#define CONSTANT_Methodref 10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_String 8
#define CONSTANT_Integer 3
#define CONSTANT_Float 4
#define CONSTANT_Long 5
#define CONSTANT_Double 6
#define CONSTANT_NameAndType 12
#define CONSTANT_Utf8 1

struct CONSTANT_Class_info : public cp_info{
  CONSTANT_Class_info() : cp_info(CONSTANT_Class) {}
  u2 name_index;
};

struct CONSTANT_Fieldref_info : public cp_info{
  CONSTANT_Fieldref_info() : cp_info(CONSTANT_Fieldref) {}
  u2 class_index;
  u2 name_and_type_index;
};

struct CONSTANT_Methodref_info  : public cp_info{
  CONSTANT_Methodref_info() : cp_info(CONSTANT_Methodref) {}
  u2 class_index;
  u2 name_and_type_index;
};

struct CONSTANT_InterfaceMethodref_info : public cp_info{
  CONSTANT_InterfaceMethodref_info() : cp_info(CONSTANT_InterfaceMethodref) {}
  u2 class_index;
  u2 name_and_type_index;
};

struct CONSTANT_String_info : public cp_info{
  CONSTANT_String_info() : cp_info(CONSTANT_String) {}
  u2 string_index;
};

struct CONSTANT_Integer_info : public cp_info{
  CONSTANT_Integer_info() : cp_info(CONSTANT_Integer) {}
  u4 bytes;
};

struct CONSTANT_Float_info : public cp_info{
  CONSTANT_Float_info() : cp_info(CONSTANT_Float) {}
  u4 bytes;
};

struct CONSTANT_Long_info : public cp_info{
  CONSTANT_Long_info() : cp_info(CONSTANT_Long) {}
  u4 high_bytes;
  u4 low_bytes;
};

struct CONSTANT_Double_info : public cp_info{
  CONSTANT_Double_info() : cp_info(CONSTANT_Double) {}
  u4 high_bytes;
  u4 low_bytes;
};

struct CONSTANT_NameAndType_info : public cp_info{
  CONSTANT_NameAndType_info() : cp_info(CONSTANT_NameAndType) {}
  u2 name_index;
  u2 descriptor_index;
};

struct CONSTANT_Utf8_info : public cp_info{
  CONSTANT_Utf8_info() : cp_info(CONSTANT_Utf8) {}
  u2 length;
  std::string bytes;
};


struct field_info{
  field_info();
  ~field_info();
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  std::vector<attribute_info*> attributes;
};

struct method_info{
  method_info();
  ~method_info();
  u2 access_flags;
  u2 name_index;
  u2 descriptor_index;
  u2 attributes_count;
  std::vector<attribute_info*> attributes;
};

struct attribute_info{
  attribute_info(u2 attribute_name_index, u4 attribute_length) {this->attribute_name_index = attribute_name_index; this->attribute_length = attribute_length;}
  virtual ~attribute_info(){}
  u2 attribute_name_index;
  u4 attribute_length;
};

struct ConstantValue_attribute : public attribute_info{
  ConstantValue_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  u2 constantvalue_index;
};

struct Code_attribute  : public attribute_info{
  struct Exexception_table{
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
  };
  Code_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  ~Code_attribute() {delete [] code;}
  u2 max_stack;
  u2 max_locals;
  u4 code_length;
  u1* code;
  u2 exception_table_length;
  std::vector<Exexception_table> exception_table;
  u2 attributes_count;
  std::vector<attribute_info*> attributes;
};

struct Exceptions_attribute  : public attribute_info{
  Exceptions_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  u2 number_of_exceptions;
  std::vector<u2> exception_index_table;
};

struct InnerClasses_attribute  : public attribute_info{
  struct Classes{
    u2 inner_class_info_index;
    u2 outer_class_info_index;
    u2 inner_name_index;
    u2 inner_class_access_flags;
  };
  InnerClasses_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  u2 number_of_classes;
  std::vector<Classes> classes;
};

struct Synthetic_attribute  : public attribute_info{
  Synthetic_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
};

struct SourceFile_attribute : public attribute_info{
  SourceFile_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  u2 sourcefile_index;
};

struct LineNumberTable_attribute : public attribute_info{
  struct Line_number_table{
    u2 start_pc;
    u2 line_number;
  };
  LineNumberTable_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  u2 line_number_table_length;
  std::vector<Line_number_table> line_number_table;
};

struct LocalVariableTable_attribute : public attribute_info{
  struct Local_variable_table{
    u2 start_pc;
    u2 length;
    u2 name_index;
    u2 descriptor_index;
    u2 index;
  };
  LocalVariableTable_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
  u2 local_variable_table_length;
  std::vector<Local_variable_table> local_variable_table;
};

struct Deprecated_attribute : public attribute_info{
  Deprecated_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(attribute_name_index, attribute_length) {}
};

#define PROC_ENUM_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_ENUM_MODE

/*
enum Opcodes{
  nop = 0x00,
  aconst_null = 0x01,
  iconst_ml = 0x02,
  iconst_0 = 0x03,
  iconst_1 = 0x04,
  iconst_2 = 0x05,
  iconst_3 = 0x06,
  iconst_4 = 0x07,
  iconst_5 = 0x08,
  lconst_0 = 0x09,
  lconst_1 = 0x0a,
  fconst_0 = 0x0b,
  fconst_1 = 0x0c,
  fconst_2 = 0x0d,
  dconst_0 = 0x0e,
  dconst_1 = 0x0f,
  bipush = 0x10,
  sipush = 0x11,
  ldc = 0x12,
  ldc_w = 0x13,
  ldc2_w = 0x14,
};
*/
}

#endif