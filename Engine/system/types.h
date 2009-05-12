#ifndef CGE_TYPES_H
#define CGE_TYPES_H

#ifdef WIN32
#include <Windows.h>
#endif

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
#ifdef WIN32
typedef __int64 int64;
#endif


#endif

