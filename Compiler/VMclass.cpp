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

VMClass::VMClass(const std::string& filename) : mSuperclass(NULL){
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
	for (std::map<std::string,VMMethod*>::iterator iter = mMethods.begin(); iter != mMethods.end(); ++iter){
		delete iter->second;
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

VMMethod* VMClass::findMethod(const std::string& name, const std::string& signature){
	VMMethod* mthd = NULL;
	std::map<std::string,VMMethod*>::iterator iter = mMethods.find(name+signature);
	if (iter != mMethods.end())
		return iter->second;
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
		VMMethod* mthd = NULL;
		//find code attribute
    for (int j = 0; j < mi->attributes_count; ++j){
			if (mi->attributes[j]->attribute_type == Java::ATTR_Code){
				Java::Code_attribute* code = static_cast<Java::Code_attribute*>(mi->attributes[j]);
				mthd = new BcVMMethod(name, signature, this, (mi->access_flags & ACC_STATIC) != 0, code);
				break;
			}
		}
		if (mthd == NULL){
			TRACE(TRACE_JAVA, TRACE_INFO, "No code attribute found");
			nativeMethod m = getVM()->findNativeMethod(buildNativeMethodName(name, signature));
			mthd = new NativeVMMethod(name, signature, this, (mi->access_flags & ACC_STATIC) != 0, m);
			if (mthd == NULL){
				TRACE(TRACE_JAVA, TRACE_ERROR, "Cannot resolve native method");
				return NULL;
			}
		}
		mMethods[name+signature] = mthd;
		mthd->print(std::cout);
		return mthd;
	}
	return NULL;
}

unsigned VMClass::findFieldIndex(const std::string& name){
	std::map<std::string,unsigned>::iterator iter = mFieldResolver.find(name);
	if (iter == mFieldResolver.end())
		return 0;
	return iter->second+1;
}

VMMethod* VMClass::getMethod(VMContext* ctx, Java::u2 method_ref){
	if (mRCP[method_ref].mthd != NULL)
		return mRCP[method_ref].mthd;
	Java::CONSTANT_Methodref_info* methodref = static_cast<Java::CONSTANT_Methodref_info*>(mClass.constant_pool[method_ref-1]);
	VMClass* classRet = getClass(ctx, methodref->class_index);
  Java::CONSTANT_NameAndType_info* nameandtypeinfo = static_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[methodref->name_and_type_index-1]);
  std::string methodname = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
  std::string signature = dynamic_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
	VMMethod* mthd = classRet->findMethod(methodname, signature);
	mRCP[method_ref] = mthd;
	return mthd;
}

FieldData* VMClass::getField(VMContext* ctx, Java::u2 field_ref){
	return &mFields[getFieldIndex(ctx, field_ref)-1];
}

unsigned VMClass::getFieldIndex(VMContext* ctx,Java::u2 field_ref){
	if (mRCP[field_ref].ui != 0)
		return mRCP[field_ref].ui;
	/*if (mClass.constant_pool[field_ref-1]->tag != CONSTANT_Fieldref){
		return 0;
	}*/
	Java::CONSTANT_Fieldref_info* fieldref = static_cast<Java::CONSTANT_Fieldref_info*>(mClass.constant_pool[field_ref-1]);
	Java::CONSTANT_NameAndType_info* nameandtypeinfo = static_cast<Java::CONSTANT_NameAndType_info*>(mClass.constant_pool[fieldref->name_and_type_index-1]);
	VMClass* cls = getClass(ctx, fieldref->class_index);
	std::string fieldname = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->name_index-1])->bytes;
	std::string type = static_cast<Java::CONSTANT_Utf8_info*>(mClass.constant_pool[nameandtypeinfo->descriptor_index-1])->bytes;
	unsigned idx = cls->findFieldIndex(fieldname);
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
	mMethods[name+signature] = new NativeVMMethod(name, signature, this, (minfo->access_flags & ACC_STATIC) != 0, mthd);
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
	if (super)
		nonstatic = super->getNonStaticFieldOffset();
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
}

unsigned VMClass::getNonStaticFieldOffset(){
	return mClass.fields_count-mFields.size();
}
