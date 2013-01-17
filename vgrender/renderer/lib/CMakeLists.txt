INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_LIBRARY(vgrender STATIC
	api.h
	api.cpp
)

TARGET_LINK_LIBRARIES(vgrender Engine ${ENGINE_LINK_LIBRARIES})