
#undef BEGIN_MAPPING
#undef END_MAPPING
#undef INSERT

#ifdef PROC_ENUM_MODE

#define BEGIN_MAPPING(name, type) \
  enum name{

#define END_MAPPING(name) \
  };

#define INSERT(name,value) \
  op_##name = value,


#elif defined(PROC_MAP_MODE)

#include <map>

#define BEGIN_MAPPING(name, type) \
  namespace name{\
  std::map<type,std::string> map_string; \
  std::map<std::string,type> map_value; \
  void init(){

#define END_MAPPING(name) \
  } }

#define INSERT(name,value) \
  map_string[value] = #name; \
  map_value[#name] = value;

#elif defined(PROC_DECL_MAP_MODE)

#include <map>

#define BEGIN_MAPPING(name, type) \
  namespace name{\
  extern std::map<type,std::string> map_string; \
  extern std::map<std::string,type> map_value; \
  void init();

#define END_MAPPING(name)	\
	}

#define INSERT(name,value)

#else
#error Preproc.h used incorrectly!!!
#endif
