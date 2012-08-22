#include <string>
#include "VMMethod.h"
#include "VMContext.h"
#include <io/Tracing.h>

TR_CHANNEL(Java_Method_NoASM)

typedef void (*Call_V)(JNIEnv* env, jobject object);
typedef jlong (*Call_J)(JNIEnv* env, jobject object);
typedef jobject (*Call_P)(JNIEnv* env, jobject object);
typedef jint (*Call_I)(JNIEnv* env, jobject object);
typedef jboolean (*Call_Z)(JNIEnv* env, jobject object);

typedef jobject (*CallP_P)(JNIEnv* env, jobject object, jobject o2);
typedef jboolean (*CallP_Z)(JNIEnv* env, jobject object, jobject o2);
typedef jlong (*CallP_J)(JNIEnv* env, jobject object, jobject o2);
typedef jint (*CallP_I)(JNIEnv* env, jobject object, jobject o2);
typedef void (*CallP_V)(JNIEnv* env, jobject object, jobject o2);
typedef jobject (*CallPP_P)(JNIEnv* env, jobject object, jobject o2, jobject o3);
typedef jobject (*CallPZP_P)(JNIEnv* env, jobject object, jobject o2, jboolean b1, jobject o3);
typedef void (*CallPII_V)(JNIEnv* env, jobject object, jobject o2, jint i1, jint i2);
typedef jint (*CallPII_I)(JNIEnv* env, jobject object, jobject o2, jint i1, jint i2);
typedef jboolean (*CallPJII_Z)(JNIEnv* env, jobject object, jobject o2, jlong l1, jint i1, jint i2);
typedef jobject (*CallPIPII_V)(JNIEnv* env, jobject object, jobject o2, jint i1, jobject o3, jint i2, jint i3);
typedef jobject (*CallPPIIPPZ_P)(JNIEnv* env, jobject object, jobject o1, jobject o2, jint i3, jint i4, jobject o5, jobject o6, jboolean b7);

typedef jobject (*CallI_P)(JNIEnv* env, jobject object, jint i);
typedef void (*CallI_V)(JNIEnv* env, jobject object, jint i);
typedef jlong (*CallI_J)(JNIEnv* env, jobject object, jint i);
typedef jlong (*CallIJ_J)(JNIEnv* env, jobject object, jint i, jlong l);

typedef jint (*CallF_I)(JNIEnv* env, jobject object, jfloat f);

typedef jlong (*CallD_J)(JNIEnv* env, jobject object, jdouble d);

typedef jobject (*CallZ_P)(JNIEnv* env, jobject object, jboolean b);
typedef jboolean (*CallZ_Z)(JNIEnv* env, jobject object, jboolean b);

typedef jlong (*CallJ_J)(JNIEnv* env, jobject object, jlong l);
typedef jboolean (*CallJ_Z)(JNIEnv* env, jobject object, jlong l);
typedef void (*CallJ_V)(JNIEnv* env, jobject object, jlong l);
typedef void (*CallJJ_V)(JNIEnv* env, jobject object, jlong l1, jlong l2);

