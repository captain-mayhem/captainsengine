INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)
INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/extern/zlib)
INCLUDE_DIRECTORIES(.)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

###WXWIDGETS###

INCLUDE(FindwxWidgets)

SET(wxWidgets_USE_LIBS base core gl media)
FIND_PACKAGE(wxWidgets REQUIRED base core gl media)
INCLUDE(${wxWidgets_USE_FILE})
ADD_DEFINITIONS(-DwxUSE_NO_MANIFEST=1)
SET(AB_LINK_LIBS ${AB_LINK_LIBS} ${wxWidgets_LIBRARIES})
###############

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

TARGET_LINK_LIBRARIES(Compiler javaruntime zlib  ${wxWidgets_LIBRARIES})

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