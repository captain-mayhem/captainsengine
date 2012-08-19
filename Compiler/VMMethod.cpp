#include "VMMethod.h"

#include <iostream>
#include <string>
#include <cstring>
#include <float.h>
#include <math.h>
#ifndef WIN32
#include <alloca.h>
#endif

#include "JavaDefs.h"
#include "VMclass.h"
#include "VMContext.h"
#include "Trace.h"
#include "JVM.h"
#include "VMArray.h"

#define PROC_DECL_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_DECL_MAP_MODE

#ifdef UNIX
#define _isnan isnan
#endif

TR_CHANNEL(Java_Method);
//TR_CHANNEL_LVL(Java_Method, TRACE_DEBUG);
TR_CHANNEL(Java_Properties);

#define HANDLE_EXCEPTION(__ip__) \
  {if (!handleException(&__ip__, ctx)) \
    return;}

using namespace std;

VMMethod::~VMMethod(){

}

bool VMMethod::handleException(unsigned *ip, VMContext* ctx){
  if (ctx->getException() == NULL)
    return true; //no exception, everything is alright
  int catchip = getClass()->getCatchIP(*ip, ctx, ctx->getException(), mMethodIndex);
  if (catchip >= 0){
    ctx->push(ctx->getException());
    ctx->throwException(NULL);
    *ip = catchip-1;
    return true;
  }
  ctx->popFrame();
  return false;
}

void VMMethod::parseSignature(){
  TR_USE(Java_Method);
	mArgSize = 0;
	if (mSignature[0] != '(')
		TR_ERROR("Unexpected method signature");
	unsigned i;
	bool count_args = true;
  std::string currArg;
	for (i = 1; i < mSignature.size(); ++i){
		if (mSignature[i] == ')'){
			++i;
			break;
		}
		else if (mSignature[i] == 'L'){
			do{
				++i;
        currArg.append(1, mSignature[i]);
			} while(mSignature[i] != ';');
      currArg.erase(currArg.size()-1);
      if (count_args){
				++mArgSize;
      }
			count_args = true;
      mSplitSignature.push_back(currArg);
      currArg.clear();
		}
		else if (mSignature[i] == 'I' || mSignature[i] == 'B' || mSignature[i] == 'C'|| mSignature[i] == 'Z' || mSignature[i] == 'F'  || mSignature[i] == 'S'){
			currArg.append(1, mSignature[i]);
      if (count_args){
        ++mArgSize;
      }
      mSplitSignature.push_back(currArg);
      currArg.clear();
			count_args = true;
		}
		else if (mSignature[i] == '['){
      if (count_args){
        ++mArgSize;
      }
			count_args = false;
      currArg.append(1, mSignature[i]);
		}
		else if (mSignature[i] == 'J' || mSignature[i] == 'D'){
      currArg.append(1, mSignature[i]);
      if (count_args){
        mSplitSignature.push_back(currArg);
        currArg.clear();
        mArgSize += 2;
      }
			count_args = true;
		}
		else
			TR_ERROR("Unexpected argument type in method signature");
	}
	//parse return
	mReturnType = parseType(mSignature[i]);
  mReturnSignature = mSignature.substr(i);
  if (mReturnSignature[0] == 'L'){
    mReturnSignature.erase(0, 1);
    mReturnSignature.erase(mReturnSignature.size()-1);
  }
}

VMMethod::ReturnType VMMethod::parseType(const char type){
  TR_USE(Java_Method);
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
		TR_BREAK("Unexpected return type in method signature");
	return Void;
}

VMClass* VMMethod::getParameterClass(VMContext* ctx, int idx){
  if (idx < 0)
    return getVM()->findClass(ctx, mReturnSignature);
  return getVM()->findClass(ctx, mSplitSignature[idx]);
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
			case Java::op_ldc2_w:
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
			case Java::op_if_icmpgt:
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
			case Java::op_ldc:
			case Java::op_istore:
			case Java::op_astore:
			case Java::op_lload:
				{
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
			case Java::op_iinc:
				{
				Java::u1 operand = mCode->code[++k];
				int8 val = mCode->code[++k];
				strm << " #" << (int)operand << "+" << (int)val;
				}
				break;
		}
		strm << "\n";
	}
}

static int method_depth = 0;

inline static Java::u4 byteOrder(unsigned val){
  Java::u4 ret = 0;
  ret |= (val << 24)&0xFF000000;
  ret |= (val << 8)&0x00FF0000;
  ret |= (val >> 8)&0x0000FF00;
  ret |= (val >> 24)&0x000000FF;
  return ret;
}

