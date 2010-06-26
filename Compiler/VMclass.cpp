#include "VMClass.h"

#include <iostream>

#include <io/BinFileReader.h>
#include <io/ZipReader.h>

#include "Trace.h"
#include "JVM.h"

#define PROC_DECL_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_DECL_MAP_MODE

#include "JavaBinFileReader.h"

VMClass::VMClass(const std::string& filename){
	 mFilename = filename;

  char* buffer = NULL;
	BinFileReader brdr(filename+".class");
  Reader* reader = &brdr;
	CGE::MemReader mrdr;
  JavaBinFileReader in(reader);
  if (!in.isWorking()){
    //try to load runtime jar
    TRACE(TRACE_JAVA,TRACE_DEBUG, "using jar mode");
		mrdr = getVM()->getClassFile(filename+".class");
		if (!mrdr.isWorking()){
      TRACE_ABORT(TRACE_JAVA, "Class %s not found in jar", filename.c_str());
    }
    reader = &mrdr;
    in.setReader(reader);
  }
  int ret = in.readClassFile(mClass);
  if (ret != 0)
    TRACE_ABORT(TRACE_JAVA, "Malformed classfile found - aborting...");

  TRACE(TRACE_JAVA, TRACE_INFO, "%s parsed successfully", filename.c_str());
}

void VMClass::print(){
  std::cout << "Constant pool:\n";
  for (int i = 0; i < mClass.constant_pool_count-1; ++i){
    Java::cp_info* cpinfo = mClass.constant_pool[i];
    std::cout << "#" << i+1 << " ";
    switch(cpinfo->tag){
      case CONSTANT_Class:
        std::cout << "Class " << ((Java::CONSTANT_Class_info*)cpinfo)->name_index;
        break;
      case CONSTANT_Fieldref:
        std::cout << "Fieldref " << ((Java::CONSTANT_Fieldref_info*)cpinfo)->class_index << "/" << ((Java::CONSTANT_Fieldref_info*)cpinfo)->name_and_type_index;
        break;
      case CONSTANT_Methodref:
        std::cout << "Methodref " << ((Java::CONSTANT_Methodref_info*)cpinfo)->class_index << "/" << ((Java::CONSTANT_Methodref_info*)cpinfo)->name_and_type_index;
        break;
      case CONSTANT_InterfaceMethodref:
        std::cout << "InterfaceMethodref " << ((Java::CONSTANT_InterfaceMethodref_info*)cpinfo)->class_index << "/" << ((Java::CONSTANT_InterfaceMethodref_info*)cpinfo)->name_and_type_index;
        break;
      case CONSTANT_String:
        std::cout << "String " << ((Java::CONSTANT_String_info*)cpinfo)->string_index;
        break;
      case CONSTANT_NameAndType:
        std::cout << "NameAndType " << ((Java::CONSTANT_NameAndType_info*)cpinfo)->name_index << "/" << ((Java::CONSTANT_NameAndType_info*)cpinfo)->descriptor_index;
        break;
      case CONSTANT_Utf8:
        std::cout << "UTF8 " << ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
        break;
    };
    std::cout << std::endl;
  }
  std::cout << "Methods:" << "\n";
  for (int i = 0; i < mClass.methods_count; ++i){
    Java::method_info* mi = mClass.methods[i];
    Java::cp_info* cpinfo = mClass.constant_pool[mi->name_index-1];
    Java::CONSTANT_Utf8_info* utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    if (!utf)
      TRACE_ABORT(TRACE_JAVA, "Method resolution invalid");
    std::cout << utf->bytes;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    if (!utf){
      TRACE(TRACE_JAVA, TRACE_ERROR, "Method resolution invalid");
      cpinfo = mClass.constant_pool[mi->descriptor_index-2];
      utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    }
    std::cout << utf->bytes;

    //find code attribute
    for (int j = 0; j < mi->attributes_count; ++j){
      Java::Code_attribute* code = dynamic_cast<Java::Code_attribute*>(mi->attributes[j]);
      if (code){
        //std::cerr << "\nCode:";
        for (unsigned k = 0; k < code->code_length; ++k){
          Java::u1 opcode = code->code[k];
          std::string cd = Opcode::map_string[opcode];
          std::cout << "\n\t"/*<< code->code[k]*/ << cd;
          switch (opcode){
            case Java::op_invokestatic:
            case Java::op_invokespecial:
            case Java::op_invokevirtual:
            case Java::op_getstatic:
            case Java::op_goto:
            case Java::op_new:
            case Java::op_if_acmpne:
            case Java::op_if_icmpge:
            case Java::op_if_icmple:
            case Java::op_ifeq:
            case Java::op_ifge:
            case Java::op_iflt:
            case Java::op_ifne:
              {
                Java::u1 b1 = code->code[++k];
                Java::u1 b2 = code->code[++k];
                Java::u2 operand = b1 << 8 | b2;
                std::cout << " #" << (int)operand;
              }
              break;
            case Java::op_ldc:{
              Java::u1 operand = code->code[++k];
              std::cout << " #" << (int)operand;
              }
              break;

          }
        }
      }
    }

    std::cout << "\n\n";
  }
}
