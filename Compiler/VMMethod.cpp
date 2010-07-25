#include "VMMethod.h"

#include <iostream>
#include <string>
#include <cstring>

#include "JavaDefs.h"
#include "VMClass.h"
#include "VMContext.h"
#include "Trace.h"
#include "JVM.h"

#define PROC_DECL_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_DECL_MAP_MODE

void VMMethod::parseSignature(){
	mArgSize = 0;
	if (mSignature[0] != '(')
		TRACE(TRACE_JAVA, TRACE_ERROR, "Unexpected method signature");
	unsigned i;
	bool count_args = true;
	for (i = 1; i < mSignature.size(); ++i){
		if (mSignature[i] == ')'){
			++i;
			break;
		}
		else if (mSignature[i] == 'L'){
			do{
				++i;
			} while(mSignature[i] != ';');
			if (count_args)
				++mArgSize;
			count_args = true;
		}
		else if (mSignature[i] == 'I' || mSignature[i] == 'B' || mSignature[i] == 'C'|| mSignature[i] == 'Z' || mSignature[i] == 'F'  || mSignature[i] == 'S'){
			if (count_args)
				++mArgSize;
			count_args = true;
		}
		else if (mSignature[i] == '['){
			if (count_args)
				++mArgSize;
			count_args = false;
		}
		else if (mSignature[i] == 'J' || mSignature[i] == 'D'){
			if (count_args)
				mArgSize += 2;
			count_args = true;
		}
		else
			TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Unexpected argument type in method signature");
	}
	//parse return
	mReturnType = parseType(mSignature[i]);
}

VMMethod::ReturnType VMMethod::parseType(const char type){
	if (type == 'V')
		return Void;
	else if (type == 'J')
		return Long;
	else if (type == 'L')
		return Reference;
	else if (type == 'I')
		return Int;
	else if (type == 'Z')
		return Boolean;
	else if (type == 'C')
		return Char;
	else if (type == '[')
		return Array;
	else if (type == 'B')
		return Byte;
	else if (type == 'F')
		return Float;
	else if (type == 'D')
		return Double;
	else if (type == 'S')
		return Short;
	else
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Unexpected return type in method signature");
	return Void;
}

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
			case Java::op_new:
			case Java::op_putstatic:
			case Java::op_anewarray:
			case Java::op_putfield:
			case Java::op_ldc_w:
			case Java::op_instanceof:
			case Java::op_checkcast:
			case Java::op_getfield:
				{
					{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					Java::u2 operand = b1 << 8 | b2;
					strm << " #" << (int)operand;
				}
				break;
				}
			case Java::op_if_acmpne:
			case Java::op_if_icmpge:
			case Java::op_if_icmple:
			case Java::op_ifeq:
			case Java::op_ifge:
			case Java::op_iflt:
			case Java::op_ifne:
			case Java::op_ifle:
			case Java::op_ifnonnull:
			case Java::op_ifnull:
			case Java::op_goto:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 operand = b1 << 8 | b2;
					strm << " -> " << operand;
				}
				break;
			case Java::op_ldc:{
				Java::u1 operand = mCode->code[++k];
				strm << " #" << (int)operand;
				}
				break;
			case Java::op_bipush:
			case Java::op_newarray:
				{
				char operand = mCode->code[++k];
				strm << " " << (int)operand;
				}
				break;
			case Java::op_sipush:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					Java::u2 operand = b1 << 8 | b2;
					strm << " #" << operand;
				}
				break;
			case Java::op_invokeinterface:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					Java::u2 operand = b1 << 8 | b2;
					Java::u1 count = mCode->code[++k];
					Java::u1 null = mCode->code[++k];
					strm << " #" << operand << " " << (int)count << " " << (int)null;
				}
		}
		strm << "\n";
	}
}

