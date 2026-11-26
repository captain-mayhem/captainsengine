SET(CLIENT ../TDClient)

SET(TDServer_SRCS
	admin.cpp
	gamestate.cpp
	main.cpp
	message.cpp
	player.cpp
	serversocket.cpp
	${CLIENT}/bitset.cpp
	${CLIENT}/creature.cpp
	${CLIENT}/door.cpp
	${CLIENT}/field.cpp
	${CLIENT}/furniture.cpp
	${CLIENT}/gameobject.cpp
	${CLIENT}/hero.cpp
	${CLIENT}/inventory.cpp
	${CLIENT}/item.cpp
	${CLIENT}/monster.cpp
	${CLIENT}/overlay.cpp
	${CLIENT}/script.cpp
	${CLIENT}/templates.cpp
	${CLIENT}/world.cpp
)

if (OS STREQUAL "android")
ADD_LIBRARY(TDServer SHARED ${TDServer_SRCS})
else()
ADD_EXECUTABLE(TDServer ${TDServer_SRCS})
endif()

TARGET_LINK_LIBRARIES(TDServer PRIVATE CGE::Engine)
IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(TDServer PRIVATE wsock32)
  SET_TARGET_PROPERTIES(TDServer PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

cge_copy_runtime(TDServer)
INSTALL(TARGETS TDServer RUNTIME DESTINATION TDServer LIBRARY DESTINATION TDServer)
cge_install_runtime(TDServer TDServer)
INSTALL(FILES engine.ini players.dat start.bat DESTINATION TDServer)
INSTALL(DIRECTORY ${CLIENT}/scripts DESTINATION TDServer PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY levels DESTINATION TDServer PATTERN ".svn" EXCLUDE)