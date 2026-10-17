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
	socketexception.h
	templates.h
	textureManager.h
	trade.h
	world.h
)

if (OS STREQUAL "android")
ADD_LIBRARY(HQClient SHARED ${HQClient_SRCS} ${HQClient_HDRS})
else()
ADD_EXECUTABLE(HQClient ${HQClient_SRCS} ${HQClient_HDRS})
endif()
target_compile_definitions(HQClient PRIVATE _CLIENT_)

TARGET_LINK_LIBRARIES(HQClient PRIVATE EngineMain CGE::Engine)
IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(HQClient PRIVATE wsock32)
  SET_TARGET_PROPERTIES(HQClient PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

INSTALL(TARGETS HQClient RUNTIME DESTINATION HQClient LIBRARY DESTINATION HQClient)
INSTALL(FILES engine.ini DESTINATION HQClient)
INSTALL(FILES editor editor.cmd DESTINATION HQClient PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
INSTALL(DIRECTORY data DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY levels DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY models DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY scripts DESTINATION HQClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY textures DESTINATION HQClient PATTERN ".svn" EXCLUDE)