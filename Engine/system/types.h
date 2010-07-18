#ifndef CGE_TYPES_H
#define CGE_TYPES_H

#ifdef WIN32
#include <Windows.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <stdint.h>
#endif

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
#ifdef WIN32
typedef INT64 int64;
typedef UINT64 uint64;
#endif
#ifdef UNIX
typedef int64_t int64;
typedef int64_t uint64;
#endif


#endif

