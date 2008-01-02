INCLUDE_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/lib)

SET(HQEditor_SRCS
	compiler.cpp
	../HQClient/door.cpp
	main.cpp
)

SET(HQEditor_HDRS
	compiler.h)

ADD_EXECUTABLE("HQEditor" ${HQEditor_SRCS} ${HQEditor_HDRS})

TARGET_LINK_LIBRARIES(HQEditor Engine)
SET_TARGET_PROPERTIES(HQEditor PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")

INSTALL(TARGETS HQEditor RUNTIME DESTINATION HQEditor)
INSTALL(FILES engine.ini make.bat copy levels SCRIPT.txt DESTINATION HQEditor)
INSTALL(FILES textures/font/font.bmp DESTINATION HQEditor/textures/font)
INSTALL(DIRECTORY basic DESTINATION HQEditor PATTERN ".svn" EXCLUDE)