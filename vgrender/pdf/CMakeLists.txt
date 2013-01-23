INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_EXECUTABLE(pdfreader
	pdfreader.cpp
	pdfreader.h
	pdfdocument.cpp
	pdfdocument.h
	main.cpp
)

TARGET_LINK_LIBRARIES(pdfreader Engine ${ENGINE_LINK_LIBRARIES})
