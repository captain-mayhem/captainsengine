
#include "ClassLoader.h"

#include <iostream>
#include <fstream>

#include "unzip.h"

#include <io/MemReader.h>
#include <io/BinFileReader.h>
#include "JavaBinFileReader.h"
#include "Trace.h"

#define PROC_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_MAP_MODE

#define CHUNKSIZE 262144

void* ClassLoader::mRuntime = NULL;

ClassLoader::ClassLoader(std::string filename){
  if (mRuntime == NULL){
#ifndef UNDER_CE
    char* tmp = getenv("ProgramFiles");
#else
    char* tmp = NULL;
#endif
    std::string prefix;
    if (tmp)
      prefix = tmp;
    mRuntime = unzOpen((prefix+"/Java/jre6/lib/rt.jar").c_str());
    if (mRuntime == NULL){
      //hack for my crappy environment
      prefix[0] = 'E';
      mRuntime = unzOpen((prefix+"/Java/jre6/lib/rt.jar").c_str());
      if (mRuntime == NULL){
        TRACE_ABORT(TRACE_JAVA, "Java runtime classes not found");
      }
    }
  }
  Opcode::init();
  mFilename = filename;

  char* buffer = NULL;
  Reader* reader = new BinFileReader(filename+".class");
  JavaBinFileReader in(reader);
  if (!in.isWorking()){
    //try to load runtime jar
    TRACE(TRACE_JAVA,TRACE_DEBUG, "using jar mode");
    int ret = unzLocateFile(mRuntime,(filename+".class").c_str(),0);
    if (ret != 0){
      TRACE_ABORT(TRACE_JAVA, "Class %s not found in jar", filename.c_str());
    }
    unzOpenCurrentFile(mRuntime);
    unz_file_info info;
    unzGetCurrentFileInfo(mRuntime,&info,NULL,0,NULL,0,NULL,0);
    buffer = new char[info.uncompressed_size];
    unzReadCurrentFile(mRuntime, buffer, info.uncompressed_size);
    unzCloseCurrentFile(mRuntime);
    delete reader;
    reader = new MemReader(buffer,info.compressed_size);
    in.setReader(reader);
  }
  int ret = in.readClassFile(mClass);
  if (ret != 0)
    TRACE_ABORT(TRACE_JAVA, "Malformed classfile found - aborting...");
  
  delete reader;
  delete [] buffer;
  TRACE(TRACE_JAVA, TRACE_INFO, "%s parsed successfully", filename.c_str());
}

ClassLoader::~ClassLoader(){
  //unzClose(mRuntime);
}

