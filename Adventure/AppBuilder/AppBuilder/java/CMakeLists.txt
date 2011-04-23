
SET(JSRCS
	de/captain/online/AdventureActivity.java
	de/captain/online/AdventureView.java
	de/captain/online/AdventureLib.java
)

MESSAGE(STATUS "using ${PLATFORM_SDK_PATH}/android.jar")

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/adventure.jar
COMMAND ${JAVA_COMPILE} -cp ${PLATFORM_SDK_PATH}/android.jar -d ${CMAKE_CURRENT_BINARY_DIR} ${JSRCS}
COMMAND ${JAVA_ARCHIVE} -cf ${CMAKE_CURRENT_BINARY_DIR}/adventure.jar -C ${CMAKE_CURRENT_BINARY_DIR} de
DEPENDS ${JSRCS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "compiling adventure.jar"
VERBATIM
)

ADD_CUSTOM_TARGET(build_adventure.jar ALL
DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/adventure.jar
)
