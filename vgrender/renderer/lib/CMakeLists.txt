INCLUDE_DIRECTORIES(
.
${CaptainsEngine_SOURCE_DIR}/Engine
)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_LIBRARY(vgrender STATIC
	api.h
	api.cpp
	framebuffer.h
	framebuffer.cpp
	line.h
	line.cpp
	shader.cpp
	shader.h
	state.cpp
	state.h
	triangle.cpp
	triangle.h
)

TARGET_LINK_LIBRARIES(vgrender Engine ${ENGINE_LINK_LIBRARIES})