void BcVMMethod::execute(VMContext* ctx){
	print(std::cout);
	unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
	ctx->pushFrame(this, argsize);
	//reserve additional locals
	for (unsigned i = 0; i < mCode->max_locals - argsize; ++i){
		ctx->push(0u);
	}
	for (unsigned k = 0; k < mCode->code_length; ++k){
    Java::u1 opcode = mCode->code[k];
		TRACE(TRACE_JAVA, TRACE_DEBUG, Opcode::map_string[opcode].c_str());
    switch (opcode){
			case Java::op_aconst_null:
				ctx->push((VMObject*)NULL);
				break;
			case Java::op_aload_0:
				ctx->push(ctx->get(0));
				break;
			case Java::op_aload_1:
				ctx->push(ctx->get(1));
				break;
			case Java::op_aload_2:
				ctx->push(ctx->get(2));
				break;
			case Java::op_aload_3:
				ctx->push(ctx->get(3));
				break;
			case Java::op_areturn:
				{
					StackData dat = ctx->pop();
					ctx->popFrame();
					ctx->push(dat);
					return;
				}
				break;
			case Java::op_ireturn:
				{
					StackData dat = ctx->pop();
					ctx->popFrame();
					ctx->push(dat);
					return;
				}
				break;
			case Java::op_arraylength:
				{
					VMArrayBase* base = (VMArrayBase*)ctx->pop().obj;
					ctx->push(base->getLength());
				}
				break;
			case Java::op_astore_0:
				{
					VMObject* obj = ctx->pop().obj;
					ctx->put(0, obj);
				}
				break;
			case Java::op_astore_1:
				{
					VMObject* obj = ctx->pop().obj;
					ctx->put(1, obj);
				}
				break;
			case Java::op_astore_2:
				{
					VMObject* obj = ctx->pop().obj;
					ctx->put(2, obj);
				}
				break;
			case Java::op_astore_3:
				{
					VMObject* obj = ctx->pop().obj;
					ctx->put(3, obj);
				}
				break;
			case Java::op_istore_0:
				{
					unsigned i = ctx->pop().ui;
					ctx->put(0, i);
				}
				break;
			case Java::op_istore_1:
				{
					unsigned i = ctx->pop().ui;
					ctx->put(1, i);
				}
				break;
			case Java::op_istore_2:
				{
					unsigned i = ctx->pop().ui;
					ctx->put(2, i);
				}
				break;
			case Java::op_istore_3:
				{
					unsigned i = ctx->pop().ui;
					ctx->put(3, i);
				}
				break;
			case Java::op_aaload:
				{
					unsigned idx = ctx->pop().ui;
					VMObjectArray* arr = (VMObjectArray*)ctx->pop().obj;
					ctx->push(arr->get(idx));
				}
				break;
			case Java::op_caload:
				{
					unsigned idx = ctx->pop().ui;
					VMCharArray* arr = (VMCharArray*)ctx->pop().obj;
					ctx->push(arr->get(idx));
				}
				break;
			case Java::op_castore:
				{
					jchar value = (jchar)ctx->pop().ui;
					unsigned idx = ctx->pop().ui;
					VMCharArray* arr = (VMCharArray*)ctx->pop().obj;
					arr->put(value, idx);
				}
				break;
			case Java::op_iastore:
				{
					jint value = ctx->pop().i;
					unsigned idx = ctx->pop().ui;
					VMIntArray* arr = (VMIntArray*)ctx->pop().obj;
					arr->put(value, idx);
				}
				break;
			case Java::op_lconst_0:
				ctx->push(0u);
				ctx->push(0u);
				break;
			case Java::op_iconst_0:
				ctx->push(0u);
				break;
			case Java::op_iconst_1:
				ctx->push(1u);
				break;
			case Java::op_iconst_2:
				ctx->push(2u);
				break;
			case Java::op_iconst_3:
				ctx->push(3u);
				break;
			case Java::op_iconst_4:
				ctx->push(4u);
				break;
			case Java::op_iconst_5:
				ctx->push(5u);
				break;
			case Java::op_fconst_0:
				ctx->push(0.0f);
				break;
      //single byte number
      case Java::op_bipush:
        {
					int constant = (int)(char)mCode->code[++k];
					ctx->push(constant);
        }
        break;
        //two byte number
      case Java::op_sipush:
        {
        Java::u1 b1 = mCode->code[++k];
        Java::u1 b2 = mCode->code[++k];
        int constant = (int)(short)(b1 << 8 | b2);
				ctx->push(constant);
        }
        break;
      case Java::op_ldc:
        {
					Java::u1 operand = mCode->code[++k];
					StackData constant = mClass->getConstant(ctx, operand);
					ctx->push(constant);
        }
        break;
      case Java::op_invokespecial:
        {
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod* mthd = mClass->getMethod(ctx, operand);
					mthd->execute(ctx);
          break;
        }
      case Java::op_lookupswitch:
      case Java::op_tableswitch:
        k+=98;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_invokeinterface:
        {
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
					Java::u1 count = mCode->code[++k];
					Java::u1 null = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMClass* execCls;
					unsigned idx = mClass->getMethodIndex(ctx, operand, execCls);
					VMMethod* temp = execCls->getMethod(idx); //TODO not very efficient
					VMObject* obj = ctx->getTop(temp->getNumArgs()).obj;
					VMMethod* mthd = obj->getObjMethod(idx);
					mthd->execute(ctx);
        }
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
				{
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					StackData constant = mClass->getConstant(ctx, operand);
					ctx->push(constant);
        }
        break;
      case Java::op_ldc2_w:
      case Java::op_iinc:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
				break;
      case Java::op_ifeq:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().ui == 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifne:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().ui != 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_iflt:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().ui < 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifge:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().ui >= 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifgt:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
				break;
      case Java::op_ifle:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().ui <= 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_if_icmpeq:
				{
					int val2 = ctx->pop().i;
					int val1 = ctx->pop().i;
					if (val1 == val2){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
				break;
      case Java::op_if_icmpne:
				{
					int val2 = ctx->pop().i;
					int val1 = ctx->pop().i;
					if (val1 != val2){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
				break;
      case Java::op_if_icmplt:
				{
					int val2 = ctx->pop().i;
					int val1 = ctx->pop().i;
					if (val1 < val2){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
				break;
      case Java::op_if_icmpge:
      case Java::op_if_icmpgt:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
				break;
      case Java::op_if_icmple:
				{
					int val2 = ctx->pop().i;
					int val1 = ctx->pop().i;
					if (val1 <= val2){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
				break;
      case Java::op_if_acmpeq:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
      case Java::op_if_acmpne:
				{
					VMObject* val2 = ctx->pop().obj;
					VMObject* val1 = ctx->pop().obj;
					if (val1 != val2){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
				break;
      case Java::op_goto:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					k += branch-3;
				}
				break;
      case Java::op_jsr:
        k+=2;
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_getstatic:
        {
          Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod::ReturnType type;
					FieldData* obj = mClass->getField(ctx, operand, type);
					if (type == Long || type == Double){
						TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Type not implemented");
					}
					ctx->push(obj->obj);
        }
        break;
      case Java::op_putstatic:
				{
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod::ReturnType type;
					FieldData* obj = mClass->getField(ctx, operand, type);
					if (type == Long || type == Double){
						TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Type not implemented");
					}
					VMObject* data = ctx->pop().obj;
					obj->obj = data;
				}
				break;
      case Java::op_getfield:
				{
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMObject* obj = ctx->pop().obj;
					VMClass* dummy;
					VMMethod::ReturnType type;
					unsigned idx = mClass->getFieldIndex(ctx, operand, dummy, type);
					if (type == Long || type == Double){
						TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Type not implemented");
					}
					FieldData* data = obj->getObjField(idx);
					ctx->push(data->obj);
				}
        break;
      case Java::op_putfield:
        {
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMClass* dummy;
					VMMethod::ReturnType type;
					unsigned idx = mClass->getFieldIndex(ctx, operand, dummy, type);
					FieldData value;
					if (type == Long || type == Double){
						value.l = (int64)ctx->pop().ui;
						value.l |= ((int64)ctx->pop().ui) << 32;
					}
					else{
						value.obj = ctx->pop().obj;
					}
					VMObject* obj = ctx->pop().obj;
					FieldData* data = obj->getObjField(idx);
					*data = value;
					//FieldData* obj = mClass->getFieldIndex(operand);
				}
        break;
      case Java::op_invokestatic:
        {
          Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod* mthd = mClass->getMethod(ctx, operand);
					mthd->execute(ctx);
          break;
        }
        break;
      case Java::op_invokevirtual:
        {
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMClass* execCls;
					unsigned idx = mClass->getMethodIndex(ctx, operand, execCls);
					VMMethod* temp = execCls->getMethod(idx); //TODO not very efficient
					VMObject* obj = ctx->getTop(temp->getNumArgs()).obj;
					VMMethod* mthd = obj->getObjMethod(idx);
					mthd->execute(ctx);
        }
        break;
			case Java::op_new:{
				Java::u1 b1 = mCode->code[++k];
        Java::u1 b2 = mCode->code[++k];
        Java::u2 operand = b1 << 8 | b2;
				VMClass* newcls = mClass->getClass(ctx, operand);
				VMObject* obj = getVM()->createObject(ctx, newcls);
				ctx->push(obj);
				break;
				}
			case Java::op_anewarray:{
				Java::u1 b1 = mCode->code[++k];
        Java::u1 b2 = mCode->code[++k];
        Java::u2 operand = b1 << 8 | b2;
				VMClass* arrcls = mClass->getClass(ctx, operand);
				int arrsize = ctx->pop().ui;
				VMObjectArray* arr = getVM()->createObjectArray(arrsize);
				ctx->push((VMObject*)arr);
				break;
				}
      case Java::op_checkcast:
				{
				Java::u1 b1 = mCode->code[++k];
        Java::u1 b2 = mCode->code[++k];
        Java::u2 operand = b1 << 8 | b2;
				VMClass* cls = mClass->getClass(ctx, operand);
				VMObject* instance = ctx->getTop(0).obj;
				if (instance == NULL)
					break; //null is always castable
				VMClass* instanceClass = instance->getClass();
				bool found = false;
				do {
					if (instanceClass == cls){
						found = true;
						break;
					}
					instanceClass = instanceClass->getSuperclass(ctx);
				} while (instanceClass != NULL);
				if (!found){
					TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s check failed", Opcode::map_string[opcode].c_str());
				}
				}
        break;
      case Java::op_instanceof:
				{
				Java::u1 b1 = mCode->code[++k];
        Java::u1 b2 = mCode->code[++k];
        Java::u2 operand = b1 << 8 | b2;
				VMClass* cls = mClass->getClass(ctx, operand);
				VMObject* instance = ctx->pop().obj;
				VMClass* instanceClass = instance->getClass();
				bool found = false;
				do {
					if (instanceClass == cls){
						found = true;
						ctx->push(1);
						break;
					}
					instanceClass = instanceClass->getSuperclass(ctx);
				} while (instanceClass != NULL);
				if (!found){
					ctx->push(0u);
				}
				}
        break;
      case Java::op_ifnull:
				{
					VMObject* obj = ctx->pop().obj;
					if (obj == NULL){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
        break;
      case Java::op_ifnonnull:
				{
					VMObject* obj = ctx->pop().obj;
					if (obj != NULL){
						Java::u1 b1 = mCode->code[++k];
						Java::u1 b2 = mCode->code[++k];
						int16 branch = b1 << 8 | b2;
						k += branch-3;
					}
					else{
					  k += 2;
					}
				}
        break;
      case Java::op_iload:
				{
				Java::u1 idx = mCode->code[++k];
				ctx->push(ctx->get(idx));
				}
				break;
			case Java::op_iload_0:
				ctx->push(ctx->get(0));
				break;
			case Java::op_iload_1:
				ctx->push(ctx->get(1));
				break;
			case Java::op_iload_2:
				ctx->push(ctx->get(2));
				break;
			case Java::op_iload_3:
				ctx->push(ctx->get(3));
				break;
			case Java::op_isub:
				{
					int v2 = ctx->pop().i;
					int v1 = ctx->pop().i;
					ctx->push(v1-v2);
				}
				break;
			case Java::op_iadd:
				{
					int v2 = ctx->pop().i;
					int v1 = ctx->pop().i;
					ctx->push(v1+v2);
				}
				break;
			case Java::op_imul:
				{
					int v2 = ctx->pop().i;
					int v1 = ctx->pop().i;
					ctx->push(v1*v2);
				}
				break;
      case Java::op_lload:
      case Java::op_fload:
      case Java::op_dload:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        k+=1;
      case Java::op_aload:
				{
					Java::u1 operand = mCode->code[++k];
					ctx->push(ctx->get(operand));
				}
				break;
      case Java::op_istore:
      case Java::op_lstore:
      case Java::op_fstore:
      case Java::op_dstore:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        k+=1;
        break;
			case Java::op_fload_1:
				ctx->push(ctx->get(1));
				break;
			case Java::op_fload_2:
				ctx->push(ctx->get(2));
				break;
      case Java::op_astore:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_ret:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
        k+=1;
        break;
      case Java::op_newarray:
				{
					uint8 type = mCode->code[++k];
					unsigned size = ctx->pop().ui;
					VMArrayBase* arr;
					switch (type){
						case Char:
							arr = getVM()->createCharArray(size);
							break;
						case Int:
							arr = getVM()->createIntArray(size);
							break;
						default:
							TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s type unimplemented", Opcode::map_string[opcode].c_str());
					};
					ctx->push(arr);
				}
        break;
			case Java::op_lcmp:{
				int64 l1;
				int64 l2;
				l1 = (int64)ctx->pop().ui;
				l1 |= ((int64)ctx->pop().ui) << 32;
				l2 = (int64)ctx->pop().ui;
				l2 |= ((int64)ctx->pop().ui) << 32;
				ctx->push(l1 == l2 ? 0 : -1);
				break;
		  }
			case Java::op_return:{
				ctx->popFrame();
				return;
				break;
			}
			case Java::op_dup:{
				ctx->dup();
				break;
			}
			case Java::op_pop:{
				ctx->pop();
				break;
			}
			case Java::op_fcmpg:
				{
					float val2 = ctx->pop().f;
					float val1 = ctx->pop().f;
					ctx->push(val1 > val2 ? 0 : -1);
				}
				break;
			case Java::op_iushr:
				{
					int value2 = ctx->pop().i;
					int value1 = ctx->pop().i;
					ctx->push(value1 >> value2);
				}
				break;
			case Java::op_ixor:
				{
					int value2 = ctx->pop().i;
					int value1 = ctx->pop().i;
					ctx->push(value1 ^ value2);
				}
				break;
			case Java::op_iand:
				{
					int value2 = ctx->pop().i;
					int value1 = ctx->pop().i;
					ctx->push(value1 & value2);
				}
				break;
			case Java::op_monitorenter:
				{
					VMObject* obj = ctx->pop().obj;
					obj->lock();
					break;
				}
			case Java::op_monitorexit:
				{
					VMObject* obj = ctx->pop().obj;
					obj->unlock();
					break;
				}
			default:
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "%s unimplemented", Opcode::map_string[opcode].c_str());
				break;
    }
  }
	ctx->popFrame();
}

void BcVMMethod::executeVoidRet(VMContext* ctx){
}

void BcVMMethod::executeLongRet(VMContext* ctx){
}

void BcVMMethod::executeRefRet(VMContext* ctx){
}

void NativeVMMethod::print(std::ostream& strm){
	strm << "Native method\n";
}

void NativeVMMethod::execute(VMContext* ctx){
	switch (mReturnType){
		case Void:
			executeVoidRet(ctx);
			break;
		case Long:
			executeLongRet(ctx);
			break;
		case Reference:
		case Array:
		case Int:
			executeRefRet(ctx);
			break;
		case Boolean:
			executeBoolRet(ctx);
			break;
		default:
			TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Unhandled return type called");
			break;
	}
}

void NativeVMMethod::executeVoidRet(VMContext* ctx){
	if (mFunction == NULL)
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Could not resolve native method");
	unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
	ctx->pushFrame(this, argsize);
	VMClass* cls = mIsStatic ? mClass : ctx->get(0).cls;
	uint8* lst = packArguments(ctx);
	delete [] lst;
	mFunction(ctx->getJNIEnv(), cls, *lst);
	ctx->popFrame();
}

void NativeVMMethod::executeLongRet(VMContext* ctx){
	if (mFunction == NULL)
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Could not resolve native method");
	unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
	ctx->pushFrame(this, argsize);
	VMClass* cls = mIsStatic ? mClass : ctx->get(0).cls;
	uint8* lst = packArguments(ctx);
	int64 ret = ((nativeLongMethod)mFunction)(ctx->getJNIEnv(), cls, *((int*)lst));
	delete [] lst;
	ctx->popFrame();
	ctx->push((uint32)(ret >> 32));
	ctx->push((uint32)ret);
}

void NativeVMMethod::executeRefRet(VMContext* ctx){
	if (mFunction == NULL)
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Could not resolve native method");
	unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
	ctx->pushFrame(this, argsize);
	VMObject* cls = mIsStatic ? mClass : ctx->get(0).cls;
	uint8* lst = packArguments(ctx);
	void* ret = ((nativeRefMethod)mFunction)(ctx->getJNIEnv(), cls, *((int*)lst));
	delete [] lst;
	ctx->popFrame();
	ctx->push((VMObject*)ret);
}

void NativeVMMethod::executeBoolRet(VMContext* ctx){
	if (mFunction == NULL)
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Could not resolve native method");
	unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
	ctx->pushFrame(this, argsize);
	VMClass* cls = mIsStatic ? mClass : ctx->get(0).cls;
	uint8* lst = packArguments(ctx);
	jboolean ret = ((nativeBoolMethod)mFunction)(ctx->getJNIEnv(), cls, *((int*)lst));
	delete [] lst;
	ctx->popFrame();
	ctx->push(ret);
}

uint8* NativeVMMethod::packArguments(VMContext* ctx){
	union {
		va_list varargs;
		uint8* array;
	} fakeArray;
	fakeArray.array = new uint8[mArgSize*sizeof(uint32)];
	uint8* curr = fakeArray.array;
	bool count_args = true;
	for (unsigned i = 1; i < mSignature.size(); ++i){
		if (mSignature[i] == ')'){
			++i;
			break;
		}
		else if (mSignature[i] == 'L'){
			do{
				++i;
			} while(mSignature[i] != ';');
			if (count_args){
				void* p = ctx->pop().obj;
				memcpy(curr, &p, sizeof(p));
			}
			count_args = true;
		}
		else if (mSignature[i] == 'I'){
			if (count_args){
				int32 i = ctx->pop().i;
				memcpy(curr, &i, sizeof(i));
			}
			count_args = true;
		}
		else if (mSignature[i] == 'Z'){
			if (count_args){
				jboolean b = ctx->pop().i;
				memcpy(curr, &b, sizeof(b));
			}
			count_args = true;
		}
		/*else if (mSignature[i] == '['){
			if (count_args)
				++mArgSize;
			count_args = false;
		}
		else if (mSignature[i] == 'J' || mSignature[i] == 'D'){
			if (count_args)
				mArgSize += 2;
			count_args = true;
		}*/
		else
			TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Unexpected argument type in method signature");
	}
	return (uint8*)fakeArray.varargs;
}
