#include "VMclass.h"

#include <iostream>
#include <cstring>

#include <system/utilities.h>

#include "Trace.h"
#include "JVM.h"
#include "VMArray.h"
#include "VMMethod.h"
#include "VMContext.h"

#define PROC_DECL_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_DECL_MAP_MODE

#include "JavaBinFileReader.h"
#include "VMLoader.h"

#ifdef WIN32
#pragma warning (disable: 4355) //this used in base member initializer list
#endif

TR_CHANNEL(Java_Class);

VMClass::VMClass(VMLoader* loader) : VMObject(this), mNonStaticFieldOffset(0), mLoader(loader) {
}

VMClass::VMClass(VMContext* ctx, VMLoader* loader, CGE::Reader& reader) : VMObject(this), mLoader(loader) {
  TR_USE(Java_Class);
  JavaBinFileReader in(&reader);
  int ret = in.readClassFile(mClass);
  if (ret != 0)
    TR_BREAK("Malformed classfile found - aborting...");

	mRCP.resize(mClass.constant_pool_count+1);
	memset(&mRCP[0], 0, (mClass.constant_pool_count+1)*sizeof(StackData));
  Java::CONSTANT_Class_info* clinfo = (Java::CONSTANT_Class_info*)mClass.constant_pool[mClass.this_class-1];
  Java::CONSTANT_Utf8_info* utf = (Java::CONSTANT_Utf8_info*)(mClass.constant_pool[clinfo->name_index-1]);
  mFilename = utf->bytes;

  TR_INFO("%s parsed successfully", mFilename.c_str());
}

VMClass::~VMClass(){
	for (std::vector<VMMethod*>::iterator iter = mMethods.begin(); iter != mMethods.end(); ++iter){
    if (*iter != NULL)
	    (*iter)->unref();
	}
}

void VMClass::initClass(VMContext* ctx, bool execClassInit){
  TR_USE(Java_Class);

  //init superclass first
  getSuperclass(ctx);

  //if there is a problem with the super class
  if (ctx->getException() != NULL)
    return;

  initFields(ctx);

  VMClass* cls = getVM()->getLoader(NULL)->find(ctx, "java/lang/Class");
  VMMethod* clsmthd = cls->getMethod(cls->findMethodIndex("<init>", "()V"));
  init(ctx, cls);
  ctx->push((VMObject*)cls);
  if (clsmthd)
    clsmthd->execute(ctx, -1);

  if (execClassInit){
    unsigned idx = findMethodIndex("<clinit>", "()V");
    VMMethod* mthd = getMethod(idx);
    if (mthd){
      TR_INFO("Found class init method");
      mthd->execute(ctx, -1);
    }
  }
}

void VMClass::print(std::ostream& strm){
  TR_USE(Java_Class);
  strm << "Constant pool:\n";
  for (int i = 0; i < mClass.constant_pool_count-1; ++i){
    Java::cp_info* cpinfo = mClass.constant_pool[i];
    strm << "#" << i+1 << " ";
    switch(cpinfo->tag){
      case CONSTANT_Class:
        strm << "Class " << ((Java::CONSTANT_Class_info*)cpinfo)->name_index;
        break;
      case CONSTANT_Fieldref:
        strm << "Fieldref " << ((Java::CONSTANT_Fieldref_info*)cpinfo)->class_index << "/" << ((Java::CONSTANT_Fieldref_info*)cpinfo)->name_and_type_index;
        break;
      case CONSTANT_Methodref:
        strm << "Methodref " << ((Java::CONSTANT_Methodref_info*)cpinfo)->class_index << "/" << ((Java::CONSTANT_Methodref_info*)cpinfo)->name_and_type_index;
        break;
      case CONSTANT_InterfaceMethodref:
        strm << "InterfaceMethodref " << ((Java::CONSTANT_InterfaceMethodref_info*)cpinfo)->class_index << "/" << ((Java::CONSTANT_InterfaceMethodref_info*)cpinfo)->name_and_type_index;
        break;
      case CONSTANT_String:
        strm << "String " << ((Java::CONSTANT_String_info*)cpinfo)->string_index;
        break;
      case CONSTANT_NameAndType:
        strm << "NameAndType " << ((Java::CONSTANT_NameAndType_info*)cpinfo)->name_index << "/" << ((Java::CONSTANT_NameAndType_info*)cpinfo)->descriptor_index;
        break;
      case CONSTANT_Utf8:
        strm << "UTF8 " << ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
        break;
    };
    strm << std::endl;
  }
  strm << "Methods:" << "\n";
  for (int i = 0; i < mClass.methods_count; ++i){
    Java::method_info* mi = mClass.methods[i];
    Java::cp_info* cpinfo = mClass.constant_pool[mi->name_index-1];
    Java::CONSTANT_Utf8_info* utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    if (!utf)
      TR_BREAK("Method resolution invalid");
    strm << utf->bytes;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    if (!utf){
      TR_ERROR("Method resolution invalid");
      cpinfo = mClass.constant_pool[mi->descriptor_index-2];
      utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    }
    strm << utf->bytes;
		bool ca = false;
		for (int j = 0; j < mi->attributes_count; ++j){
			if (mi->attributes[j] && mi->attributes[j]->attribute_type == Java::ATTR_Code){
				ca = true;
				break;
			}
		}
		if (!ca)
			strm << "  (native)";
    strm << "\n";
  }
}

