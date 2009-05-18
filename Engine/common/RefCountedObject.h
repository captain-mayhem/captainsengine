#ifndef CGE_REFCOUNTED_OBJECT_H
#define CGE_REFCOUNTED_OBJECT_H

#include <system/AtomicCounter.h>

namespace CGE{

class RefCountedObject{
public:
  RefCountedObject() : mRefCount(0){}
  virtual ~RefCountedObject(){
  }
  void ref(){
    mRefCount.increment();
  }
  void unref(){
    mRefCount.decrement();
    if (mRefCount.isZero()){
      delete this;
    }
  }
private:
  AtomicCounter mRefCount;
};

template<typename T>
class Ptr{
public:
  Ptr(){
    mObject = NULL;
  }
  Ptr(T* obj, bool isWeak = false){
    mObject = obj;
    if (mObject)
      mObject->ref();
  }
  Ptr(const Ptr& ptr){
    mObject = ptr.mObject;
    if (mObject)
      mObject->ref();
  }
  template <typename U>
  Ptr(const Ptr<U>& ptr){
    mObject = static_cast<T*>(ptr.operator->());
    if (mObject)
      mObject->ref();
  }
  ~Ptr(){
    if (mObject)
      mObject->unref();
  }
  Ptr& operator=(const Ptr& ptr){
    if (mObject == ptr.mObject)
      return *this;
    if (mObject)
      mObject->unref();
    mObject = ptr.mObject;
    if (mObject)
      mObject->ref();
    return *this;
  }
  template <typename U>
  Ptr& operator=(const Ptr<U>& ptr){
    if (mObject == ptr.operator->())
      return *this;
    if (mObject)
      mObject->unref();
    mObject = ptr.operator->();
    if (mObject)
      mObject->ref();
    return *this;
  }
  T& operator*() const{
    assert(mObject);
    return *mObject;
  }
  T* operator->() const{
    return mObject;
  }
  bool operator==(const Ptr<T>& ptr) const{
    return mObject == ptr.mObject;
  }
  bool operator<(const Ptr<T>& ptr) const{
    return mObject < ptr.mObject;
  }
  T& operator[](int i){
    return mObject[i];
  }
protected:
  T* mObject;
};

}

namespace std {
  template <typename T> 
  inline void swap(CGE::Ptr<T>& t1, CGE::Ptr<T>& t2){
    std::cerr << "WTF";
    //t1.swap (t2);
  }
}

#endif
