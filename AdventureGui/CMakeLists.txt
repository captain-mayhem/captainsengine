INCLUDE_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/lib)

SET(AdventureGui_SRCS
	main.cpp
	AdventureCore.cpp
	sqlite3.c
	SQL.cpp
)

SET(AdventureGui_HDRS
	main.h
	AdventureCore.h
	sqlite3.h
	sqlite3ext.h
	SQL.h
)

ADD_EXECUTABLE("AdventureGui" ${AdventureGui_SRCS} ${AdventureGui_HDRS})

TARGET_LINK_LIBRARIES(AdventureGui Engine)
IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(AdventureGui PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")

#INSTALL(TARGETS HQEditor RUNTIME DESTINATION HQEditor)
#INSTALL(FILES engine.ini SCRIPT.txt DESTINATION HQEditor)
#INSTALL(FILES make.bat copy levels DESTINATION HQEditor PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
#INSTALL(FILES textures/font/font.bmp DESTINATION HQEditor/textures/font)
#INSTALL(DIRECTORY basic DESTINATION HQEditor PATTERN ".svn" EXCLUDE)
