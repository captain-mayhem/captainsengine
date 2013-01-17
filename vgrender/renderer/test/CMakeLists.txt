INCLUDE_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine)

LINK_DIRECTORIES(${CaptainsEngine_SOURCE_DIR}/Engine/lib)

ADD_EXECUTABLE(vgrendertest
	main.cpp
)

TARGET_LINK_LIBRARIES(vgrendertest vgrender)