unsigned VMClass::findMethodIndex(const std::string& name, const std::string& signature){
	std::map<std::string,unsigned>::iterator iter = mMethodResolver.find(name+signature);
	if (iter != mMethodResolver.end())
		return iter->second;
	return 0;
}

VMMethod* VMClass::getMethod(unsigned methodid){
	if (methodid != 0 && mMethods[methodid-1] != NULL)
		return mMethods[methodid-1];
	return NULL;
}

unsigned VMClass::findFieldIndex(const std::string& name){
	std::map<std::string,unsigned>::iterator iter = mFieldResolver.find(name);
	if (iter == mFieldResolver.end())
		return 0;
	return iter->second;
}

FieldData* VMClass::getField(unsigned fieldid){
	return &mFields[fieldid-1];
}

FieldData* VMClass::getStaticField(unsigned fieldid){
	return &mFields[fieldid-mSuperFields-1];
}

unsigned VMClass::getMethodIndex(VMContext* ctx, Java::u2 method_ref, VMClass*& classRet){
	Java::CONSTANT_Methodref_info* methodref = static_cast<Java::CONSTANT_Methodref_info*>(mClass.constant_pool[method_ref-1]);
	classRet = getClass(ctx, methodref->class_index);
  if (mRCP[method_ref].ui != 0)
		return mRCP[method_ref].ui;
	Java::CONSTANT_NameAndType_info* nameandtypeinfo = static_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[methodref->name_and_type_index-1]);
  std::string methodname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
  std::string signature = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
	unsigned idx = classRet->findMethodIndex(methodname, signature);
	mRCP[method_ref].ui = idx;
	return idx;
}

VMMethod* VMClass::getMethod(VMContext* ctx, Java::u2 method_ref){
	VMClass* cls;
	unsigned idx = getMethodIndex(ctx, method_ref, cls);
	return cls->getMethod(idx);
}

FieldData* VMClass::getField(VMContext* ctx, Java::u2 field_ref, VMMethod::ReturnType& type){
	VMClass* cls;
	unsigned idx = getFieldIndex(ctx, field_ref, cls, type);
	return cls->getField(idx);//&mFields[getFieldIndex(ctx, field_ref)-1];
}

FieldData* VMClass::getStaticField(VMContext* ctx, Java::u2 field_ref, VMMethod::ReturnType& type){
	VMClass* cls;
	unsigned idx = getStaticFieldIndex(ctx, field_ref, cls, type);
	return cls->getStaticField(idx);//&mFields[getFieldIndex(ctx, field_ref)-1];
}

unsigned VMClass::getFieldIndex(VMContext* ctx,Java::u2 field_ref, VMClass*& classRet, VMMethod::ReturnType& type){
	Java::CONSTANT_Fieldref_info* fieldref = static_cast<Java::CONSTANT_Fieldref_info*>(mClass.constant_pool[field_ref-1]);
	classRet = getClass(ctx, fieldref->class_index);
	Java::CONSTANT_NameAndType_info* nameandtypeinfo = static_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[fieldref->name_and_type_index-1]);
	std::string typestring = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
	type = VMMethod::parseType(typestring[0]);
	if (mRCP[field_ref].ui != 0)
		return mRCP[field_ref].ui;
	std::string fieldname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
	unsigned idx = classRet->findFieldIndex(fieldname);
	//if (idx != 0)
	mRCP[field_ref].ui = idx;
	return idx;
}

