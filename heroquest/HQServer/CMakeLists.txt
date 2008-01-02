INCLUDE_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/lib)

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
	${CLIENT}/socket.cpp
	${CLIENT}/templates.cpp
	${CLIENT}/world.cpp
)

ADD_EXECUTABLE(HQServer ${HQServer_SRCS})

TARGET_LINK_LIBRARIES(HQServer Engine)
IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(HQServer wsock32)
  SET_TARGET_PROPERTIES(HQServer PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS HQServer RUNTIME DESTINATION HQServer)
INSTALL(FILES engine.ini players.dat start.bat DESTINATION HQServer)
INSTALL(DIRECTORY ${CLIENT}/scripts DESTINATION HQServer PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY levels DESTINATION HQServer PATTERN ".svn" EXCLUDE)
