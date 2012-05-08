
#include "JavaBinFileReader.h"
#include <iostream>
#include <string>
#include "Trace.h"

TR_CHANNEL(Java_File);

JavaBinFileReader::JavaBinFileReader(Reader* reader){
  mReader = reader;
}

JavaBinFileReader::~JavaBinFileReader(){

}

Java::u4 JavaBinFileReader::readU4(){
  unsigned int tmp = mReader->readUInt();
  Java::u4 ret = 0;
  ret |= (tmp << 24)&0xFF000000;
  ret |= (tmp << 8)&0x00FF0000;
  ret |= (tmp >> 8)&0x0000FF00;
  ret |= (tmp >> 24)&0x000000FF;
  return ret;
}

Java::u2 JavaBinFileReader::readU2(){
  unsigned short tmp = mReader->readUShort();
  Java::u2 ret = 0;
  ret |= (tmp << 8)&0xFF00;
  ret |= (tmp >> 8)&0x00FF;
  return ret;
}

unsigned JavaBinFileReader::readClassFile(Java::ClassFile& jclass){
  jclass.magic = readU4();
  if (jclass.magic != 0xCAFEBABE){
    return 1;
  }
  jclass.minor_version = readU2();
  jclass.major_version = readU2();
  jclass.constant_pool_count = readU2();
  jclass.constant_pool_count = readConstantPool(jclass.constant_pool, jclass.constant_pool_count-1);
  jclass.access_flags = readU2();
  jclass.this_class = readU2();
  jclass.super_class = readU2();
  jclass.interfaces_count = readU2();
  jclass.interfaces.resize(jclass.interfaces_count);
  for (int i = 0; i < jclass.interfaces_count; ++i){
    jclass.interfaces[i] = readU2();
  }
  jclass.fields_count = readU2();
  jclass.fields.resize(jclass.fields_count);
  for (int i = 0; i < jclass.fields_count; ++i){
    jclass.fields[i] = new Java::field_info();
    jclass.fields[i]->access_flags = readU2();
    jclass.fields[i]->name_index = readU2();
    jclass.fields[i]->descriptor_index = readU2();
    jclass.fields[i]->attributes_count = readU2();
    readAttributes(jclass.fields[i]->attributes, jclass.fields[i]->attributes_count, jclass);
  }
  jclass.methods_count = readU2();
  jclass.methods.resize(jclass.methods_count);
  for (int i = 0; i < jclass.methods_count; ++i){
    jclass.methods[i] = new Java::method_info();
    jclass.methods[i]->access_flags = readU2();
    jclass.methods[i]->name_index = readU2();
    jclass.methods[i]->descriptor_index = readU2();
    jclass.methods[i]->attributes_count = readU2();
    readAttributes(jclass.methods[i]->attributes, jclass.methods[i]->attributes_count, jclass);
  }
  jclass.attributes_count = readU2();
  readAttributes(jclass.attributes, jclass.attributes_count, jclass);
  return 0;
}