FieldData NativeVMMethod::executeNoASM(VMContext* ctx, VMMethod::ReturnType ret, VMClass* cls){
  TR_USE(Java_Method_NoASM);
  JNIEnv* env = ctx->getJNIEnv();
  FieldData retval;
  retval.obj = NULL;
  int base = mIsStatic ? 0 : 1;
  if (mSplitSignature.size() == 0){
    if (ret == VMMethod::Void){
      Call_V mthd = (Call_V)mFunction;
      mthd(env, cls);
      return retval;
    }
    else if (ret == VMMethod::Long){
      Call_J mthd = (Call_J)mFunction;
      retval.l = mthd(env, cls);
      return retval;
    }
    else if (ret == VMMethod::Reference){
      Call_P mthd = (Call_P)mFunction;
      retval.obj = (VMObject*)mthd(env, cls);
      return retval;
    }
    else if (ret == VMMethod::Int){
      Call_I mthd = (Call_I)mFunction;
      retval.i = mthd(env, cls);
      return retval;
    }
    else if (ret == VMMethod::Boolean){
      Call_Z mthd = (Call_Z)mFunction;
      retval.b = mthd(env, cls);
      return retval;
    }
    TR_BREAK("Cannot call method signature");
  }
  if (mSplitSignature[0].size() > 1){
    VMObject* o1 = ctx->get(base++).obj;
    if (mSplitSignature.size() > 1){
      if (mSplitSignature[1] == "I"){
        int i1 = ctx->get(base++).i;
        if (mSplitSignature.size() > 2){
          if (mSplitSignature[2].size() > 1){
            VMObject* o2 = ctx->get(base++).obj;
            if (mSplitSignature.size() > 3){
              if (mSplitSignature[3] == "I"){
                int i2 = ctx->get(base++).i;
                if (mSplitSignature.size() > 4){
                  if (mSplitSignature[4] == "I"){
                    int i3 = ctx->get(base++).i;
                    if (mSplitSignature.size() > 5){
                      TR_BREAK("Cannot call method signature");
                    }
                    if (ret == VMMethod::Void){
                      CallPIPII_V mthd = (CallPIPII_V)mFunction;
                      mthd(env, cls, o1, i1, o2, i2, i3);
                      return retval;
                    }
                    TR_BREAK("Cannot call method signature");
                  }
                }
              }
            }
          }
          else if (mSplitSignature[2] == "I"){
            int i2 = ctx->get(base++).i;
            if (mSplitSignature.size() > 3){
              TR_BREAK("Cannot call method signature");
            }
            if (ret == VMMethod::Void){
              CallPII_V mthd = (CallPII_V)mFunction;
              mthd(env, cls, o1, i1, i2);
              return retval;
            }
            else if (ret == VMMethod::Int){
              CallPII_I mthd = (CallPII_I)mFunction;
              retval.i = mthd(env, cls, o1, i1, i2);
              return retval;
            }
            TR_BREAK("Cannot call method signature");
          }
        }
      }
      else if (mSplitSignature[1] == "J"){
        jlong l1 = 0;
        l1 = ((int64)ctx->get(base++).ui) << 0;
        l1 |= ((int64)ctx->get(base++).ui) << 32;
        if (mSplitSignature.size() > 2){
          if (mSplitSignature[2] == "I"){
            int i1 = ctx->get(base++).i;
            if (mSplitSignature.size() > 3){
              if (mSplitSignature[3] == "I"){
                int i2 = ctx->get(base++).i;
                if (mSplitSignature.size() > 4){
                  TR_BREAK("Cannot call method signature");
                }
                if (ret == VMMethod::Boolean){
                  CallPJII_Z mthd = (CallPJII_Z)mFunction;
                  retval.b = mthd(env, cls, o1, l1, i1, i2);
                  return retval;
                }
              }
            }
          }
        }
      }
      else if (mSplitSignature[1] == "Z"){
        jboolean b1 = (jboolean)ctx->get(base++).i;
        if (mSplitSignature.size() > 2){
          if (mSplitSignature[2].size() > 1){
            jobject o2 = ctx->get(base++).obj;
            if (mSplitSignature.size() > 3){
              TR_BREAK("Cannot call method signature");
            }
            if (ret == VMMethod::Reference){
              CallPZP_P mthd = (CallPZP_P)mFunction;
              retval.obj = (VMObject*)mthd(env, cls, o1, b1, o2);
              return retval;
            }
          }
        }
      }
      else if (mSplitSignature[1].size() > 1){
        jobject o2 = ctx->get(base++).obj;
        if (mSplitSignature.size() > 2){
          if (mSplitSignature[2] == "I"){
            jint i3 = ctx->get(base++).i;
            if (mSplitSignature.size() > 3){
              if (mSplitSignature[3] == "I"){
                jint i4 = ctx->get(base++).i;
                if (mSplitSignature.size() > 4){
                  if (mSplitSignature[4].size() > 1){
                    jobject o5 = ctx->get(base++).obj;
                    if (mSplitSignature.size() > 5){
                      if (mSplitSignature[5].size() > 1){
                        jobject o6 = ctx->get(base++).obj;
                        if (mSplitSignature.size() > 6){
                          if (mSplitSignature[6] == "Z"){
                            jboolean b7 = (jboolean)ctx->get(base++).i;
                            if (mSplitSignature.size() > 7){
                              TR_BREAK("Cannot call method signature");
                            }
                            if (ret == VMMethod::Reference){
                              CallPPIIPPZ_P mthd = (CallPPIIPPZ_P)mFunction;
                              retval.obj = (VMObject*)mthd(env, cls, o1, o2, i3, i4, o5, o6, b7);
                              return retval;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          TR_BREAK("Cannot call method signature");
        }
        if (ret == VMMethod::Reference){
          CallPP_P mthd = (CallPP_P)mFunction;
          retval.obj = (VMObject*)mthd(env, cls, o1, o2);
          return retval;
        }
      }
      TR_BREAK("Cannot call method signature");
    }
    if (ret == VMMethod::Reference){
      CallP_P mthd = (CallP_P)mFunction;
      retval.obj = (VMObject*)mthd(env, cls, o1);
      return retval;
    }
    if (ret == VMMethod::Boolean){
      CallP_Z mthd = (CallP_Z)mFunction;
      retval.b = mthd(env, cls, o1);
      return retval;
    }
    if (ret == VMMethod::Long){
      CallP_J mthd = (CallP_J)mFunction;
      retval.l = mthd(env, cls, o1);
      return retval;
    }
    if (ret == VMMethod::Int){
      CallP_I mthd = (CallP_I)mFunction;
      retval.i = mthd(env, cls, o1);
      return retval;
    }
    if (ret == VMMethod::Void){
      CallP_V mthd = (CallP_V)mFunction;
      mthd(env, cls, o1);
      return retval;
    }
    TR_BREAK("Cannot call method signature");
  }
  if (mSplitSignature[0] == "I"){
    jint i = ctx->get(base++).i;
    if (mSplitSignature.size() > 1){
      if (mSplitSignature[1] == "J"){
        jlong l1 = 0;
        l1 = ((int64)ctx->get(base++).ui) << 0;
        l1 |= ((int64)ctx->get(base++).ui) << 32;
        if (mSplitSignature.size() > 2){
          TR_BREAK("Cannot call method signature");
        }
        if (ret == VMMethod::Long){
          CallIJ_J mthd = (CallIJ_J)mFunction;
          retval.l = mthd(env, cls, i, l1);
          return retval;
        }
      }
      TR_BREAK("Cannot call method signature");
    }
    if (ret == VMMethod::Reference){
      CallI_P mthd = (CallI_P)mFunction;
      retval.obj = (VMObject*)mthd(env, cls, i);
      return retval;
    }
    if (ret == VMMethod::Void){
      CallI_V mthd = (CallI_V)mFunction;
      mthd(env, cls, i);
      return retval;
    }
    if (ret == VMMethod::Long){
      CallI_J mthd = (CallI_J)mFunction;
      retval.l = mthd(env, cls, i);
      return retval;
    }
    TR_BREAK("Cannot call method signature");
  }
  if (mSplitSignature[0] == "F"){
    float f = ctx->get(base++).f;
    if (mSplitSignature.size() > 1){
      TR_BREAK("Cannot call method signature");
    }
    if (ret == VMMethod::Int){
      CallF_I mthd = (CallF_I)mFunction;
      retval.i = mthd(env, cls, f);
      return retval;
    }
    TR_BREAK("Cannot call method signature");
  }
  if (mSplitSignature[0] == "D"){
    double d;
    FieldData tmp;
    tmp.l = ((int64)ctx->get(base++).ui) << 0;
    tmp.l |= ((int64)ctx->get(base++).ui) << 32;
    d = tmp.d;
    if (mSplitSignature.size() > 1){
      TR_BREAK("Cannot call method signature");
    }
    if (ret == VMMethod::Long){
      CallD_J mthd = (CallD_J)mFunction;
      retval.l = mthd(env, cls, d);
      return retval;
    }
    TR_BREAK("Cannot call method signature");
  }
  if (mSplitSignature[0] == "Z"){
    jint i = ctx->get(base++).i;
    jboolean b = (jboolean)i;
    if (mSplitSignature.size() > 1){
      TR_BREAK("Cannot call method signature");
    }
    if (ret == VMMethod::Array){
      CallZ_P mthd = (CallZ_P)mFunction;
      retval.obj = (VMObject*)mthd(env, cls, b);
      return retval;
    }
    else if (ret == VMMethod::Boolean){
      CallZ_Z mthd = (CallZ_Z)mFunction;
      retval.b = mthd(env, cls, b);
      return retval;
    }
    TR_BREAK("Cannot call method signature");
  }
  else if (mSplitSignature[0] == "J"){
    jlong l1 = 0;
    l1 = ((int64)ctx->get(base++).ui) << 0;
    l1 |= ((int64)ctx->get(base++).ui) << 32;
    if (mSplitSignature.size() > 1){
      if (mSplitSignature[1] == "J"){
        jlong l2 = 0;
        l2 = ((int64)ctx->get(base++).ui) << 0;
        l2 |= ((int64)ctx->get(base++).ui) << 32;
        if (mSplitSignature.size() > 2){
          TR_BREAK("Cannot call method signature");
        }
        if (ret == VMMethod::Void){
          CallJJ_V mthd = (CallJJ_V)mFunction;
          mthd(env, cls, l1, l2);
          return retval;
        }
      }
      TR_BREAK("Cannot call method signature");
    }
    if (ret == VMMethod::Long){
      CallJ_J mthd = (CallJ_J)mFunction;
      retval.l = mthd(env, cls, l1);
      return retval;
    }
    else if (ret == VMMethod::Byte){
      CallJ_Z mthd = (CallJ_Z)mFunction;
      retval.b = mthd(env, cls, l1);
      return retval;
    }
    else if (ret == VMMethod::Void){
      CallJ_V mthd = (CallJ_V)mFunction;
      mthd(env, cls, l1);
      return retval;
    }
  }
  TR_BREAK("Cannot call method signature");
  return retval;
}
