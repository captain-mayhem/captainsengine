INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

SET(CLIENT ../HQClient)

SET(HQServer_SRCS
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
ADD_LIBRARY(HQServer SHARED ${HQServer_SRCS})
else()
ADD_EXECUTABLE(HQServer ${HQServer_SRCS})
endif()

TARGET_LINK_LIBRARIES(HQServer EngineMain ${ENGINE_LINK_LIBRARIES})
IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(HQServer wsock32)
  SET_TARGET_PROPERTIES(HQServer PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS HQServer RUNTIME DESTINATION HQServer LIBRARY DESTINATION HQServer)
INSTALL(FILES engine.ini players.dat start.bat DESTINATION HQServer)
INSTALL(DIRECTORY ${CLIENT}/scripts DESTINATION HQServer PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY levels DESTINATION HQServer PATTERN ".svn" EXCLUDE)