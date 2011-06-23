#ifndef JAVADEFS_H
#define JAVADEFS_H

#include <vector>
//#include <system/allocation.h>

#include "jni.h"

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

#define ACC_PUBLIC 0x0001
#define ACC_PRIVATE 0x0002
#define ACC_PROTECTED 0x0004
#define ACC_STATIC 0x0008
#define ACC_FINAL 0x0010
#define ACC_SUPER 0x0020
#define ACC_SYNCHRONIZED 0x0020
#define ACC_VOLATILE 0x0040
#define ACC_TRANSIENT 0x0080
#define ACC_NATIVE 0x0100
#define ACC_INTERFACE 0x0200
#define ACC_ABSTRACT 0x0400
#define ACC_STRICT 0x800

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

enum attrib_type{
	ATTR_ConstatntValue,
	ATTR_Code,
	ATTR_Exceptions,
	ATTR_InnerClass,
	ATTR_Synthetic,
	ATTR_SourceFile,
	ATTR_LineNumberTable,
	ATTR_LocalVariableTable,
	ATTR_Deprecated,
	ATTR_Signature,
	ATTR_RuntimeVisibleAnnotations
};

struct attribute_info{
	attribute_info(attrib_type attribute_type, u2 attribute_name_index, u4 attribute_length) {this->attribute_type = attribute_type, this->attribute_name_index = attribute_name_index; this->attribute_length = attribute_length;}
  virtual ~attribute_info(){}
	attrib_type attribute_type;
  u2 attribute_name_index;
  u4 attribute_length;
};

struct ConstantValue_attribute : public attribute_info{
  ConstantValue_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_ConstatntValue, attribute_name_index, attribute_length) {}
  u2 constantvalue_index;
};

struct Code_attribute  : public attribute_info{
  struct Exexception_table{
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
  };
  Code_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_Code, attribute_name_index, attribute_length) {}
  ~Code_attribute();
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
  Exceptions_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_Exceptions, attribute_name_index, attribute_length) {}
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
  InnerClasses_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_InnerClass, attribute_name_index, attribute_length) {}
  u2 number_of_classes;
  std::vector<Classes> classes;
};

struct Synthetic_attribute  : public attribute_info{
  Synthetic_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_Synthetic, attribute_name_index, attribute_length) {}
};

struct SourceFile_attribute : public attribute_info{
  SourceFile_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_SourceFile, attribute_name_index, attribute_length) {}
  u2 sourcefile_index;
};

struct LineNumberTable_attribute : public attribute_info{
  struct Line_number_table{
    u2 start_pc;
    u2 line_number;
  };
  LineNumberTable_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_LineNumberTable, attribute_name_index, attribute_length) {}
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
  LocalVariableTable_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_LocalVariableTable, attribute_name_index, attribute_length) {}
  u2 local_variable_table_length;
  std::vector<Local_variable_table> local_variable_table;
};

struct Deprecated_attribute : public attribute_info{
  Deprecated_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_Deprecated, attribute_name_index, attribute_length) {}
};

struct Signature_attribute : public attribute_info{
	Signature_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_Signature, attribute_name_index, attribute_length) {}
	u2 signature_index;
};

struct RuntimeVisibleAnnotations_attribute : public attribute_info{
	struct annotation{
		struct element_value_pair{
			u2 element_name_index;
			struct element_value {
				u1 tag;
				union{
					u2 const_value_index;
					struct {
						u2 type_name_index;
						u2 const_name_index;
					} enum_const_value;
					u2 class_info_index;
					annotation* annotation_value;
					struct {
						u2 num_values;
						element_value* values;
					} array_value;
				} value;
			};
		};
		u2 type_index;
		u2 num_element_value_pairs;
		std::vector<element_value_pair> element_value_pairs;
	};
	RuntimeVisibleAnnotations_attribute(u2 attribute_name_index, u4 attribute_length) : attribute_info(ATTR_RuntimeVisibleAnnotations, attribute_name_index, attribute_length) {}
	u2 num_annotations;
	std::vector<annotation> annotations;
};

#define PROC_ENUM_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_ENUM_MODE

}

class VMMethod;
class VMObject;
class VMClass;

union StackData{
		StackData() {}
		StackData(StackData* sd) {stp = sd;}
		StackData(VMMethod* md) {mthd = md;}
		StackData(uint32 uin) {ui = uin;}
		StackData(int32 in) {i = in;}
		StackData(float fl) {f = fl;}
		StackData(VMObject* ob) {obj = ob;}
		StackData(VMClass* cs) {cls = cs;}
		VMMethod* mthd;
		StackData* stp;
		int32 i;
		uint32 ui;
		float f;
		VMObject* obj;
		VMClass* cls;
};

union FieldData{
	VMMethod* mthd;
	StackData* stp;
	int32 i;
	uint32 ui;
	float f;
	VMObject* obj;
	VMClass* cls;
  unsigned char b;

	double d;
	int64 l;
};

typedef void (*nativeMethod)(JNIEnv* env, jobject object, ...);
typedef int64 (*nativeLongMethod)(JNIEnv* env, jobject object, ...);
typedef void* (*nativeRefMethod)(JNIEnv* env, jobject object, ...);
typedef jboolean (*nativeBoolMethod)(JNIEnv* env, jobject object, ...);
typedef jdouble (*nativeDoubleMethod)(JNIEnv* env, jobject object, ...);

#endif