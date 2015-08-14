#ifndef TRACING_H
#define TRACING_H

#include <cstdarg>
#include <map>
#include <fstream>

//Trace levels
#define TRACE_OFF 0
#define TRACE_FATAL_ERROR 1
#define TRACE_ERROR 2
#define TRACE_WARNING 3
#define TRACE_INFO 4
#define TRACE_DEBUG 5
#define TRACE_DEBUG_DETAIL 6
#define TRACE_CUSTOM 7

namespace CGE{

class TraceOutputter{
public:
  virtual ~TraceOutputter() {}
  virtual bool init()=0;
  virtual void trace(unsigned channel, int level, const char* function, const char* message)=0;
};

class TraceChannel{
public:
  TraceChannel(const char* name);
  TraceChannel(const char* name, int level);
  void realize();
  void trace(int level, const char* function, const char* message);
  bool isEnabled(int level);
  unsigned getChannel() { return mChannel; }
protected:
  const char* mName;
  int mLevel;
  unsigned mChannel;
};

class TraceObject{
public:
  TraceObject(TraceChannel* channel);
  void trace(int level, const char* function, const char* message, ...);
  bool isEnabled(int level);
protected:
  TraceChannel* mChannel;
};

class LogOutputter : public TraceOutputter{
public:
  LogOutputter();
  LogOutputter(std::string const& file);
  ~LogOutputter();
  virtual bool init();
  virtual void trace(unsigned channel, int level, const char* function, const char* message);
protected:
  std::string mLogName;
  std::ofstream mLog;
};

}

#define CGE_STRINGIFY(x) INTERNAL_STRINGIFY(x)
#define INTERNAL_STRINGIFY(x) #x

#define TR_CHANNEL(name) CGE::TraceChannel name(CGE_STRINGIFY(name));
#define TR_CHANNEL_EXT(name) extern CGE::TraceChannel name;
#define TR_CHANNEL_LVL(name, level) CGE::TraceChannel name(CGE_STRINGIFY(name), level);
#define TR_USE(name) CGE::TraceObject tracescopeobject(&name);
#define TR_TRACE(level, message, ...) do {if (tracescopeobject.isEnabled(level)) tracescopeobject.trace(level, __FUNCTION__, message, ##__VA_ARGS__);} while(0)
#define TR_IS_ENABLED(level) tracescopeobject.isEnabled(level)
#define TR_GET_ID(name) name.getChannel()

#define TR_BREAK(message, ...) TR_TRACE(TRACE_FATAL_ERROR, message, ##__VA_ARGS__)
#define TR_ERROR(message, ...) TR_TRACE(TRACE_ERROR, message, ##__VA_ARGS__)
#define TR_WARN(message, ...) TR_TRACE(TRACE_WARNING, message, ##__VA_ARGS__)
#define TR_INFO(message, ...) TR_TRACE(TRACE_INFO, message, ##__VA_ARGS__)
#define TR_DEBUG(message, ...) TR_TRACE(TRACE_DEBUG, message, ##__VA_ARGS__)
#define TR_DETAIL(message, ...) TR_TRACE(TRACE_DEBUG_DETAIL, message, ##__VA_ARGS__)

#endif
