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
	IF (${CPU} STREQUAL x64)
		ADD_DEFINITIONS(-DARCH_X64)
		ENABLE_LANGUAGE(ASM_MASM)
		SET(MACHINE X64)
		SET(ASM_FILES asm/x64/call.asm
		)
		STRING(REPLACE ".asm" ".obj" ASM_OBJS ${ASM_FILES})
		STRING(REPLACE "asm/x64" ${CMAKE_CURRENT_BINARY_DIR} ASM_OBJS ${ASM_OBJS})
		STRING(REPLACE "asm/x64" "${CMAKE_CURRENT_SOURCE_DIR}/asm/x64" ASM_FULL_FILES ${ASM_FILES})
		MESSAGE(STATUS ASM Files: ${ASM_OBJS})
	ELSE (${CPU} STREQUAL x64)
		SET(MACHINE I386)
	ENDIF (${CPU} STREQUAL x64)
	SET_TARGET_PROPERTIES(Compiler PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:${MACHINE} /debug /SUBSYSTEM:Console")
ELSEIF (${OS} STREQUAL "linux")
	IF (${CPU} STREQUAL x64)
		ADD_DEFINITIONS(-DARCH_X64)
		ENABLE_LANGUAGE(ASM-ATT)
		SET(ASM_FILES asm/x64/call.s)
		MESSAGE(STATUS "Using assembler")
		#SET(CMAKE_ASM-ATT_FLAGS -k)
	ENDIF(${CPU} STREQUAL x64)
ENDIF (${OS} STREQUAL "windows")

ADD_LIBRARY(javaruntime SHARED
	runtime/Java_io.cpp
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
	VMLoader.h
	VMMethod.h
	VMObject.h

	JavaBinFileReader.cpp
	JavaDefs.cpp
	jni.cpp
	JVM.cpp
	VMArray.cpp
	VMclass.cpp
	VMContext.cpp
	VMLoader.cpp
	VMMethod.cpp
	VMObject.cpp
	
	CallNoASM.cpp
	
	${ASM_FILES}
	${ASM_OBJS}
)


IF (OS STREQUAL "windows")
IF (${CPU} STREQUAL x64)
SET_SOURCE_FILES_PROPERTIES(${ASM_OBJS} PROPERTIES
EXTERNAL_OBJECT TRUE
GENERATED TRUE
)
ADD_CUSTOM_COMMAND(OUTPUT ${ASM_OBJS}
COMMAND ${CMAKE_ASM_MASM_COMPILER} /DWIN_X64 /Zi /c /Cp /Fl /Fo ${ASM_OBJS} ${ASM_FULL_FILES}
DEPENDS ${ASM_FILES}
)

ADD_CUSTOM_TARGET(build_asm
DEPENDS ${ASM_OBJS}
)
ENDIF (${CPU} STREQUAL x64)
ENDIF (OS STREQUAL "windows")

ADD_LIBRARY(zip SHARED
	zip/zip.cpp
)

TARGET_LINK_LIBRARIES( javaruntime Engine ${ENGINE_LINK_LIBRARIES})
