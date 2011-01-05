SET(PRE image)

SET(Engine_image_SRCS
	${PRE}/image.cpp
	${PRE}/loader.cpp
)

SET(Engine_image_HDRS
	${PRE}/image.h
	${PRE}/loader.h
)

INSTALL(FILES ${Engine_image_HDRS} DESTINATION Engine/include/${PRE})