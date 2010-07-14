#include "VMClass.h"

#include <iostream>

#include <io/BinFileReader.h>
#include <io/ZipReader.h>
#include <system/utilities.h>

#include "Trace.h"
#include "JVM.h"
#include "VMMethod.h"

#define PROC_DECL_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_DECL_MAP_MODE

#include "JavaBinFileReader.h"

VMClass::VMClass(const std::string& filename) : mSuperclass(NULL), mClassObject(NULL) {
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

	mRCP.resize(mClass.constant_pool_count+1);
	memset(&mRCP[0], 0, (mClass.constant_pool_count+1)*sizeof(StackData));
}

VMClass::~VMClass(){
	for (std::vector<VMMethod*>::iterator iter = mMethods.begin(); iter != mMethods.end(); ++iter){
		if ((*iter)->getClass() == this)
			delete *iter;
	}
}

void VMClass::print(std::ostream& strm){
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
      TRACE_ABORT(TRACE_JAVA, "Method resolution invalid");
    strm << utf->bytes;
    cpinfo = mClass.constant_pool[mi->descriptor_index-1];
    utf = dynamic_cast<Java::CONSTANT_Utf8_info*>(cpinfo);
    if (!utf){
      TRACE(TRACE_JAVA, TRACE_ERROR, "Method resolution invalid");
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
	return iter->second+1;
}

FieldData* VMClass::getField(unsigned fieldid){
	return &mFields[fieldid-1];
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

FieldData* VMClass::getField(VMContext* ctx, Java::u2 field_ref){
	VMClass* cls;
	unsigned idx = getFieldIndex(ctx, field_ref, cls);
	return cls->getField(idx);//&mFields[getFieldIndex(ctx, field_ref)-1];
}

unsigned VMClass::getFieldIndex(VMContext* ctx,Java::u2 field_ref, VMClass*& classRet){
	Java::CONSTANT_Fieldref_info* fieldref = static_cast<Java::CONSTANT_Fieldref_info*>(mClass.constant_pool[field_ref-1]);
	classRet = getClass(ctx, fieldref->class_index);
	if (mRCP[field_ref].ui != 0)
		return mRCP[field_ref].ui;
		Java::CONSTANT_NameAndType_info* nameandtypeinfo = static_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[fieldref->name_and_type_index-1]);
	std::string fieldname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
	std::string type = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
	unsigned idx = classRet->findFieldIndex(fieldname);
	//if (idx != 0)
	mRCP[field_ref].ui = idx;
	return idx;
}

std::string VMClass::buildNativeMethodName(const std::string& functionname, const std::string& signature){
	std::string result = mFilename;
	CGE::Utilities::replaceWith(result, '/', '_');
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
	mMethods[idx] = new NativeVMMethod(name, signature, this, (minfo->access_flags & ACC_STATIC) != 0, mthd);
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
	VMClass* cls = getVM()->findClass(ctx, classname);
	mRCP[class_ref] = cls;
	return cls;
}

void VMClass::initFields(VMContext* ctx){
	VMClass* super = getSuperclass(ctx);
	unsigned nonstatic = 0;
	unsigned methods = 0;
	//superclass info
	if (super){
		nonstatic = super->getNonStaticFieldOffset();
		super->copyMethodData(mMethodResolver, mMethods);
		methods = mMethods.size();
	}

	//fields
	for (unsigned i = 0; i < mClass.fields_count; ++i){
		Java::field_info* info = mClass.fields[i];
		std::string fieldname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[info->name_index-1])->bytes;
		if ((info->access_flags & ACC_STATIC) != 0){
			mFieldResolver[fieldname] = mFields.size();
			mFields.resize(mFields.size()+1);
		}
		else{
			mFieldResolver[fieldname] = nonstatic++;
		}
	}

	//methods
	int offset = mMethods.size()+1;
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
			if (mi->attributes[j]->attribute_type == Java::ATTR_Code){
				Java::Code_attribute* code = static_cast<Java::Code_attribute*>(mi->attributes[j]);
				mthd = new BcVMMethod(methodname, sig, this, (mi->access_flags & ACC_STATIC) != 0, code);
				break;
			}
		}
		if (mthd == NULL){
			TRACE(TRACE_JAVA, TRACE_INFO, "No code attribute found");
			nativeMethod m = getVM()->findNativeMethod(buildNativeMethodName(methodname, sig));
			mthd = new NativeVMMethod(methodname, sig, this, (mi->access_flags & ACC_STATIC) != 0, m);
			if (mthd == NULL){
				TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Cannot resolve native method");
			}
		}
		//is vtable method?
		if (mthd->getName()[0] != '<'){
			std::map<std::string,unsigned>::iterator iter = mMethodResolver.find(methodname+sig);
			int idx;
			if (iter == mMethodResolver.end()){
				idx = methods++;
				mMethods.resize(methods);
				mMethodResolver[methodname+sig] = idx+1;
			}
			else{
				idx = iter->second;
			}
			mMethods[idx] = mthd;
		}
		else{ //no vtable
			plainMethods.push_back(mthd);
		}
	}
	//add plain methods
	mVtableEnd = mMethods.size();
	mMethods.resize(mVtableEnd+plainMethods.size());
	for (unsigned i = 0; i < plainMethods.size(); ++i){
		mMethodResolver[plainMethods[i]->getName()+plainMethods[i]->getSignature()] = ++methods;
		mMethods[methods-1] = plainMethods[i];
	}
}

unsigned VMClass::getNonStaticFieldOffset(){
	return mClass.fields_count-mFields.size();
}

StackData VMClass::getConstant(VMContext* ctx, Java::u2 constant_ref){
	if (mRCP[constant_ref].ui != 0)
		return mRCP[constant_ref];
	StackData ret;
	Java::cp_info* info = mClass.constant_pool[constant_ref-1];
	if (info->tag == CONSTANT_Integer){
		Java::CONSTANT_Integer_info* intinf = (Java::CONSTANT_Integer_info*)(info);
		ret.ui = intinf->bytes;
	}
	/*else if (info->tag == CONSTANT_Long){

	}*/
	else if (info->tag == CONSTANT_Float){
		Java::CONSTANT_Float_info* fltinf = (Java::CONSTANT_Float_info*)(info);
		ret.f = *((float*)&fltinf->bytes);
		//op.data.mFloat = fltinf->bytes;
	}
	/*else if (info->tag == CONSTANT_Double){

	}*/
	//else if (info->tag == CONSTANT_String){
	/*Java::CONSTANT_String_info* str = (Java::CONSTANT_String_info*)(info);
	Java::CONSTANT_Utf8_info* utf = (Java::CONSTANT_Utf8_info*)(mClass.constant_pool[str->string_index-1]);
	area.getClassIndex("java/lang/String");
	area.mStrings.push_back(utf->bytes);*/
	//}
	else{
	  TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "Unhandled type");
	}
	mRCP[constant_ref] = ret;
	return ret;
}

void VMClass::copyMethodData(std::map<std::string,unsigned>& methodresolver, std::vector<VMMethod*>& methods){
	//methodresolver.insert(mMethodResolver.begin(), mMethodResolver.end());
	//methods.insert(methods.begin(), mMethods.begin(), mMethods.end());
	methods.resize(mVtableEnd);
	for (unsigned i = 0; i < mVtableEnd; ++i){
		VMMethod* mthd = mMethods[i];
		std::string id = mthd->getName()+mthd->getSignature();
		unsigned val = mMethodResolver[id];
		methodresolver.insert(std::make_pair(id, val));
		methods[i] = mthd;
	}
}