void BcVMMethod::execute(VMContext* ctx, unsigned ret){
  TR_USE(Java_Method);
	//if (TRACE_IS_ENABLED(TRACE_JAVA))
	//	print(std::cout);
	unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
	if (TR_IS_ENABLED(TRACE_DEBUG))
		++method_depth;
	TR_DEBUG("%i: %s (%s)", method_depth, mName.c_str(), mClass->getName().c_str());
	ctx->pushFrame(this, ret, argsize);
	//reserve additional locals
	for (unsigned i = 0; i < mCode->max_locals - argsize; ++i){
		ctx->push(0u);
	}
  if (mName == "getProperty" && mClass->getName() == "java/util/Properties"){
    TR_USE(Java_Properties);
    std::string data = JVM::stringToString(ctx->get(1).obj);
    TR_INFO("property %s requested", data.c_str());
  }
  if (mName == "setProperty" && mClass->getName() == "java/util/Properties"){
    TR_USE(Java_Properties);
    std::string data = JVM::stringToString(ctx->get(1).obj);
    std::string data2 = JVM::stringToString(ctx->get(2).obj);
    TR_INFO("property %s is set to %s", data.c_str(), data2.c_str());
  }
  /*if (mClass->getName() == "java/lang/CharacterDataLatin1" && mName == "toLowerCase")
    TR_BREAK("Test");*/
	for (unsigned k = 0; k < mCode->code_length; ++k){
    Java::u1 opcode = mCode->code[k];
		TR_DEBUG(Opcode::map_string[opcode].c_str());
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
					if (TR_IS_ENABLED(TRACE_DEBUG))
						--method_depth;
					TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
					ctx->push(dat);
					return;
				}
				break;
			case Java::op_ireturn:
				{
					StackData dat = ctx->pop();
					ctx->popFrame();
					if (TR_IS_ENABLED(TRACE_DEBUG))
						--method_depth;
					TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
					ctx->push(dat);
					return;
				}
				break;
			case Java::op_lreturn:
				{
					StackData dat1 = ctx->pop();
					StackData dat2 = ctx->pop();
					ctx->popFrame();
          if (TR_IS_ENABLED(TRACE_DEBUG))
            --method_depth;
          TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
					ctx->push(dat1);
					ctx->push(dat2);
					return;
				}
				break;
      case Java::op_dreturn:
				{
					StackData dat1 = ctx->pop();
					StackData dat2 = ctx->pop();
					ctx->popFrame();
          if (TR_IS_ENABLED(TRACE_DEBUG))
            --method_depth;
          TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
					ctx->push(dat1);
					ctx->push(dat2);
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
			case Java::op_lstore_0:
				{
					unsigned i = ctx->pop().ui;
					unsigned i2 = ctx->pop().ui;
					ctx->put(0, i);
					ctx->put(1, i2);
				}
				break;
      case Java::op_lstore_1:
        {
          unsigned i = ctx->pop().ui;
          unsigned i2 = ctx->pop().ui;
          ctx->put(1, i);
          ctx->put(2, i2);
        }
        break;
			case Java::op_lstore_2:
				{
					unsigned i = ctx->pop().ui;
					unsigned i2 = ctx->pop().ui;
					ctx->put(2, i);
					ctx->put(3, i2);
				}
				break;
      case Java::op_lstore_3:
        {
          unsigned i = ctx->pop().ui;
          unsigned i2 = ctx->pop().ui;
          ctx->put(3, i);
          ctx->put(4, i2);
        }
        break;
			case Java::op_aaload:
				{
					unsigned idx = ctx->pop().ui;
					VMObjectArray* arr = (VMObjectArray*)ctx->pop().obj;
					ctx->push(arr->get(idx));
				}
				break;
      case Java::op_baload:
        {
          unsigned idx = ctx->pop().ui;
          VMByteArray* arr = (VMByteArray*)ctx->pop().obj;
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
      case Java::op_iaload:
				{
					unsigned idx = ctx->pop().ui;
					VMIntArray* arr = (VMIntArray*)ctx->pop().obj;
					ctx->push(arr->get(idx));
				}
				break;
      case Java::op_laload:
        {
          unsigned idx = ctx->pop().ui;
          VMLongArray* arr = (VMLongArray*)ctx->pop().obj;
          jlong l = arr->get(idx);
          ctx->push((uint32)(l >> 0));
          ctx->push((uint32)(l >> 32));
        }
        break;
			case Java::op_aastore:
				{
					VMObject* value = ctx->pop().obj;
					unsigned idx = ctx->pop().ui;
					VMObjectArray* arr = (VMObjectArray*)ctx->pop().obj;
					arr->put(value, idx);
				}
				break;
      case Java::op_bastore:
        {
          jbyte value = (jbyte)ctx->pop().ui;
          unsigned idx = ctx->pop().ui;
          VMByteArray* arr = (VMByteArray*)ctx->pop().obj;
          arr->put(value, idx);
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
      case Java::op_fastore:
				{
					jfloat value = ctx->pop().f;
					unsigned idx = ctx->pop().ui;
					VMFloatArray* arr = (VMFloatArray*)ctx->pop().obj;
					arr->put(value, idx);
				}
				break;
      case Java::op_dastore:
				{
          FieldData value;
          value.l = ((int64)ctx->pop().ui) << 32;
					value.l |= ((int64)ctx->pop().ui) << 0;
					unsigned idx = ctx->pop().ui;
					VMDoubleArray* arr = (VMDoubleArray*)ctx->pop().obj;
					arr->put(value.d, idx);
				}
				break;
      case Java::op_lastore:
				{
          jlong value;
          value = ((int64)ctx->pop().ui) << 32;
					value |= ((int64)ctx->pop().ui) << 0;
					unsigned idx = ctx->pop().ui;
					VMLongArray* arr = (VMLongArray*)ctx->pop().obj;
					arr->put(value, idx);
				}
				break;
			case Java::op_lconst_0:
				ctx->push(0u);
				ctx->push(0u);
				break;
			case Java::op_lconst_1:
				ctx->push(1u);
				ctx->push(0u);
				break;
      case Java::op_dconst_0:
        FieldData constant;
        constant.d = 0.0;
        ctx->push((uint32)(constant.l >> 0));
				ctx->push((uint32)(constant.l >> 32));
        break;
      case Java::op_dconst_1:
        /*FieldData constant;*/
        constant.d = 1.0;
        ctx->push((uint32)(constant.l >> 0));
				ctx->push((uint32)(constant.l >> 32));
        break;
      case Java::op_iconst_m1:
				ctx->push(-1);
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
      case Java::op_fconst_1:
				ctx->push(1.0f);
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
					FieldData constant = mClass->getConstant(ctx, operand);
					ctx->push(constant.obj);
        }
        break;
      case Java::op_invokespecial:
        {
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod* mthd = mClass->getMethod(ctx, operand);
					mthd->execute(ctx, k);
          HANDLE_EXCEPTION(k);
          break;
        }
      case Java::op_lookupswitch:
        {
          unsigned kaligned = (k+4)/4*4; //padding
          if (kaligned-k != 1)
            TR_BREAK("Check if correct (boundaries)");
          unsigned* address = (unsigned*)&mCode->code[kaligned];
          Java::u4 default_offset = byteOrder(*address++);
          Java::u4 n = byteOrder(*address++);
          int item = ctx->pop().i;
          for (unsigned i = 0; i < n; ++i){
            int key = byteOrder(*address++);
            int value = byteOrder(*address++);
            if (key == item){
              k += value;
              goto skipdefault;
            }
            else if (key > item) //table is sorted, no need to go further
              break;
          }
          k += default_offset;
skipdefault:
          k -= 1;
          break;
        }
      case Java::op_tableswitch:
        k+=98;
				TR_BREAK("%s unimplemented", Opcode::map_string[opcode].c_str());
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
					mthd->execute(ctx, k);
          HANDLE_EXCEPTION(k);
        }
        break;
      case Java::op_jsr_w:
      case Java::op_goto_w:
        k+=4;
				TR_BREAK("%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_multianewarray:
        k+=3;
				TR_BREAK("%s unimplemented", Opcode::map_string[opcode].c_str());
        break;
      case Java::op_ldc_w:
				{
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					FieldData constant = mClass->getConstant(ctx, operand);
					ctx->push(constant.obj);
        }
        break;
      case Java::op_ldc2_w:
				{
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					FieldData constant = mClass->getConstant(ctx, operand);
					ctx->push((uint32)(constant.l >> 0));
					ctx->push((uint32)(constant.l >> 32));
        }
        break;
      case Java::op_iinc:
				{
					Java::u1 idx = mCode->code[++k];
          int val = (int8)mCode->code[++k];
					ctx->put(idx, ctx->get(idx).i+val);
				}
				break;
      case Java::op_ifeq:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().i == 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifne:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().i != 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_iflt:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().i < 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifgt:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().i > 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifge:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().i >= 0){
						k += branch-3;
					}
				}
				break;
      case Java::op_ifle:
				{
					Java::u1 b1 = mCode->code[++k];
					Java::u1 b2 = mCode->code[++k];
					int16 branch = b1 << 8 | b2;
					if (ctx->pop().i <= 0){
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
				{
					int val2 = ctx->pop().i;
					int val1 = ctx->pop().i;
					if (val1 >= val2){
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
			case Java::op_if_icmpgt:
				{
					int val2 = ctx->pop().i;
					int val1 = ctx->pop().i;
					if (val1 > val2){
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
				{
					VMObject* val2 = ctx->pop().obj;
					VMObject* val1 = ctx->pop().obj;
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
        {
          ctx->push(k+3-1);
          Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          int16 branch = b1 << 8 | b2;
          k += branch-3;
        }
        break;
      case Java::op_getstatic:
        {
          Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod::ReturnType type;
					FieldData* obj = mClass->getStaticField(ctx, operand, type);
					if (type == Long || type == Double){
						ctx->push((uint32)(obj->l >> 0));
						ctx->push((uint32)(obj->l >> 32));
					}
					else
						ctx->push(obj->obj);
        }
        break;
      case Java::op_putstatic:
				{
					Java::u1 b1 = mCode->code[++k];
          Java::u1 b2 = mCode->code[++k];
          Java::u2 operand = b1 << 8 | b2;
					VMMethod::ReturnType type;
					FieldData* obj = mClass->getStaticField(ctx, operand, type);
					FieldData value;
					if (type == Long || type == Double){
						value.l = ((int64)ctx->pop().ui) << 32;
						value.l |= ((int64)ctx->pop().ui) << 0;
					}
					else{
						value.obj = ctx->pop().obj;
					}
					*obj = value;
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
          if (idx == 0)
            TR_BREAK("field %i not existing", (int)operand);
					FieldData* data = obj->getObjField(idx);
					if (type == Long || type == Double){
						ctx->push((uint32)(data->l >> 0));
						ctx->push((uint32)(data->l >> 32));
					}
					else
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
						value.l = ((int64)ctx->pop().ui) << 32;
						value.l |= ((int64)ctx->pop().ui) << 0;
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
					mthd->execute(ctx, k);
          HANDLE_EXCEPTION(k);
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
          if (idx == 0)
            TR_BREAK("Cannot resolve method");
					VMMethod* temp = execCls->getMethod(idx); //TODO not very efficient
					VMObject* obj = ctx->getTop(temp->getNumArgs()).obj;
					VMMethod* mthd = obj->getObjMethod(idx);
					mthd->execute(ctx, k);
          HANDLE_EXCEPTION(k);
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
				VMObjectArray* arr = getVM()->createObjectArray(ctx, arrcls, arrsize);
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
					TR_BREAK("%s check failed", Opcode::map_string[opcode].c_str());
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
        if (instance == NULL){
          ctx->push(0u);
          break;
        }
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
      case Java::op_idiv:
				{
					int v2 = ctx->pop().i;
					int v1 = ctx->pop().i;
					ctx->push(v1/v2);
				}
				break;
      case Java::op_irem:
				{
					int v2 = ctx->pop().i;
					int v1 = ctx->pop().i;
          //(value2 - ((value1 / value2) * value2))
					ctx->push(v1%v2);
				}
				break;
      case Java::op_ineg:
        {
          int v = ctx->pop().i;
          ctx->push(v*-1);
        }
        break;
      case Java::op_fmul:
				{
					float v2 = ctx->pop().f;
					float v1 = ctx->pop().f;
					ctx->push(v1*v2);
				}
				break;
      case Java::op_lload:
				{
					Java::u1 operand = mCode->code[++k];
					ctx->push(ctx->get(operand+1));
					ctx->push(ctx->get(operand+0));
				}
				break;
      case Java::op_dload:
				{
					Java::u1 operand = mCode->code[++k];
					ctx->push(ctx->get(operand+1));
					ctx->push(ctx->get(operand+0));
				}
				break;
      case Java::op_fload:
				TR_BREAK("%s unimplemented", Opcode::map_string[opcode].c_str());
        k+=1;
      case Java::op_aload:
				{
					Java::u1 operand = mCode->code[++k];
					ctx->push(ctx->get(operand));
				}
				break;
      case Java::op_istore:
				{
					unsigned idx = mCode->code[++k];
					unsigned i = ctx->pop().ui;
					ctx->put(idx, i);
				}
				break;
      case Java::op_dstore:
				{
					unsigned idx = mCode->code[++k];
					unsigned i = ctx->pop().ui;
					unsigned i2 = ctx->pop().ui;
					ctx->put(idx, i);
					ctx->put(idx+1, i2);
				}
        break;
      case Java::op_lstore:
        {
					unsigned idx = mCode->code[++k];
					unsigned i = ctx->pop().ui;
					unsigned i2 = ctx->pop().ui;
					ctx->put(idx, i);
					ctx->put(idx+1, i2);
				}
        break;
      case Java::op_fstore:
				TR_BREAK("%s unimplemented", Opcode::map_string[opcode].c_str());
        k+=1;
        break;
			case Java::op_fload_0:
				ctx->push(ctx->get(0));
				break;
			case Java::op_fload_1:
				ctx->push(ctx->get(1));
				break;
			case Java::op_fload_2:
				ctx->push(ctx->get(2));
				break;
      case Java::op_fload_3:
        ctx->push(ctx->get(3));
        break;
			case Java::op_dload_0:
				ctx->push(ctx->get(1));
				ctx->push(ctx->get(0));
				break;
      case Java::op_dload_2:
				ctx->push(ctx->get(3));
				ctx->push(ctx->get(2));
				break;
			case Java::op_lload_0:
				ctx->push(ctx->get(1));
				ctx->push(ctx->get(0));
				break;
      case Java::op_lload_1:
				ctx->push(ctx->get(2));
				ctx->push(ctx->get(1));
				break;
			case Java::op_lload_2:
				ctx->push(ctx->get(3));
				ctx->push(ctx->get(2));
				break;
      case Java::op_lload_3:
        ctx->push(ctx->get(4));
        ctx->push(ctx->get(3));
        break;
			case Java::op_astore:
				{
					unsigned idx = mCode->code[++k];
					VMObject* obj = ctx->pop().obj;
					ctx->put(idx, obj);
				}
        break;
      case Java::op_ret:
        {
          unsigned idx = mCode->code[++k];
          int ret = ctx->get(idx).i;
          k = ret;
        }
        break;
      case Java::op_newarray:
				{
					uint8 type = mCode->code[++k];
					unsigned size = ctx->pop().ui;
					VMArrayBase* arr;
					switch (type){
						case Char:
							arr = getVM()->createCharArray(ctx, size);
							break;
            case Byte:
              arr = getVM()->createByteArray(ctx, size);
              break;
						case Int:
							arr = getVM()->createIntArray(ctx, size);
							break;
            case Float:
              arr = getVM()->createFloatArray(ctx, size);
              break;
            case Double:
              arr = getVM()->createDoubleArray(ctx, size);
              break;
            case Long:
              arr = getVM()->createLongArray(ctx, size);
              break;
						default:
							TR_BREAK("%s type unimplemented", Opcode::map_string[opcode].c_str());
					};
					ctx->push(arr);
				}
        break;
			case Java::op_lcmp:{
				int64 l1;
				int64 l2;
				l2 = ((int64)ctx->pop().ui) << 32;
				l2 |= ((int64)ctx->pop().ui) << 0;
				l1 = ((int64)ctx->pop().ui) << 32;
				l1 |= ((int64)ctx->pop().ui) << 0;
				if (l1 == l2)
					ctx->push(0);
				else if (l1 > l2)
					ctx->push(1);
				else
					ctx->push(-1);
				break;
		  }
      case Java::op_dcmpl:{
				FieldData d1;
				FieldData d2;
				d2.l = ((int64)ctx->pop().ui) << 32;
				d2.l |= ((int64)ctx->pop().ui) << 0;
				d1.l = ((int64)ctx->pop().ui) << 32;
				d1.l |= ((int64)ctx->pop().ui) << 0;
        if (_isnan(d1.d) || _isnan(d2.d))
          ctx->push(-1);
				else if (d1.d == d2.d)
					ctx->push(0);
				else if (d1.d > d2.d)
					ctx->push(1);
				else
					ctx->push(-1);
				break;
		  }
      case Java::op_dcmpg:{
				FieldData d1;
				FieldData d2;
				d2.l = ((int64)ctx->pop().ui) << 32;
				d2.l |= ((int64)ctx->pop().ui) << 0;
				d1.l = ((int64)ctx->pop().ui) << 32;
				d1.l |= ((int64)ctx->pop().ui) << 0;
        if (_isnan(d1.d) || _isnan(d2.d))
          ctx->push(1);
				else if (d1.d == d2.d)
					ctx->push(0);
				else if (d1.d > d2.d)
					ctx->push(1);
				else
					ctx->push(-1);
				break;
		  }
			case Java::op_return:{
				ctx->popFrame();
				if (TR_IS_ENABLED(TRACE_DEBUG))
						--method_depth;
					TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
				return;
				break;
			}
			case Java::op_dup:{
				ctx->dup();
				break;
			}
			case Java::op_dup_x1:{
				ctx->insert(ctx->getTop(0), 2);
				break;
			}
			case Java::op_dup2:{
				ctx->dup2();
				break;
			}
			case Java::op_pop:{
				ctx->pop();
				break;
			}
      case Java::op_pop2:{
        ctx->pop();
        ctx->pop();
        break;
      }
			case Java::op_fcmpg:
				{
					float val2 = ctx->pop().f;
					float val1 = ctx->pop().f;
          if (_isnan(val1) || _isnan(val2))
            ctx->push(1);
				  else if (val1 == val2)
					  ctx->push(0);
				  else if (val1 > val2)
					  ctx->push(1);
				  else
					  ctx->push(-1);
				}
				break;
      case Java::op_fcmpl:
				{
					float val2 = ctx->pop().f;
					float val1 = ctx->pop().f;
          if (_isnan(val1) || _isnan(val2))
            ctx->push(-1);
				  else if (val1 == val2)
					  ctx->push(0);
				  else if (val1 > val2)
					  ctx->push(1);
				  else
					  ctx->push(-1);
				}
				break;
			case Java::op_iushr:
				{
					uint32 value2 = ctx->pop().ui;
					uint32 value1 = ctx->pop().ui;
					ctx->push(value1 >> value2);
				}
				break;
			case Java::op_ishr:
				{
					int value2 = ctx->pop().i;
					int value1 = ctx->pop().i;
					ctx->push(value1 >> value2);
				}
				break;
      case Java::op_ishl:
				{
					int value2 = ctx->pop().i;
					int value1 = ctx->pop().i;
					ctx->push(value1 << value2);
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
			case Java::op_ior:
				{
					int value2 = ctx->pop().i;
					int value1 = ctx->pop().i;
					ctx->push(value1 | value2);
				}
				break;
			case Java::op_land:
				{
					int64 l1;
					int64 l2;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					l2 = ((int64)ctx->pop().ui) << 32;
					l2 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 & l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
      case Java::op_lor:
				{
					int64 l1;
					int64 l2;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					l2 = ((int64)ctx->pop().ui) << 32;
					l2 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 | l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
			case Java::op_lshl:
				{
					int64 l1;
					uint32 l2;
					l2 = ctx->pop().ui;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 << l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
      case Java::op_lshr:
				{
					int64 l1;
					uint32 l2;
					l2 = ctx->pop().ui;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 >> l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
      case Java::op_lushr:
				{
					uint64 l1;
					uint32 l2;
					l2 = ctx->pop().ui;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 >> l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
			case Java::op_ladd:
				{
					int64 l1;
					int64 l2;
					l2 = ((int64)ctx->pop().ui) << 32;
					l2 |= ((int64)ctx->pop().ui) << 0;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 + l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
      case Java::op_lsub:
				{
					int64 l1;
					int64 l2;
					l2 = ((int64)ctx->pop().ui) << 32;
					l2 |= ((int64)ctx->pop().ui) << 0;
					l1 = ((int64)ctx->pop().ui) << 32;
					l1 |= ((int64)ctx->pop().ui) << 0;
					int64 res = l1 - l2;
					ctx->push((uint32)(res >> 0));
					ctx->push((uint32)(res >> 32));
				}
				break;
      case Java::op_dadd:
				{
					FieldData d1;
					FieldData d2;
					d2.l = ((int64)ctx->pop().ui) << 32;
					d2.l |= ((int64)ctx->pop().ui) << 0;
					d1.l = ((int64)ctx->pop().ui) << 32;
					d1.l |= ((int64)ctx->pop().ui) << 0;
          FieldData res;
					res.d = d1.d + d2.d;
					ctx->push((uint32)(res.l >> 0));
					ctx->push((uint32)(res.l >> 32));
				}
				break;
      case Java::op_dsub:
				{
					FieldData d1;
					FieldData d2;
					d2.l = ((int64)ctx->pop().ui) << 32;
					d2.l |= ((int64)ctx->pop().ui) << 0;
					d1.l = ((int64)ctx->pop().ui) << 32;
					d1.l |= ((int64)ctx->pop().ui) << 0;
          FieldData res;
					res.d = d1.d - d2.d;
					ctx->push((uint32)(res.l >> 0));
					ctx->push((uint32)(res.l >> 32));
				}
				break;
      case Java::op_dmul:
				{
					FieldData d1;
					FieldData d2;
					d2.l = ((int64)ctx->pop().ui) << 32;
					d2.l |= ((int64)ctx->pop().ui) << 0;
					d1.l = ((int64)ctx->pop().ui) << 32;
					d1.l |= ((int64)ctx->pop().ui) << 0;
          FieldData res;
					res.d = d1.d * d2.d;
					ctx->push((uint32)(res.l >> 0));
					ctx->push((uint32)(res.l >> 32));
				}
				break;
			case Java::op_i2l:
				{
					unsigned value = ctx->pop().ui;
					ctx->push(value);
          ctx->push(0);
					break;
				}
      case Java::op_l2i:
				{
					jlong value;
          value = ((int64)ctx->pop().ui) << 32;
					value |= ((int64)ctx->pop().ui) << 0;
					ctx->push((int)value);
					break;
				}
      case Java::op_i2f:
				{
					int value = ctx->pop().i;
					ctx->push((float)value);
					break;
				}
      case Java::op_i2b:
        {
          unsigned value = ctx->pop().i;
          ctx->push(value & 0xff);
          break;
        }
      case Java::op_i2c:
        {
          unsigned value = ctx->pop().i;
          ctx->push(value & 0xffff);
          break;
        }
      case Java::op_f2i:
				{
					float value = ctx->pop().f;
					ctx->push((int)value);
					break;
				}
      case Java::op_d2i:
				{
					FieldData value;
          value.l = ((int64)ctx->pop().ui) << 32;
					value.l |= ((int64)ctx->pop().ui) << 0;
					ctx->push((int)value.d);
					break;
				}
      case Java::op_i2d:
				{
					int value = ctx->pop().i;
          FieldData res;
          res.d = (double)value;
					ctx->push((uint32)(res.l >> 0));
					ctx->push((uint32)(res.l >> 32));
					break;
				}
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
      case Java::op_athrow:
        {
          VMObject* exception = ctx->pop().obj;
          ctx->throwException(exception);
          //ctx->getJNIEnv()->ExceptionDescribe();
          HANDLE_EXCEPTION(k);
          break;
        }
			default:
				TR_BREAK("%s unimplemented", Opcode::map_string[opcode].c_str());
				break;
    }
  }
	ctx->popFrame();
	if (TR_IS_ENABLED(TRACE_DEBUG))
		--method_depth;
	TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
}

void BcVMMethod::executeLongRet(VMContext* ctx){
}

void NativeVMMethod::print(std::ostream& strm){
	strm << "Native method\n";
}

void NativeVMMethod::execute(VMContext* ctx, unsigned ret){
  TR_USE(Java_Method);
  if (mReturnType == Double)
    TR_BREAK("Implement me");
  if (mFunction == NULL)
    TR_BREAK("Could not resolve native method");
  unsigned argsize = mIsStatic ? mArgSize : mArgSize+1;
  if (TR_IS_ENABLED(TRACE_DEBUG))
    ++method_depth;
  TR_DEBUG("%i: %s", method_depth, mName.c_str());
  ctx->pushFrame(this, ret, argsize);
  VMClass* cls = mIsStatic ? mClass : ctx->get(0).cls;

  FieldData retval = executeNoASM(ctx, mReturnType, cls);
#ifdef ARCH_X64
  //FieldData retval = executeX64(ctx, mReturnType, cls);
#else
  FieldData retval = executeNative(ctx, mReturnType, cls);
#endif
  ctx->popFrame();
  if (TR_IS_ENABLED(TRACE_DEBUG))
    --method_depth;
  TR_DEBUG("<- %s (%s)", mName.c_str(), mClass->getName().c_str());
  if (mReturnType == VMMethod::Boolean || mReturnType == VMMethod::Byte)
    ctx->push(retval.b);
  else if (mReturnType == VMMethod::Reference || mReturnType == VMMethod::Int || mReturnType == VMMethod::Array)
    ctx->push(retval.obj);
  else if (mReturnType == VMMethod::Long){
    ctx->push((unsigned)(retval.l >> 0));
    ctx->push((unsigned)(retval.l >> 32));
  }
}

FieldData NativeVMMethod::executeNative(VMContext* ctx, VMMethod::ReturnType ret, VMClass* cls){
  TR_USE(Java_Method);
  void* tmp;
  for(int i = mArgSize-1; i >= 0; --i){
    tmp = ctx->get(mIsStatic ? i : i+1).obj;
#ifdef WIN32
#ifndef ARCH_X64
    __asm{
      mov eax, tmp;
      push eax;
    }
#endif
#endif
  }
  //for(int i = mArgSize-1; i >= 0; --i){
  //  ctx->pop();
  //}
  nativeMethod mthd = mFunction;
  int popargs = (mArgSize+2)*sizeof(void*);
  tmp = ctx->getJNIEnv();
  FieldData retval;
  if (ret == VMMethod::Boolean || ret == VMMethod::Byte){
    jboolean bret = 0;
#ifdef WIN32
#ifndef ARCH_X64
    __asm{
      mov eax, cls;
      push eax;
      mov ebx, tmp;
      push ebx;
      call mthd;
      add esp, popargs;
      mov bret, al;
    }
#endif
#endif
    retval.b = bret;
  }
  else if (ret == VMMethod::Void){
#ifdef WIN32
#ifndef ARCH_X64
    __asm{
      mov eax, cls;
      push eax;
      mov ebx, tmp;
      push ebx;
      call mthd;
      add esp, popargs;
    }
#endif
#endif
    retval.obj = NULL;
  }
  else if (ret == VMMethod::Reference || ret == VMMethod::Array || ret == VMMethod::Int){
    VMObject* objret = NULL;
#ifdef WIN32
#ifndef ARCH_X64
    __asm{
      mov eax, cls;
      push eax;
      mov ebx, tmp;
      push ebx;
      call mthd;
      add esp, popargs;
      mov objret, eax;
    }
#endif
#endif
    retval.obj = objret;
  }
  else if (ret == VMMethod::Long){
    jlong longret = 0;
#ifdef WIN32
#ifndef ARCH_X64
    __asm{
      mov eax, cls;
      push eax;
      mov ebx, tmp;
      push ebx;
      call mthd;
      add esp, popargs;
      mov DWORD PTR longret[4], edx;
      mov DWORD PTR longret[0], eax;
    }
#endif
#endif
    retval.l = longret;
  }
	return retval;
}

#ifdef ARCH_X64
extern "C"{
  jlong CallFunction(JNIEnv* env, jobject object, jlong r8, jlong r9, nativeMethod mthd, jdouble xmm0, jdouble xmm1, int stacksize, void* stack);
}
#else
jlong CallFunction(JNIEnv* env, jobject object, jlong r8, jlong r9, nativeMethod mthd, jdouble xmm0, jdouble xmm1, int stacksize, void* stack){
  return 0;
}
#endif

FieldData NativeVMMethod::executeX64(VMContext* ctx, VMMethod::ReturnType ret, VMClass* cls){
  TR_USE(Java_Method);
  nativeMethod mthd = mFunction;
  JNIEnv* env = ctx->getJNIEnv();
  jlong r8 = 0;
  jlong r9 = 0;
  double xmm0 = 0;
  double xmm1 = 0;
  int regsize = min(2, (int)mSplitSignature.size());
  int base = mIsStatic ? 0 : 1;
  for (int i = 0; i < regsize; ++i){
    //if (mSplitSignature[i] == "F" || mSplitSignature[i] == "D")
    //  TR_BREAK("Implement me");
    if (mSplitSignature[i].size() > 1 || mSplitSignature[i] == "I" || mSplitSignature[i] == "Z"){ //reference
      StackData sd = ctx->get(base++);
      if (i == 0)
        r8 = (jlong)sd.obj;
      else
        r9 = (jlong)sd.obj;
    }
    else if (mSplitSignature[i] == "F"){
      StackData sd = ctx->get(base++);
      if (i == 0)
        xmm0 = sd.f;
      else
        xmm1 = sd.f;
    }
    else if (mSplitSignature[i] == "D"){
      jdouble d;
      StackData sd1 = ctx->get(base++);
      StackData sd2 = ctx->get(base++);
      FieldData value;
      value.l = ((int64)sd2.ui) << 32;
      value.l |= ((int64)sd1.ui) << 0;
      d = value.d;
      if (i == 0)
        xmm0 = d;
      else
        xmm1 = d;
    }
    else if (mSplitSignature[i] == "J"){
      jlong l;
      StackData sd1 = ctx->get(base++);
      StackData sd2 = ctx->get(base++);
      FieldData value;
      value.l = ((int64)sd2.ui) << 32;
      value.l |= ((int64)sd1.ui) << 0;
      l = value.l;
      if (i == 0)
        r8 = l;
      else
        r9 = l;
    }
    else{
      TR_BREAK("Implement me");
    }
  }
  std::vector<jlong> stack;
  int used = 8;
  //TODO apparently no packing: optimize for it
  for (unsigned i = 2; i < mSplitSignature.size(); ++i){
    int size = 0;
    if (mSplitSignature[i].size() > 1)
      size = 8;
    else if (mSplitSignature[i] == "I")
      size = 4;
    else
      TR_BREAK("Implement me");
    //if (8-used < size){
      stack.push_back(0);
      used = 0;
    //}
    used += size;
    jlong tmp = (jlong)ctx->get(base++).obj;
    char* copy = (char*)&stack[stack.size()-1];
    char* src = (char*)&tmp;
    memcpy(copy+used-size, &tmp/*+8-size*/, size);
  }
  FieldData data;
  data.obj = (VMObject*)CallFunction(env, cls, r8, r9, mthd, xmm0, xmm1, 32+(int)stack.size()*8, stack.empty() ? NULL : &stack[0]);
  return data;
}
