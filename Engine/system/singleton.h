namespace Engine{

template <class T> 
class Singleton{
public:
  static void init(){
    instance_ = new T();
  }
  static void kill(){
    delete instance_;
  }
  static T* instance(){
    return instance_;
  }
protected:
  //! the class instance
  static T* instance_;
  Singleton(){instance_ = NULL;}
};

}