Java::u2 JavaBinFileReader::readConstantPool(std::vector<Java::cp_info*>& constant_pool, Java::u2 numEntries){
  constant_pool.resize(numEntries);
  for (int i = 0; i < numEntries; ++i){
    Java::u1 tag = readU1();
    switch(tag){
      case CONSTANT_Class:{
        Java::CONSTANT_Class_info* info = new Java::CONSTANT_Class_info();
        info->name_index = readU2();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_Fieldref:{
        Java::CONSTANT_Fieldref_info* info = new Java::CONSTANT_Fieldref_info();
        info->class_index = readU2();
        info->name_and_type_index = readU2();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_Methodref:{
        Java::CONSTANT_Methodref_info* info = new Java::CONSTANT_Methodref_info();
        info->class_index = readU2();
        info->name_and_type_index = readU2();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_InterfaceMethodref:{
        Java::CONSTANT_InterfaceMethodref_info* info = new Java::CONSTANT_InterfaceMethodref_info();
        info->class_index = readU2();
        info->name_and_type_index = readU2();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_String:{
        Java::CONSTANT_String_info* info = new Java::CONSTANT_String_info();
        info->string_index = readU2();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_Integer:{
        Java::CONSTANT_Integer_info* info = new Java::CONSTANT_Integer_info();
        info->bytes = readU4();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_Float:{
        Java::CONSTANT_Float_info* info = new Java::CONSTANT_Float_info();
        info->bytes = readU4();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_Long:{
        Java::CONSTANT_Long_info* info = new Java::CONSTANT_Long_info();
        info->high_bytes = readU4();
        info->low_bytes = readU4();
        constant_pool[i] = info;
        constant_pool[i+1] = info;
        ++i;
        }
        break;
      case CONSTANT_Double:{
        Java::CONSTANT_Double_info* info = new Java::CONSTANT_Double_info();
        info->high_bytes = readU4();
        info->low_bytes = readU4();
        constant_pool[i] = info;
        constant_pool[i+1] = info;
        ++i;
        }
        break;
      case CONSTANT_NameAndType:{
        Java::CONSTANT_NameAndType_info* info = new Java::CONSTANT_NameAndType_info();
        info->name_index = readU2();
        info->descriptor_index = readU2();
        constant_pool[i] = info;
        }
        break;
      case CONSTANT_Utf8:{
        Java::CONSTANT_Utf8_info* info = new Java::CONSTANT_Utf8_info();
        info->length = readU2();
        Java::u1* tmp = new Java::u1[info->length+1];
        mReader->readBytes(tmp,info->length);
        tmp[info->length] = '\0';
        info->bytes = std::string((char*)tmp);
        delete [] tmp;
        constant_pool[i] = info;
        }
        break;
      default:
        TR_USE(Java_File);
        TR_ERROR("Unknown Constant type in class file %i", (int)tag);
        mReader->skip(-1);
        --numEntries;
        constant_pool.resize(numEntries);
        //mReader->skip(-1);
        break;
    }
  }
  return numEntries+1;
}

void JavaBinFileReader::readAttributes(std::vector<Java::attribute_info*>& attributes, Java::u2 numEntries, const Java::ClassFile& jclass){
  attributes.resize(numEntries);
  for (int i = 0; i < numEntries; ++i){
    Java::u2 attribute_name_index = readU2();
    Java::u4 attribute_length = readU4();
    Java::cp_info* info = jclass.constant_pool[attribute_name_index-1];
    if (info->tag != CONSTANT_Utf8){
      std::cerr << "Invalid attribute name index detected";
    }
    std::string name = ((Java::CONSTANT_Utf8_info*)info)->bytes;
    if (name == "ConstantValue"){
      Java::ConstantValue_attribute* attr = new Java::ConstantValue_attribute(attribute_name_index, attribute_length);
      attr->constantvalue_index = readU2();
      attributes[i] = attr;
    }
    else if (name == "Code"){
      Java::Code_attribute* attr = new Java::Code_attribute(attribute_name_index, attribute_length);
      attr->max_stack = readU2();
      attr->max_locals = readU2();
      attr->code_length = readU4();
      attr->code = new Java::u1[attr->code_length];
      mReader->readBytes(attr->code, attr->code_length);
      attr->exception_table_length = readU2();
      attr->exception_table.resize(attr->exception_table_length);
      for (int j = 0; j < attr->exception_table_length; ++j){
        attr->exception_table[j].start_pc = readU2();
        attr->exception_table[j].end_pc = readU2();
        attr->exception_table[j].handler_pc = readU2();
        attr->exception_table[j].catch_type = readU2();
      }
      attr->attributes_count = readU2();
      readAttributes(attr->attributes, attr->attributes_count, jclass);
      attributes[i] = attr;
    }
    else if (name == "Exceptions"){
      Java::Exceptions_attribute* attr = new Java::Exceptions_attribute(attribute_name_index, attribute_length);
      attr->number_of_exceptions = readU2();
      attr->exception_index_table.resize(attr->number_of_exceptions);
      for (int j = 0; j < attr->number_of_exceptions; ++j){
        attr->exception_index_table[j] = readU2();
      }
      attributes[i] = attr;
    }
    else if (name == "InnerClasses"){
      Java::InnerClasses_attribute* attr = new Java::InnerClasses_attribute(attribute_name_index, attribute_length);
      attr->number_of_classes = readU2();
      attr->classes.resize(attr->number_of_classes);
      for (int j = 0; j < attr->number_of_classes; ++j){
        attr->classes[j].inner_class_info_index = readU2();
        attr->classes[j].outer_class_info_index = readU2();
        attr->classes[j].inner_name_index = readU2();
        attr->classes[j].inner_class_access_flags = readU2();
      }
      attributes[i] = attr;
    }
    else if (name == "Synthetic"){
      Java::Synthetic_attribute* attr = new Java::Synthetic_attribute(attribute_name_index, attribute_length);
      attributes[i] = attr;
    }
    else if (name == "SourceFile"){
      Java::SourceFile_attribute* attr = new Java::SourceFile_attribute(attribute_name_index, attribute_length);
      attr->sourcefile_index = readU2();
      attributes[i] = attr;
    }
    else if (name == "LineNumberTable"){
      Java::LineNumberTable_attribute* attr = new Java::LineNumberTable_attribute(attribute_name_index, attribute_length);
      attr->line_number_table_length = readU2();
      attr->line_number_table.resize(attr->line_number_table_length);
      for (int j = 0; j < attr->line_number_table_length; ++j){
        attr->line_number_table[j].start_pc = readU2();
        attr->line_number_table[j].line_number = readU2();
      }
      attributes[i] = attr;
    }
    else if (name == "LocalVariableTable"){
      Java::LocalVariableTable_attribute* attr = new Java::LocalVariableTable_attribute(attribute_name_index, attribute_length);
      attr->local_variable_table_length = readU2();
      attr->local_variable_table.resize(attr->local_variable_table_length);
      for (int j = 0; j < attr->local_variable_table_length; ++j){
        attr->local_variable_table[j].start_pc = readU2();
        attr->local_variable_table[j].length = readU2();
        attr->local_variable_table[j].name_index = readU2();
        attr->local_variable_table[j].descriptor_index = readU2();
        attr->local_variable_table[j].index = readU2();
      }
      attributes[i] = attr;
    }
    else if (name == "Deprecated"){
      Java::Deprecated_attribute* attr = new Java::Deprecated_attribute(attribute_name_index, attribute_length);
      attributes[i] = attr;
    }
		else if (name == "Signature"){
			Java::Signature_attribute* attr = new Java::Signature_attribute(attribute_name_index, attribute_length);
			attr->signature_index = readU2();
      attributes[i] = attr;
		}
    else{
      TR_USE(Java_File);
      mReader->skip(attribute_length);
      TR_WARN("Unknown Attribute: %s", name.c_str());
    }
  }
}

