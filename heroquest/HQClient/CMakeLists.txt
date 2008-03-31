INCLUDE_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${HeroEngine_SOURCE_DIR}/Engine/lib)

SET(HQClient_SRCS
	bitset.cpp
	camera.cpp
	clientsocket.cpp
	common.cpp
	creature.cpp
	door.cpp
	field.cpp
	furniture.cpp
	gameobject.cpp
	gamestate.cpp
	hero.cpp
	inventory.cpp
	item.cpp
	main.cpp
	math.cpp
	menu.cpp
	message.cpp
	monster.cpp
	overlay.cpp
	player.cpp
	renderer.cpp
	script.cpp
	socket.cpp
	templates.cpp
	textureManager.cpp
	trade.cpp
	world.cpp
)

SET(HQClient_HDRS
	bitset.h
	camera.h
	clientsocket.h
	common.h
	creature.h
	door.h
	field.h
	furniture.h
	gameobject.h
	gamestate.h
	hero.h
	inventory.h
	item.h
	math.h
	menu.h
	message.h
	monster.h
	opcodes.h
	overlay.h
	player.h
	renderer.h
	script.h
	socket.h
	socketexception.h
	templates.h
	textureManager.h
	trade.h
	world.h
)

IF (${OS} STREQUAL "windows")
  ADD_DEFINITIONS(/D_CLIENT_)
ENDIF (${OS} STREQUAL "windows")
IF (${OS} STREQUAL "linux")
  ADD_DEFINITIONS(-D_CLIENT_)
ENDIF (${OS} STREQUAL "linux")

ADD_EXECUTABLE(HQClient ${HQClient_SRCS} ${HQClient_HDRS})

TARGET_LINK_LIBRARIES(HQClient Engine)
IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(HQClient wsock32)
  SET_TARGET_PROPERTIES(HQClient PROPERTIES LINK_FLAGS " /STACK:10000000 /machine:I386 /debug /SUBSYSTEM:Windows")
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS HQClient RUNTIME DESTINATION HQClient)
INSTALL(FILES engine.ini DESTINATION HQClient)
INSTALL(FILES editor editor.cmd DESTINATION HQClient PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
INSTALL(DIRECTORY data DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY levels DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY models DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY scripts DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY textures DESTINATION HQClient PATTERN ".svn" EXCLUDE)
