SET(LIB_SRC
	../lib/line.cpp
	../lib/line.h
	../lib/tristrip.cpp
	../lib/tristrip.h
)

ADD_EXECUTABLE(SceneryViewer main.cpp ${LIB_SRC})

SET_TARGET_PROPERTIES(SceneryViewer PROPERTIES WIN32_EXECUTABLE TRUE)

TARGET_LINK_LIBRARIES(SceneryViewer EngineMain)