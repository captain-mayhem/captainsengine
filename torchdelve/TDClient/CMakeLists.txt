SET(TDClient_SRCS
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
	widgets.cpp
	world.cpp
)

SET(TDClient_HDRS
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
	widgets.h
	world.h
)

if (OS STREQUAL "android")
ADD_LIBRARY(TDClient SHARED ${TDClient_SRCS} ${TDClient_HDRS})
else()
ADD_EXECUTABLE(TDClient ${TDClient_SRCS} ${TDClient_HDRS})
endif()
target_compile_definitions(TDClient PRIVATE _CLIENT_)
# the shaders are compiled in as torchdelve/...
cge_embed_shaders(TDClient ${CMAKE_CURRENT_SOURCE_DIR}/shaders torchdelve/ registerTorchdelveShaders)

TARGET_LINK_LIBRARIES(TDClient PRIVATE CGE::Engine)
IF (${OS} STREQUAL "windows")
  TARGET_LINK_LIBRARIES(TDClient PRIVATE wsock32)
  SET_TARGET_PROPERTIES(TDClient PROPERTIES WIN32_EXECUTABLE TRUE)
ENDIF (${OS} STREQUAL "windows")

cge_copy_runtime(TDClient)
INSTALL(TARGETS TDClient RUNTIME DESTINATION TDClient LIBRARY DESTINATION TDClient)
cge_install_runtime(TDClient TDClient)
INSTALL(FILES engine.ini DESTINATION TDClient)
INSTALL(FILES editor editor.cmd DESTINATION TDClient PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
INSTALL(DIRECTORY data DESTINATION TDClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY levels DESTINATION TDClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY models DESTINATION TDClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY scripts DESTINATION TDClient PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY textures DESTINATION TDClient PATTERN ".svn" EXCLUDE)