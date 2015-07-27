
SET (ANT ANT-NOTFOUND CACHE FILEPATH "The path to Ant")
MESSAGE(STATUS "Ant: ${ANT}")

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/bin/AdventureActivity-debug.apk
	#copy binary lib
	COMMAND cmake -E copy ${CMAKE_CURRENT_BINARY_DIR}/../libAdventure.so ${CMAKE_CURRENT_SOURCE_DIR}/libs/armeabi
	#build java part
	COMMAND cmake -DANT=${ANT} -DTARGET=debug -P antcall.cmake
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "compiling Android Package"
	VERBATIM
)

ADD_CUSTOM_TARGET(build_adventure.jar ALL
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin/AdventureActivity-debug.apk
)