unsigned VMClass::getStaticFieldIndex(VMContext* ctx,Java::u2 field_ref, VMClass*& classRet, VMMethod::ReturnType& type){
	Java::CONSTANT_Fieldref_info* fieldref = static_cast<Java::CONSTANT_Fieldref_info*>(mClass.constant_pool[field_ref-1]);
	classRet = getClass(ctx, fieldref->class_index);
  Java::CONSTANT_NameAndType_info* nameandtypeinfo = static_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[fieldref->name_and_type_index-1]);
	std::string typestring = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
	type = VMMethod::parseType(typestring[0]);
  if (mRCP[field_ref].ui != 0){
    classRet = classRet->mStaticFieldResolver[mRCP[field_ref].ui];
	  return mRCP[field_ref].ui;
  }
	std::string fieldname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
	unsigned idx = classRet->findFieldIndex(fieldname);
  classRet = classRet->mStaticFieldResolver[idx];//getClass(ctx, fieldref->class_index);
	mRCP[field_ref].ui = idx;
	return idx;
}

std::string VMClass::buildNativeMethodName(const std::string& functionname, const std::string& signature){
	std::string result = mFilename;
	CGE::Utilities::replaceWith(result, '/', '_');
  CGE::Utilities::replaceWith(result, "$", "_00024");
	result = "Java_"+result+"_"+functionname;
#ifdef WIN32
	/*std::string params;
	if (signature == "()V")
		params = "8";
	else
		TRACE(TRACE_JAVA, TRACE_ERROR, "Cannot resolve signature");
	result = "_"+result+"@"+params;*/
#endif
	return result;
}

void VMClass::registerMethod(const std::string& name, const std::string& signature, nativeMethod mthd){
	Java::method_info* minfo = NULL;
	for (int i = 0; i < mClass.methods_count; ++i){
    Java::method_info* mi = mClass.methods[i];
    Java::cp_info* cpinfo = mClass.constant_pool[mi->name_index-1];
    std::string methodname = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    if (methodname != name)
      continue;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    std::string sig = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    if (sig != signature)
      continue;
		minfo = mi;
		break;
	}
	unsigned idx = findMethodIndex(name, signature);
	mMethods[idx] = new NativeVMMethod(name, signature, this, (minfo->access_flags & ACC_STATIC) != 0, -1, mthd);
}

VMClass* VMClass::getSuperclass(VMContext* ctx){
	if (mClass.super_class == 0)
		return NULL;
	return getClass(ctx, mClass.super_class);
}

VMClass* VMClass::getClass(VMContext* ctx, Java::u2 class_ref){
	if (mRCP[class_ref].cls != NULL)
		return mRCP[class_ref].cls;
	Java::CONSTANT_Class_info* classinfo = static_cast<Java::CONSTANT_Class_info*>(mClass.constant_pool[class_ref-1]);
	std::string classname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[classinfo->name_index-1])->bytes;
	VMClass* cls = mLoader->find(ctx, classname);
	mRCP[class_ref] = cls;
	return cls;
}

void VMClass::initFields(VMContext* ctx){
  TR_USE(Java_Class);
	VMClass* super = getSuperclass(ctx);
	unsigned nonstatic = 0;
  unsigned statfields = 0;
	unsigned methods = 0;
	//superclass info
	if (super){
		nonstatic = super->getNonStaticFieldOffset();
    statfields = super->getStaticFieldOffset();
		super->copyMethodData(mMethodResolver, mMethods);
    super->copyFieldData(mFieldResolver, mStaticFieldResolver);
		methods = (unsigned)mMethods.size();
	}
  mSuperFields = statfields;

	//fields
	for (unsigned i = 0; i < mClass.fields_count; ++i){
		Java::field_info* info = mClass.fields[i];
		std::string fieldname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[info->name_index-1])->bytes;
		if ((info->access_flags & ACC_STATIC) != 0){
			mFieldResolver[fieldname] = ++statfields;
			mFields.resize(statfields-mSuperFields);
      mStaticFieldResolver[statfields] = this;
		}
		else{
			mFieldResolver[fieldname] = ++nonstatic;
		}
	}
  mNonStaticFieldOffset = nonstatic;

	//methods
	int offset = (int)mMethods.size()+1;
	std::vector<VMMethod*> plainMethods;
	for (int i = 0; i < mClass.methods_count; ++i){
    Java::method_info* mi = mClass.methods[i];
    Java::cp_info* cpinfo = mClass.constant_pool[mi->name_index-1];
    std::string methodname = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    std::string sig = ((Java::CONSTANT_Utf8_info*)cpinfo)->bytes;
		VMMethod* mthd = NULL;
		//find code attribute
    for (int j = 0; j < mi->attributes_count; ++j){
			if (mi->attributes[j] && mi->attributes[j]->attribute_type == Java::ATTR_Code){
				Java::Code_attribute* code = static_cast<Java::Code_attribute*>(mi->attributes[j]);
				mthd = new BcVMMethod(methodname, sig, this, (mi->access_flags & ACC_STATIC) != 0, i, code);
				break;
			}
		}
		if (mthd == NULL){
			TR_INFO("No code attribute found");
			nativeMethod m = mLoader->findNativeMethod(buildNativeMethodName(methodname, sig));
			mthd = new NativeVMMethod(methodname, sig, this, (mi->access_flags & ACC_STATIC) != 0, i, m);
			if (mthd == NULL){
				TR_BREAK("Cannot resolve native method");
			}
		}
		//is vtable method?
		if (mthd->getName()[0] != '<'){
			std::map<std::string,unsigned>::iterator iter = mMethodResolver.find(methodname+sig);
			int idx;
			if (iter == mMethodResolver.end()){
				idx = ++methods;
				mMethods.resize(methods);
				mMethodResolver[methodname+sig] = idx;
			}
			else{
				idx = iter->second;
        mMethods[idx-1]->unref();
			}
			mMethods[idx-1] = mthd;
		}
		else{ //no vtable
			plainMethods.push_back(mthd);
		}
	}
	//add plain methods
	mVtableEnd = (unsigned)mMethods.size();
	mMethods.resize(mVtableEnd+plainMethods.size());
	for (unsigned i = 0; i < plainMethods.size(); ++i){
		mMethodResolver[plainMethods[i]->getName()+plainMethods[i]->getSignature()] = ++methods;
		mMethods[methods-1] = plainMethods[i];
	}

  //interfaces
  /*for (int i = 0; i < mClass.interfaces_count; ++i){
    VMClass* itfcls = getClass(ctx, mClass.interfaces[i]);
    for (std::map<std::string, unsigned>::iterator iter = itfcls->mMethodResolver.begin(); iter != itfcls->mMethodResolver.end(); ++iter){
      if (mMethodResolver.find(iter->first) == mMethodResolver.end())
        mMethodResolver[iter->first] = 10000+iter->second;
    }
  }*/
}

