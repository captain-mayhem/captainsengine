#include "VMMethod.h"

#include <iostream>
#include <string>

#include "JavaDefs.h"
#include "VMClass.h"
#include "VMContext.h"
#include "Trace.h"

#define PROC_DECL_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_DECL_MAP_MODE

void BcVMMethod::print(std::ostream& strm){
	for (unsigned k = 0; k < mCode->code_length; ++k){
		Java::u1 opcode = mCode->code[k];
		std::string cd = Opcode::map_string[opcode];
		strm << "\t"/*<< code->code[k]*/ << cd;
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
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					Java::u2 operand = b1 << 8 | b2;
					strm << " #" << (int)operand;
				}
				break;
			case Java::op_ldc:{
				Java::u1 operand = mCode->code[++k];
				strm << " #" << (int)operand;
				}
				break;

		}
		strm << "\n";
	}
}

void BcVMMethod::execute(VMContext* ctx, VMClass* cls){
	for (unsigned k = 0; k < mCode->code_length; ++k){
    Java::u1 opcode = mCode->code[k];
		TRACE(TRACE_JAVA, TRACE_DEBUG, Opcode::map_string[opcode].c_str());
    switch (opcode){
      //single byte number
      case Java::op_bipush:
        {
        Java::u1 constant = mCode->code[++k];
        }
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
        //two byte number
      case Java::op_sipush:
        {
        Java::u1 b1 = mCode->code[++k];
        Java::u1 b2 = mCode->code[++k];
        Java::u2 constant = b1 << 8 | b2;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        }
        break;
      case Java::op_ldc:
        {
					TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        /*Java::u1 operand = mCode->code[++k];
        Java::cp_info* info = mClass.constant_pool[operand-1];
        if (info->tag == CONSTANT_Integer){
          Java::CONSTANT_Integer_info* intinf = (Java::CONSTANT_Integer_info*)(info);
          op.data.mInt = intinf->bytes;
        }*/
        /*else if (info->tag == CONSTANT_Long){

        }
        else if (info->tag == CONSTANT_Float){
          Java::CONSTANT_Float_info* fltinf = (Java::CONSTANT_Float_info*)(info);
          op.data.mFloat = fltinf->bytes;
        }
        else if (info->tag == CONSTANT_Double){

        }*/
        //else if (info->tag == CONSTANT_String){
          /*Java::CONSTANT_String_info* str = (Java::CONSTANT_String_info*)(info);
          Java::CONSTANT_Utf8_info* utf = (Java::CONSTANT_Utf8_info*)(mClass.constant_pool[str->string_index-1]);
          area.getClassIndex("java/lang/String");
          area.mStrings.push_back(utf->bytes);*/
        //}
        //else{
        //  TRACE(TRACE_JAVA, TRACE_ERROR, "Unhandled opcode in ldc");
        //}
        //op.data.i = (int)info;
        //op.data1 = constant;
        }
        break;
      case Java::op_invokespecial:
        {
					TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        /*Java::u1 b1 = code->code[++k];
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
        break;*/
                                  }
      case Java::op_lookupswitch:
      case Java::op_tableswitch:
        k+=98;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_invokeinterface:
        k+=4;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_jsr_w:
      case Java::op_goto_w:
        k+=4;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_multianewarray:
        k+=3;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
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
          Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMField* fld = cls->findField(ctx, operand);
					TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        }
        break;
      case Java::op_putstatic:
      case Java::op_getfield:
      case Java::op_putfield:
        k+=2;
        break;
      case Java::op_invokestatic:
        {
          Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMClass* execCls;
					VMMethod* mthd = cls->getMethod(ctx, operand, execCls);
					mthd->execute(ctx, execCls);
					//TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
          break;
        }
        break;
      case Java::op_invokevirtual:
        {
					TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
          /*Java::u1 b1 = code->code[++k];
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
          op.data.mUint = methodidx;*/
        }
        break;
      case Java::op_new:
      case Java::op_anewarray:
      case Java::op_checkcast:
      case Java::op_instanceof:
      case Java::op_ifnull:
      case Java::op_ifnonnull:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
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
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        k+=1;
        break;
			default:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
				break;
    }
  }
}

void NativeVMMethod::print(std::ostream& strm){
	strm << "Native method\n";
}

void NativeVMMethod::execute(VMContext* ctx, VMClass* cls){
	mFunction(ctx->getJNIEnv(), cls);
}