void ClassLoader::print(){
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

int ClassLoader::load(MethodArea& area){
  if (mClass.super_class != 0){
    //load superclass first
    Java::cp_info* cpinfo = mClass.constant_pool[mClass.super_class-1];
    Java::CONSTANT_Class_info* classinf = dynamic_cast<Java::CONSTANT_Class_info*>(cpinfo);
    if (classinf == NULL){
      TRACE_ABORT(TRACE_JAVA, "Classfile inconsistent: Superclass index");
    }
    cpinfo = mClass.constant_pool[classinf->name_index-1];
    Java::CONSTANT_Utf8_info* stringinf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    if (stringinf == NULL)
      TRACE_ABORT(TRACE_JAVA, "Classfile inconsistent: Superclass string");
    TRACE(TRACE_JAVA, TRACE_DEBUG, "Superclass %s", stringinf->bytes.c_str());
    area.getClassIndex(stringinf->bytes);
  }
  ClassData* data = new ClassData();
  data->mLoader = this;
  area.mClassData.push_back(data);
  unsigned entry = (unsigned)area.mClassData.size()-1;
  area.mClassResolver[mFilename] = entry;
  
  //resolveMethods(area, entry);
  MethodEntry me("<clinit>", "()V", entry);
  resolveMethod(area, me);
  return entry;
}

void ClassLoader::resolveMethods(MethodArea& area, unsigned classidx){/*
  for (int i = 0; i < mClass.methods_count; ++i){
    Codes codes;
    Java::method_info* mi = mClass.methods[i];
    Java::cp_info* cpinfo = mClass.constant_pool[mi->name_index-1];
    std::string methodname = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    std::string signature = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    MethodEntry entry(methodname, signature, classidx);

    //find code attribute
    for (int j = 0; j < mi->attributes_count; ++j){
      Java::Code_attribute* code = dynamic_cast<Java::Code_attribute*>(mi->attributes[j]);
      if (code){
        for (unsigned k = 0; k < code->code_length; ++k){
          Java::u1 opcode = code->code[k];
          Operation op;
          op.opcode = opcode;
          switch (opcode){
            //single byte number
            case Java::op_bipush:{
              Java::u1 constant = code->code[++k];
              op.data.i = constant;
              }
              break;
            //two byte number
            case Java::op_sipush:{
              Java::u1 b1 = code->code[++k];
              Java::u1 b2 = code->code[++k];
              Java::u2 constant = b1 << 8 | b2;
              op.data.i = constant;
              }
              break;
            case Java::op_ldc:{
              Java::u1 operand = code->code[++k];
              Java::cp_info* info = mClass.constant_pool[operand-1];
              //op.data.i = (int)info;
              //op.data1 = constant;
              }
              break;
            case Java::op_invokespecial:{
              Java::u1 b1 = code->code[++k];
              Java::u1 b2 = code->code[++k];
              Java::u2 operand = b1 << 8 | b2;
              Java::CONSTANT_Methodref_info* methodref = dynamic_cast<Java::CONSTANT_Methodref_info*>(mClass.constant_pool[operand-1]);
              Java::CONSTANT_Class_info* classinfo = dynamic_cast<Java::CONSTANT_Class_info*>(mClass.constant_pool[methodref->class_index-1]);
              Java::CONSTANT_NameAndType_info* nameandtypeinfo = dynamic_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[methodref->name_and_type_index-1]);
              std::string classname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[classinfo->name_index-1])->bytes;
              unsigned classidx = area.getClassIndex(classname);
              std::string methodname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
              std::string signature = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
              unsigned idx = area.getMethodIndex(MethodEntry(methodname,signature,classidx));
              op.data.u = idx;
              break;
            }
            case Java::op_lookupswitch:
            case Java::op_tableswitch:
              k+=98;
              break;
            case Java::op_invokeinterface:
            case Java::op_jsr_w:
            case Java::op_goto_w:
              k+=4;
              break;
            case Java::op_multianewarray:
              k+=3;
              break;
            case Java::op_ldc_w:
            case Java::op_ldc2_w:
            case Java::op_iinc:
            case Java::op_ifeq:
            case Java::op_ifne:
            case Java::op_iflt:
            case Java::op_ifge:
            case Java::op_ifgt:
            case Java::op_ifle:
            case Java::op_if_icmpeq:
            case Java::op_if_icmpne:
            case Java::op_if_icmplt:
            case Java::op_if_icmpge:
            case Java::op_if_icmpgt:
            case Java::op_if_icmple:
            case Java::op_if_acmpeq:
            case Java::op_if_acmpne:
            case Java::op_goto:
            case Java::op_jsr:
            case Java::op_getstatic:
            case Java::op_putstatic:
            case Java::op_getfield:
            case Java::op_putfield:
            case Java::op_invokestatic:
            case Java::op_invokevirtual:
            case Java::op_new:
            case Java::op_anewarray:
            case Java::op_instanceof:
            case Java::op_checkcast:
            case Java::op_ifnull:
            case Java::op_ifnonnull:
              k+=2;
              break;
            case Java::op_iload:
            case Java::op_lload:
            case Java::op_fload:
            case Java::op_dload:
            case Java::op_aload:
            case Java::op_istore:
            case Java::op_lstore:
            case Java::op_fstore:
            case Java::op_dstore:
            case Java::op_astore:
            case Java::op_ret:
            case Java::op_newarray:
              k+=1;
              break;
          };
          codes.push_back(op);
        }
        area.mMethods.push_back(codes);
      }
    }
    //method without code segment
    if (codes.empty()){
      area.mMethodResolver[entry] = 1;
    }
    else{
      area.mMethodResolver[entry] = area.mMethods.size()-1;
    }
  }*/
}

unsigned ClassLoader::resolveMethod(MethodArea& area, const MethodEntry& method){
  for (int i = 0; i < mClass.methods_count; ++i){
    Java::method_info* mi = mClass.methods[i];
    Java::cp_info* cpinfo = mClass.constant_pool[mi->name_index-1];
    std::string methodname = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    if (methodname != method.mName)
      continue;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    std::string signature = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    if (signature != method.mSignature)
      continue;

    //find code attribute
    for (int j = 0; j < mi->attributes_count; ++j){
      Java::Code_attribute* code = dynamic_cast<Java::Code_attribute*>(mi->attributes[j]);
      if (!code)
        continue;
      area.mMethods.push_back(Codes());
      int idx = area.mMethods.size()-1;
      Codes codes = area.mMethods[idx];
      area.mMethodResolver[method] = idx;
      for (unsigned k = 0; k < code->code_length; ++k){
        Java::u1 opcode = code->code[k];
        Operation op;
        op.opcode = opcode;
        switch (opcode){
          //single byte number
          case Java::op_bipush:
            {
            Java::u1 constant = code->code[++k];
            op.data.mInt = constant;
            }
            break;
            //two byte number
          case Java::op_sipush:
            {
            Java::u1 b1 = code->code[++k];
            Java::u1 b2 = code->code[++k];
            Java::u2 constant = b1 << 8 | b2;
            op.data.mInt = constant;
            }
            break;
          case Java::op_ldc:
            {
            Java::u1 operand = code->code[++k];
            Java::cp_info* info = mClass.constant_pool[operand-1];
            if (info->tag == CONSTANT_Integer){
              Java::CONSTANT_Integer_info* intinf = (Java::CONSTANT_Integer_info*)(info);
              op.data.mInt = intinf->bytes;
            }
            /*else if (info->tag == CONSTANT_Long){

            }
            else if (info->tag == CONSTANT_Float){
              Java::CONSTANT_Float_info* fltinf = (Java::CONSTANT_Float_info*)(info);
              op.data.mFloat = fltinf->bytes;
            }
            else if (info->tag == CONSTANT_Double){

            }*/
            else if (info->tag == CONSTANT_String){
              Java::CONSTANT_String_info* str = (Java::CONSTANT_String_info*)(info);
              Java::CONSTANT_Utf8_info* utf = (Java::CONSTANT_Utf8_info*)(mClass.constant_pool[str->string_index-1]);
              area.getClassIndex("java/lang/String");
              area.mStrings.push_back(utf->bytes);
            }
            else{
              TRACE(TRACE_JAVA, TRACE_ERROR, "Unhandled opcode in ldc");
            }
            //op.data.i = (int)info;
            //op.data1 = constant;
            }
            break;
          case Java::op_invokespecial:
            {
            Java::u1 b1 = code->code[++k];
            Java::u1 b2 = code->code[++k];
            Java::u2 operand = b1 << 8 | b2;
            Java::CONSTANT_Methodref_info* methodref = dynamic_cast<Java::CONSTANT_Methodref_info*>(mClass.constant_pool[operand-1]);
            Java::CONSTANT_Class_info* classinfo = dynamic_cast<Java::CONSTANT_Class_info*>(mClass.constant_pool[methodref->class_index-1]);
            Java::CONSTANT_NameAndType_info* nameandtypeinfo = dynamic_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[methodref->name_and_type_index-1]);
            std::string classname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[classinfo->name_index-1])->bytes;
            unsigned classidx = area.getClassIndex(classname);
            std::string methodname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
            std::string signature = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
            unsigned idx = area.getMethodIndex(MethodEntry(methodname,signature, classidx));
            op.data.mUint = idx;
            break;
                                      }
          case Java::op_lookupswitch:
          case Java::op_tableswitch:
            k+=98;
            break;
          case Java::op_invokeinterface:
            k+=4;
            break;
          case Java::op_jsr_w:
          case Java::op_goto_w:
            k+=4;
            break;
          case Java::op_multianewarray:
            k+=3;
            break;
          case Java::op_ldc_w:
          case Java::op_ldc2_w:
          case Java::op_iinc:
          case Java::op_ifeq:
          case Java::op_ifne:
          case Java::op_iflt:
          case Java::op_ifge:
          case Java::op_ifgt:
          case Java::op_ifle:
          case Java::op_if_icmpeq:
          case Java::op_if_icmpne:
          case Java::op_if_icmplt:
          case Java::op_if_icmpge:
          case Java::op_if_icmpgt:
          case Java::op_if_icmple:
          case Java::op_if_acmpeq:
          case Java::op_if_acmpne:
          case Java::op_goto:
          case Java::op_jsr:
            k+=2;
            break;
          case Java::op_getstatic:
            {
              Java::u1 b1 = code->code[++k];
              Java::u1 b2 = code->code[++k];
              Java::u2 operand = b1 << 8 | b2;
              Java::CONSTANT_Fieldref_info* fieldref = dynamic_cast<Java::CONSTANT_Fieldref_info*>(mClass.constant_pool[operand-1]);
              Java::CONSTANT_Class_info* classinfo = dynamic_cast<Java::CONSTANT_Class_info*>(mClass.constant_pool[fieldref->class_index-1]);
              Java::CONSTANT_NameAndType_info* nameandtypeinfo = dynamic_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[fieldref->name_and_type_index-1]);
              std::string classname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[classinfo->name_index-1])->bytes;
              unsigned classidx = area.getClassIndex(classname);
              std::string fieldname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
              std::string type = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
              unsigned fieldidx = resolveField(area, MethodEntry(fieldname, type, classidx));
              op.data.mUint = fieldidx;
            }
            break;
          case Java::op_putstatic:
          case Java::op_getfield:
          case Java::op_putfield:
            k+=2;
            break;
          case Java::op_invokestatic:
            {
              Java::u1 b1 = code->code[++k];
              Java::u1 b2 = code->code[++k];
              Java::u2 operand = b1 << 8 | b2;
              Java::CONSTANT_Methodref_info* methodref = dynamic_cast<Java::CONSTANT_Methodref_info*>(mClass.constant_pool[operand-1]);
              Java::CONSTANT_Class_info* classinfo = dynamic_cast<Java::CONSTANT_Class_info*>(mClass.constant_pool[methodref->class_index-1]);
              Java::CONSTANT_NameAndType_info* nameandtypeinfo = dynamic_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[methodref->name_and_type_index-1]);
              std::string classname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[classinfo->name_index-1])->bytes;
              unsigned classidx = area.getClassIndex(classname);
              std::string methodname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
              std::string signature = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
              unsigned idx = area.getMethodIndex(MethodEntry(methodname,signature, classidx));
              op.data.mUint = idx;
              break;
            }
            break;
          case Java::op_invokevirtual:
            {
              Java::u1 b1 = code->code[++k];
              Java::u1 b2 = code->code[++k];
              Java::u2 operand = b1 << 8 | b2;
              Java::CONSTANT_Methodref_info* methodref = dynamic_cast<Java::CONSTANT_Methodref_info*>(mClass.constant_pool[operand-1]);
              Java::CONSTANT_Class_info* classinfo = dynamic_cast<Java::CONSTANT_Class_info*>(mClass.constant_pool[methodref->class_index-1]);
              Java::CONSTANT_NameAndType_info* nameandtypeinfo = dynamic_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[methodref->name_and_type_index-1]);
              std::string classname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[classinfo->name_index-1])->bytes;
              unsigned classidx = area.getClassIndex(classname);
              std::string methodname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
              std::string signature = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
              //int32 methodidx = area.getMethodIndex(MethodEntry(methodname,signature,classidx));
              //area.mClassData[classidx].mVtable;
              uint32 methodidx = resolveVtable(area, MethodEntry(methodname, signature, classidx));
              op.data.mUint = methodidx;
            }
            break;
          case Java::op_new:
          case Java::op_anewarray:
          case Java::op_checkcast:
          case Java::op_instanceof:
          case Java::op_ifnull:
          case Java::op_ifnonnull:
            k+=2;
            break;
          case Java::op_iload:
          case Java::op_lload:
          case Java::op_fload:
          case Java::op_dload:
          case Java::op_aload:
          case Java::op_istore:
          case Java::op_lstore:
          case Java::op_fstore:
          case Java::op_dstore:
          case Java::op_astore:
          case Java::op_ret:
          case Java::op_newarray:
            k+=1;
            break;
        };
        codes.push_back(op);
      }
      return idx;
    }
    //no code segment found
    area.mMethods.pop_back();
    area.mMethodResolver[method] = 1;
    return 1;
  }
  //no method found
  return 0;
}

unsigned ClassLoader::resolveField(MethodArea& area, const MethodEntry& field){
  std::map<MethodEntry,uint32>::iterator iter = area.mFieldResolver.find(field);
  uint32 idx = 0;
  if (iter == area.mFieldResolver.end()){
    ClassData* data = area.mClassData[field.mClassindex];
    data->mFields.push_back(Types());
    idx = (unsigned)data->mFields.size()-1;
  }
  else
    idx = (*iter).second;
  return idx;
}

unsigned ClassLoader::resolveVtable(MethodArea& area, const MethodEntry& method){
  std::map<MethodEntry,uint32>::iterator iter = area.mVtableResolver.find(method);
  uint32 vidx = 0;
  if (iter == area.mVtableResolver.end()){
    ClassData* data = area.mClassData[method.mClassindex];
    uint32 methodidx = area.getMethodIndex(method);
    data->mVtable.push_back(methodidx);
    vidx = (unsigned)data->mVtable.size()-1;
  }
  else
    vidx = (*iter).second;
  return vidx;
}