unsigned VMClass::getNonStaticFieldOffset(){
  return mNonStaticFieldOffset;
	//return mClass.fields_count-mFields.size();
}

unsigned VMClass::getStaticFieldOffset(){
  return (unsigned)mFields.size();
}

FieldData VMClass::getConstant(VMContext* ctx, Java::u2 constant_ref){
  TR_USE(Java_Class);
	FieldData ret;
	if (mRCP[constant_ref].obj != 0){
		ret.obj = mRCP[constant_ref].obj;
		return ret;
	}
	Java::cp_info* info = mClass.constant_pool[constant_ref-1];
	if (info->tag == CONSTANT_Integer){
		Java::CONSTANT_Integer_info* intinf = (Java::CONSTANT_Integer_info*)(info);
		ret.ui = intinf->bytes;
	}
	else if (info->tag == CONSTANT_Long){
		Java::CONSTANT_Long_info* longinf = (Java::CONSTANT_Long_info*)(info);
		ret.l = (((int64)longinf->high_bytes) << 32) | longinf->low_bytes;
		return ret; //do not save in RCP
	}
	else if (info->tag == CONSTANT_Float){
		Java::CONSTANT_Float_info* fltinf = (Java::CONSTANT_Float_info*)(info);
		ret.f = *((float*)&fltinf->bytes);
	}
	else if (info->tag == CONSTANT_Double){
		Java::CONSTANT_Double_info* dblinf = (Java::CONSTANT_Double_info*)(info);
		ret.l = (((int64)dblinf->high_bytes) << 32) | dblinf->low_bytes;
		return ret; //do not save in RCP
	}
	else if (info->tag == CONSTANT_String){
		Java::CONSTANT_String_info* str = (Java::CONSTANT_String_info*)(info);
		ret.obj = getConstant(ctx, str->string_index).obj;
	}
	else if (info->tag == CONSTANT_Utf8){
		Java::CONSTANT_Utf8_info* utf = (Java::CONSTANT_Utf8_info*)(info);
		//VMObject* obj = getVM()->create(ctx, utf->bytes.c_str());
		VMClass* cls = mLoader->find(ctx, "java/lang/String");

		//VMClass* chcls = getVM()->findClass(ctx, "java/lang/Character");

		//cls->print(std::cout);
		VMObject* obj = getVM()->createObject(ctx, cls);
		FieldData* val = obj->getObjField(cls->findFieldIndex("value"));
    int size = JVM::utf8to16(utf->bytes.c_str(), NULL, 0);
		VMCharArray* strdata = getVM()->createCharArray(ctx, size-1);
		if (size > 1){
      unsigned short* utf16 = new unsigned short[size-1];
      JVM::utf8to16(utf->bytes.c_str(), utf16, size-1);
			strdata->setData(utf16);
			delete [] utf16;
		}
		val->obj = strdata;
		unsigned idx = cls->findFieldIndex("offset");
		if (idx > 0){
			val = obj->getObjField(idx);
			val->ui = 0;
		}
		idx = cls->findFieldIndex("count");
		if (idx > 0){
			val = obj->getObjField(idx);
			val->ui = utf->length;
		}
		ret.obj = obj;
		getVM()->internalizeString(utf->bytes, obj);
	}
	else if (info->tag == CONSTANT_Class){
		VMClass* cls = getClass(ctx, constant_ref);
		ret.cls = cls;
	}
	else{
	  TR_BREAK("Unhandled type");
	}
	mRCP[constant_ref] = ret.obj;
	return ret;
}

