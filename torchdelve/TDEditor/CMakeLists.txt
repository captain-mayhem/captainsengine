SET(TDEditor_SRCS
	compiler.cpp
	../TDClient/door.cpp
	main.cpp
	../../editor/tdmExport.cpp
	../../editor/tdmExport.h
)

SET(TDEditor_HDRS
	compiler.h)

if (OS STREQUAL "android")
ADD_LIBRARY(TDEditor SHARED ${TDEditor_SRCS} ${TDEditor_HDRS})
else()
ADD_EXECUTABLE(TDEditor ${TDEditor_SRCS} ${TDEditor_HDRS})
endif()

TARGET_LINK_LIBRARIES(TDEditor PRIVATE CGE::Engine)
IF (${OS} STREQUAL "windows")
  SET_TARGET_PROPERTIES(TDEditor PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

cge_copy_runtime(TDEditor)
INSTALL(TARGETS TDEditor RUNTIME DESTINATION TDEditor LIBRARY DESTINATION TDEditor)
cge_install_runtime(TDEditor TDEditor)
INSTALL(FILES engine.ini SCRIPT.txt DESTINATION TDEditor)
INSTALL(FILES make.bat copy levels DESTINATION TDEditor PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
INSTALL(DIRECTORY basic DESTINATION TDEditor PATTERN ".svn" EXCLUDE)