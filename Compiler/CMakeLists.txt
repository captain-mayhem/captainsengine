INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/extern/zlib)
INCLUDE_DIRECTORIES(.)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_DEFINITIONS(
	-DENABLE_TRACING
)

SET(Compiler_SRCS
	Compiler.cpp
	#ClassLoader.cpp
	#Interpreter.cpp
	#MethodArea.cpp
  )

SET(Compiler_HDRS
	#ClassLoader.h
	#Interpreter.h
	#MethodArea.h
)

ADD_EXECUTABLE(Compiler ${Compiler_SRCS} ${Compiler_HDRS})

TARGET_LINK_LIBRARIES(Compiler javaruntime zlib)

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(Compiler PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Console")
ENDIF (${OS} STREQUAL "windows")

ADD_LIBRARY(javaruntime SHARED
	runtime/Runtime.cpp
	runtime/Runtime.h

	JavaBinFileReader.h
	JavaDefs.h
	JavaOpcodes.h
	Preproc.h
	Trace.h
	jni.h
	JVM.h
	VMArray.h
	VMclass.h
	VMContext.h
	VMMethod.h
	VMObject.h

	JavaBinFileReader.cpp
	JavaDefs.cpp
	jni.cpp
	JVM.cpp
	VMArray.cpp
	VMclass.cpp
	VMContext.cpp
	VMMethod.cpp
	VMObject.cpp
)

TARGET_LINK_LIBRARIES( javaruntime Engine)