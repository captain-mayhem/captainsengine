INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/extern/zlib)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_DEFINITIONS(
	-DENABLE_TRACING
)

SET(Compiler_SRCS
	Compiler.cpp
	ClassLoader.cpp
	Interpreter.cpp
	JavaBinFileReader.cpp
	JavaDefs.cpp
	MethodArea.cpp
	)

SET(Compiler_HDRS
	ClassLoader.h
	Interpreter.h
	JavaBinFileReader.h
	JavaDefs.h
	JavaOpcodes.h
	MethodArea.h
	Preproc.h
	Trace.h
	)

ADD_EXECUTABLE(Compiler ${Compiler_SRCS} ${Compiler_HDRS})

TARGET_LINK_LIBRARIES(Compiler Engine zlib)

IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(OSMRacer PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")