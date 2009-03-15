#ifndef TRACING_H
#define TRACING_H

//Trace levels
#define TRACE_FATAL_ERROR 1
#define TRACE_ERROR 2
#define TRACE_WARNING 3
#define TRACE_INFO 4
#define TRACE_DEBUG 5
#define TRACE_DEBUG_DETAIL 6
#define TRACE_CUSTOM 7

//Trace groups
#define TRACE_GROUP_NONE 0x0
#define TRACE_GROUP_ALL 0xFFFFFFFF
#define TRACE_GROUP_USER_FIRST 1 << 11
#define TRACE_GROUP_USER_LAST 1 << 31

#ifdef ENABLE_TRACING
#define TRACE(group, level, message, ...) internal_trace(group, level, __FILE__, __FUNCTION__, message, __VA_ARGS__)
#define TRACE_ABORT(group, message, ...) internal_trace(group, TRACE_FATAL_ERROR, __FILE__, __FUNCTION__, message, __VA_ARGS__)
#else
#define TRACE(group, level, message, ...)
#endif

void internal_trace(int group, int level, char* file, char* function, char* message, ...);

#endif