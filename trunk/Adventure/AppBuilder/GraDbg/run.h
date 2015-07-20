
#ifdef GraDbgRuntime_EXPORTS 
#define IMPEXP __declspec(dllexport)
#else
#define IMPEXP __declspec(dllimport)
#endif

IMPEXP void* interpret(const std::string& cmd);