void VMClass::copyMethodData(std::map<std::string,unsigned>& methodresolver, std::vector<VMMethod*>& methods){
	//methodresolver.insert(mMethodResolver.begin(), mMethodResolver.end());
	//methods.insert(methods.begin(), mMethods.begin(), mMethods.end());
	methods.resize(mVtableEnd);
	for (unsigned i = 0; i < mVtableEnd; ++i){
		VMMethod* mthd = mMethods[i];
    mthd->ref();
		std::string id = mthd->getName()+mthd->getSignature();
		unsigned val = mMethodResolver[id];
		methodresolver.insert(std::make_pair(id, val));
		methods[val-1] = mthd;
	}
}

void VMClass::copyFieldData(std::map<std::string,unsigned>& fieldresolver, std::map<unsigned,VMClass*>& staticfieldresolver){
  for (std::map<std::string,unsigned>::iterator iter = mFieldResolver.begin(); iter != mFieldResolver.end(); ++iter){
    fieldresolver.insert(*iter);
  }
  for (std::map<unsigned,VMClass*>::iterator iter = mStaticFieldResolver.begin(); iter != mStaticFieldResolver.end(); ++iter){
    staticfieldresolver.insert(*iter);
  }
}

std::string VMClass::getSourceFileName(){
  for (int i = 0; i < mClass.attributes_count; ++i){
    if (mClass.attributes[i]->attribute_type == Java::ATTR_SourceFile){
      Java::SourceFile_attribute* sa = (Java::SourceFile_attribute*)mClass.attributes[i];
      Java::cp_info* cpinfo = mClass.constant_pool[sa->sourcefile_index-1];
      Java::CONSTANT_Utf8_info* utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
      return utf->bytes;
    }
  }
  return "";
}

int VMClass::getLineNumber(int ip, int methodIndex){
  if (ip < 0 || methodIndex < 0)
    return ip;
  Java::method_info* info = mClass.methods[methodIndex];
  for (int i = 0; i < info->attributes_count; ++i){
    if (info->attributes[i]->attribute_type == Java::ATTR_Code){
      Java::Code_attribute* ca = (Java::Code_attribute*)info->attributes[i];
      for (int j = 0; j < ca->attributes_count; ++j){
        if (ca->attributes[j]->attribute_type == Java::ATTR_LineNumberTable){
          int lineNumber = -1;
          Java::LineNumberTable_attribute* lna = (Java::LineNumberTable_attribute*)ca->attributes[j];
          for (int k = 0; k < lna->line_number_table_length; ++k){
            if (lna->line_number_table[k].start_pc <= ip)
              lineNumber = lna->line_number_table[k].line_number;
            else
              return lineNumber;
          }
          return lineNumber;
        }
      }
    }
  }
  return -1;
}

int VMClass::getCatchIP(int ip, VMContext* ctx, VMObject* exception, int methodIndex){
  if (methodIndex < 0)
    return -1;
  Java::method_info* info = mClass.methods[methodIndex];
  for (int i = 0; i < info->attributes_count; ++i){
    if (info->attributes[i]->attribute_type == Java::ATTR_Code){
      Java::Code_attribute* ca = (Java::Code_attribute*)info->attributes[i];
      for (int j = 0; j < ca->exception_table_length; ++j){
        Java::Code_attribute::Exception_table extab = ca->exception_table[j];
        //does the ip range fit
        if (ip >= extab.start_pc && ip < extab.end_pc){
          if (extab.catch_type == 0)
            return extab.handler_pc;
          //check if catch type fits
          VMClass* handledtype = getClass(ctx, extab.catch_type);
          VMClass* exceptiontype = exception->getClass();
          while (exceptiontype != NULL){
            if (exceptiontype == handledtype)
              return extab.handler_pc;
            exceptiontype = exceptiontype->getSuperclass(ctx);
          }
        }
      }
    }
  }
  return -1;
}
